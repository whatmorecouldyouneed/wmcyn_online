#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WMCYNVideoChannel.generated.h"

class UMaterialInstanceDynamic;
class UMediaPlayer;
class UMediaTexture;
class UStaticMeshComponent;

/** Plain link and playback rules, shared by the server and clients. */
namespace WMCYNVideoPolicy
{
    constexpr int32 MaxUrlLength = 2048;
    constexpr double SubmitCooldownSeconds = 3.0;

    /** Empty when the link may be played; otherwise the message to show the user. */
    WMCYNRUNTIME_API FString Validate(const FString& Url);
    /** YouTube and Twitch pages aren't media files, so each client resolves them for itself. */
    WMCYNRUNTIME_API bool NeedsResolver(const FString& Url);
    WMCYNRUNTIME_API double SeekSeconds(double Now, double Started, double Duration);
    /** Live streams report no usable duration, so they stay at their live edge. */
    WMCYNRUNTIME_API bool IsTimelineSeekable(double DurationSeconds);

    // Written before opening a link, so a decoder crash can be recognised on the next launch.
    WMCYNRUNTIME_API void SetCrashMarker(FName ChannelId, const FString& Url);
    WMCYNRUNTIME_API void ClearCrashMarker(FName ChannelId);
    WMCYNRUNTIME_API FString ReadCrashMarker(FName ChannelId);
}

USTRUCT()
struct FWMCYNVideoState
{
    GENERATED_BODY()
    /** Empty means the channel default. Only a page or media link ever travels, never a resolved one. */
    UPROPERTY() FString Url;
    UPROPERTY() int32 Revision = 0;
    UPROPERTY() double Started = 0;
    UPROPERTY() FString SetBy;
};

/**
 * Server-owned shared screen. AFCore's Comp_Video stays the screen and its stock player path is
 * unused; this adds the link replication it lacks. A mirror shows the same material, so it
 * can't drift from the source and needs no second decoder.
 */
UCLASS()
class WMCYNRUNTIME_API AWMCYNVideoChannel : public AActor
{
    GENERATED_BODY()
public:
    AWMCYNVideoChannel();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Empty when accepted; otherwise the message to show the submitter. */
    FString Submit(const FString& Url, bool bReset, const FString& VerifiedUid);
    void SetLocalVolume(float Value);
    float GetLocalVolume() const { return Volume; }
    FString GetStatus() const { return Status; }
    FString GetEffectiveUrl() const { return State.Url.IsEmpty() ? DefaultUrl : State.Url; }
    bool IsKnownChannel() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WMCYN|Video") FName ChannelId;
    /** Trusted level data, so it's exempt from the submit policy. Empty stays blank. */
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") FString DefaultUrl;
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") TObjectPtr<AActor> SourceScreen;
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") TObjectPtr<AActor> MirrorScreen;
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") FName MirrorSlot = TEXT("DISP");
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") TObjectPtr<UMaterialInterface> VideoMaterial;

private:
    UFUNCTION() void OnRep_State();
    UFUNCTION() void Opened(FString Url);
    UFUNCTION() void Failed(FString Url);
    bool SetupAdapter();
    void ApplyState();
    void ApplyVolume();
    void OnResolved(const FString& Url, int32 Revision, const FString& Direct, const FString& Error);
    void OpenPlayUrl(const FString& Url);
    bool ShouldDecode() const;
    double ServerTime() const;

    UPROPERTY(ReplicatedUsing=OnRep_State) FWMCYNVideoState State;
    UPROPERTY(Transient) TObjectPtr<UMediaPlayer> Player;
    UPROPERTY(Transient) TObjectPtr<UMediaTexture> Texture;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Material;
    UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> AFVideo;

    FString Status = TEXT("Blank");
    FString CrashedUrl;
    /** What the player actually opened. Resolved links stay local and are never replicated. */
    FString PlayUrl;
    TMap<FString, double> LastSubmissions;
    float Volume = 0.5f;
    int32 OpenRevision = -1;
    int32 ResolveRevision = -1;
    double OpenTime = 0.0;
    bool bOpened = false;
    bool bActive = false;
    bool bCrashMarkerSet = false;
    bool bRetriedResolve = false;
};
