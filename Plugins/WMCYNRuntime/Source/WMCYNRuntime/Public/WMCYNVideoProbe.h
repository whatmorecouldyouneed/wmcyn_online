#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WMCYNVideoProbe.generated.h"

class UMediaPlayer;
class UMediaTexture;
class UMediaSoundComponent;
class UStaticMeshComponent;
class UCameraComponent;

/** Isolated packaged-device diagnostic. Never place in the production world. */
UCLASS()
class WMCYNRUNTIME_API AWMCYNVideoProbe : public AActor
{
    GENERATED_BODY()
public:
    AWMCYNVideoProbe();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditAnywhere) TObjectPtr<UMaterialInterface> ScreenMaterial;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Screen;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UMediaSoundComponent> Audio;
private:
    UFUNCTION() void Opened(FString IgnoredUrl);
    UFUNCTION() void Failed(FString IgnoredUrl);
    void Advance();
    void Report();
    UPROPERTY() TObjectPtr<UMediaPlayer> Player;
    UPROPERTY() TObjectPtr<UMediaTexture> Texture;
    int32 Phase = -1;
    double PhaseStarted = 0;
    double LastReport = 0;
    TArray<float> Frames;
    bool bOpened = false;
};
