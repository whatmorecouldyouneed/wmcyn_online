#include "WMCYNComfort.h"

float WMCYNComfort::TurnDegrees(const float Stick, const float Ramp, const float DeltaSeconds)
{
    return FMath::Clamp(Stick, -1.0f, 1.0f) * FMath::Clamp(Ramp, 0.0f, 1.0f) * MaxTurnDegreesPerSecond * FMath::Max(DeltaSeconds, 0.0f);
}

float WMCYNComfort::NextTurnRamp(const float Ramp, const float DeltaSeconds)
{
    return FMath::Min(Ramp + FMath::Max(DeltaSeconds, 0.0f) / TurnRampSeconds, 1.0f);
}

float WMCYNComfort::CrouchInput(const float Stick)
{
    if (FMath::Abs(Stick) < CrouchIntentThreshold)
    {
        return 0.0f;
    }
    // Restart from zero at the threshold, so crouching eases in instead of jumping to 70% speed.
    const float Sign = FMath::Sign(Stick);
    return Sign * (FMath::Min(FMath::Abs(Stick), 1.0f) - CrouchIntentThreshold) / (1.0f - CrouchIntentThreshold);
}

FVector WMCYNComfort::NextRemoteOffset(const FVector& Offset, const FVector& LastRoot, const FVector& Root, const float DeltaSeconds)
{
    if (FVector::DistSquared(LastRoot, Root) > FMath::Square(RemoteSnapDistance))
    {
        return FVector::ZeroVector;
    }
    return (Offset + LastRoot - Root) * FMath::Exp(-RemoteSmoothingRate * FMath::Max(DeltaSeconds, 0.0f));
}
