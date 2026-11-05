#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WMCYNComfort.generated.h"

/** Comfort rules for VR movement, kept apart from the pawn so they can be tested without a headset. */
namespace WMCYNComfort
{
    constexpr float MaxTurnDegreesPerSecond = 90.0f;
    constexpr float TurnRampSeconds = 0.08f;
    /** About 45 degrees below the stick's horizontal, so a turn can't start a crouch. */
    constexpr float CrouchIntentThreshold = 0.7f;
    constexpr float MaxWalkCmPerSecond = 350.0f;
    /** High enough to reach full speed and stop within a few frames. Ramps are what cause sickness. */
    constexpr float MoveAcceleration = 8192.0f;
    /** Follows a remote player's root closely while hiding network steps. */
    constexpr float RemoteSmoothingRate = 20.0f;
    constexpr float RemoteSnapDistance = 200.0f;

    WMCYNRUNTIME_API float TurnDegrees(float Stick, float Ramp, float DeltaSeconds);
    WMCYNRUNTIME_API float NextTurnRamp(float Ramp, float DeltaSeconds);
    WMCYNRUNTIME_API float CrouchInput(float Stick);
    /** Moves the visual offset by the root's jump, then decays it toward zero. */
    WMCYNRUNTIME_API FVector NextRemoteOffset(const FVector& Offset, const FVector& LastRoot, const FVector& Root, float DeltaSeconds);
}

/** Blueprint entry points for the First Signal pawn's input events. */
UCLASS()
class WMCYNRUNTIME_API UWMCYNComfortLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** The vertical stick value that counts as a deliberate crouch; anything less is zero. */
    UFUNCTION(BlueprintPure, Category = "WMCYN|Comfort")
    static float CrouchInput(float Stick) { return WMCYNComfort::CrouchInput(Stick); }
};
