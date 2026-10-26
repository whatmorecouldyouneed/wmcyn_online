#include "WMCYNCameraToolComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/WidgetComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "ISpectatorScreenController.h"
#include "IHeadMountedDisplay.h"
#include "HeadMountedDisplayTypes.h"
#include "EngineUtils.h"
#include "WMCYNInterviewCamera.h"
#include "WMCYNFirstSignalPresenceComponent.h"
#include "MotionControllerComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNCamera, Log, All);

UWMCYNCameraToolComponent::UWMCYNCameraToolComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

bool UWMCYNCameraToolComponent::SetCameraEnabled(bool bEnabled)
{
    if (bEnabled && !CanOperateCamera()) return false;
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !Pawn->IsLocallyControlled() || GetNetMode() == NM_DedicatedServer) return false;
#if !PLATFORM_WINDOWS
    if (bEnabled) return false; // PCVR pilot: do not allocate a second view on Quest yet.
#endif
    if (bEnabled && !Capture)
    {
        Target = NewObject<UTextureRenderTarget2D>(this);
        Target->RenderTargetFormat = RTF_RGBA8;
        Target->InitAutoFormat(640, 360);
        Capture = NewObject<USceneCaptureComponent2D>(Pawn);
        Capture->TextureTarget = Target;
        Capture->CaptureSource = SCS_FinalColorLDR;
        Capture->bCaptureEveryFrame = false;
        Capture->bCaptureOnMovement = false;
        Capture->FOVAngle = 60.0f;
        Pawn->AddInstanceComponent(Capture);
        Capture->RegisterComponent();
        FActorSpawnParameters Params;
        Params.Owner = Pawn;
        Params.ObjectFlags |= RF_Transient;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        CameraProp = GetWorld()->SpawnActor<AWMCYNInterviewCamera>(Params);
        if (!CameraProp)
        {
            Capture->DestroyComponent();
            Capture = nullptr;
            Target = nullptr;
            return false;
        }
        CameraProp->SetPreview(Target);
        Capture->AttachToComponent(CameraProp->GetLens(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        ResetCameraPose();
    }
    bCameraEnabled = bEnabled;
    bHeld = false;
    bGripWasDown = true; // Opening/recalling while gripping must not pick up accidentally.
    SetComponentTickEnabled(bEnabled);
    if (CameraProp) CameraProp->SetActorHiddenInGame(!bEnabled);
    if (!bEnabled) SetSpectatorOutput(false);
    if (Capture)
    {
        RefreshCaptureExclusions();
        Capture->bCaptureEveryFrame = bEnabled;
        if (CameraProp) Capture->HideActorComponents(CameraProp);
        Capture->SetVisibility(bEnabled);
    }
    return true;
}

bool UWMCYNCameraToolComponent::CanOperateCamera() const
{
    const APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !Pawn->IsLocallyControlled() || GetNetMode() == NM_DedicatedServer) return false;
#if !UE_BUILD_SHIPPING
    // Explicitly local preview only. Never grants online Recording authority.
    if (GetNetMode() == NM_Standalone) return true;
#endif
    const auto* Presence = Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>();
    return Presence && Presence->HasVerifiedRecordingCapability();
}

void UWMCYNCameraToolComponent::RefreshCaptureExclusions()
{
    if (!Capture) return;
    Capture->HiddenComponents.Reset();
    // Menus can be rebuilt after capture starts. Hide their newly created widgets too.
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        TInlineComponentArray<UWidgetComponent*> Widgets(*It);
        for (UWidgetComponent* Widget : Widgets) Capture->HiddenComponents.Add(Widget);
    }
    if (CameraProp) Capture->HideActorComponents(CameraProp);
}

void UWMCYNCameraToolComponent::ResetCameraPose()
{
    if (!CanOperateCamera()) return;
    if (!CameraProp || !GetOwner()) return;
    const UCameraComponent* Head = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!Head) return;
    // Rear display faces the user; lens points out into the room.
    const FRotator Yaw(0, Head->GetComponentRotation().Yaw, 0);
    const FVector Position = Head->GetComponentLocation() + Yaw.Vector() * 45.0f - FVector(0,0,15);
    CameraProp->SetActorLocationAndRotation(Position, Yaw);
    bHeld = false;
    bGripWasDown = true;
}

bool UWMCYNCameraToolComponent::IsNearDoor(const FVector& HandLocation) const
{
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (!It->GetClass()->GetPathName().StartsWith(TEXT("/Game/WMCYN/Environment/Crib/Doors/"))) continue;
        // Existing door adapters poll raw grip. Yield before their 35 cm pickup range,
        // including swept leaves, rather than consuming or rewriting their input.
        if (const FBoolProperty* Held = FindFProperty<FBoolProperty>(It->GetClass(), TEXT("GripHeld")))
            if (Held->GetPropertyValue_InContainer(*It)) return true;
        if (It->GetComponentsBoundingBox(true).ExpandBy(50).IsInsideOrOn(HandLocation)) return true;
    }
    return false;
}

void UWMCYNCameraToolComponent::UpdateGrip(const FTransform& Hand, bool bTracked, bool bDown, bool bDoorPriority)
{
    if (!CameraProp || !bCameraEnabled) return;
    if (bHeld && (!bDown || !bTracked || bDoorPriority))
    {
        bHeld = false;
        UE_LOG(LogWMCYNCamera, Display, TEXT("Camera released: shot held"));
    }
    if (!bHeld && bDown && !bGripWasDown && bTracked && !bDoorPriority &&
        FVector::DistSquared(Hand.GetLocation(), CameraProp->GetGripLocation()) <= FMath::Square(20.0f))
    {
        GripOffset = CameraProp->GetActorTransform().GetRelativeTransform(Hand);
        bHeld = true;
        UE_LOG(LogWMCYNCamera, Display, TEXT("Camera gripped"));
    }
    if (bHeld) CameraProp->SetActorTransform(GripOffset * Hand);
    // Tracking loss requires release/re-press before resuming.
    bGripWasDown = bDown || !bTracked;
}

void UWMCYNCameraToolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!CanOperateCamera())
    {
        SetCameraEnabled(false);
        return;
    }
    ExclusionRefreshSeconds += DeltaTime;
    if (ExclusionRefreshSeconds >= 1.f)
    {
        RefreshCaptureExclusions();
        ExclusionRefreshSeconds = 0;
    }
    APawn* Pawn = Cast<APawn>(GetOwner());
    auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!Pawn || !Pawn->IsLocallyControlled() || !PC)
    {
        bHeld = false;
        if (Capture) Capture->bCaptureEveryFrame = false;
        SetSpectatorOutput(false);
        return;
    }
    if (!RightHand)
    {
        TInlineComponentArray<UMotionControllerComponent*> Hands(Pawn);
        for (auto* Hand : Hands)
            if (Hand->GetName() == TEXT("RightController") || Hand->GetName() == TEXT("Right Controller"))
            { RightHand = Hand; break; }
        if (!RightHand)
            for (auto* Hand : Hands)
                if (Hand->GetTrackingMotionSource() == FName(TEXT("Right"))) { RightHand = Hand; break; }
        if (RightHand) AddTickPrerequisiteComponent(RightHand);
    }
    const bool bDown = PC->GetInputAnalogKeyState(FKey(TEXT("OculusTouch_Right_Grip_Axis"))) > .5f ||
        PC->IsInputKeyDown(FKey(TEXT("OculusTouch_Right_Grip_Click")));
    const FTransform Hand = RightHand ? RightHand->GetComponentTransform() : FTransform::Identity;
    UpdateGrip(Hand, RightHand && RightHand->IsTracked(), bDown, IsNearDoor(Hand.GetLocation()));
}

void UWMCYNCameraToolComponent::SetFieldOfView(float Degrees)
{
    if (!CanOperateCamera()) return;
    if (Capture && FMath::IsFinite(Degrees)) Capture->FOVAngle = FMath::Clamp(Degrees, 30.0f, 100.0f);
}

bool UWMCYNCameraToolComponent::HasShot(int32 Slot) const
{
    return Slot >= 0 && Slot < 3 && Shots.Contains(Slot);
}

bool UWMCYNCameraToolComponent::SaveShot(int32 Slot)
{
    if (!CanOperateCamera()) return false;
    const APawn* Pawn = Cast<APawn>(GetOwner());
    if (Slot < 0 || Slot >= 3 || !Pawn || !Pawn->IsLocallyControlled() ||
        !bCameraEnabled || !CameraProp || !Capture) return false;
    FShot Shot;
    Shot.Pose = CameraProp->GetActorTransform();
    Shot.FieldOfView = Capture->FOVAngle;
    Shots.Add(Slot, Shot);
    return true;
}

bool UWMCYNCameraToolComponent::SelectShot(int32 Slot)
{
    if (!CanOperateCamera()) return false;
    const APawn* Pawn = Cast<APawn>(GetOwner());
    if (!Pawn || !Pawn->IsLocallyControlled() || !bCameraEnabled || !CameraProp || !HasShot(Slot)) return false;
    const FShot& Shot = Shots.FindChecked(Slot);
    bHeld = false;
    bGripWasDown = true;
    CameraProp->SetActorTransform(Shot.Pose);
    SetFieldOfView(Shot.FieldOfView);
    return true;
}

bool UWMCYNCameraToolComponent::SetSpectatorOutput(bool bEnabled)
{
    if (bEnabled && !CanOperateCamera()) return false;
#if PLATFORM_WINDOWS
    IHeadMountedDisplay* HMD = GEngine && GEngine->XRSystem.IsValid() ? GEngine->XRSystem->GetHMDDevice() : nullptr;
    ISpectatorScreenController* Spectator = HMD ? HMD->GetSpectatorScreenController() : nullptr;
    if (!Spectator || (bEnabled && !bCameraEnabled)) return false;
    if (bEnabled)
    {
        if (!bSpectatorOutput)
        {
            PreviousSpectatorMode = static_cast<uint8>(Spectator->GetSpectatorScreenMode());
            PreviousSpectatorTexture = Spectator->GetSpectatorScreenTexture();
        }
        Spectator->SetSpectatorScreenTexture(Target);
        Spectator->SetSpectatorScreenMode(ESpectatorScreenMode::Texture);
    }
    else if (bSpectatorOutput)
    {
        Spectator->SetSpectatorScreenTexture(PreviousSpectatorTexture);
        Spectator->SetSpectatorScreenMode(static_cast<ESpectatorScreenMode>(PreviousSpectatorMode));
        PreviousSpectatorTexture = nullptr;
    }
    bSpectatorOutput = bEnabled;
    return true;
#else
    return false;
#endif
}

void UWMCYNCameraToolComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    SetSpectatorOutput(false);
    if (Capture) Capture->DestroyComponent();
    if (CameraProp) CameraProp->Destroy();
    CameraProp = nullptr;
    Capture = nullptr;
    Target = nullptr;
    Super::EndPlay(Reason);
}
