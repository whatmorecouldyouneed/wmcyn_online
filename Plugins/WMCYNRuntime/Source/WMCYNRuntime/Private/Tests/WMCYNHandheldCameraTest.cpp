#include "Misc/AutomationTest.h"
#include "WMCYNCameraToolComponent.h"
#include "WMCYNInterviewCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNHandheldCameraTest, "WMCYN.UI.HandheldCamera",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNHandheldCameraTest::RunTest(const FString& Parameters)
{
    IConsoleVariable* Resolution = IConsoleManager::Get().FindConsoleVariable(TEXT("wmcyn.Camera.CaptureHeight"));
    const int32 PreviousHeight = Resolution->GetInt();
    Resolution->Set(360, ECVF_SetByCode);
    ON_SCOPE_EXIT { Resolution->Set(PreviousHeight, ECVF_SetByCode); };
    const auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
        .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
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
        Resolution->Set(1080, ECVF_SetByCode);
        Tool->UpdateCaptureResolution();
        TestEqual(TEXT("Recording target width"), Tool->GetPreview()->SizeX, 1920);
        TestEqual(TEXT("Recording target height"), Tool->GetPreview()->SizeY, 1080);
        TestTrue(TEXT("Resolution reuses shared target"), Target == Tool->GetPreview());
        Resolution->Set(720, ECVF_SetByCode);
        Tool->UpdateCaptureResolution();
        TestEqual(TEXT("Intermediate target width"), Tool->GetPreview()->SizeX, 1280);
        Resolution->Set(-1, ECVF_SetByCode);
        Tool->UpdateCaptureResolution();
        TestEqual(TEXT("Invalid resolution falls back safely"), Tool->GetPreview()->SizeY, 360);
        auto* Menu = NewObject<UWidgetComponent>(Pawn);
        Pawn->AddInstanceComponent(Menu);
        Menu->RegisterComponent();
        auto* Nameplate = NewObject<UWidgetComponent>(Pawn);
        Pawn->AddInstanceComponent(Nameplate);
        Nameplate->ComponentTags.Add(TEXT("WMCYNCameraVisible"));
        Nameplate->RegisterComponent();
        Tool->RefreshCaptureExclusions();
        TestTrue(TEXT("Menu excluded from program"), Tool->Capture->HiddenComponents.Contains(Menu));
        TestFalse(TEXT("Nameplate retained in program"), Tool->Capture->HiddenComponents.Contains(Nameplate));
        TestFalse(TEXT("Unsaved shot cannot be selected"), Tool->SelectShot(0));
        TestFalse(TEXT("Invalid shot cannot be saved"), Tool->SaveShot(3));
        const FTransform SavedPose = Prop->GetActorTransform();
        TestTrue(TEXT("Save master"), Tool->SaveShot(0));
        Prop->AddActorWorldOffset(FVector(100, 200, 0));
        Tool->SetFieldOfView(40);
        TestTrue(TEXT("Save interviewer"), Tool->SaveShot(1));
        Prop->AddActorWorldOffset(FVector(-200, 0, 0));
        Tool->SetFieldOfView(55);
        TestTrue(TEXT("Save guest"), Tool->SaveShot(2));
        TestTrue(TEXT("Select interviewer"), Tool->SelectShot(1));
        TestEqual(TEXT("Interviewer FOV restored"), Tool->GetFieldOfView(), 40.f);
        TestTrue(TEXT("Select guest"), Tool->SelectShot(2));
        TestEqual(TEXT("Guest FOV restored"), Tool->GetFieldOfView(), 55.f);
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
        PC->UnPossess();
        TestTrue(TEXT("Unpossessed camera can shut down"), Tool->SetCameraEnabled(false));
        TestFalse(TEXT("Ownership loss stops capture"), Tool->Capture->bCaptureEveryFrame);
        TestTrue(TEXT("Ownership loss hides camera"), Prop->IsHidden());
        Tool->EndPlay(EEndPlayReason::Destroyed);
        TestNull(TEXT("Cleanup removes prop reference"),Tool->GetCameraProp());
        TestNull(TEXT("Cleanup removes target reference"),Tool->GetPreview());
    }
    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}
#endif
