#include "Misc/AutomationTest.h"
#include "WMCYNLocalVoiceTalker.h"
#include "WMCYNCameraToolComponent.h"
#include "Components/AudioComponent.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNMenuToolsTest,
    "WMCYN.UI.LocalToolBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWMCYNMenuToolsTest::RunTest(const FString& Parameters)
{
    auto* Talker = NewObject<UWMCYNLocalVoiceTalker>();
    auto* Other = NewObject<UWMCYNLocalVoiceTalker>();
    auto* Playback = NewObject<UAudioComponent>();
    Talker->SetListenerVolume(0.25f);
    Talker->OnTalkingBegin(Playback);
    TestEqual(TEXT("Gain applies on new speech"), Playback->VolumeMultiplier, 0.25f);
    Talker->SetListenerVolume(0);
    TestEqual(TEXT("Zero locally mutes active speech"), Playback->VolumeMultiplier, 0.0f);
    TestEqual(TEXT("Another talker unaffected"), Other->GetListenerVolume(), 1.0f);
    Talker->SetListenerVolume(2);
    TestEqual(TEXT("Gain bounded"), Talker->GetListenerVolume(), 1.0f);
    Talker->SetListenerVolume(std::numeric_limits<float>::quiet_NaN());
    TestEqual(TEXT("Reject NaN"), Talker->GetListenerVolume(), 1.0f);
    Talker->OnTalkingEnd();
    Talker->SetListenerVolume(0.5f);
    Talker->OnTalkingBegin(Playback);
    TestEqual(TEXT("Next utterance retains choice"), Playback->VolumeMultiplier, 0.5f);
    Talker->OnTalkingEnd();
    auto* Camera = NewObject<UWMCYNCameraToolComponent>();
    TestFalse(TEXT("Camera defaults off"), Camera->IsCameraEnabled());
    TestFalse(TEXT("No owner cannot capture"), Camera->SetCameraEnabled(true));
    TestFalse(TEXT("Inactive camera cannot enable spectator feed"), Camera->SetSpectatorOutput(true));
    TestNull(TEXT("No eager render target allocation"), Camera->GetPreview());
    return true;
}
#endif
