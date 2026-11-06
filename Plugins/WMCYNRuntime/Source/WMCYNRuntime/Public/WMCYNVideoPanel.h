#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SlateEnums.h"
#include "WMCYNVideoPanel.generated.h"

class AWMCYNVideoChannel;
class UUserWidget;
class UWidgetComponent;

/** In-world channel control. Binds Designer-owned controls by name; the widget holds no logic. */
UCLASS()
class WMCYNRUNTIME_API AWMCYNVideoPanel : public AActor
{
    GENERATED_BODY()
public:
    AWMCYNVideoPanel();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") TObjectPtr<AWMCYNVideoChannel> Channel;
    UPROPERTY(EditAnywhere, Category="WMCYN|Video") FText Title;
private:
    bool BindWidget();
    void InstallLaptopToggle(UWidgetComponent* Host);
    void Refresh();
    void Send(const FString& Url, bool bReset);
    UFUNCTION() void HandleSubmit();
    UFUNCTION() void HandleReset();
    UFUNCTION() void HandlePaste();
    UFUNCTION() void HandleVolume(float Value);
    UFUNCTION() void HandleCommitted(const FText& Text, ETextCommit::Type Method);
    UPROPERTY(Transient) TObjectPtr<UUserWidget> Widget;
};
