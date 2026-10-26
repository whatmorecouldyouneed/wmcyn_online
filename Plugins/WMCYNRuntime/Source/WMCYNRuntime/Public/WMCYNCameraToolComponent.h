#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WMCYNCameraToolComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class AWMCYNInterviewCamera;
class UMotionControllerComponent;

/** Local preview camera. Never changes possession or the tracked headset camera. */
UCLASS()
class WMCYNRUNTIME_API UWMCYNCameraToolComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UWMCYNCameraToolComponent();
    UFUNCTION(BlueprintCallable, Category="WMCYN|Camera")
    bool SetCameraEnabled(bool bEnabled);
    UFUNCTION(BlueprintCallable, Category="WMCYN|Camera")
    void ResetCameraPose();
    UFUNCTION(BlueprintCallable, Category="WMCYN|Camera")
    void SetFieldOfView(float Degrees);
    UFUNCTION(BlueprintCallable, Category="WMCYN|Camera")
    bool SetSpectatorOutput(bool bEnabled);
    /** Slots: 0 master, 1 interviewer, 2 guest. Local to this pawn/world lifetime. */
    UFUNCTION(BlueprintCallable, Category="WMCYN|Camera")
    bool SaveShot(int32 Slot);
    UFUNCTION(BlueprintCallable, Category="WMCYN|Camera")
    bool SelectShot(int32 Slot);
    UFUNCTION(BlueprintPure, Category="WMCYN|Camera")
    bool HasShot(int32 Slot) const;
    UFUNCTION(BlueprintPure, Category="WMCYN|Camera")
    bool IsCameraEnabled() const { return bCameraEnabled; }
    bool IsSpectatorOutputEnabled() const { return bSpectatorOutput; }
    bool IsHeld() const { return bHeld; }
    bool CanOperateCamera() const;
    UFUNCTION(BlueprintPure, Category="WMCYN|Camera")
    AWMCYNInterviewCamera* GetCameraProp() const { return CameraProp; }
    UTextureRenderTarget2D* GetPreview() const { return Target; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    bool IsNearDoor(const FVector& HandLocation) const;
    void UpdateGrip(const FTransform& Hand, bool bTracked, bool bDown, bool bDoorPriority);
    void RefreshCaptureExclusions();
    float ExclusionRefreshSeconds = 0;
    friend class FWMCYNHandheldCameraTest;
    UPROPERTY(Transient) TObjectPtr<AWMCYNInterviewCamera> CameraProp;
    UPROPERTY(Transient) TObjectPtr<UMotionControllerComponent> RightHand;
    FTransform GripOffset;
    bool bHeld = false;
    bool bGripWasDown = true;
    struct FShot { FTransform Pose; float FieldOfView = 60.f; };
    TMap<int32, FShot> Shots;
    UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> Capture;
    UPROPERTY(Transient) TObjectPtr<UTextureRenderTarget2D> Target;
    bool bCameraEnabled = false;
    bool bSpectatorOutput = false;
    uint8 PreviousSpectatorMode = 0;
    UPROPERTY(Transient) TObjectPtr<UTexture> PreviousSpectatorTexture;
};
