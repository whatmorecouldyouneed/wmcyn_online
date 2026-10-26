#pragma once

#include "CoreMinimal.h"
#include "WMCYNBackendSubsystem.h"
#include "WMCYNRememberedSignInControls.generated.h"

class UUserWidget;

/** Binds Designer-owned controls without reparenting the AFCore login widget. */
UCLASS()
class WMCYNRUNTIME_API UWMCYNRememberedSignInControls : public UObject
{
    GENERATED_BODY()
public:
    void Initialize(UUserWidget* InWidget);
private:
    void Refresh();
    UFUNCTION() void Continue();
    UFUNCTION() void SwitchAccount();
    UFUNCTION() void RememberChanged(bool bChecked);
    UFUNCTION() void StateChanged(EWMCYNBackendLoginState State, const FString& Message);
    UPROPERTY(Transient) TWeakObjectPtr<UUserWidget> Widget;
    UPROPERTY(Transient) TObjectPtr<UWMCYNBackendSubsystem> Backend;
};
