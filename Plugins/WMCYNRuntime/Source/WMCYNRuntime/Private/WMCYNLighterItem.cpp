#include "WMCYNLighterItem.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AWMCYNLighterItem::AWMCYNLighterItem()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
    Flame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Flame"));
    Flame->SetupAttachment(RootComponent);
    Flame->SetStaticMesh(Plane.Object);
    Flame->SetRelativeLocation(FVector(0, 0, 15));
    // The plane's +Y is the flame texture's tip direction.
    Flame->SetRelativeScale3D(FVector(0.06f, 0.12f, 1.0f));
    Flame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Flame->SetCastShadow(false);
    Flame->SetVisibility(false);
    Spark = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Spark"));
    Spark->SetupAttachment(RootComponent);
    Spark->SetAutoActivate(false);
}

void AWMCYNLighterItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWMCYNLighterItem, State);
}

int8 AWMCYNLighterItem::RollStrike()
{
    // Source: Random.Range(0, 9) < 2 on ints, so 2 strikes in 9 fail.
    return FMath::RandRange(0, 8) < 2 ? -1 : 1;
}

void AWMCYNLighterItem::ServerUse(const bool bDown)
{
    SetState(bDown ? RollStrike() : 0);
}

void AWMCYNLighterItem::HandleTaken()
{
    SetState(0);
}

void AWMCYNLighterItem::HandleReleased()
{
    SetState(0);
}

FVector AWMCYNLighterItem::GetFlameLocation() const
{
    return Flame->GetComponentLocation();
}

void AWMCYNLighterItem::SetState(const int8 NewState)
{
    State = NewState;
    PlayStateEffects();
    ForceNetUpdate();
}

void AWMCYNLighterItem::OnRep_State()
{
    PlayStateEffects();
}

void AWMCYNLighterItem::PlayStateEffects()
{
    if (State == AppliedState && State == 0) return;
    AppliedState = State;
    Flame->SetVisibility(State == 1);
    if (GetNetMode() == NM_DedicatedServer || State == 0) return;
    const auto& Sounds = State == 1 ? LightSounds : FailSounds;
    if (Sounds.Num() > 0)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sounds[FMath::RandHelper(Sounds.Num())], GetFlameLocation());
    }
    Spark->Activate(true);
}

void AWMCYNLighterItem::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (State != 1 || GetNetMode() == NM_DedicatedServer) return;
    const APlayerController* PC = GetWorld()->GetFirstPlayerController();
    const APlayerCameraManager* View = PC ? PC->PlayerCameraManager.Get() : nullptr;
    if (!View) return;
    // Flame is a flat card, so it yaws toward each viewer as the source does.
    FVector ToViewer = View->GetCameraLocation() - Flame->GetComponentLocation();
    ToViewer.Z = 0.0f;
    if (!ToViewer.Normalize()) return;
    const FQuat Target = FRotationMatrix::MakeFromZY(ToViewer, FVector::UpVector).ToQuat();
    Flame->SetWorldRotation(FMath::QInterpTo(Flame->GetComponentQuat(), Target, DeltaSeconds, 8.0f));
}
