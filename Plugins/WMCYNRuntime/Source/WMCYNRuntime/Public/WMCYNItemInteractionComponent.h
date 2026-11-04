#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WMCYNItemInteractionComponent.generated.h"

class AWMCYNJointItem;
class AWMCYNSessionItem;
class UWMCYNHighEffectComponent;

/** Local grip, trigger and contact input for tray items. All state changes go to the server. */
UCLASS()
class WMCYNRUNTIME_API UWMCYNItemInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    static constexpr float GripReach = 20.0f;
    static constexpr float CarrierGripReach = 35.0f;
    /** A carrier taken from further away is pulled in, not held at arm's length. */
    static constexpr float CarrierHoldDistance = 15.0f;
    static constexpr float EmberContactRadius = 3.0f;

    UWMCYNItemInteractionComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    AWMCYNSessionItem* GetHeldItem(bool bRightHand) const;

private:
    struct FHand
    {
        TWeakObjectPtr<AWMCYNSessionItem> Item;
        bool bRight = false;
        bool bGripWasDown = true;
        bool bTriggerWasDown = false;
        float PendingSeconds = 0.0f;
    };

    void UpdateHand(FHand& Hand, const FTransform& Pose, bool bTracked, bool bGrip, bool bTrigger, bool bGripYielded, float DeltaTime);
    void Take(FHand& Hand, const FTransform& Pose);
    void Drop(FHand& Hand);
    void SendUse(AWMCYNSessionItem* Item, bool bDown);
    void RefreshItems();
    void UpdateContacts();
    void ReportContact(AWMCYNJointItem* Joint, AWMCYNSessionItem* Other);
    bool IsCameraTakingGrip(const FVector& HandLocation) const;
    AWMCYNSessionItem* FindNearestItem(const FVector& HandLocation) const;

    friend class FWMCYNTraySessionGripTest;
    FHand Left;
    FHand Right;
    UPROPERTY(Transient) TArray<TObjectPtr<AWMCYNSessionItem>> Items;
    UPROPERTY(Transient) TObjectPtr<UWMCYNHighEffectComponent> HighEffect;
    TSet<TPair<const AWMCYNSessionItem*, const AWMCYNSessionItem*>> ActiveContacts;
    float ItemRefreshSeconds = 0.0f;
};
