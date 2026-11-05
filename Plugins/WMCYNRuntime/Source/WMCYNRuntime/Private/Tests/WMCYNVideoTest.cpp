#include "Misc/AutomationTest.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Components/Slider.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MediaSoundComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "WMCYNVideoChannel.h"
#include "WMCYNVideoPanel.h"
#include "WMCYNVideoResolver.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNVideoPolicyTest, "WMCYN.Video.LinkPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNVideoPolicyTest::RunTest(const FString& Parameters)
{
    using namespace WMCYNVideoPolicy;
    for (const TCHAR* Good : {TEXT("https://example.com/show/stream.m3u8"), TEXT("http://cdn.example.com:8080/a.mp4?t=1"),
        TEXT("https://media.w3.org/2010/05/sintel/trailer.mp4")})
    {
        TestTrue(FString::Printf(TEXT("Allowed: %s"), Good), Validate(Good).IsEmpty());
    }
    for (const TCHAR* Bad : {TEXT(""), TEXT("ftp://example.com/a.mp4"), TEXT("file:///C:/a.mp4"), TEXT("example.com/a.mp4"),
        TEXT("http://localhost/a.mp4"), TEXT("http://127.0.0.1/a.mp4"), TEXT("http://10.0.0.5/a.mp4"),
        TEXT("http://172.16.0.1/a.mp4"), TEXT("http://192.168.1.2:8000/a.mp4"), TEXT("http://169.254.169.254/latest"),
        TEXT("http://100.64.0.1/a.mp4"), TEXT("http://[::1]/a.mp4"), TEXT("http://user:pw@example.com/a.mp4"),
        TEXT("http://example.com@127.0.0.1/a.mp4"), TEXT("http://printer/a.mp4"), TEXT("http://nas.local/a.mp4"),
        TEXT("https://example.com/a b.mp4"), TEXT("https://www.youtube.com/watch?v=a\" --exec \"calc")})
    {
        TestFalse(FString::Printf(TEXT("Refused: %s"), Bad), Validate(Bad).IsEmpty());
    }
    TestFalse(TEXT("Over-long link refused"), Validate(TEXT("https://example.com/") + FString::ChrN(MaxUrlLength, TEXT('a'))).IsEmpty());
    TestTrue(TEXT("YouTube needs a resolver"), NeedsResolver(TEXT("https://www.youtube.com/watch?v=abc")));
    TestTrue(TEXT("Short YouTube needs a resolver"), NeedsResolver(TEXT("https://youtu.be/abc")));
    TestTrue(TEXT("Twitch needs a resolver"), NeedsResolver(TEXT("https://www.twitch.tv/someone")));
    TestFalse(TEXT("Lookalike host is not YouTube"), NeedsResolver(TEXT("https://notyoutube.com/a.mp4")));
    TestTrue(TEXT("YouTube links are accepted"), Validate(TEXT("https://www.youtube.com/watch?v=abc")).IsEmpty());
    TestTrue(TEXT("Twitch links are accepted"), Validate(TEXT("https://www.twitch.tv/someone")).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNVideoResolverTest, "WMCYN.Video.ResolverOutput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNVideoResolverTest::RunTest(const FString& Parameters)
{
    using namespace WMCYNVideoResolver;
    TestEqual(TEXT("Takes the URL line"), ParseDirectUrl(TEXT("https://cdn.example.com/a.m3u8\n")), FString(TEXT("https://cdn.example.com/a.m3u8")));
    TestEqual(TEXT("Skips noise"), ParseDirectUrl(TEXT("WARNING: something\r\nhttps://cdn.example.com/a.mp4\r\n")), FString(TEXT("https://cdn.example.com/a.mp4")));
    TestTrue(TEXT("Errors yield nothing"), ParseDirectUrl(TEXT("ERROR: Video unavailable")).IsEmpty());
    TestTrue(TEXT("Empty yields nothing"), ParseDirectUrl(FString()).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNVideoTimelineTest, "WMCYN.Video.Timeline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNVideoTimelineTest::RunTest(const FString& Parameters)
{
    using namespace WMCYNVideoPolicy;
    TestEqual(TEXT("Late joiner seeks to elapsed time"), SeekSeconds(130.0, 100.0, 600.0), 30.0);
    TestEqual(TEXT("Never seeks before the start"), SeekSeconds(90.0, 100.0, 600.0), 0.0);
    TestTrue(TEXT("Stays inside a finished file"), SeekSeconds(1000.0, 100.0, 600.0) < 600.0);
    TestTrue(TEXT("Files use the timeline"), IsTimelineSeekable(600.0));
    TestFalse(TEXT("Zero duration is live"), IsTimelineSeekable(0.0));
    TestFalse(TEXT("Huge duration is live"), IsTimelineSeekable(FTimespan::MaxValue().GetTotalSeconds()));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNVideoCrashGuardTest, "WMCYN.Video.CrashGuard",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNVideoCrashGuardTest::RunTest(const FString& Parameters)
{
    using namespace WMCYNVideoPolicy;
    const FName Channel(TEXT("AutomationChannel"));
    ClearCrashMarker(Channel);
    TestTrue(TEXT("No marker by default"), ReadCrashMarker(Channel).IsEmpty());
    SetCrashMarker(Channel, TEXT("https://example.com/a.mp4"));
    TestEqual(TEXT("Marker survives"), ReadCrashMarker(Channel), FString(TEXT("https://example.com/a.mp4")));
    ClearCrashMarker(Channel);
    TestTrue(TEXT("Marker clears"), ReadCrashMarker(Channel).IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNVideoChannelTest, "WMCYN.Video.Channel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNVideoChannelTest::RunTest(const FString& Parameters)
{
    const auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
        .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    auto* Living = World->SpawnActor<AWMCYNVideoChannel>();
    Living->ChannelId = TEXT("LivingRoom");
    Living->DefaultUrl = TEXT("http://default.example.com/live.m3u8");
    auto* Stage = World->SpawnActor<AWMCYNVideoChannel>();
    Stage->ChannelId = TEXT("Stage");

    TestTrue(TEXT("Channels replicate"), Living->GetIsReplicated());
    TestEqual(TEXT("Living room starts on its default"), Living->GetEffectiveUrl(), Living->DefaultUrl);
    TestTrue(TEXT("Stage starts blank"), Stage->GetEffectiveUrl().IsEmpty());
    TestFalse(TEXT("Unverified submit refused"), Stage->Submit(TEXT("https://example.com/a.mp4"), false, FString()).IsEmpty());
    TestFalse(TEXT("Bad link refused"), Stage->Submit(TEXT("http://127.0.0.1/a.mp4"), false, TEXT("uid-1")).IsEmpty());
    TestTrue(TEXT("Refused link stays out"), Stage->GetEffectiveUrl().IsEmpty());
    TestTrue(TEXT("Refused link costs no cooldown"), Stage->Submit(TEXT("https://example.com/a.mp4"), false, TEXT("uid-1")).IsEmpty());
    TestEqual(TEXT("Link applied"), Stage->GetEffectiveUrl(), FString(TEXT("https://example.com/a.mp4")));
    TestTrue(TEXT("Stage change leaves the living room alone"), Living->GetEffectiveUrl() == Living->DefaultUrl);
    TestFalse(TEXT("Cooldown spans channels"), Living->Submit(TEXT("https://example.com/b.mp4"), false, TEXT("uid-1")).IsEmpty());
    TestTrue(TEXT("Other users aren't blocked"), Living->Submit(TEXT("https://example.com/b.mp4"), false, TEXT("uid-2")).IsEmpty());
    TestTrue(TEXT("Reset accepted"), Living->Submit(FString(), true, TEXT("uid-3")).IsEmpty());
    TestEqual(TEXT("Reset returns to the default"), Living->GetEffectiveUrl(), Living->DefaultUrl);
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWMCYNVideoVolumeTest, "WMCYN.Video.Volume",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FWMCYNVideoVolumeTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Living room is five times its old level"), WMCYNVideoPolicy::VolumeGain(TEXT("LivingRoom"), 0.5f), 2.5f);
    TestEqual(TEXT("Other screens keep their level"), WMCYNVideoPolicy::VolumeGain(TEXT("Stage"), 0.5f), 0.5f);
    TestEqual(TEXT("Zero mutes"), WMCYNVideoPolicy::VolumeGain(TEXT("LivingRoom"), 0.0f), 0.0f);
    TestEqual(TEXT("Slider range bounded"), WMCYNVideoPolicy::VolumeGain(TEXT("LivingRoom"), 2.0f), 5.0f);

    UClass* PanelWidget = LoadClass<UUserWidget>(nullptr, TEXT("/Game/WMCYN/UI/WBP_WMCYN_VideoPanel.WBP_WMCYN_VideoPanel_C"));
    if (!TestNotNull(TEXT("Panel widget loads"), PanelWidget)) return false;
    const auto Init = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
        .RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Init);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);

    auto* Screen = World->SpawnActor<AActor>();
    auto* Sound = NewObject<UMediaSoundComponent>(Screen);
    Sound->bAutoActivate = false;
    Sound->RegisterComponent();
    auto* Channel = World->SpawnActor<AWMCYNVideoChannel>();
    Channel->ChannelId = TEXT("LivingRoom");
    Channel->SourceScreen = Screen;
    auto* Panel = World->SpawnActor<AWMCYNVideoPanel>();
    Panel->Channel = Channel;
    auto* Host = NewObject<UWidgetComponent>(Panel);
    Host->RegisterComponent();
    Host->SetWidget(CreateWidget<UUserWidget>(World, PanelWidget));
    Panel->Tick(0.0f);

    FString SavedVolume;
    const bool bHadSavedVolume = GConfig->GetString(TEXT("WMCYN.Video"), TEXT("LivingRoom_Volume"), SavedVolume, GGameUserSettingsIni);
    auto* Slider = Cast<USlider>(Host->GetWidget()->GetWidgetFromName(TEXT("SLD_Volume")));
    if (TestNotNull(TEXT("Panel has its slider"), Slider))
    {
        // This world never begins play, and actors drop dynamic delegates until it has.
        World->InitializeActorsForPlay(FURL());
        // The same broadcast a laser drag produces.
        Slider->OnValueChanged.Broadcast(0.2f);
        TestEqual(TEXT("Slider reaches the channel"), Channel->GetLocalVolume(), 0.2f);
        TestEqual(TEXT("Slider reaches the audio"), Sound->GetAudioComponent()->VolumeMultiplier, 1.0f);
        Slider->OnValueChanged.Broadcast(1.0f);
        TestEqual(TEXT("Top of the slider is five times"), Sound->GetAudioComponent()->VolumeMultiplier, 5.0f);
    }
    if (bHadSavedVolume) GConfig->SetString(TEXT("WMCYN.Video"), TEXT("LivingRoom_Volume"), *SavedVolume, GGameUserSettingsIni);
    else GConfig->RemoveKey(TEXT("WMCYN.Video"), TEXT("LivingRoom_Volume"), GGameUserSettingsIni);
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return true;
}
#endif
