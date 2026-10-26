#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Styling/SlateBrush.h"
#include "WMCYNInterviewCamera.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UWidgetComponent;
class UTextureRenderTarget2D;

/** Local camera prop; capture and spectator routing remain owned by the pawn tool. */
UCLASS(NotBlueprintable)
class WMCYNRUNTIME_API AWMCYNInterviewCamera : public AActor
{
    GENERATED_BODY()
public:
    AWMCYNInterviewCamera();
    void SetPreview(UTextureRenderTarget2D* Texture);
    USceneComponent* GetLens() const { return Lens; }
    FVector GetGripLocation() const;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Grip;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> LensHousing;
    UPROPERTY() TObjectPtr<USceneComponent> Lens;
    UPROPERTY() TObjectPtr<UWidgetComponent> Display;
    FSlateBrush PreviewBrush;
};
