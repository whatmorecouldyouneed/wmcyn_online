#include "WMCYNDoorGrip.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

bool WMCYNIsHandNearDoor(UWorld* World, const FVector& HandLocation)
{
    for (TActorIterator<AActor> It(World); It; ++It)
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
