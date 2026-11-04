#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
#include "WMCYNSessionItem.generated.h"

class APawn;
class AWMCYNSessionItem;
class UStaticMeshComponent;

/**
 * Held: Location/Rotation are the grip offset from the hand. Carried: they are the offset from
 * the carrying item. Otherwise they are the world pose.
 */
USTRUCT()
struct FWMCYNItemPlacement
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<APawn> Holder;
    UPROPERTY() TObjectPtr<AWMCYNSessionItem> Carrier;
    UPROPERTY() bool bRightHand = false;
    UPROPERTY() FVector_NetQuantize100 Location = FVector::ZeroVector;
    UPROPERTY() FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Server-owned tray item. Follows the source's kinematic pickups: no physics, and items stay
 * exactly where released. While held, every machine attaches it to the holder's tracked hand,
 * so no per-frame item traffic is needed.
 */
UCLASS(Abstract)
class WMCYNRUNTIME_API AWMCYNSessionItem : public AActor
{
    GENERATED_BODY()
public:
    /** Generous because a tracked hand can sit well away from the pawn's root. */
    static constexpr float MaxServerReach = 350.0f;

    AWMCYNSessionItem();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    APawn* GetHolder() const { return Placement.Holder; }
    bool IsGrabbable() const { return bGrabbable; }
    /** Carriers such as the tray are easier to reach and yield to anything lying on them. */
    virtual bool IsCarrier() const { return false; }
    FVector GetGripLocation() const;
    /** Measured to the mesh bounds, so large items can be taken by their edge. */
    float GetGripDistanceSquared(const FVector& HandLocation) const;
    bool IsInServerReach(const AActor* Other) const;

    bool TryTake(APawn* Pawn, bool bRightHand, const FVector& GripLocation, const FRotator& GripRotation);
    bool TryDrop(APawn* Pawn, const FVector& Location, const FRotator& Rotation);
    virtual void ServerUse(bool bDown) {}
    void ServerSetCarrier(AWMCYNSessionItem* NewCarrier);
    AWMCYNSessionItem* GetCarrier() const { return Placement.Carrier; }

    /** Local attach ahead of the server's answer; ReapplyPlacement undoes a refused take. */
    void PredictHold(APawn* Pawn, bool bRightHand, const FTransform& GripOffset);
    void ReapplyPlacement();

protected:
    virtual void HandleTaken() {}
    virtual void HandleReleased() {}
    void Release();

    UPROPERTY(VisibleAnywhere, Category="WMCYN|Item") TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditAnywhere, Category="WMCYN|Item") bool bGrabbable = true;

private:
    void AttachToHolder(APawn* Pawn, bool bRightHand, const FVector& Location, const FRotator& Rotation);
    UFUNCTION() void OnRep_Placement();
    bool bServerHeld = false;
    UPROPERTY(ReplicatedUsing=OnRep_Placement) FWMCYNItemPlacement Placement;
};
