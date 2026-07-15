#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WMCYNBackendSubsystem.h"
#include "WMCYNFirstSignalLoginAsyncAction.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWMCYNFirstSignalLoginProgress, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FWMCYNFirstSignalLoginReady,
    const FString&,
    Username,
    const FString&,
    DisplayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWMCYNFirstSignalLoginFailed, const FString&, Message);

UCLASS()
class WMCYNRUNTIME_API UWMCYNFirstSignalLoginAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    static bool StartForLoginWidget(
        const UObject* WorldContextObject,
        UUserWidget* LoginWidget,
        const FString& Identifier,
        const FString& Password,
        bool bAllowPIEDevelopmentFallback = true);

    UFUNCTION(
        BlueprintCallable,
        Category = "WMCYN|Backend",
        meta = (
            BlueprintInternalUseOnly = "true",
            WorldContext = "WorldContextObject",
            AdvancedDisplay = "bAllowPIEDevelopmentFallback",
            DisplayName = "Authenticate and Load First Signal"))
    static UWMCYNFirstSignalLoginAsyncAction* AuthenticateAndLoadFirstSignal(
        const UObject* WorldContextObject,
        const FString& Identifier,
        const FString& Password,
        bool bAllowPIEDevelopmentFallback = true);

    virtual void Activate() override;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalLoginProgress OnProgress;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalLoginReady OnReady;

    UPROPERTY(BlueprintAssignable)
    FWMCYNFirstSignalLoginFailed OnFailed;

private:
    UFUNCTION()
    void HandleLoginStateChanged(EWMCYNBackendLoginState State, const FString& Message);

    UFUNCTION()
    void HandleLoginReady(
        const FString& Username,
        const FString& DisplayName,
        const FString& WorldHost,
        int32 WorldPort);

    bool ShouldUsePIEDevelopmentFallback() const;
    void SetLoginStatus(const FString& Message) const;
    void CompleteLoginWidget();
    void Finish();

    UPROPERTY()
    TObjectPtr<UWMCYNBackendSubsystem> BackendSubsystem;

    UPROPERTY()
    TObjectPtr<UUserWidget> LoginWidget;

    TWeakObjectPtr<const UObject> WorldContext;
    FString LoginIdentifier;
    FString LoginPassword;
    bool bAllowLocalPIEFallback = true;
    bool bFinished = false;
};
