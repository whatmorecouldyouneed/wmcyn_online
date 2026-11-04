#pragma once

#include "CoreMinimal.h"

class UWorld;

/** True when a Crib door should keep this hand's grip. Tools yield rather than rewrite door input. */
WMCYNRUNTIME_API bool WMCYNIsHandNearDoor(UWorld* World, const FVector& HandLocation);
