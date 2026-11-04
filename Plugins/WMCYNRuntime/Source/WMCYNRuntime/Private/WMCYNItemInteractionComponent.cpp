#include "WMCYNItemInteractionComponent.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "WMCYNCameraToolComponent.h"
#include "WMCYNDoorGrip.h"
#include "WMCYNFirstSignalPresenceComponent.h"
#include "WMCYNHighEffectComponent.h"
#include "WMCYNInterviewCamera.h"
#include "WMCYNJointItem.h"
#include "WMCYNLighterItem.h"
#include "WMCYNSessionFixture.h"

namespace
{
constexpr float ItemRefreshInterval = 1.0f;
constexpr float RefusedTakeSeconds = 1.0f;

bool IsPressed(const APlayerController* PC, const FKey& Axis, const FKey& Click)
{
    return PC->GetInputAnalogKeyState(Axis) > 0.5f || PC->IsInputKeyDown(Click);
}
}

UWMCYNItemInteractionComponent::UWMCYNItemInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    Right.bRight = true;
}

AWMCYNSessionItem* UWMCYNItemInteractionComponent::GetHeldItem(const bool bRightHand) const
{
    return (bRightHand ? Right : Left).Item.Get();
}

void UWMCYNItemInteractionComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    APawn* Pawn = Cast<APawn>(GetOwner());
    const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const auto* Presence = Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
    if (!PC || !Pawn->IsLocallyControlled() || !Presence || !Presence->CanUseWorldItems()) return;

    ItemRefreshSeconds -= DeltaTime;
    if (ItemRefreshSeconds <= 0.0f)
    {
        RefreshItems();
        ItemRefreshSeconds = ItemRefreshInterval;
    }
    const bool bMenuOpen = Presence->IsRuntimeMenuVisible();
    const auto Poll = [&](FHand& Hand, const FKey& GripAxis, const FKey& GripClick, const FKey& TriggerAxis, const FKey& TriggerClick)
    {
        const USceneComponent* Controller = Presence->GetTrackedHand(Hand.bRight);
        const FTransform Pose = Controller ? Controller->GetComponentTransform() : FTransform::Identity;
        const bool bYield = !Controller || WMCYNIsHandNearDoor(GetWorld(), Pose.GetLocation()) ||
            (Hand.bRight && IsCameraTakingGrip(Pose.GetLocation()));
        UpdateHand(Hand, Pose, Controller && Presence->IsHandTracked(Hand.bRight),
            IsPressed(PC, GripAxis, GripClick), !bMenuOpen && IsPressed(PC, TriggerAxis, TriggerClick), bYield, DeltaTime);
    };
    Poll(Left, FKey(TEXT("OculusTouch_Left_Grip_Axis")), FKey(TEXT("OculusTouch_Left_Grip_Click")),
        FKey(TEXT("OculusTouch_Left_Trigger_Axis")), FKey(TEXT("OculusTouch_Left_Trigger_Click")));
    Poll(Right, FKey(TEXT("OculusTouch_Right_Grip_Axis")), FKey(TEXT("OculusTouch_Right_Grip_Click")),
        FKey(TEXT("OculusTouch_Right_Trigger_Axis")), FKey(TEXT("OculusTouch_Right_Trigger_Click")));
    UpdateContacts();
}

void UWMCYNItemInteractionComponent::UpdateHand(FHand& Hand, const FTransform& Pose, const bool bTracked, const bool bGrip,
    const bool bTrigger, const bool bGripYielded, const float DeltaTime)
{
    if (!bTracked)
    {
        if (Hand.Item.IsValid()) Drop(Hand);
        // Tracking loss requires release/re-press before resuming.
        Hand.bGripWasDown = true;
        Hand.bTriggerWasDown = false;
        return;
    }
    AWMCYNSessionItem* Item = Hand.Item.Get();
    if (Item && Item->GetHolder() != GetOwner())
    {
        // Server hasn't confirmed the take: wait briefly, then accept that it was refused.
        Hand.PendingSeconds -= DeltaTime;
        if (Hand.PendingSeconds <= 0.0f)
        {
            Item->ReapplyPlacement();
            Hand.Item = nullptr;
            Hand.bTriggerWasDown = false;
            Item = nullptr;
        }
    }
    if (bGrip && !Hand.bGripWasDown && !bGripYielded)
    {
        if (Item) Drop(Hand);
        else Take(Hand, Pose);
    }
    Hand.bGripWasDown = bGrip;
    Item = Hand.Item.Get();
    if (!Item)
    {
        Hand.bTriggerWasDown = false;
        return;
    }
    if (bTrigger == Hand.bTriggerWasDown) return;
    Hand.bTriggerWasDown = bTrigger;
    SendUse(Item, bTrigger);
    const auto* Joint = Cast<AWMCYNJointItem>(Item);
    if (bTrigger && Joint && Joint->IsBurning())
    {
        if (!HighEffect)
        {
            HighEffect = NewObject<UWMCYNHighEffectComponent>(GetOwner());
            GetOwner()->AddInstanceComponent(HighEffect);
            HighEffect->RegisterComponent();
        }
        HighEffect->Play(Joint->GetHighOverlayMaterial());
    }
}

void UWMCYNItemInteractionComponent::Take(FHand& Hand, const FTransform& Pose)
{
    AWMCYNSessionItem* Item = FindNearestItem(Pose.GetLocation());
    if (!Item) return;
    APawn* Pawn = Cast<APawn>(GetOwner());
    FTransform Grip = Item->GetActorTransform().GetRelativeTransform(Pose);
    if (Item->IsCarrier()) Grip.SetLocation(Grip.GetLocation().GetClampedToMaxSize(CarrierHoldDistance));
    Hand.Item = Item;
    Hand.PendingSeconds = RefusedTakeSeconds;
    if (Pawn->HasAuthority())
    {
        Item->TryTake(Pawn, Hand.bRight, Grip.GetLocation(), Grip.Rotator());
        return;
    }
    Item->PredictHold(Pawn, Hand.bRight, Grip);
    Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>()->RequestItemTake(Item, Hand.bRight, Grip);
}

void UWMCYNItemInteractionComponent::Drop(FHand& Hand)
{
    AWMCYNSessionItem* Item = Hand.Item.Get();
    Hand.Item = nullptr;
    Hand.bTriggerWasDown = false;
    if (!Item) return;
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (Pawn->HasAuthority())
    {
        Item->TryDrop(Pawn, Item->GetActorLocation(), Item->GetActorRotation());
        return;
    }
    Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>()->RequestItemDrop(Item, Item->GetActorLocation(), Item->GetActorRotation());
}

void UWMCYNItemInteractionComponent::SendUse(AWMCYNSessionItem* Item, const bool bDown)
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (Pawn->HasAuthority())
    {
        if (Item->GetHolder() == Pawn) Item->ServerUse(bDown);
        return;
    }
    Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>()->RequestItemUse(Item, bDown);
}

void UWMCYNItemInteractionComponent::RefreshItems()
{
    Items.Reset();
    for (TActorIterator<AWMCYNSessionItem> It(GetWorld()); It; ++It) Items.Add(*It);
}

bool UWMCYNItemInteractionComponent::IsCameraTakingGrip(const FVector& HandLocation) const
{
    const auto* Camera = GetOwner()->FindComponentByClass<UWMCYNCameraToolComponent>();
    if (!Camera || !Camera->IsCameraEnabled()) return false;
    const AWMCYNInterviewCamera* Prop = Camera->GetCameraProp();
    return Camera->IsHeld() || (Prop && FVector::DistSquared(HandLocation, Prop->GetGripLocation()) <= FMath::Square(GripReach));
}

AWMCYNSessionItem* UWMCYNItemInteractionComponent::FindNearestItem(const FVector& HandLocation) const
{
    AWMCYNSessionItem* Nearest = nullptr;
    AWMCYNSessionItem* NearestCarrier = nullptr;
    float NearestDistance = FMath::Square(GripReach);
    float NearestCarrierDistance = FMath::Square(CarrierGripReach);
    for (AWMCYNSessionItem* Item : Items)
    {
        if (!Item || !Item->IsGrabbable() || Item->GetHolder()) continue;
        const float Distance = Item->GetGripDistanceSquared(HandLocation);
        if (Item->IsCarrier())
        {
            if (Distance <= NearestCarrierDistance)
            {
                NearestCarrier = Item;
                NearestCarrierDistance = Distance;
            }
        }
        else if (Distance <= NearestDistance)
        {
            Nearest = Item;
            NearestDistance = Distance;
        }
    }
    // Anything lying on the tray wins, so the tray never blocks taking a joint.
    return Nearest ? Nearest : NearestCarrier;
}

void UWMCYNItemInteractionComponent::UpdateContacts()
{
    const AWMCYNSessionItem* HeldLeft = Left.Item.Get();
    const AWMCYNSessionItem* HeldRight = Right.Item.Get();
    if (!HeldLeft && !HeldRight)
    {
        ActiveContacts.Reset();
        return;
    }
    const auto IsHeldHere = [&](const AWMCYNSessionItem* Item) { return Item == HeldLeft || Item == HeldRight; };
    TSet<TPair<const AWMCYNSessionItem*, const AWMCYNSessionItem*>> Touching;
    for (AWMCYNSessionItem* Candidate : Items)
    {
        auto* Joint = Cast<AWMCYNJointItem>(Candidate);
        if (!Joint) continue;
        const FVector Ember = Joint->GetEmberLocation();
        for (AWMCYNSessionItem* Other : Items)
        {
            if (!Other || Other == Joint || (!IsHeldHere(Joint) && !IsHeldHere(Other))) continue;
            bool bTouching = false;
            if (const auto* Lighter = Cast<AWMCYNLighterItem>(Other))
            {
                bTouching = Lighter->IsLit() && FVector::DistSquared(Ember, Lighter->GetFlameLocation()) <= FMath::Square(EmberContactRadius);
            }
            else if (const auto* OtherJoint = Cast<AWMCYNJointItem>(Other))
            {
                bTouching = OtherJoint->IsBurning() && FVector::DistSquared(Ember, OtherJoint->GetEmberLocation()) <= FMath::Square(EmberContactRadius);
            }
            else if (const auto* Fixture = Cast<AWMCYNSessionFixture>(Other))
            {
                bTouching = Fixture->GetKind() != EWMCYNFixtureKind::Decor && Fixture->ContainsPoint(Ember);
            }
            if (!bTouching) continue;
            const TPair<const AWMCYNSessionItem*, const AWMCYNSessionItem*> Pair(Joint, Other);
            Touching.Add(Pair);
            // Source reacts on trigger enter, so a held contact fires once.
            if (!ActiveContacts.Contains(Pair)) ReportContact(Joint, Other);
        }
    }
    ActiveContacts = MoveTemp(Touching);
}

void UWMCYNItemInteractionComponent::ReportContact(AWMCYNJointItem* Joint, AWMCYNSessionItem* Other)
{
    APawn* Pawn = Cast<APawn>(GetOwner());
    if (Pawn->HasAuthority())
    {
        Joint->ServerApplyContact(Other);
        return;
    }
    Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>()->RequestItemContact(Joint, Other);
}
