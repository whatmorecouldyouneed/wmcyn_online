#pragma once

#include "CoreMinimal.h"
#include "WMCYNJointBurnState.h"
#include "WMCYNSessionItem.h"
#include "WMCYNJointItem.generated.h"

class UMaterialInterface;
class UNiagaraComponent;

/** Runs the burn state on the server and draws it everywhere else. */
UCLASS()
class WMCYNRUNTIME_API AWMCYNJointItem : public AWMCYNSessionItem
{
    GENERATED_BODY()
public:
    AWMCYNJointItem();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void ServerUse(bool bDown) override;

    /** Applies a reported touch. False when the other item can't currently do anything. */
    bool ServerApplyContact(AWMCYNSessionItem* Other);

    bool IsBurning() const { return bReplicatedBurning; }
    FVector GetEmberLocation() const;
    UMaterialInterface* GetHighOverlayMaterial() const { return HighOverlayMaterial; }
    float GetBurnParam() const { return ReplicatedParam; }

protected:
    virtual void HandleReleased() override;

private:
    void PublishBurnState();
    void UpdateVisuals(float DeltaSeconds);
    void ApplySmoke();
    UFUNCTION() void OnRep_BurnFlags();

    FWMCYNJointBurnState Burn;
    UPROPERTY(Replicated) float ReplicatedParam = 0.0f;
    UPROPERTY(ReplicatedUsing=OnRep_BurnFlags) bool bReplicatedBurning = false;
    UPROPERTY(ReplicatedUsing=OnRep_BurnFlags) bool bReplicatedBoost = false;
    UPROPERTY(ReplicatedUsing=OnRep_BurnFlags) bool bReplicatedExhaling = false;

    UPROPERTY(VisibleAnywhere, Category="WMCYN|Item") TObjectPtr<USceneComponent> Ember;
    UPROPERTY(VisibleAnywhere, Category="WMCYN|Item") TObjectPtr<UNiagaraComponent> EmberSmoke;
    UPROPERTY(VisibleAnywhere, Category="WMCYN|Item") TObjectPtr<UNiagaraComponent> ExhaleSmoke;
    /** Hard reference so the overlay cooks with the placed item. */
    UPROPERTY(EditAnywhere, Category="WMCYN|Item") TObjectPtr<UMaterialInterface> HighOverlayMaterial;

    // Prefab root scale; the mesh is baked at 8.28 cm long.
    FVector JointScale = FVector(3.0f, 3.0f, 2.0f);
    float VisualParam = 0.0f;
    float VisualThreshold = 1.0f;
    bool bSmokeActive = false;
    bool bExhaleActive = false;
};
