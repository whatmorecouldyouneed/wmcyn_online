#pragma once

#include "CoreMinimal.h"
#include "WMCYNSessionItem.h"
#include "WMCYNLighterItem.generated.h"

class UNiagaraComponent;
class USoundBase;

/** Port of CigaretteLighterController. State: 0 off, 1 lit, -1 failed strike. */
UCLASS()
class WMCYNRUNTIME_API AWMCYNLighterItem : public AWMCYNSessionItem
{
    GENERATED_BODY()
public:
    AWMCYNLighterItem();
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void ServerUse(bool bDown) override;

    bool IsLit() const { return State == 1; }
    FVector GetFlameLocation() const;
    static int8 RollStrike();

protected:
    virtual void HandleTaken() override;
    virtual void HandleReleased() override;

private:
    void SetState(int8 NewState);
    void PlayStateEffects();
    UFUNCTION() void OnRep_State();
    UPROPERTY(ReplicatedUsing=OnRep_State) int8 State = 0;
    UPROPERTY(VisibleAnywhere, Category="WMCYN|Item") TObjectPtr<UStaticMeshComponent> Flame;
    UPROPERTY(VisibleAnywhere, Category="WMCYN|Item") TObjectPtr<UNiagaraComponent> Spark;
    UPROPERTY(EditAnywhere, Category="WMCYN|Item") TArray<TObjectPtr<USoundBase>> LightSounds;
    UPROPERTY(EditAnywhere, Category="WMCYN|Item") TArray<TObjectPtr<USoundBase>> FailSounds;
    int8 AppliedState = 0;
};
