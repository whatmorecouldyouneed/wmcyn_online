#include "Misc/AutomationTest.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "WMCYNJointBurnState.h"
#include "WMCYNJointItem.h"
#include "WMCYNLighterItem.h"
#include "WMCYNSessionFixture.h"
#include "WMCYNItemInteractionComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace
{
int32 SecondsUntilFinished(FWMCYNJointBurnState& Burn)
{
    int32 Seconds = 0;
    while (Burn.bBurning && Seconds < 1000)
    {
        for (int32 Step = 0; Step < 10; ++Step) Burn.Tick(0.1f);
        ++Seconds;
    }
    return Seconds;
}

UWorld* MakeWorld()
{
    const auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
        .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    return World;
}

// Bare APawn has no root, so it would report the origin wherever it was spawned.
APawn* MakePawn(UWorld* World, const FVector& Location)
{
    APawn* Pawn = World->SpawnActor<APawn>();
    auto* Root = NewObject<USceneComponent>(Pawn);
    Pawn->SetRootComponent(Root);
    Root->RegisterComponent();
    Pawn->SetActorLocation(Location);
    return Pawn;
}

void DestroyWorld(UWorld* World)
{
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNTraySessionBurnTest, "WMCYN.Items.Burn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNTraySessionBurnTest::RunTest(const FString& Parameters)
{
    FWMCYNJointBurnState Idle;
    Idle.StartBurn();
    const int32 IdleSeconds = SecondsUntilFinished(Idle);
    TestTrue(TEXT("Idle burn takes 200 s"), FMath::Abs(IdleSeconds - 200) <= 1);
    TestFalse(TEXT("Finished stops burning"), Idle.bBurning);
    TestEqual(TEXT("Finished reaches one"), Idle.Param, 1.0f);

    FWMCYNJointBurnState Inhaled;
    Inhaled.StartBurn();
    Inhaled.UseDown();
    TestTrue(TEXT("Inhale boosts"), Inhaled.bBoost);
    // Boost stops at .8, so 80 s boosted, then 40 s at the idle rate.
    const int32 BoostSeconds = SecondsUntilFinished(Inhaled);
    TestTrue(TEXT("Held inhale takes 120 s"), FMath::Abs(BoostSeconds - 120) <= 1);
    TestFalse(TEXT("Boost is blocked past .8"), Inhaled.bBoost);

    FWMCYNJointBurnState Cold;
    Cold.UseDown();
    Cold.UseUp();
    TestFalse(TEXT("Unlit inhale has no exhale"), Cold.bExhaling);

    FWMCYNJointBurnState Breath;
    Breath.StartBurn();
    Breath.UseDown();
    Breath.UseUp();
    TestTrue(TEXT("Burning inhale exhales"), Breath.bExhaling);
    TestFalse(TEXT("Release ends boost"), Breath.bBoost);
    for (int32 Step = 0; Step < 14; ++Step) Breath.Tick(0.1f);
    TestTrue(TEXT("Exhale lasts past 1.4 s"), Breath.bExhaling);
    Breath.Tick(0.2f);
    TestFalse(TEXT("Exhale ends after 1.5 s"), Breath.bExhaling);

    FWMCYNJointBurnState Relight;
    Relight.StartBurn();
    for (int32 Step = 0; Step < 100; ++Step) Relight.Tick(0.1f);
    const float Burned = Relight.Param;
    Relight.StopBurn();
    Relight.Tick(5.0f);
    TestEqual(TEXT("Ashing keeps progress"), Relight.Param, Burned);
    Relight.StartBurn();
    TestTrue(TEXT("Relights"), Relight.bBurning);
    Relight.Reset();
    TestEqual(TEXT("Re-roll resets"), Relight.Param, 0.0f);
    TestFalse(TEXT("Re-roll stops burning"), Relight.bBurning);

    TestEqual(TEXT("Fresh ember height"), FWMCYNJointBurnState::EmberHeight(0.0f), 8.21f);
    TestEqual(TEXT("Finished ember height"), FWMCYNJointBurnState::EmberHeight(1.0f), 3.46f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNTraySessionLighterTest, "WMCYN.Items.LighterStrike",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNTraySessionLighterTest::RunTest(const FString& Parameters)
{
    constexpr int32 Strikes = 90000;
    int32 Failures = 0;
    for (int32 Index = 0; Index < Strikes; ++Index) Failures += AWMCYNLighterItem::RollStrike() == -1;
    const double Rate = double(Failures) / Strikes;
    TestTrue(TEXT("Failure rate is 2 in 9"), FMath::Abs(Rate - 2.0 / 9.0) < 0.01);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNTraySessionAuthorityTest, "WMCYN.Items.ServerAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNTraySessionAuthorityTest::RunTest(const FString& Parameters)
{
    UWorld* World = MakeWorld();
    APawn* First = MakePawn(World, FVector::ZeroVector);
    APawn* Second = MakePawn(World, FVector::ZeroVector);
    APawn* Far = MakePawn(World, FVector(5000, 0, 0));
    auto* Joint = World->SpawnActor<AWMCYNJointItem>();
    auto* OtherJoint = World->SpawnActor<AWMCYNJointItem>();
    auto* Lighter = World->SpawnActor<AWMCYNLighterItem>();
    auto* Fixture = World->SpawnActor<AWMCYNSessionFixture>();
    auto* DistantLighter = World->SpawnActor<AWMCYNLighterItem>(FVector(5000, 0, 0), FRotator::ZeroRotator);

    TestTrue(TEXT("Items replicate"), Joint->GetIsReplicated());
    TestFalse(TEXT("Out-of-reach take refused"), Joint->TryTake(Far, true, FVector::ZeroVector, FRotator::ZeroRotator));
    TestTrue(TEXT("First holder takes"), Joint->TryTake(First, true, FVector::ZeroVector, FRotator::ZeroRotator));
    TestFalse(TEXT("One holder per item"), Joint->TryTake(Second, true, FVector::ZeroVector, FRotator::ZeroRotator));
    TestFalse(TEXT("Non-holder can't drop"), Joint->TryDrop(Second, FVector(10, 0, 0), FRotator::ZeroRotator));
    TestFalse(TEXT("Decor can't be taken"), Fixture->TryTake(First, true, FVector::ZeroVector, FRotator::ZeroRotator));

    TestFalse(TEXT("Unlit lighter can't light"), Joint->ServerApplyContact(Lighter));
    Lighter->TryTake(Second, false, FVector::ZeroVector, FRotator::ZeroRotator);
    for (int32 Attempt = 0; Attempt < 64 && !Lighter->IsLit(); ++Attempt) Lighter->ServerUse(true);
    TestTrue(TEXT("Lighter lights"), Lighter->IsLit());
    TestTrue(TEXT("Lit lighter lights joint"), Joint->ServerApplyContact(Lighter));
    TestTrue(TEXT("Joint is burning"), Joint->IsBurning());
    TestTrue(TEXT("Burning joint passes the light"), OtherJoint->ServerApplyContact(Joint));
    TestFalse(TEXT("Joint can't touch itself"), Joint->ServerApplyContact(Joint));
    TestFalse(TEXT("Decor does nothing"), OtherJoint->ServerApplyContact(Fixture));

    DistantLighter->ServerUse(true);
    for (int32 Attempt = 0; Attempt < 64 && !DistantLighter->IsLit(); ++Attempt) DistantLighter->ServerUse(true);
    TestFalse(TEXT("Out-of-range contact refused"), OtherJoint->ServerApplyContact(DistantLighter));

    Lighter->TryDrop(Second, FVector::ZeroVector, FRotator::ZeroRotator);
    TestFalse(TEXT("Dropping puts the lighter out"), Lighter->IsLit());
    TestTrue(TEXT("Holder drops"), Joint->TryDrop(First, FVector(30, 40, 50), FRotator::ZeroRotator));
    TestEqual(TEXT("Stays where released"), Joint->GetActorLocation(), FVector(30, 40, 50));
    TestNull(TEXT("Released item has no holder"), Joint->GetHolder());
    DestroyWorld(World);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNTraySessionCarryTest, "WMCYN.Items.TrayCarry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNTraySessionCarryTest::RunTest(const FString& Parameters)
{
    UWorld* World = MakeWorld();
    APawn* Pawn = MakePawn(World, FVector::ZeroVector);
    APawn* Other = MakePawn(World, FVector::ZeroVector);
    auto* Tray = World->SpawnActor<AWMCYNSessionFixture>();
    Tray->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(
        LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Tray->SetCarriesItems(true);
    auto* Riding = World->SpawnActor<AWMCYNJointItem>(FVector(10, 0, 40), FRotator::ZeroRotator);
    auto* Beside = World->SpawnActor<AWMCYNJointItem>(FVector(300, 0, 0), FRotator::ZeroRotator);

    TestTrue(TEXT("Tray can be taken"), Tray->TryTake(Pawn, true, FVector::ZeroVector, FRotator::ZeroRotator));
    TestTrue(TEXT("Item on the tray rides along"), Riding->GetCarrier() == Tray);
    TestNull(TEXT("Item beside the tray stays"), Beside->GetCarrier());
    TestTrue(TEXT("Tray drops"), Tray->TryDrop(Pawn, FVector(50, 60, 70), FRotator::ZeroRotator));
    TestEqual(TEXT("Carried item moved with the tray"), Riding->GetActorLocation(), FVector(60, 60, 110));
    TestTrue(TEXT("Still carried at rest"), Riding->GetCarrier() == Tray);
    TestTrue(TEXT("Item can be taken off the tray"), Riding->TryTake(Other, false, FVector::ZeroVector, FRotator::ZeroRotator));
    TestNull(TEXT("Taking it clears the carrier"), Riding->GetCarrier());
    TestTrue(TEXT("Item drops"), Riding->TryDrop(Other, FVector(200, 0, 0), FRotator::ZeroRotator));
    TestEqual(TEXT("Dropped item is free of the tray"), Riding->GetActorLocation(), FVector(200, 0, 0));
    DestroyWorld(World);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNTraySessionGripTest, "WMCYN.Items.Grip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNTraySessionGripTest::RunTest(const FString& Parameters)
{
    UWorld* World = MakeWorld();
    APawn* Pawn = MakePawn(World, FVector(10, 0, 100));
    auto* Tool = NewObject<UWMCYNItemInteractionComponent>(Pawn);
    Pawn->AddInstanceComponent(Tool);
    Tool->RegisterComponent();
    auto* Joint = World->SpawnActor<AWMCYNJointItem>(FVector(10, 0, 100), FRotator::ZeroRotator);
    Tool->RefreshItems();
    auto& Hand = Tool->Right;
    const FTransform Near(FQuat::Identity, Joint->GetGripLocation());
    const FTransform Away(FQuat::Identity, Joint->GetGripLocation() + FVector(200, 0, 0));

    Tool->UpdateHand(Hand, Near, true, true, false, false, 0.016f);
    TestFalse(TEXT("Grip already down doesn't take"), Hand.Item.IsValid());
    Tool->UpdateHand(Hand, Near, true, false, false, false, 0.016f);
    Tool->UpdateHand(Hand, Near, true, true, false, true, 0.016f);
    TestFalse(TEXT("Yielded grip stays with door or camera"), Hand.Item.IsValid());
    Tool->UpdateHand(Hand, Near, true, false, false, false, 0.016f);
    Tool->UpdateHand(Hand, Away, true, true, false, false, 0.016f);
    TestFalse(TEXT("Out of reach grip takes nothing"), Hand.Item.IsValid());
    Tool->UpdateHand(Hand, Near, true, false, false, false, 0.016f);
    Tool->UpdateHand(Hand, Near, true, true, false, false, 0.016f);
    TestTrue(TEXT("Fresh nearby tap takes"), Joint->GetHolder() == Pawn);
    Tool->UpdateHand(Hand, Near, true, false, false, false, 0.016f);
    TestTrue(TEXT("Releasing grip keeps the item"), Joint->GetHolder() == Pawn);
    Tool->UpdateHand(Hand, Near, true, true, false, false, 0.016f);
    TestNull(TEXT("Second tap drops"), Joint->GetHolder());
    Tool->UpdateHand(Hand, Near, true, false, false, false, 0.016f);
    Tool->UpdateHand(Hand, Near, true, true, false, false, 0.016f);
    TestTrue(TEXT("Can take again"), Joint->GetHolder() == Pawn);
    Tool->UpdateHand(Hand, Near, false, true, false, false, 0.016f);
    TestNull(TEXT("Tracking loss drops"), Joint->GetHolder());
    Tool->UpdateHand(Hand, Near, true, true, false, false, 0.016f);
    TestNull(TEXT("Tracking return needs a fresh press"), Joint->GetHolder());
    DestroyWorld(World);
    return true;
}
#endif
