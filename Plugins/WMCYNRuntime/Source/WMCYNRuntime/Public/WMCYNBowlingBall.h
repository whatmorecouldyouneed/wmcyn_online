#pragma once
#include "WMCYNSessionItem.h"
#include "WMCYNBowlingBall.generated.h"

class AStaticMeshActor;

/** A placed pickup controls one rack; the released ball and pins are server physics actors. */
UCLASS()
class WMCYNRUNTIME_API AWMCYNBowlingBall : public AWMCYNSessionItem
{
    GENERATED_BODY()
public:
    UPROPERTY(EditInstanceOnly) TObjectPtr<UStaticMesh> PinMesh;
    /** Head pin centre, in world centimetres. Rack spreads along the pickup's initial forward axis. */
    UPROPERTY(EditInstanceOnly) FVector HeadPin = FVector::ZeroVector;
    virtual void BeginPlay() override;
    virtual void Tick(float Seconds) override;
    virtual void EndPlay(EEndPlayReason::Type Reason) override;
protected:
    virtual void HandleTaken() override;
    virtual void HandleReleased() override;
private:
    AStaticMeshActor* SpawnBody(UStaticMesh* Shape, FVector Position, float Mass);
    void ResetRack();
    FTransform Home;
    FVector LastHand = FVector::ZeroVector, ThrowVelocity = FVector::ZeroVector;
    UPROPERTY() TArray<TObjectPtr<AStaticMeshActor>> Pins;
    UPROPERTY() TObjectPtr<AStaticMeshActor> RollingBall;
    float RollTime = 0.f;
};
