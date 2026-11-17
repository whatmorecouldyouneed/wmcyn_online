#include "Misc/AutomationTest.h"
#include "WMCYNSeat.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNSeatTest,"WMCYN.Interview.SeatReservation",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FWMCYNSeatTest::RunTest(const FString&)
{
    const auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
        .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    auto* W = UWorld::CreateWorld(EWorldType::Game,false,NAME_None,nullptr,true,ERHIFeatureLevel::Num,&Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);
    auto* Floor = W->SpawnActor<AActor>();
    auto* Box = NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(500,500,10));
    Box->SetCollisionProfileName(TEXT("BlockAll"));
    Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0,0,-10));
    auto* Seat = W->SpawnActor<AWMCYNSeat>(FVector(0,0,45),FRotator::ZeroRotator);
    auto* A = W->SpawnActor<ACharacter>(FVector(90,0,100),FRotator::ZeroRotator);
    auto* B = W->SpawnActor<ACharacter>(FVector(90,85,100),FRotator::ZeroRotator);
    W->SpawnActor<APlayerController>()->Possess(A);
    W->SpawnActor<APlayerController>()->Possess(B);
    const auto Before = A->GetCapsuleComponent()->GetCollisionEnabled();
    TestTrue(TEXT("First occupant accepted"), Seat->TrySit(A));
    TestFalse(TEXT("Occupied seat rejects another player"), Seat->TrySit(B));
    TestFalse(TEXT("Other user cannot release seat"), Seat->TryStand(B));
    TestTrue(TEXT("Stand succeeds on clear floor"),Seat->TryStand(A));
    TestTrue(TEXT("Collision restored"),Before==A->GetCapsuleComponent()->GetCollisionEnabled());
    TestNull(TEXT("Anchor released"),Seat->GetOccupant());
    GEngine->DestroyWorldContext(W);
    W->DestroyWorld(false);
    return true;
}
#endif
