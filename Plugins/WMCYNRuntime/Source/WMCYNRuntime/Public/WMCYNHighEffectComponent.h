#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WMCYNHighEffectComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Local-only, head-locked tint. A reconstruction: the source script is missing and only its
 * 60 second duration and a slow opacity ramp survive. Never moves the camera.
 */
UCLASS()
class WMCYNRUNTIME_API UWMCYNHighEffectComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    static constexpr float DurationSeconds = 60.0f;
    static constexpr float RampSeconds = 20.0f;
    static constexpr float FadeSeconds = 10.0f;
    static constexpr float MaxOpacity = 0.18f;

    UWMCYNHighEffectComponent();
    void Play(UMaterialInterface* Material);
    void Stop();
    float GetOpacity() const { return Opacity; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    static bool IsEnabledByUser();
    static void SetEnabledByUser(bool bEnabled);

private:
    UPROPERTY(Transient) TObjectPtr<UStaticMeshComponent> Shell;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> ShellMaterial;
    UPROPERTY(Transient) TObjectPtr<UStaticMesh> SphereMesh;
    float Remaining = 0.0f;
    float Opacity = 0.0f;
};
