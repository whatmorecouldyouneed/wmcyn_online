#pragma once
#include "GameFramework/Actor.h"
#include "WMCYNSeat.generated.h"

class ACharacter;
class UTextRenderComponent;
class UPoseableMeshComponent;
class USkeletalMeshComponent;

// One cushion per anchor. The origin is the cushion surface; +X is forward.
UCLASS()
class WMCYNRUNTIME_API AWMCYNSeat : public AActor
{
    GENERATED_BODY()
public:
    AWMCYNSeat();
    bool TrySit(ACharacter* User);
    bool TryStand(ACharacter* User);
    void Release(ACharacter* User);
    void ShowPrompt(bool Visible, bool Seated);
    ACharacter* GetOccupant() const { return Occupant; }
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Props) const override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(ReplicatedUsing=OnRep_Occupant) TObjectPtr<ACharacter> Occupant;
    UFUNCTION() void OnRep_Occupant();
    void ClearPresentation();
    UPROPERTY(Transient) TObjectPtr<UPoseableMeshComponent> SeatBody;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> SourceBody;
    uint8 PreviousAnimTick = 0;
    TWeakObjectPtr<ACharacter> CollisionUser;
    ECollisionEnabled::Type PreviousCollision = ECollisionEnabled::QueryAndPhysics;
    void RestoreCapsule();
    UPROPERTY() TObjectPtr<UTextRenderComponent> Prompt;
    uint8 OldMode = 0;
    uint8 OldCustomMode = 0;
};
