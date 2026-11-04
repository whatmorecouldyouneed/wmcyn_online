#include "WMCYNVideoProbe.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSoundComponent.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNVideoProbe, Log, All);

AWMCYNVideoProbe::AWMCYNVideoProbe()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->bLockToHmd = true;
    Screen = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Screen"));
    Screen->SetupAttachment(RootComponent);
    Screen->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Audio = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("Audio"));
    Audio->SetupAttachment(Screen);
    Audio->bIsUISound = true;
}

void AWMCYNVideoProbe::BeginPlay()
{
    Super::BeginPlay();
    if (IsRunningDedicatedServer() || !FParse::Param(FCommandLine::Get(), TEXT("WMCYNVideoProbe")))
    {
        SetActorTickEnabled(false);
        return;
    }
    Player = NewObject<UMediaPlayer>(this);
    Player->PlayOnOpen = true;
    Player->SetDesiredPlayerName(TEXT("ElectraPlayer"));
    Player->OnMediaOpened.AddDynamic(this, &ThisClass::Opened);
    Player->OnMediaOpenFailed.AddDynamic(this, &ThisClass::Failed);
    Texture = NewObject<UMediaTexture>(this);
    Texture->SetMediaPlayer(Player);
    Texture->UpdateResource();
    Audio->SetMediaPlayer(Player);
    Audio->SetVolumeMultiplier(0.25f);
    if (ScreenMaterial)
    {
        auto* Material = UMaterialInstanceDynamic::Create(ScreenMaterial, this);
        Material->SetTextureParameterValue(TEXT("VideoTexture"), Texture);
        Screen->SetMaterial(0, Material);
    }
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetViewTarget(this);
    UE_LOG(LogWMCYNVideoProbe, Display, TEXT("probe=start isolated=true requested_player=ElectraPlayer"));
    Advance();
}

void AWMCYNVideoProbe::Advance()
{
    Player->Close();
    ++Phase;
    bOpened = false;
    Frames.Reset();
    PhaseStarted = LastReport = FPlatformTime::Seconds();
    // Fixed public test fixtures only; this diagnostic does not accept user URLs.
    const TCHAR* Url = Phase == 1 ? TEXT("https://media.w3.org/2010/05/sintel/trailer.mp4") :
        Phase == 2 ? TEXT("https://bitdash-a.akamaihd.net/content/sintel/hls/playlist.m3u8") : nullptr;
    UE_LOG(LogWMCYNVideoProbe, Display, TEXT("phase=%d state=begin"), Phase);
    if (Url && !Player->OpenUrl(Url)) Failed(FString());
    if (Phase >= 3)
    {
        Audio->SetMediaPlayer(nullptr);
        SetActorTickEnabled(false);
        UE_LOG(LogWMCYNVideoProbe, Display, TEXT("probe=finished acceptance=unverified"));
    }
}

void AWMCYNVideoProbe::Opened(FString IgnoredUrl)
{
    bOpened = true;
    Player->SetLooping(true);
    UE_LOG(LogWMCYNVideoProbe, Display, TEXT("phase=%d state=opened player=%s"), Phase, *Player->GetPlayerName().ToString());
}

void AWMCYNVideoProbe::Failed(FString IgnoredUrl)
{
    UE_LOG(LogWMCYNVideoProbe, Warning, TEXT("phase=%d state=open_failed"), Phase);
}

void AWMCYNVideoProbe::Report()
{
    if (Frames.IsEmpty()) return;
    Frames.Sort();
    const int32 Index = FMath::Clamp(FMath::CeilToInt(Frames.Num() * .95f) - 1, 0, Frames.Num()-1);
    UE_LOG(LogWMCYNVideoProbe, Display,
        TEXT("phase=%d opened=%d playing=%d media_seconds=%.2f width=%d height=%d samples=%d game_frame_p95_ms=%.2f"),
        Phase, bOpened, Player->IsPlaying(), Player->GetTime().GetTotalSeconds(),
        FMath::RoundToInt(Texture->GetSurfaceWidth()), FMath::RoundToInt(Texture->GetSurfaceHeight()), Frames.Num(), Frames[Index]);
    Frames.Reset();
}

void AWMCYNVideoProbe::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Player) return;
    Frames.Add(DeltaSeconds * 1000.f);
    const double Now = FPlatformTime::Seconds();
    if (Now - LastReport >= 5) { Report(); LastReport = Now; }
    if (Now - PhaseStarted >= (Phase == 0 ? 30 : 60)) { Report(); Advance(); }
}

void AWMCYNVideoProbe::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Player)
    {
        Player->OnMediaOpened.RemoveAll(this);
        Player->OnMediaOpenFailed.RemoveAll(this);
        Player->Close();
    }
    Audio->SetMediaPlayer(nullptr);
    Super::EndPlay(Reason);
}
