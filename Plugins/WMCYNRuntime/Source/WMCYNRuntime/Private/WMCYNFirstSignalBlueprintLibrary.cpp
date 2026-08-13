#include "WMCYNFirstSignalBlueprintLibrary.h"

#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/UnrealType.h"
#include "WMCYNFirstSignalLoginAsyncAction.h"
#include "WMCYNFirstSignalPairingAsyncAction.h"
#include "WMCYNFirstSignalPresenceComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNFirstSignalUI, Log, All);

bool UWMCYNFirstSignalBlueprintLibrary::SubmitLocalFirstSignalIdentity(
    const UObject* WorldContextObject,
    const FString& Username,
    const FString& DisplayName)
{
    if (!WorldContextObject)
    {
        return false;
    }

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
    UWMCYNFirstSignalPresenceComponent* Presence =
        Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
    if (!Presence)
    {
        return false;
    }

    Presence->SubmitIdentity(Username, DisplayName);
    Presence->CompleteLocalLoginGate();
    return true;
}

bool UWMCYNFirstSignalBlueprintLibrary::SubmitFirstSignalLogin(
    const UObject* WorldContextObject,
    UUserWidget* LoginWidget,
    const FString& Identifier,
    const FString& Password,
    const bool bAllowPIEDevelopmentFallback)
{
    const FString CleanIdentifier = Identifier.TrimStartAndEnd();
    const FString CleanPassword = Password.TrimStartAndEnd();

    // Pairing is the preferred in-headset path. Leaving both fields empty lets
    // the existing Enter World button drive pairing without breaking the
    // credential fallback for desktop or manual testing.
    if (CleanIdentifier.IsEmpty() && CleanPassword.IsEmpty())
    {
        return UWMCYNFirstSignalPairingAsyncAction::StartForLoginWidget(
            WorldContextObject,
            LoginWidget);
    }

    return UWMCYNFirstSignalLoginAsyncAction::StartForLoginWidget(
        WorldContextObject,
        LoginWidget,
        CleanIdentifier,
        CleanPassword,
        bAllowPIEDevelopmentFallback);
}

bool UWMCYNFirstSignalBlueprintLibrary::BeginFirstSignalPairing(
    const UObject* WorldContextObject,
    UUserWidget* LoginWidget)
{
    return UWMCYNFirstSignalPairingAsyncAction::StartForLoginWidget(
        WorldContextObject,
        LoginWidget);
}

bool UWMCYNFirstSignalBlueprintLibrary::PrepareLocalFirstSignalWidgetInteraction(
    const UObject* WorldContextObject,
    UWidgetComponent* WidgetHost)
{
    if (!WorldContextObject || !WidgetHost)
    {
        return false;
    }

#if PLATFORM_ANDROID
    WidgetHost->SetVisibility(false);
    WidgetHost->SetHiddenInGame(true);
    WidgetHost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UE_LOG(
        LogWMCYNFirstSignalUI,
        Display,
        TEXT("WMCYN UI: Quest/Android is using the platform keyboard; AFCore keyboard host suppressed"));
    return false;
#else
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
    UWMCYNFirstSignalPresenceComponent* Presence =
        Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
    UWidgetInteractionComponent* WidgetInteraction =
        Presence ? Presence->GetKeyboardInputInteraction() : nullptr;

    if (!WidgetInteraction)
    {
        return false;
    }

    FObjectPropertyBase* LastInteractionProperty =
        FindFProperty<FObjectPropertyBase>(WidgetHost->GetClass(), TEXT("lastWidgetInteractionComponent"));
    if (!LastInteractionProperty ||
        !LastInteractionProperty->PropertyClass->IsChildOf(UWidgetInteractionComponent::StaticClass()))
    {
        return false;
    }

    LastInteractionProperty->SetObjectPropertyValue_InContainer(WidgetHost, WidgetInteraction);
    UE_LOG(
        LogWMCYNFirstSignalUI,
        Display,
        TEXT("WMCYN UI: registered %s as the AFCore keyboard source interaction"),
        *WidgetInteraction->GetName());
    return true;
#endif
}

bool UWMCYNFirstSignalBlueprintLibrary::RequestLocalFirstSignalRespawn(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return false;
    }

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
    UWMCYNFirstSignalPresenceComponent* Presence =
        Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
    if (!Presence)
    {
        return false;
    }

    Presence->RequestRespawnToPresenceSlot();
    return true;
}

void UWMCYNFirstSignalBlueprintLibrary::PlayFirstSignalFootstep(
    const UObject* WorldContextObject,
    const bool bRightFoot,
    const float Intensity,
    const FTransform& WorldTransform,
    USoundBase* LeftFootstep,
    USoundBase* RightFootstep)
{
    USoundBase* Footstep = bRightFoot ? RightFootstep : LeftFootstep;
    if (!WorldContextObject || !Footstep)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        WorldContextObject,
        Footstep,
        WorldTransform.GetLocation(),
        WorldTransform.Rotator(),
        FMath::Max(Intensity, 0.0f));
}
