#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WMCYNBackendSubsystem.h"
#include "WMCYNFirstSignalPairingAsyncAction.generated.h"

class UUserWidget;
class UBorder;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWMCYNFirstSignalPairingProgress, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FWMCYNFirstSignalPairingCodeReady,
    const FString&,
    Code,
    const FString&,
    ExpiresAt);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FWMCYNFirstSignalPairingReady,
    const FString&,
    Username,
    const FString&,
    DisplayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWMCYNFirstSignalPairingFailed, const FString&, Message);

UCLASS()
class WMCYNRUNTIME_API UWMCYNFirstSignalPairingAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    static void PrimeLoginWidget(UUserWidget* InLoginWidget);

    static bool StartForLoginWidget(
        const UObject* WorldContextObject,
        UUserWidget* LoginWidget);

    UFUNCTION(
        BlueprintCallable,
        Category = "WMCYN|Backend",
        meta = (
            BlueprintInternalUseOnly = "true",
            WorldContext = "WorldContextObject",
            DisplayName = "Begin First Signal Pairing"))
    static UWMCYNFirstSignalPairingAsyncAction* BeginFirstSignalPairing(
        const UObject* WorldContextObject);

    virtual void Activate() override;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalPairingProgress OnProgress;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalPairingCodeReady OnCodeReady;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalPairingReady OnReady;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalPairingFailed OnFailed;

private:
    void RequestPairingCode();
    void PollPairingExchange();
    void ApplyPairingWidgetPresentation();
    void EnsurePairingWidget();
    void SetLoginStatus(const FString& Message) const;
    void SetPairingCodeText(const FString& Code) const;
    void SetWidgetText(const TCHAR* WidgetName, const FString& Text) const;
    void SetWidgetVisibility(const TCHAR* WidgetName, ESlateVisibility Visibility) const;
    void CompleteLoginWidget();
    void StopPolling();
    void Finish();

    UFUNCTION()
    void HandleLoginStateChanged(EWMCYNBackendLoginState State, const FString& Message);

    UFUNCTION()
    void HandleLoginReady(
        const FString& Username,
        const FString& DisplayName,
        const FString& WorldHost,
        int32 WorldPort);

    UPROPERTY()
    TObjectPtr<UWMCYNBackendSubsystem> BackendSubsystem;

    UPROPERTY()
    TObjectPtr<UUserWidget> LoginWidget;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> PairingPanel = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PairingTitleText = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PairingCodeText = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PairingStatusText = nullptr;

    TWeakObjectPtr<const UObject> WorldContext;
    FString PairingCode;
    FString PairingCodeExpiresAt;
    FTimerHandle PollTimerHandle;
    bool bFinished = false;
};
