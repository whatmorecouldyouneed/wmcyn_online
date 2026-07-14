#include "WMCYNFirstSignalBlueprintLibrary.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "WMCYNFirstSignalPresenceComponent.h"

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
    return true;
}
