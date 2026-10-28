#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "WMCYNFirstSignalPresenceComponent.generated.h"

class ACharacter;
class APlayerState;
class UAnimInstance;
class UCameraComponent;
class UMaterialInstanceDynamic;
class UMotionControllerComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UVOIPTalker;
class UUserWidget;
class UInputComponent;
class UWidgetInteractionComponent;
class UWidgetComponent;
class FTextProperty;
class FStrProperty;
class UWMCYNRememberedSignInControls;

UCLASS(ClassGroup = (WMCYN), meta = (BlueprintSpawnableComponent))
class WMCYNRUNTIME_API UWMCYNFirstSignalPresenceComponent : public UActorComponent
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TObjectPtr<UWMCYNRememberedSignInControls> RememberedSignInControls;

public:
    bool HasVerifiedRecordingCapability() const { return bVerifiedRecordingCapability; }
    UWMCYNFirstSignalPresenceComponent();
private:
    UPROPERTY(Replicated)
    bool bVerifiedRecordingCapability = false;
public:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Identity")
    void SubmitIdentity(const FString& Username, const FString& DisplayName);

    UWidgetInteractionComponent* GetPreferredWidgetInteraction() const;
    UWidgetInteractionComponent* GetKeyboardInputInteraction() const;
    void CompleteLocalLoginGate();

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Presence")
    void RequestRespawnToPresenceSlot();

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Menu")
    void SetLocalMicrophoneMuted(bool bMuted);

    UFUNCTION(BlueprintPure, Category = "WMCYN|Menu")
    FString GetLocalMicrophoneStatus() const;

    bool IsLocalMicrophoneMuted() const { return bLocalMicrophoneMuted; }

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Menu")
    void ResetRuntimeMenuPosition();

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Menu")
    void SetRuntimeMenuPinned(bool bPinned);

protected:
    UFUNCTION(Server, Unreliable)
    void ServerUpdateTrackedPose(
        FVector_NetQuantize10 InHeadLocation,
        FRotator InHeadRotation,
        FVector_NetQuantize10 InLeftHandLocation,
        FRotator InLeftHandRotation,
        FVector_NetQuantize10 InRightHandLocation,
        FRotator InRightHandRotation);

    UFUNCTION(Server, Reliable)
    void ServerSubmitIdentity(const FString& Username, const FString& DisplayName);

    UFUNCTION(Server, Reliable)
    void ServerRequestRespawnToPresenceSlot();

    UFUNCTION(Server, Reliable)
    void ServerSubmitAvatarPresentation(
        const FString& SkeletalMeshPath,
        const FString& HeadSkeletalMeshPath,
        const FString& AnimClassPath,
        const FString& CacheKey,
        bool bIsDefaultAvatar);

    /** AndroidPermission proxy callback for the microphone request. */
    UFUNCTION()
    void HandleMicPermissionResult(const TArray<FString>& Permissions, const TArray<bool>& GrantResults);

    UFUNCTION()
    void OnRep_AvatarPresentation();

private:
    bool bLocalMicrophoneMuted = false;
    bool bRuntimeMenuPinned = false;
    void ApplyIdentityToPlayerState(APlayerState* PlayerState, const FString& Username, const FString& DisplayName, bool bForceReplication);
    bool TryApplyPlayerInfoDisplayName(UActorComponent* Component, const FString& DisplayName) const;
    FString ResolvePreferredDisplayName(const APlayerState* PlayerState) const;
    bool LooksLikeUuid(const FString& Value) const;
    void CacheNativeComponents();
    void ConfigureWidgetInteraction();
    void UpdateWidgetInteractionPresentation();
    void UpdatePointerBeam(bool bVisible);
    void PrimeLoginWidgetPairingPanel();
    void EnsureLoginPanelPresent();
    void TryAutoEnterFromVerifiedLogin();
    AActor* ResolveLoginPanelActor();
    void ApplyLocalLoginGateLock();
    void ConfigureRemoteTracking();
    void CaptureAndSendPose();
    void ApplyReplicatedPose(float DeltaTime);
    void ApplyIdentity(const FString& Username, const FString& DisplayName);
    void RequestAvatarManifestIfNeeded();
    void ApplyAvatarPresentation(
        const FString& SkeletalMeshPath,
        const FString& HeadSkeletalMeshPath,
        const FString& AnimClassPath,
        const FString& CacheKey,
        bool bIsDefaultAvatar);
    void RespawnToPresenceSlot();
    void CreateNameplate();
    void RefreshNameplate();
    void CreateRuntimeMenu();
    void EnsureRuntimeMenuInputBinding();
    void HandleRuntimeMenuToggle();
    void UpdateRuntimeMenuInput();
    void SetRuntimeMenuVisible(bool bVisible);
    void PositionRuntimeMenu();
    void RegisterVoice();
    void ActivateLocalVoice();
    void HandleVoiceSessionCreated(FName SessionName, bool bWasSuccessful);
    void EnsureAvatarPresentationComponents();
    void RestoreAvatarSourceTickPolicy();
    TOptional<EVisibilityBasedAnimTickOption> SavedAvatarSourceTickPolicy;
    TWeakObjectPtr<USkeletalMeshComponent> AvatarTickPolicySource;

    UPROPERTY(Replicated)
    FVector_NetQuantize10 ReplicatedHeadLocation;

    UPROPERTY(Replicated)
    FRotator ReplicatedHeadRotation;

    UPROPERTY(Replicated)
    FVector_NetQuantize10 ReplicatedLeftHandLocation;

    UPROPERTY(Replicated)
    FRotator ReplicatedLeftHandRotation;

    UPROPERTY(Replicated)
    FVector_NetQuantize10 ReplicatedRightHandLocation;

    UPROPERTY(Replicated)
    FRotator ReplicatedRightHandRotation;

    UPROPERTY(Replicated)
    FVector_NetQuantize10 ReplicatedMeshRelativeLocation;

    UPROPERTY(Replicated)
    FRotator ReplicatedMeshRelativeRotation;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    bool bHasReplicatedPose = false;

    UPROPERTY(ReplicatedUsing = OnRep_AvatarPresentation)
    FString ReplicatedAvatarSkeletalMeshPath;

    UPROPERTY(Replicated)
    FString ReplicatedAvatarHeadSkeletalMeshPath;

    UPROPERTY(Replicated)
    FString ReplicatedAvatarAnimClassPath;

    UPROPERTY(Replicated)
    FString ReplicatedAvatarCacheKey;

    UPROPERTY(Replicated)
    bool bReplicatedAvatarIsDefault = true;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    int32 ReplicatedPoseSequence = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    int32 ServerPoseUpdateCount = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    int32 RemotePoseApplyCount = 0;

    UPROPERTY(Transient)
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY(Transient)
    TObjectPtr<UCameraComponent> TrackedCamera;

    UPROPERTY(Transient)
    TObjectPtr<UMotionControllerComponent> LeftController;

    UPROPERTY(Transient)
    TObjectPtr<UMotionControllerComponent> RightController;

    UPROPERTY(Transient)
    TObjectPtr<USceneComponent> HeadAnchor;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> BodyMesh;

    /** WMCYN-owned presentation components; native Mimic components remain authoritative for tracking. */
    UPROPERTY(Transient)
    TObjectPtr<USceneComponent> AvatarPresentationRoot;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> AvatarBodyVisual;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> AvatarHeadVisual;

    /**
     * Garments follow AvatarBodyVisual by leader pose rather than evaluating a
     * second retarget: they carry the same body skeleton, so copying its pose
     * is both correct and cheaper. Optional - a family with no garments leaves
     * these components present but empty and hidden.
     */
    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> AvatarOutfitVisual;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> AvatarShoesVisual;

    /**
     * Hair, eyebrows and goatee. One GroomComponent per groom, each attached to
     * AvatarHeadVisual and bound to it, so they follow the face rather than the
     * body. Sized by the matched family, so a family with no grooms leaves this
     * empty.
     */
    UPROPERTY(Transient)
    TArray<TObjectPtr<class UGroomComponent>> AvatarGroomVisuals;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetComponent> Nameplate;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Identity")
    TSoftClassPtr<UUserWidget> NameplateWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Identity")
    TSoftClassPtr<UWidgetComponent> NameplateHostClass;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Identity")
    TSoftObjectPtr<UObject> NameplateThemeAsset;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|UI")
    TSoftObjectPtr<UObject> RuntimeMenuThemeAsset;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetComponent> RuntimeMenu;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|UI")
    TSoftClassPtr<UUserWidget> RuntimeMenuWidgetClass;

    /** Soft references keep the verified first avatar in packaged cooks without loading it during construction. */
    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Avatar")
    TSoftObjectPtr<USkeletalMesh> RuntimeAvatarBodyCookReference;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Avatar")
    TSoftObjectPtr<USkeletalMesh> RuntimeAvatarHeadCookReference;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Avatar")
    TSoftClassPtr<UAnimInstance> RuntimeAvatarAnimCookReference;

    UPROPERTY(Transient)
    TObjectPtr<UVOIPTalker> VoiceTalker;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetInteractionComponent> PreferredWidgetInteraction;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetInteractionComponent> KeyboardInputInteraction;

    /** Rendered pointer beam; debug-draw lines do not render on packaged Quest. */
    UPROPERTY(Transient)
    TObjectPtr<UStaticMeshComponent> PointerBeam;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> PointerBeamMaterial;

    float NextPoseSendTime = 0.0f;
    float NextNameplateRefreshTime = 0.0f;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    bool bRemoteTrackingConfigured = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    int32 ConfiguredWidgetInteractionCount = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    bool bVoiceRegistered = false;
    bool bVoiceActivationRequested = false;
    bool bMicPermissionRequested = false;
    bool bLoginGateLockApplied = false;
    bool bLoginGateCompleted = false;
    bool bAvatarManifestRequested = false;
    int32 AvatarManifestAttempts = 0;
    float NextAvatarManifestRequestTime = 0.0f;
    FString AvatarManifestUserId;
    bool bAvatarSubmissionRejectedLogged = false;
    bool bRuntimeMenuVisible = false;
    bool bAutoEntryAttempted = false;
    bool bLoginWidgetPairingPrimed = false;
    bool bLoginPanelRestoreFailed = false;
    bool bRuntimeMenuCreateFailed = false;
    int32 RuntimeMenuInputBindingHandle = INDEX_NONE;
    TWeakObjectPtr<UInputComponent> RuntimeMenuInputComponent;
    TWeakObjectPtr<AActor> ResolvedLoginPanelActor;
    FDelegateHandle CreateSessionDelegateHandle;
    FString AppliedAvatarCacheKey;
};
