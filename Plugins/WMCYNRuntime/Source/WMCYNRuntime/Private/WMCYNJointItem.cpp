#include "WMCYNJointItem.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "WMCYNLighterItem.h"
#include "WMCYNSessionFixture.h"

namespace
{
// Material thresholds from CigaretteController's serialized defaults.
constexpr float NormalBurningThreshold = 0.55f;
constexpr float BoostBurningThreshold = 0.384f;
constexpr float IdleThreshold = 1.0f;
constexpr float ServerTickSeconds = 0.1f;
}

AWMCYNJointItem::AWMCYNJointItem()
{
    Mesh->SetRelativeScale3D(JointScale);
    Mesh->SetCastShadow(false);
    Ember = CreateDefaultSubobject<USceneComponent>(TEXT("Ember"));
    Ember->SetupAttachment(RootComponent);
    Ember->SetRelativeLocation(FVector(0, 0, FWMCYNJointBurnState::EmberHeight(0.0f) * JointScale.Z));
    EmberSmoke = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EmberSmoke"));
    EmberSmoke->SetupAttachment(Ember);
    EmberSmoke->SetAutoActivate(false);
    ExhaleSmoke = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExhaleSmoke"));
    ExhaleSmoke->SetupAttachment(RootComponent);
    ExhaleSmoke->SetAutoActivate(false);
}

void AWMCYNJointItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWMCYNJointItem, ReplicatedParam);
    DOREPLIFETIME(AWMCYNJointItem, bReplicatedBurning);
    DOREPLIFETIME(AWMCYNJointItem, bReplicatedBoost);
    DOREPLIFETIME(AWMCYNJointItem, bReplicatedExhaling);
}

void AWMCYNJointItem::BeginPlay()
{
    Super::BeginPlay();
    if (GetNetMode() == NM_DedicatedServer)
    {
        SetActorTickInterval(ServerTickSeconds);
        return;
    }
    Mesh->SetCustomPrimitiveDataFloat(0, VisualParam);
    Mesh->SetCustomPrimitiveDataFloat(1, VisualThreshold);
}

void AWMCYNJointItem::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (HasAuthority() && (Burn.bBurning || Burn.bExhaling))
    {
        const bool bWasBurning = Burn.bBurning;
        const bool bWasExhaling = Burn.bExhaling;
        Burn.Tick(DeltaSeconds);
        PublishBurnState();
        if (bWasBurning != Burn.bBurning || bWasExhaling != Burn.bExhaling) ForceNetUpdate();
    }
    if (GetNetMode() != NM_DedicatedServer) UpdateVisuals(DeltaSeconds);
}

void AWMCYNJointItem::ServerUse(const bool bDown)
{
    if (bDown) Burn.UseDown();
    else Burn.UseUp();
    PublishBurnState();
    ForceNetUpdate();
}

void AWMCYNJointItem::HandleReleased()
{
    // Dropping mid-inhale finishes the breath; the source never leaves boost latched.
    Burn.UseUp();
    PublishBurnState();
}

bool AWMCYNJointItem::ServerApplyContact(AWMCYNSessionItem* Other)
{
    if (!HasAuthority() || !Other || Other == this || !IsInServerReach(Other)) return false;
    if (const auto* Lighter = Cast<AWMCYNLighterItem>(Other))
    {
        if (!Lighter->IsLit()) return false;
        Burn.StartBurn();
    }
    else if (const auto* Joint = Cast<AWMCYNJointItem>(Other))
    {
        if (!Joint->IsBurning()) return false;
        Burn.StartBurn();
    }
    else if (const auto* Fixture = Cast<AWMCYNSessionFixture>(Other))
    {
        if (Fixture->GetKind() == EWMCYNFixtureKind::Ashtray) Burn.StopBurn();
        else if (Fixture->GetKind() == EWMCYNFixtureKind::Papers) Burn.Reset();
        else return false;
    }
    else
    {
        return false;
    }
    PublishBurnState();
    ForceNetUpdate();
    return true;
}

FVector AWMCYNJointItem::GetEmberLocation() const
{
    return Ember->GetComponentLocation();
}

void AWMCYNJointItem::PublishBurnState()
{
    ReplicatedParam = Burn.Param;
    bReplicatedBurning = Burn.bBurning;
    bReplicatedBoost = Burn.bBoost;
    bReplicatedExhaling = Burn.bExhaling;
    OnRep_BurnFlags();
}

void AWMCYNJointItem::OnRep_BurnFlags()
{
    if (GetNetMode() != NM_DedicatedServer) ApplySmoke();
}

void AWMCYNJointItem::ApplySmoke()
{
    if (bSmokeActive != bReplicatedBurning)
    {
        bSmokeActive = bReplicatedBurning;
        if (bSmokeActive) EmberSmoke->Activate(true);
        else EmberSmoke->Deactivate();
    }
    const APawn* Holder = GetHolder();
    const UCameraComponent* Head = Holder ? Holder->FindComponentByClass<UCameraComponent>() : nullptr;
    const bool bExhale = bReplicatedExhaling && Head;
    if (bExhaleActive == bExhale) return;
    bExhaleActive = bExhale;
    if (bExhale)
    {
        // Source starts the breath 6 cm below the holder's head, moving forward from the mouth.
        ExhaleSmoke->AttachToComponent(const_cast<UCameraComponent*>(Head), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        ExhaleSmoke->SetRelativeLocation(FVector(8, 0, -6));
        ExhaleSmoke->Activate(true);
    }
    else
    {
        ExhaleSmoke->Deactivate();
    }
}

void AWMCYNJointItem::UpdateVisuals(const float DeltaSeconds)
{
    const float PreviousParam = VisualParam;
    const float PreviousThreshold = VisualThreshold;
    VisualParam = FMath::FInterpTo(VisualParam, ReplicatedParam, DeltaSeconds, 20.0f);
    const float ThresholdTarget = bReplicatedBurning
        ? (bReplicatedBoost ? BoostBurningThreshold : NormalBurningThreshold) : IdleThreshold;
    VisualThreshold = FMath::FInterpTo(VisualThreshold, ThresholdTarget, DeltaSeconds, ReplicatedParam < 0.05f ? 1000.0f : 10.0f);
    if (VisualParam != PreviousParam)
    {
        Mesh->SetCustomPrimitiveDataFloat(0, VisualParam);
        Ember->SetRelativeLocation(FVector(0, 0, FWMCYNJointBurnState::EmberHeight(VisualParam) * JointScale.Z));
    }
    if (VisualThreshold != PreviousThreshold) Mesh->SetCustomPrimitiveDataFloat(1, VisualThreshold);
}
