#include "WMCYNFirstSignalPresenceComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "HeadMountedDisplayTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MotionControllerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/VoiceConfig.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNPresence, Log, All);

UWMCYNFirstSignalPresenceComponent::UWMCYNFirstSignalPresenceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
    SetIsReplicatedByDefault(true);
}

void UWMCYNFirstSignalPresenceComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    CacheNativeComponents();
    ConfigureWidgetInteraction();
    ApplyLocalLoginGateLock();
    CreateNameplate();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick(this, &UWMCYNFirstSignalPresenceComponent::RegisterVoice);
    }
}

void UWMCYNFirstSignalPresenceComponent::ConfigureWidgetInteraction()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<UWidgetInteractionComponent*> WidgetInteractions;
    Owner->GetComponents(WidgetInteractions);

    PreferredWidgetInteraction = nullptr;
    for (UWidgetInteractionComponent* WidgetInteraction : WidgetInteractions)
    {
        if (WidgetInteraction && WidgetInteraction != KeyboardInputInteraction &&
            WidgetInteraction->GetName().Contains(TEXT("Right"), ESearchCase::IgnoreCase))
        {
            PreferredWidgetInteraction = WidgetInteraction;
            break;
        }
    }

    if (!PreferredWidgetInteraction)
    {
        if (UWidgetInteractionComponent** FoundInteraction = WidgetInteractions.FindByPredicate(
                [](const UWidgetInteractionComponent* WidgetInteraction)
                {
                    return WidgetInteraction &&
                        !WidgetInteraction->GetName().Equals(TEXT("WMCYN_KeyboardInputInteraction")) &&
                        WidgetInteraction->PointerIndex == 0;
                }))
        {
            PreferredWidgetInteraction = *FoundInteraction;
        }
    }

    if (!PreferredWidgetInteraction && WidgetInteractions.Num() > 0)
    {
        PreferredWidgetInteraction = WidgetInteractions[0];
    }

    ConfiguredWidgetInteractionCount = 0;
    for (UWidgetInteractionComponent* WidgetInteraction : WidgetInteractions)
    {
        if (!WidgetInteraction || WidgetInteraction == KeyboardInputInteraction)
        {
            continue;
        }

        WidgetInteraction->TraceChannel = ECC_Visibility;
        WidgetInteraction->InteractionDistance = FMath::Max(WidgetInteraction->InteractionDistance, 750.0f);
        WidgetInteraction->bEnableHitTesting = true;
        WidgetInteraction->bShowDebug = false;
        WidgetInteraction->DebugColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);
        WidgetInteraction->DebugLineThickness = 0.6f;
        WidgetInteraction->DebugSphereLineThickness = 0.8f;
        ++ConfiguredWidgetInteractionCount;
    }

    if (PreferredWidgetInteraction && OwnerCharacter)
    {
        USkeletalMeshComponent* BodyMesh = OwnerCharacter->GetMesh();
        if (BodyMesh && BodyMesh->GetBoneIndex(TEXT("index_03_r")) != INDEX_NONE)
        {
            PreferredWidgetInteraction->AttachToComponent(
                BodyMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                TEXT("index_03_r"));
            PreferredWidgetInteraction->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
            PreferredWidgetInteraction->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
        }

        KeyboardInputInteraction = NewObject<UWidgetInteractionComponent>(
            OwnerCharacter,
            TEXT("WMCYN_KeyboardInputInteraction"));
        if (KeyboardInputInteraction)
        {
            KeyboardInputInteraction->SetupAttachment(OwnerCharacter->GetRootComponent());
            KeyboardInputInteraction->VirtualUserIndex = PreferredWidgetInteraction->VirtualUserIndex + 1;
            KeyboardInputInteraction->PointerIndex = 0;
            KeyboardInputInteraction->InteractionSource = EWidgetInteractionSource::World;
            KeyboardInputInteraction->bEnableHitTesting = false;
            KeyboardInputInteraction->bShowDebug = false;
            OwnerCharacter->AddInstanceComponent(KeyboardInputInteraction);
            KeyboardInputInteraction->RegisterComponent();
        }
    }

    UE_LOG(
        LogWMCYNPresence,
        Display,
        TEXT("WMCYN UI: configured %d native widget interaction ray(s) for Visibility"),
        ConfiguredWidgetInteractionCount);
}

UWidgetInteractionComponent* UWMCYNFirstSignalPresenceComponent::GetPreferredWidgetInteraction() const
{
    return PreferredWidgetInteraction;
}

UWidgetInteractionComponent* UWMCYNFirstSignalPresenceComponent::GetKeyboardInputInteraction() const
{
    return KeyboardInputInteraction;
}

void UWMCYNFirstSignalPresenceComponent::UpdateWidgetInteractionPresentation()
{
    if (!PreferredWidgetInteraction)
    {
        return;
    }

    const bool bShowContextRay = OwnerCharacter && OwnerCharacter->IsLocallyControlled() &&
        !bLoginGateCompleted;
    PreferredWidgetInteraction->bShowDebug = bShowContextRay;
}

void UWMCYNFirstSignalPresenceComponent::ApplyLocalLoginGateLock()
{
    if (bLoginGateCompleted || bLoginGateLockApplied || !OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PlayerController)
    {
        return;
    }

    PlayerController->SetIgnoreMoveInput(true);
    PlayerController->SetIgnoreLookInput(true);
    if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }

    bLoginGateLockApplied = true;
    UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Login: locomotion and stick turning locked"));
}

void UWMCYNFirstSignalPresenceComponent::CompleteLocalLoginGate()
{
    bLoginGateCompleted = true;
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
    }

    if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
    {
        if (APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
        {
            PlayerController->ResetIgnoreMoveInput();
            PlayerController->ResetIgnoreLookInput();
        }
    }

    bLoginGateLockApplied = false;
    UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Login: locomotion and stick turning unlocked"));
}

void UWMCYNFirstSignalPresenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
    {
        if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface(); SessionInterface.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void UWMCYNFirstSignalPresenceComponent::TickComponent(
    const float DeltaTime,
    const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
    }

    if (!TrackedCamera || !LeftController || !RightController)
    {
        CacheNativeComponents();
    }

    if (!OwnerCharacter)
    {
        return;
    }

    if (OwnerCharacter->IsLocallyControlled())
    {
        ApplyLocalLoginGateLock();
        UpdateWidgetInteractionPresentation();
        CaptureAndSendPose();
    }
    else
    {
        ConfigureRemoteTracking();
        ApplyReplicatedPose(DeltaTime);
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (Now >= NextNameplateRefreshTime)
    {
        NextNameplateRefreshTime = Now + 0.25f;
        RefreshNameplate();
        if (!bVoiceRegistered)
        {
            RegisterVoice();
        }
    }

    if (Nameplate && Nameplate->IsVisible())
    {
        if (const APlayerController* LocalController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
        {
            if (const APlayerCameraManager* CameraManager = LocalController->PlayerCameraManager)
            {
                const FVector ToViewer = CameraManager->GetCameraLocation() - Nameplate->GetComponentLocation();
                if (!ToViewer.IsNearlyZero())
                {
                    Nameplate->SetWorldRotation(ToViewer.Rotation());
                }
            }
        }
    }
}

void UWMCYNFirstSignalPresenceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedHeadLocation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedHeadRotation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedLeftHandLocation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedLeftHandRotation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedRightHandLocation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedRightHandRotation, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, bHasReplicatedPose, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(UWMCYNFirstSignalPresenceComponent, ReplicatedPoseSequence, COND_SkipOwner);
}

void UWMCYNFirstSignalPresenceComponent::SubmitIdentity(const FString& Username, const FString& DisplayName)
{
    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<ACharacter>(GetOwner());
    }

    if (!OwnerCharacter)
    {
        return;
    }

    if (OwnerCharacter->HasAuthority())
    {
        ApplyIdentity(Username, DisplayName);
    }
    else
    {
        ServerSubmitIdentity(Username, DisplayName);
    }
}

void UWMCYNFirstSignalPresenceComponent::ServerSubmitIdentity_Implementation(
    const FString& Username,
    const FString& DisplayName)
{
    ApplyIdentity(Username, DisplayName);
}

void UWMCYNFirstSignalPresenceComponent::ApplyIdentity(const FString& Username, const FString& DisplayName)
{
    if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
    {
        return;
    }

    APlayerState* PlayerState = OwnerCharacter->GetPlayerState();
    if (!PlayerState)
    {
        return;
    }

    FString CleanUsername = Username.TrimStartAndEnd().Left(32);
    FString CleanDisplayName = DisplayName.TrimStartAndEnd().Left(32);
    if (CleanUsername.IsEmpty())
    {
        return;
    }
    if (CleanDisplayName.IsEmpty())
    {
        CleanDisplayName = CleanUsername;
    }

    if (FStrProperty* UsernameProperty = FindFProperty<FStrProperty>(PlayerState->GetClass(), TEXT("username")))
    {
        UsernameProperty->SetPropertyValue_InContainer(PlayerState, CleanUsername);
    }
    if (FStrProperty* DisplayNameProperty = FindFProperty<FStrProperty>(PlayerState->GetClass(), TEXT("displayName")))
    {
        DisplayNameProperty->SetPropertyValue_InContainer(PlayerState, CleanDisplayName);
    }

    PlayerState->SetPlayerName(CleanDisplayName);
    PlayerState->ForceNetUpdate();
    UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Identity: server accepted %s"), *CleanDisplayName);
}

void UWMCYNFirstSignalPresenceComponent::ServerUpdateTrackedPose_Implementation(
    const FVector_NetQuantize10 InHeadLocation,
    const FRotator InHeadRotation,
    const FVector_NetQuantize10 InLeftHandLocation,
    const FRotator InLeftHandRotation,
    const FVector_NetQuantize10 InRightHandLocation,
    const FRotator InRightHandRotation)
{
    ReplicatedHeadLocation = InHeadLocation;
    ReplicatedHeadRotation = InHeadRotation;
    ReplicatedLeftHandLocation = InLeftHandLocation;
    ReplicatedLeftHandRotation = InLeftHandRotation;
    ReplicatedRightHandLocation = InRightHandLocation;
    ReplicatedRightHandRotation = InRightHandRotation;
    bHasReplicatedPose = true;
    ++ReplicatedPoseSequence;
    ++ServerPoseUpdateCount;

    if (ServerPoseUpdateCount == 1)
    {
        UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Tracking: server received native head/hand pose for %s"), *GetNameSafe(GetOwner()));
    }
}

void UWMCYNFirstSignalPresenceComponent::CacheNativeComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TrackedCamera = Owner->FindComponentByClass<UCameraComponent>();

    TArray<UMotionControllerComponent*> MotionControllers;
    Owner->GetComponents(MotionControllers);
    for (UMotionControllerComponent* MotionController : MotionControllers)
    {
        const FString ControllerName = MotionController->GetName();
        const FString MotionSource = MotionController->GetTrackingMotionSource().ToString();
        if (!LeftController && (ControllerName.Contains(TEXT("Left")) || MotionSource.Contains(TEXT("Left"))))
        {
            LeftController = MotionController;
        }
        else if (!RightController && (ControllerName.Contains(TEXT("Right")) || MotionSource.Contains(TEXT("Right"))))
        {
            RightController = MotionController;
        }
    }

    TArray<USceneComponent*> SceneComponents;
    Owner->GetComponents(SceneComponents);
    for (USceneComponent* SceneComponent : SceneComponents)
    {
        if (SceneComponent && SceneComponent->GetName().Contains(TEXT("Top of Head")))
        {
            HeadAnchor = SceneComponent;
            break;
        }
    }

    if (!HeadAnchor)
    {
        HeadAnchor = TrackedCamera;
    }
}

void UWMCYNFirstSignalPresenceComponent::ConfigureRemoteTracking()
{
    if (bRemoteTrackingConfigured)
    {
        return;
    }

    if (TrackedCamera)
    {
        TrackedCamera->bLockToHmd = false;
    }
    if (LeftController)
    {
        LeftController->SetComponentTickEnabled(false);
    }
    if (RightController)
    {
        RightController->SetComponentTickEnabled(false);
    }

    bRemoteTrackingConfigured = TrackedCamera && LeftController && RightController;
}

void UWMCYNFirstSignalPresenceComponent::CaptureAndSendPose()
{
    if (!TrackedCamera || !LeftController || !RightController || !GetWorld())
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < NextPoseSendTime)
    {
        return;
    }
    NextPoseSendTime = Now + 0.05f;

    const FVector_NetQuantize10 HeadLocation = TrackedCamera->GetRelativeLocation();
    const FRotator HeadRotation = TrackedCamera->GetRelativeRotation();
    const FVector_NetQuantize10 LeftLocation = LeftController->GetRelativeLocation();
    const FRotator LeftRotation = LeftController->GetRelativeRotation();
    const FVector_NetQuantize10 RightLocation = RightController->GetRelativeLocation();
    const FRotator RightRotation = RightController->GetRelativeRotation();

    if (OwnerCharacter->HasAuthority())
    {
        ReplicatedHeadLocation = HeadLocation;
        ReplicatedHeadRotation = HeadRotation;
        ReplicatedLeftHandLocation = LeftLocation;
        ReplicatedLeftHandRotation = LeftRotation;
        ReplicatedRightHandLocation = RightLocation;
        ReplicatedRightHandRotation = RightRotation;
        bHasReplicatedPose = true;
        ++ReplicatedPoseSequence;
    }
    else
    {
        ServerUpdateTrackedPose(HeadLocation, HeadRotation, LeftLocation, LeftRotation, RightLocation, RightRotation);
    }
}

void UWMCYNFirstSignalPresenceComponent::ApplyReplicatedPose(const float DeltaTime)
{
    if (!bHasReplicatedPose)
    {
        return;
    }

    const bool bSnap = RemotePoseApplyCount == 0;
    const auto InterpolateLocation = [bSnap, DeltaTime](const FVector& Current, const FVector& Target)
    {
        return bSnap ? Target : FMath::VInterpTo(Current, Target, DeltaTime, 18.0f);
    };
    const auto InterpolateRotation = [bSnap, DeltaTime](const FRotator& Current, const FRotator& Target)
    {
        return bSnap ? Target : FMath::RInterpTo(Current, Target, DeltaTime, 18.0f);
    };

    if (TrackedCamera)
    {
        TrackedCamera->SetRelativeLocationAndRotation(
            InterpolateLocation(TrackedCamera->GetRelativeLocation(), ReplicatedHeadLocation),
            InterpolateRotation(TrackedCamera->GetRelativeRotation(), ReplicatedHeadRotation));
    }
    if (LeftController)
    {
        LeftController->SetRelativeLocationAndRotation(
            InterpolateLocation(LeftController->GetRelativeLocation(), ReplicatedLeftHandLocation),
            InterpolateRotation(LeftController->GetRelativeRotation(), ReplicatedLeftHandRotation));
    }
    if (RightController)
    {
        RightController->SetRelativeLocationAndRotation(
            InterpolateLocation(RightController->GetRelativeLocation(), ReplicatedRightHandLocation),
            InterpolateRotation(RightController->GetRelativeRotation(), ReplicatedRightHandRotation));
    }

    ++RemotePoseApplyCount;
    if (RemotePoseApplyCount == 1)
    {
        UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Tracking: remote native pose active for %s"), *GetNameSafe(GetOwner()));
    }
}

void UWMCYNFirstSignalPresenceComponent::CreateNameplate()
{
    if (Nameplate || !GetOwner())
    {
        return;
    }

    if (!HeadAnchor)
    {
        CacheNativeComponents();
    }

    Nameplate = NewObject<UTextRenderComponent>(GetOwner(), TEXT("WMCYN_Nameplate_Runtime"));
    if (!Nameplate)
    {
        return;
    }

    Nameplate->RegisterComponent();
    Nameplate->SetHorizontalAlignment(EHTA_Center);
    Nameplate->SetVerticalAlignment(EVRTA_TextCenter);
    Nameplate->SetWorldSize(12.0f);
    Nameplate->SetTextRenderColor(FColor::White);
    Nameplate->SetCastShadow(false);
    Nameplate->SetOwnerNoSee(true);

    if (HeadAnchor)
    {
        Nameplate->AttachToComponent(HeadAnchor, FAttachmentTransformRules::KeepRelativeTransform);
        Nameplate->SetRelativeLocation(FVector(0.0f, 0.0f, 24.0f));
    }
}

void UWMCYNFirstSignalPresenceComponent::RefreshNameplate()
{
    if (!Nameplate || !OwnerCharacter)
    {
        return;
    }

    const APlayerState* PlayerState = OwnerCharacter->GetPlayerState();
    const FString DisplayName = ResolveDisplayName(PlayerState);
    Nameplate->SetText(FText::FromString(DisplayName.IsEmpty() ? TEXT("WMCYN User") : DisplayName));
    Nameplate->SetVisibility(!OwnerCharacter->IsLocallyControlled());
}

FString UWMCYNFirstSignalPresenceComponent::ResolveDisplayName(const APlayerState* PlayerState) const
{
    if (!PlayerState)
    {
        return FString();
    }

    if (const FStrProperty* DisplayNameProperty = FindFProperty<FStrProperty>(PlayerState->GetClass(), TEXT("displayName")))
    {
        const FString DisplayName = DisplayNameProperty->GetPropertyValue_InContainer(PlayerState);
        if (!DisplayName.IsEmpty())
        {
            return DisplayName;
        }
    }

    return PlayerState->GetPlayerName();
}

void UWMCYNFirstSignalPresenceComponent::RegisterVoice()
{
    if (bVoiceRegistered || !OwnerCharacter)
    {
        return;
    }

    APlayerState* PlayerState = OwnerCharacter->GetPlayerState();
    if (!PlayerState || !PlayerState->GetUniqueId().IsValid())
    {
        return;
    }

    if (!VoiceTalker)
    {
        VoiceTalker = NewObject<UVOIPTalker>(OwnerCharacter, TEXT("WMCYN_VOIPTalker_Runtime"));
        if (!VoiceTalker)
        {
            return;
        }
        VoiceTalker->RegisterComponent();
        VoiceTalker->Settings.ComponentToAttachTo = HeadAnchor ? HeadAnchor.Get() : TrackedCamera.Get();
    }

    VoiceTalker->RegisterWithPlayerState(PlayerState);
    bVoiceRegistered = true;

    UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Voice: native pawn talker registered for %s"), *PlayerState->GetPlayerName());

    if (OwnerCharacter->IsLocallyControlled())
    {
        ActivateLocalVoice();
    }
}

void UWMCYNFirstSignalPresenceComponent::ActivateLocalVoice()
{
    if (bVoiceActivationRequested || !OwnerCharacter)
    {
        return;
    }
    bVoiceActivationRequested = true;
    UVOIPStatics::SetMicThreshold(0.0f);

    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    const IOnlineSessionPtr SessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
    if (!SessionInterface.IsValid() || SessionInterface->GetNamedSession(NAME_GameSession))
    {
        HandleVoiceSessionCreated(NAME_GameSession, SessionInterface.IsValid());
        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = 4;
    SessionSettings.bIsLANMatch = true;
    SessionSettings.bShouldAdvertise = false;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bUsesPresence = false;

    CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UWMCYNFirstSignalPresenceComponent::HandleVoiceSessionCreated));

    if (!SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings))
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
        HandleVoiceSessionCreated(NAME_GameSession, false);
    }
}

void UWMCYNFirstSignalPresenceComponent::HandleVoiceSessionCreated(const FName SessionName, const bool bWasSuccessful)
{
    if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
    {
        if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface(); SessionInterface.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
        }
    }

    if (APlayerController* Controller = Cast<APlayerController>(OwnerCharacter ? OwnerCharacter->GetController() : nullptr))
    {
        Controller->ConsoleCommand(TEXT("ToggleSpeaking 1"), true);
    }

    UE_LOG(LogWMCYNPresence, Display, TEXT("WMCYN Voice: native local capture requested (%s)"), bWasSuccessful ? TEXT("session ready") : TEXT("fallback"));
}
