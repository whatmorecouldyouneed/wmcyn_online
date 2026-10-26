#include "Misc/AutomationTest.h"
#include "WMCYNCameraToolComponent.h"
#include "WMCYNInterviewCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNHandheldCameraTest, "WMCYN.UI.HandheldCamera",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNHandheldCameraTest::RunTest(const FString& Parameters)
{
    const auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
        .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Init);
    APawn* Pawn = World->SpawnActor<APawn>();
    auto* Head = NewObject<UCameraComponent>(Pawn);
    Pawn->SetRootComponent(Head);
    Pawn->AddInstanceComponent(Head);
    Head->RegisterComponent();
    Head->SetWorldLocation(FVector(0,0,170));
    auto* PC = World->SpawnActor<APlayerController>();
    PC->SetAsLocalPlayerController();
    PC->Possess(Pawn);
    TestTrue(TEXT("Harness has locally possessed pawn"), Pawn->IsLocallyControlled());
    auto* Tool = NewObject<UWMCYNCameraToolComponent>(Pawn);
    Pawn->AddInstanceComponent(Tool);
    Tool->RegisterComponent();
    // This synthetic world does not dispatch BeginPlay automatically.
    Tool->BeginPlay();
    if (TestTrue(TEXT("Local camera enables"), Tool->SetCameraEnabled(true)))
    {
        auto* Prop = Tool->GetCameraProp();
        TestFalse(TEXT("Local prop not replicated"), Prop->GetIsReplicated());
        TestEqual(TEXT("Single preview width"), Tool->GetPreview()->SizeX, 640);
        TestEqual(TEXT("Single preview height"), Tool->GetPreview()->SizeY, 360);
        TestTrue(TEXT("Lens owns capture pose"), Tool->Capture->GetAttachParent() == Prop->GetLens());
        FTransform Hand(FQuat::Identity, Prop->GetGripLocation());
        Tool->UpdateGrip(Hand,true,true,false);
        TestFalse(TEXT("Opening with grip down does not auto-grab"),Tool->IsHeld());
        Tool->UpdateGrip(Hand,true,false,false);
        Tool->UpdateGrip(Hand,true,true,false);
        TestTrue(TEXT("Fresh nearby grip picks up"), Tool->IsHeld());
        Hand.AddToTranslation(FVector(30,20,10));
        Tool->UpdateGrip(Hand,true,true,false);
        const FTransform Shot = Prop->GetActorTransform();
        Tool->UpdateGrip(Hand,true,false,false);
        Hand.AddToTranslation(FVector(100,0,0));
        Tool->UpdateGrip(Hand,true,false,false);
        TestTrue(TEXT("Release freezes world pose"), Shot.Equals(Prop->GetActorTransform()));
        Hand.SetLocation(Prop->GetGripLocation());
        Tool->UpdateGrip(Hand,true,true,true);
        TestFalse(TEXT("Door priority prevents camera grab"),Tool->IsHeld());
        Tool->UpdateGrip(Hand,true,false,false);
        Tool->UpdateGrip(Hand,true,true,false);
        Tool->UpdateGrip(Hand,false,true,false);
        TestFalse(TEXT("Tracking loss releases"),Tool->IsHeld());
        Tool->UpdateGrip(Hand,true,true,false);
        TestFalse(TEXT("Tracking regain requires new press"),Tool->IsHeld());
        Tool->ResetCameraPose();
        TestTrue(TEXT("Recall within reach"), FVector::Dist(Prop->GetActorLocation(), Head->GetComponentLocation()) < 60);
        Tool->SetFieldOfView(200);
        TestEqual(TEXT("FOV bounded"),Tool->Capture->FOVAngle,100.f);
        const auto* Target = Tool->GetPreview();
        TestFalse(TEXT("Unsaved shot cannot be selected"), Tool->SelectShot(0));
        TestFalse(TEXT("Invalid shot cannot be saved"), Tool->SaveShot(3));
        const FTransform SavedPose = Prop->GetActorTransform();
        TestTrue(TEXT("Save master"), Tool->SaveShot(0));
        Prop->AddActorWorldOffset(FVector(100, 200, 0));
        Tool->SetFieldOfView(40);
        TestTrue(TEXT("Save interviewer"), Tool->SaveShot(1));
        TestTrue(TEXT("Select master"), Tool->SelectShot(0));
        TestTrue(TEXT("Shot restores world pose"), SavedPose.Equals(Prop->GetActorTransform()));
        TestEqual(TEXT("Shot restores FOV"), Tool->Capture->FOVAngle, 100.f);
        TestTrue(TEXT("Shot reuses capture target"), Target == Tool->GetPreview());
        Tool->SetCameraEnabled(false);
        TestFalse(TEXT("Disabled camera does not reactivate on shot selection"), Tool->SelectShot(0));
        TestFalse(TEXT("Off stops capture"),Tool->Capture->bCaptureEveryFrame);
        TestFalse(TEXT("Off stops tool tick"),Tool->IsComponentTickEnabled());
        TestTrue(TEXT("Off hides prop"),Prop->IsHidden());
        Tool->SetCameraEnabled(true);
        TestTrue(TEXT("Toggle reuses target"),Target == Tool->GetPreview());
        Tool->EndPlay(EEndPlayReason::Destroyed);
        TestNull(TEXT("Cleanup removes prop reference"),Tool->GetCameraProp());
        TestNull(TEXT("Cleanup removes target reference"),Tool->GetPreview());
    }
    World->DestroyWorld(false);
    return true;
}
#endif
