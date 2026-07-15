#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WMCYNFirstSignalPresenceComponent.generated.h"

class ACharacter;
class APlayerState;
class UCameraComponent;
class UMotionControllerComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UVOIPTalker;
class UUserWidget;
class UWidgetInteractionComponent;
class UWidgetComponent;

UCLASS(ClassGroup = (WMCYN), meta = (BlueprintSpawnableComponent))
class WMCYNRUNTIME_API UWMCYNFirstSignalPresenceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWMCYNFirstSignalPresenceComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Identity")
    void SubmitIdentity(const FString& Username, const FString& DisplayName);

    UWidgetInteractionComponent* GetPreferredWidgetInteraction() const;
    UWidgetInteractionComponent* GetKeyboardInputInteraction() const;
    void CompleteLocalLoginGate();

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

private:
    void CacheNativeComponents();
    void ConfigureWidgetInteraction();
    void UpdateWidgetInteractionPresentation();
    void ApplyLocalLoginGateLock();
    void ConfigureRemoteTracking();
    void CaptureAndSendPose();
    void ApplyReplicatedPose(float DeltaTime);
    void ApplyIdentity(const FString& Username, const FString& DisplayName);
    void CreateNameplate();
    void RefreshNameplate();
    void CreateRuntimeMenu();
    void UpdateRuntimeMenuInput();
    void SetRuntimeMenuVisible(bool bVisible);
    void PositionRuntimeMenu();
    void RegisterVoice();
    void ActivateLocalVoice();
    void HandleVoiceSessionCreated(FName SessionName, bool bWasSuccessful);

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

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    bool bHasReplicatedPose = false;

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
    TObjectPtr<UWidgetComponent> Nameplate;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Identity")
    TSoftClassPtr<UUserWidget> NameplateWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Identity")
    TSoftClassPtr<UWidgetComponent> NameplateHostClass;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|Identity")
    TSoftObjectPtr<UObject> NameplateThemeAsset;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetComponent> RuntimeMenu;

    UPROPERTY(EditDefaultsOnly, Category = "WMCYN|UI")
    TSoftClassPtr<UUserWidget> RuntimeMenuWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UVOIPTalker> VoiceTalker;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetInteractionComponent> PreferredWidgetInteraction;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetInteractionComponent> KeyboardInputInteraction;

    float NextPoseSendTime = 0.0f;
    float NextNameplateRefreshTime = 0.0f;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    bool bRemoteTrackingConfigured = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    int32 ConfiguredWidgetInteractionCount = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "WMCYN|Diagnostics", meta = (AllowPrivateAccess = "true"))
    bool bVoiceRegistered = false;
    bool bVoiceActivationRequested = false;
    bool bLoginGateLockApplied = false;
    bool bLoginGateCompleted = false;
    bool bRuntimeMenuVisible = false;
    FDelegateHandle CreateSessionDelegateHandle;
};
