#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WMCYNFirstSignalSettingsWidget.generated.h"

class UButton;

UCLASS()
class WMCYNRUNTIME_API UWMCYNFirstSignalSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleRespawnClicked();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> RespawnButton;
};
