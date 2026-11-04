#include "WMCYNSessionItem.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "WMCYNFirstSignalPresenceComponent.h"

AWMCYNSessionItem::AWMCYNSessionItem()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicatingMovement(false);
    SetNetUpdateFrequency(2.0f);
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("ItemRoot")));
    RootComponent->SetMobility(EComponentMobility::Movable);
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetCanEverAffectNavigation(false);
}

void AWMCYNSessionItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWMCYNSessionItem, Placement);
}

void AWMCYNSessionItem::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        Placement.Location = GetActorLocation();
        Placement.Rotation = GetActorRotation();
    }
}

void AWMCYNSessionItem::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // A holder who logs out or respawns must not take the item with them.
    if (HasAuthority() && bServerHeld && !IsValid(Placement.Holder.Get())) Release();
}

FVector AWMCYNSessionItem::GetGripLocation() const
{
    return Mesh->Bounds.Origin;
}

float AWMCYNSessionItem::GetGripDistanceSquared(const FVector& HandLocation) const
{
    return Mesh->Bounds.GetBox().ComputeSquaredDistanceToPoint(HandLocation);
}

bool AWMCYNSessionItem::IsInServerReach(const AActor* Other) const
{
    if (!Other) return false;
    // Hands are only approximately posed on the server, so a held item counts from its holder.
    const AActor* Anchor = Placement.Holder ? static_cast<const AActor*>(Placement.Holder) : this;
    const auto* OtherItem = Cast<AWMCYNSessionItem>(Other);
    const AActor* OtherAnchor = OtherItem && OtherItem->Placement.Holder ? OtherItem->Placement.Holder.Get() : Other;
    return FVector::DistSquared(Anchor->GetActorLocation(), OtherAnchor->GetActorLocation()) <= FMath::Square(MaxServerReach);
}

bool AWMCYNSessionItem::TryTake(APawn* Pawn, const bool bRightHand, const FVector& GripLocation, const FRotator& GripRotation)
{
    if (!HasAuthority() || !bGrabbable || !Pawn || Placement.Holder || !IsInServerReach(Pawn)) return false;
    bServerHeld = true;
    Placement.Carrier = nullptr;
    Placement.Holder = Pawn;
    Placement.bRightHand = bRightHand;
    Placement.Location = GripLocation;
    Placement.Rotation = GripRotation;
    AttachToHolder(Pawn, bRightHand, GripLocation, GripRotation);
    HandleTaken();
    ForceNetUpdate();
    return true;
}

bool AWMCYNSessionItem::TryDrop(APawn* Pawn, const FVector& Location, const FRotator& Rotation)
{
    if (!HasAuthority() || !Pawn || Placement.Holder != Pawn) return false;
    if (FVector::DistSquared(Location, Pawn->GetActorLocation()) > FMath::Square(MaxServerReach)) return false;
    bServerHeld = false;
    Placement.Holder = nullptr;
    Placement.Carrier = nullptr;
    Placement.Location = Location;
    Placement.Rotation = Rotation;
    ReapplyPlacement();
    HandleReleased();
    ForceNetUpdate();
    return true;
}

void AWMCYNSessionItem::Release()
{
    bServerHeld = false;
    Placement.Holder = nullptr;
    Placement.Location = GetActorLocation();
    Placement.Rotation = GetActorRotation();
    ReapplyPlacement();
    HandleReleased();
    ForceNetUpdate();
}

void AWMCYNSessionItem::ServerSetCarrier(AWMCYNSessionItem* NewCarrier)
{
    if (!HasAuthority() || !NewCarrier || Placement.Holder) return;
    const FTransform Offset = GetActorTransform().GetRelativeTransform(NewCarrier->GetActorTransform());
    Placement.Carrier = NewCarrier;
    Placement.Location = Offset.GetLocation();
    Placement.Rotation = Offset.Rotator();
    ReapplyPlacement();
    ForceNetUpdate();
}

void AWMCYNSessionItem::PredictHold(APawn* Pawn, const bool bRightHand, const FTransform& GripOffset)
{
    AttachToHolder(Pawn, bRightHand, GripOffset.GetLocation(), GripOffset.Rotator());
}

void AWMCYNSessionItem::ReapplyPlacement()
{
    if (Placement.Holder)
    {
        AttachToHolder(Placement.Holder, Placement.bRightHand, Placement.Location, Placement.Rotation);
        return;
    }
    if (Placement.Carrier)
    {
        AttachToActor(Placement.Carrier, FAttachmentTransformRules(
            EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false));
        SetActorRelativeLocation(Placement.Location);
        SetActorRelativeRotation(Placement.Rotation);
        return;
    }
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorLocationAndRotation(Placement.Location, Placement.Rotation);
}

void AWMCYNSessionItem::OnRep_Placement()
{
    ReapplyPlacement();
}

void AWMCYNSessionItem::AttachToHolder(APawn* Pawn, const bool bRightHand, const FVector& Location, const FRotator& Rotation)
{
    const auto* Presence = Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>();
    USceneComponent* Hand = Presence ? Presence->GetTrackedHand(bRightHand) : nullptr;
    AttachToComponent(Hand ? Hand : Pawn->GetRootComponent(), FAttachmentTransformRules(
        EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false));
    SetActorRelativeLocation(Location);
    SetActorRelativeRotation(Rotation);
}
