#include "WMCYNBowlingBall.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DefaultPawn.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void AWMCYNBowlingBall::BeginPlay()
{
    Super::BeginPlay();
    Home = GetActorTransform();
    if (HasAuthority()) ResetRack();
#if !UE_BUILD_SHIPPING
    if (GetNetMode() == NM_Standalone && HeadPin.Y < 600 &&
        FParse::Param(FCommandLine::Get(),TEXT("WMCYNBowlingSmoke")))
    {
        FTimerHandle H;
        GetWorldTimerManager().SetTimer(H, FTimerDelegate::CreateWeakLambda(this,[this]()
        {
            int Ready = 0;
            for (AStaticMeshActor* P : Pins)
                if (IsValid(P) && P->GetActorUpVector().Z > .97 &&
                    FMath::Abs(P->GetActorLocation().Z - HeadPin.Z) < 10) ++Ready;
            const FVector Start(Home.GetLocation().X-60,HeadPin.Y,HeadPin.Z+3);
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            auto* Tester = GetWorld()->SpawnActor<ADefaultPawn>(Start,Home.Rotator(),Params);
            if (Tester) Tester->SetActorEnableCollision(false);
            bool Taken = Tester && TryTake(Tester,true,FVector::ZeroVector,FRotator::ZeroRotator);
            ThrowVelocity = Home.GetRotation().GetForwardVector()*850;
            bool Dropped = Taken && TryDrop(Tester,Start,Home.Rotator());
            if (Tester) Tester->Destroy();
            UE_LOG(LogTemp,Display,TEXT("BOWLING_SMOKE ready=%d take=%d drop=%d rolling=%d"),Ready,Taken,Dropped,IsValid(RollingBall));
        }),2.f,false);
    }
#endif
}

AStaticMeshActor* AWMCYNBowlingBall::SpawnBody(UStaticMesh* Shape, FVector Position, float Mass)
{
    if (!Shape) return nullptr;
    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* A = GetWorld()->SpawnActor<AStaticMeshActor>(Position,Home.Rotator(),P);
    if (!A) return nullptr;
    auto* C = A->GetStaticMeshComponent();
    C->SetMobility(EComponentMobility::Movable);
    A->SetReplicates(true);
    A->SetReplicateMovement(true);
    C->SetIsReplicated(true);
    C->SetStaticMesh(Shape);
    C->SetCollisionProfileName(TEXT("PhysicsActor"));
    C->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
    C->SetSimulatePhysics(true);
    C->SetMassOverrideInKg(NAME_None,Mass);
    C->BodyInstance.bUseCCD = true;
    return A;
}

void AWMCYNBowlingBall::ResetRack()
{
    if (RollingBall) RollingBall->Destroy();
    RollingBall = nullptr;
    for (AStaticMeshActor* Pin : Pins) if (IsValid(Pin)) Pin->Destroy();
    Pins.Reset();
    for (int Row=0; Row<4; ++Row)
        for (int Col=0; Col<=Row; ++Col)
            Pins.Add(SpawnBody(PinMesh, HeadPin + Home.GetRotation().RotateVector(
                FVector(Row*26.3,(Col-.5*Row)*30.4,0)),1.f));
    SetActorTransform(Home);
    Release();
    SetActorHiddenInGame(false);
    bGrabbable = true;
    RollTime = 0;
    ForceNetUpdate();
}

void AWMCYNBowlingBall::HandleTaken()
{
    LastHand = GetActorLocation();
    ThrowVelocity = FVector::ZeroVector;
}

void AWMCYNBowlingBall::HandleReleased()
{
    if (!bGrabbable || GetActorLocation().Equals(Home.GetLocation(),1.f)) return;
    RollingBall = SpawnBody(Mesh->GetStaticMesh(),GetActorLocation(),6.f);
    if (!RollingBall) return;
    RollingBall->GetStaticMeshComponent()->SetPhysicsLinearVelocity(ThrowVelocity.GetClampedToMaxSize(1250.f));
    bGrabbable = false;
    SetActorHiddenInGame(true);
}

void AWMCYNBowlingBall::Tick(float Seconds)
{
    Super::Tick(Seconds);
    if (!HasAuthority()) return;
    if (GetHolder() && Seconds > SMALL_NUMBER)
    {
        const FVector Now = GetActorLocation();
        ThrowVelocity = FMath::VInterpTo(ThrowVelocity,(Now-LastHand)/Seconds,Seconds,18.f);
        LastHand = Now;
    }
    if (RollingBall && ((RollTime += Seconds) > 10.f || RollingBall->GetActorLocation().Z < Home.GetLocation().Z-250))
    {
        int Down = 0;
        for (AStaticMeshActor* Pin : Pins)
            if (IsValid(Pin) && Pin->GetActorUpVector().Z < .75f) ++Down;
        UE_LOG(LogTemp,Display,TEXT("WMCYN Bowling: %d pins down"),Down);
        ResetRack();
    }
}

void AWMCYNBowlingBall::EndPlay(EEndPlayReason::Type Reason)
{
    if (HasAuthority() && Reason == EEndPlayReason::Destroyed)
    {
        if (RollingBall) RollingBall->Destroy();
        for (AStaticMeshActor* Pin : Pins) if (IsValid(Pin)) Pin->Destroy();
    }
    Super::EndPlay(Reason);
}
