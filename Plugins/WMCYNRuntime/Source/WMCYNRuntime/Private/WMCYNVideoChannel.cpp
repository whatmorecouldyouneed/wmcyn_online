#include "WMCYNVideoChannel.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#include "Misc/ConfigCacheIni.h"
#include "Net/UnrealNetwork.h"
#include "UObject/UnrealType.h"
#include "WMCYNVideoResolver.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNVideo, Log, All);

namespace
{
const TCHAR* const VideoConfigSection = TEXT("WMCYN.Video");
constexpr double OpenTimeoutSeconds = 20.0;
constexpr double CrashMarkerSeconds = 30.0;
constexpr double DriftToleranceSeconds = 3.0;
constexpr double MaxSeekableHours = 24.0;
constexpr double QuestDecodeRange = 1500.0;
const TCHAR* const LivingRoomChannel = TEXT("LivingRoom");
constexpr float LivingRoomGain = 5.0f;
// The placed sound fell silent at 3.3 m, short of its own panel at 3.65 m. The stage is 45 m away.
constexpr float LivingRoomFalloff = 800.0f;

FString ExtractHost(const FString& Url)
{
    int32 SchemeEnd = Url.Find(TEXT("://"));
    if (SchemeEnd == INDEX_NONE) return FString();
    FString Authority = Url.Mid(SchemeEnd + 3);
    int32 End = INDEX_NONE;
    for (const TCHAR Terminator : {TEXT('/'), TEXT('?'), TEXT('#')})
    {
        int32 Found;
        if (Authority.FindChar(Terminator, Found) && (End == INDEX_NONE || Found < End)) End = Found;
    }
    if (End != INDEX_NONE) Authority.LeftInline(End);
    int32 Colon;
    if (Authority.FindChar(TEXT(':'), Colon)) Authority.LeftInline(Colon);
    return Authority.ToLower();
}

bool IsHostOrSubdomain(const FString& Host, const TCHAR* Domain)
{
    return Host == Domain || Host.EndsWith(FString(TEXT(".")) + Domain);
}

bool IsPrivateHost(const FString& Host)
{
    if (Host == TEXT("localhost") || Host.EndsWith(TEXT(".localhost")) || Host.EndsWith(TEXT(".local")) ||
        Host.EndsWith(TEXT(".internal")) || !Host.Contains(TEXT("."))) return true;
    TArray<FString> Parts;
    Host.ParseIntoArray(Parts, TEXT("."));
    if (Parts.Num() != 4) return false;
    for (const FString& Part : Parts) if (!Part.IsNumeric()) return false;
    const int32 A = FCString::Atoi(*Parts[0]);
    const int32 B = FCString::Atoi(*Parts[1]);
    return A == 0 || A == 10 || A == 127 || (A == 100 && B >= 64 && B <= 127) || (A == 169 && B == 254) ||
        (A == 172 && B >= 16 && B <= 31) || (A == 192 && B == 168) || A >= 224;
}
}

bool WMCYNVideoPolicy::NeedsResolver(const FString& Url)
{
    const FString Host = ExtractHost(Url);
    for (const TCHAR* Domain : {TEXT("youtube.com"), TEXT("youtu.be"), TEXT("twitch.tv")})
        if (IsHostOrSubdomain(Host, Domain)) return true;
    return false;
}

FString WMCYNVideoPolicy::Validate(const FString& Url)
{
    if (Url.IsEmpty()) return TEXT("Enter a link");
    if (Url.Len() > MaxUrlLength) return TEXT("Link too long");
    if (!Url.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase) && !Url.StartsWith(TEXT("http://"), ESearchCase::IgnoreCase))
        return TEXT("Link must start with http or https");
    for (const TCHAR Character : Url)
    {
        // Quotes matter too: resolver links travel as a process argument on PCVR.
        if (Character <= TEXT(' ') || Character == 127 || Character == TEXT('"') || Character == TEXT('\\')) return TEXT("Link has invalid characters");
    }
    const FString Authority = Url.Mid(Url.Find(TEXT("://")) + 3);
    int32 Slash = INDEX_NONE;
    Authority.FindChar(TEXT('/'), Slash);
    // Credentials and IPv6 literals can disguise a local destination.
    const FString Head = Slash == INDEX_NONE ? Authority : Authority.Left(Slash);
    if (Head.Contains(TEXT("@")) || Head.Contains(TEXT("["))) return TEXT("Link address not allowed");
    const FString Host = ExtractHost(Url);
    if (Host.IsEmpty() || IsPrivateHost(Host)) return TEXT("Link address not allowed");
    return FString();
}

double WMCYNVideoPolicy::SeekSeconds(const double Now, const double Started, const double Duration)
{
    return FMath::Clamp(Now - Started, 0.0, FMath::Max(0.0, Duration - 0.1));
}

bool WMCYNVideoPolicy::IsTimelineSeekable(const double DurationSeconds)
{
    return DurationSeconds > 0.0 && DurationSeconds < MaxSeekableHours * 3600.0;
}

void WMCYNVideoPolicy::SetCrashMarker(const FName ChannelId, const FString& Url)
{
    GConfig->SetString(VideoConfigSection, *(ChannelId.ToString() + TEXT("_Opening")), *Url, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void WMCYNVideoPolicy::ClearCrashMarker(const FName ChannelId)
{
    GConfig->RemoveKey(VideoConfigSection, *(ChannelId.ToString() + TEXT("_Opening")), GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

FString WMCYNVideoPolicy::ReadCrashMarker(const FName ChannelId)
{
    FString Url;
    GConfig->GetString(VideoConfigSection, *(ChannelId.ToString() + TEXT("_Opening")), Url, GGameUserSettingsIni);
    return Url;
}

AWMCYNVideoChannel::AWMCYNVideoChannel()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AWMCYNVideoChannel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWMCYNVideoChannel, State);
}

float WMCYNVideoPolicy::VolumeGain(const FName ChannelId, const float SliderValue)
{
    return FMath::Clamp(SliderValue, 0.0f, 1.0f) * (ChannelId == LivingRoomChannel ? LivingRoomGain : 1.0f);
}

bool AWMCYNVideoChannel::IsKnownChannel() const
{
    return ChannelId == TEXT("LivingRoom") || ChannelId == TEXT("Stage") || ChannelId == TEXT("Basement");
}

double AWMCYNVideoChannel::ServerTime() const
{
    const AGameStateBase* GameState = GetWorld()->GetGameState();
    return GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

FString AWMCYNVideoChannel::Submit(const FString& Url, const bool bReset, const FString& VerifiedUid)
{
    if (!HasAuthority() || VerifiedUid.IsEmpty()) return TEXT("Sign in required");
    const double Now = ServerTime();
    for (TActorIterator<AWMCYNVideoChannel> It(GetWorld()); It; ++It)
    {
        const double* Last = It->LastSubmissions.Find(VerifiedUid);
        if (Last && Now - *Last < WMCYNVideoPolicy::SubmitCooldownSeconds) return TEXT("Wait a few seconds");
    }
    if (!bReset)
    {
        const FString Error = WMCYNVideoPolicy::Validate(Url);
        if (!Error.IsEmpty()) return Error;
    }
    LastSubmissions.Add(VerifiedUid, Now);
    State.Url = bReset ? FString() : Url;
    ++State.Revision;
    State.Started = Now;
    // Opaque audit identifier: never a client-supplied name or the raw account id.
    State.SetBy = FString::Printf(TEXT("%08x"), GetTypeHash(VerifiedUid));
    ForceNetUpdate();
    OnRep_State();
    UE_LOG(LogWMCYNVideo, Display, TEXT("channel=%s revision=%d account=%s action=%s"),
        *ChannelId.ToString(), State.Revision, *State.SetBy, bReset ? TEXT("reset") : TEXT("submit"));
    return FString();
}

void AWMCYNVideoChannel::BeginPlay()
{
    Super::BeginPlay();
    if (GetNetMode() == NM_DedicatedServer) return;
    // A marker that survived the last run means this client died while opening that link.
    CrashedUrl = WMCYNVideoPolicy::ReadCrashMarker(ChannelId);
    if (!CrashedUrl.IsEmpty())
    {
        WMCYNVideoPolicy::ClearCrashMarker(ChannelId);
        UE_LOG(LogWMCYNVideo, Warning, TEXT("channel=%s state=skipping link that crashed the last run"), *ChannelId.ToString());
    }
    GConfig->GetFloat(VideoConfigSection, *(ChannelId.ToString() + TEXT("_Volume")), Volume, GGameUserSettingsIni);
    // A replicated state can set the screen up before this runs, with the unsaved default.
    ApplyVolume();
}

void AWMCYNVideoChannel::OnRep_State()
{
    if (GetNetMode() != NM_DedicatedServer) ApplyState();
}

bool AWMCYNVideoChannel::SetupAdapter()
{
    if (Player) return true;
    if (!SourceScreen || !VideoMaterial) return false;
    TInlineComponentArray<UStaticMeshComponent*> Components(SourceScreen);
    for (UStaticMeshComponent* Component : Components)
    {
        if (Component->GetClass()->GetName() == TEXT("Comp_Video_C"))
        {
            AFVideo = Component;
            break;
        }
    }
    if (!AFVideo)
    {
        Status = TEXT("Screen unavailable");
        return false;
    }
    auto* PlayerProperty = FindFProperty<FObjectPropertyBase>(AFVideo->GetClass(), TEXT("MediaPlayer"));
    auto* TextureProperty = FindFProperty<FObjectPropertyBase>(AFVideo->GetClass(), TEXT("MediaTexture"));
    auto* MaterialProperty = FindFProperty<FObjectPropertyBase>(AFVideo->GetClass(), TEXT("VideoMaterial"));
    auto* FaceProperty = FindFProperty<FIntProperty>(AFVideo->GetClass(), TEXT("MaterialFace"));
    if (!PlayerProperty || !TextureProperty || !MaterialProperty || !FaceProperty)
    {
        Status = TEXT("Screen unavailable");
        UE_LOG(LogWMCYNVideo, Error, TEXT("channel=%s state=afcore-adapter-mismatch"), *ChannelId.ToString());
        return false;
    }
    // Each channel owns its player and texture: AFCore's shared MT_Video would let rooms overwrite each other.
    Player = NewObject<UMediaPlayer>(this);
    Player->PlayOnOpen = true;
    Player->SetDesiredPlayerName(TEXT("ElectraPlayer"));
    Texture = NewObject<UMediaTexture>(this);
    Texture->SetMediaPlayer(Player);
    Texture->UpdateResource();
    Material = UMaterialInstanceDynamic::Create(VideoMaterial, this);
    Material->SetTextureParameterValue(TEXT("MediaTexture"), Texture);
    PlayerProperty->SetObjectPropertyValue_InContainer(AFVideo, Player);
    TextureProperty->SetObjectPropertyValue_InContainer(AFVideo, Texture);
    MaterialProperty->SetObjectPropertyValue_InContainer(AFVideo, Material);
    AFVideo->SetMaterial(FaceProperty->GetPropertyValue_InContainer(AFVideo), Material);
    if (MirrorScreen)
    {
        TInlineComponentArray<UStaticMeshComponent*> Mirrors(MirrorScreen);
        for (UStaticMeshComponent* Mirror : Mirrors)
        {
            const int32 Slot = Mirror->GetMaterialIndex(MirrorSlot);
            if (Slot != INDEX_NONE) Mirror->SetMaterial(Slot, Material);
        }
    }
    TInlineComponentArray<UMediaSoundComponent*> Sounds(SourceScreen);
    for (UMediaSoundComponent* Sound : Sounds)
    {
        Sound->ComponentTags.AddUnique(TEXT("MediaSound"));
        Sound->SetMediaPlayer(Player);
        if (ChannelId == LivingRoomChannel)
        {
            // Set here, not in the map, so this client still matches the live server.
            Sound->AttenuationOverrides.FalloffDistance = LivingRoomFalloff;
            // The sound auto-activated at load, so its running copy needs the change too.
            if (UAudioComponent* Audio = Sound->GetAudioComponent()) Audio->AdjustAttenuation(Sound->AttenuationOverrides);
        }
    }
    ApplyVolume();
    Player->OnMediaOpened.AddDynamic(this, &ThisClass::Opened);
    Player->OnMediaOpenFailed.AddDynamic(this, &ThisClass::Failed);
    return true;
}

bool AWMCYNVideoChannel::ShouldDecode() const
{
#if PLATFORM_ANDROID
    // Quest decodes only the nearest playing channel in range, so rooms never stack decoders.
    const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!Camera) return false;
    const FVector View = Camera->GetCameraLocation();
    const AWMCYNVideoChannel* Nearest = nullptr;
    double Best = FMath::Square(QuestDecodeRange);
    for (TActorIterator<AWMCYNVideoChannel> It(GetWorld()); It; ++It)
    {
        if (It->GetEffectiveUrl().IsEmpty() || !It->SourceScreen) continue;
        const double Distance = FVector::DistSquared(View, It->SourceScreen->GetActorLocation());
        if (Distance < Best)
        {
            Best = Distance;
            Nearest = *It;
        }
    }
    return Nearest == this;
#else
    return true;
#endif
}

void AWMCYNVideoChannel::ApplyState()
{
    if (!SetupAdapter()) return;
    Player->Close();
    bOpened = false;
    OpenRevision = State.Revision;
    if (bCrashMarkerSet)
    {
        WMCYNVideoPolicy::ClearCrashMarker(ChannelId);
        bCrashMarkerSet = false;
    }
    const FString Url = GetEffectiveUrl();
    if (Url.IsEmpty())
    {
        Status = TEXT("Blank");
        return;
    }
    if (!bActive)
    {
        Status = TEXT("Paused while away");
        return;
    }
    if (Url != DefaultUrl)
    {
        Status = WMCYNVideoPolicy::Validate(Url);
        if (!Status.IsEmpty()) return;
    }
    if (Url == CrashedUrl)
    {
        // Skip it once. Submitting it again is a deliberate retry.
        CrashedUrl.Empty();
        Status = TEXT("Skipped: this link crashed last time");
        return;
    }
    OpenTime = ServerTime();
    WMCYNVideoPolicy::SetCrashMarker(ChannelId, Url);
    bCrashMarkerSet = true;
    if (WMCYNVideoPolicy::NeedsResolver(Url))
    {
        if (ResolveRevision != State.Revision)
        {
            ResolveRevision = State.Revision;
            bRetriedResolve = false;
        }
        Status = TEXT("Finding video");
        const int32 Revision = State.Revision;
        WMCYNVideoResolver::Resolve(Url, [WeakThis = TWeakObjectPtr<AWMCYNVideoChannel>(this), Url, Revision](const FString& Direct, const FString& Error)
        {
            if (WeakThis.IsValid()) WeakThis->OnResolved(Url, Revision, Direct, Error);
        });
        return;
    }
    OpenPlayUrl(Url);
}

void AWMCYNVideoChannel::OnResolved(const FString& Url, const int32 Revision, const FString& Direct, const FString& Error)
{
    // A newer submit or reset wins over a slow resolve.
    if (Revision != State.Revision || Url != GetEffectiveUrl() || Status != TEXT("Finding video")) return;
    if (Error.IsEmpty())
    {
        OpenPlayUrl(Direct);
        return;
    }
    Status = Error;
    if (bCrashMarkerSet)
    {
        WMCYNVideoPolicy::ClearCrashMarker(ChannelId);
        bCrashMarkerSet = false;
    }
    UE_LOG(LogWMCYNVideo, Warning, TEXT("channel=%s revision=%d state=resolve-failed"), *ChannelId.ToString(), State.Revision);
}

void AWMCYNVideoChannel::OpenPlayUrl(const FString& Url)
{
    PlayUrl = Url;
    Status = TEXT("Opening");
    OpenTime = ServerTime();
    if (!Player->OpenUrl(Url)) Failed(Url);
}

void AWMCYNVideoChannel::Opened(FString Url)
{
    if (OpenRevision != State.Revision || Url != PlayUrl) return;
    bOpened = true;
    Status = TEXT("Playing");
    const double Duration = Player->GetDuration().GetTotalSeconds();
    if (Player->SupportsSeeking() && WMCYNVideoPolicy::IsTimelineSeekable(Duration))
    {
        Player->Seek(FTimespan::FromSeconds(WMCYNVideoPolicy::SeekSeconds(ServerTime(), State.Started, Duration)));
    }
    UE_LOG(LogWMCYNVideo, Display, TEXT("channel=%s revision=%d state=opened player=%s"),
        *ChannelId.ToString(), State.Revision, *Player->GetPlayerName().ToString());
}

void AWMCYNVideoChannel::Failed(FString Url)
{
    if (OpenRevision != State.Revision || Url != PlayUrl) return;
    if (WMCYNVideoPolicy::NeedsResolver(GetEffectiveUrl()) && !bRetriedResolve)
    {
        // Direct links expire, so one fresh resolve comes before giving up.
        bRetriedResolve = true;
        ApplyState();
        return;
    }
    Status = TEXT("Playback failed; submit to retry");
    bOpened = false;
    if (bCrashMarkerSet)
    {
        WMCYNVideoPolicy::ClearCrashMarker(ChannelId);
        bCrashMarkerSet = false;
    }
    UE_LOG(LogWMCYNVideo, Warning, TEXT("channel=%s revision=%d state=failed"), *ChannelId.ToString(), State.Revision);
}

void AWMCYNVideoChannel::ApplyVolume()
{
    if (!SourceScreen) return;
    TInlineComponentArray<UMediaSoundComponent*> Sounds(SourceScreen);
    for (UMediaSoundComponent* Sound : Sounds) Sound->SetVolumeMultiplier(WMCYNVideoPolicy::VolumeGain(ChannelId, Volume));
}

void AWMCYNVideoChannel::SetLocalVolume(const float Value)
{
    Volume = FMath::Clamp(Value, 0.0f, 1.0f);
    GConfig->SetFloat(VideoConfigSection, *(ChannelId.ToString() + TEXT("_Volume")), Volume, GGameUserSettingsIni);
    ApplyVolume();
}

void AWMCYNVideoChannel::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (GetNetMode() == NM_DedicatedServer) return;
    const bool bShouldDecode = ShouldDecode();
    if (bActive != bShouldDecode || !Player)
    {
        bActive = bShouldDecode;
        ApplyState();
    }
    if (!Player) return;
    const double Now = ServerTime();
    if (bActive && Status == TEXT("Opening") && Now - OpenTime > OpenTimeoutSeconds)
    {
        Player->Close();
        Failed(PlayUrl);
    }
    if (bCrashMarkerSet && bOpened && Now - OpenTime > CrashMarkerSeconds)
    {
        WMCYNVideoPolicy::ClearCrashMarker(ChannelId);
        bCrashMarkerSet = false;
    }
    const double Duration = Player->GetDuration().GetTotalSeconds();
    if (bOpened && Player->IsPlaying() && Player->SupportsSeeking() && WMCYNVideoPolicy::IsTimelineSeekable(Duration))
    {
        const double Target = WMCYNVideoPolicy::SeekSeconds(Now, State.Started, Duration);
        if (FMath::Abs(Target - Player->GetTime().GetTotalSeconds()) > DriftToleranceSeconds)
        {
            Player->Seek(FTimespan::FromSeconds(Target));
        }
    }
}

void AWMCYNVideoChannel::EndPlay(const EEndPlayReason::Type Reason)
{
    if (Player)
    {
        Player->OnMediaOpened.RemoveAll(this);
        Player->OnMediaOpenFailed.RemoveAll(this);
        Player->Close();
    }
    if (bCrashMarkerSet) WMCYNVideoPolicy::ClearCrashMarker(ChannelId);
    Super::EndPlay(Reason);
}
