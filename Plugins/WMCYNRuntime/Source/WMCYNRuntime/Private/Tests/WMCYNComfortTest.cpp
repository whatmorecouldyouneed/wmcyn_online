#include "Misc/AutomationTest.h"
#include "InputCoreTypes.h"
#include "WMCYNBackendSubsystem.h"
#include "WMCYNComfort.h"
#include "WMCYNKeyboardInteractionComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNComfortTurnTest, "WMCYN.Comfort.Turn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNComfortTurnTest::RunTest(const FString& Parameters)
{
    const auto TurnedInOneSecond = [](const float FramesPerSecond)
    {
        const float Delta = 1.0f / FramesPerSecond;
        float Ramp = 1.0f;
        float Degrees = 0.0f;
        for (int32 Frame = 0; Frame < FMath::RoundToInt(FramesPerSecond); ++Frame)
        {
            Degrees += WMCYNComfort::TurnDegrees(1.0f, Ramp, Delta);
            Ramp = WMCYNComfort::NextTurnRamp(Ramp, Delta);
        }
        return Degrees;
    };
    TestEqual(TEXT("Same speed at 72 fps"), TurnedInOneSecond(72.0f), WMCYNComfort::MaxTurnDegreesPerSecond, 0.01f);
    TestEqual(TEXT("Same speed at 120 fps"), TurnedInOneSecond(120.0f), WMCYNComfort::MaxTurnDegreesPerSecond, 0.01f);
    TestEqual(TEXT("A turn starts from rest"), WMCYNComfort::TurnDegrees(1.0f, 0.0f, 0.016f), 0.0f);
    TestEqual(TEXT("Left is the mirror of right"), WMCYNComfort::TurnDegrees(-0.5f, 1.0f, 0.1f), -4.5f, 0.001f);
    TestEqual(TEXT("Stick is bounded"), WMCYNComfort::TurnDegrees(9.0f, 1.0f, 0.1f), 9.0f, 0.001f);
    TestEqual(TEXT("Ramp finishes"), WMCYNComfort::NextTurnRamp(0.9f, 1.0f), 1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNComfortCrouchTest, "WMCYN.Comfort.Crouch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNComfortCrouchTest::RunTest(const FString& Parameters)
{
    // A full-deflection turn 11.5 degrees low is what used to lower the body.
    TestEqual(TEXT("A low-angled turn doesn't crouch"), WMCYNComfort::CrouchInput(-0.2f), 0.0f);
    TestEqual(TEXT("Below the threshold is zero"), WMCYNComfort::CrouchInput(-0.69f), 0.0f);
    TestEqual(TEXT("The threshold eases in from zero"), WMCYNComfort::CrouchInput(-0.7f), 0.0f, 0.001f);
    TestEqual(TEXT("Full down still crouches fully"), WMCYNComfort::CrouchInput(-1.0f), -1.0f, 0.001f);
    TestEqual(TEXT("Full up still stands fully"), WMCYNComfort::CrouchInput(1.0f), 1.0f, 0.001f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNComfortRemoteTest, "WMCYN.Comfort.RemoteSmoothing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNComfortRemoteTest::RunTest(const FString& Parameters)
{
    const FVector Start = FVector::ZeroVector;
    const FVector Step(20.0f, 0.0f, 0.0f);
    const FVector Offset = WMCYNComfort::NextRemoteOffset(FVector::ZeroVector, Start, Step, 0.0f);
    TestEqual(TEXT("A network step is hidden at first"), Offset.X, -20.0, 0.001);
    TestTrue(TEXT("The offset decays"), FMath::Abs(WMCYNComfort::NextRemoteOffset(Offset, Step, Step, 0.1f).X) < 5.0);
    TestTrue(TEXT("A teleport snaps"), WMCYNComfort::NextRemoteOffset(Offset, Start, FVector(5000.0f, 0.0f, 0.0f), 0.0f).IsZero());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNKeyboardTest, "WMCYN.Comfort.QuestKeyboard",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNKeyboardTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Delete erases"), WMCYNKeyboard::TextForKey(EKeys::BackSpace), FString(TEXT("\b")));
    TestEqual(TEXT("Space types a space"), WMCYNKeyboard::TextForKey(EKeys::SpaceBar), FString(TEXT(" ")));
    TestTrue(TEXT("Control types nothing"), WMCYNKeyboard::TextForKey(EKeys::LeftControl).IsEmpty());
    TestTrue(TEXT("Shift types nothing"), WMCYNKeyboard::TextForKey(EKeys::LeftShift).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNLoginMessageTest, "WMCYN.Comfort.LoginMessages",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNLoginMessageTest::RunTest(const FString& Parameters)
{
    TestFalse(TEXT("World offline hides the code"), UWMCYNBackendSubsystem::DescribeLoginFailure(TEXT("world_unavailable")).Contains(TEXT("_")));
    TestTrue(TEXT("Old build says to update"), UWMCYNBackendSubsystem::DescribeLoginFailure(TEXT("incompatible_build")).Contains(TEXT("Update")));
    TestTrue(TEXT("An unknown code stays visible"), UWMCYNBackendSubsystem::DescribeLoginFailure(TEXT("new_code")).Contains(TEXT("new_code")));
    return true;
}
#endif
