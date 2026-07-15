#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WMCYNBackendSubsystem.generated.h"

UENUM(BlueprintType)
enum class EWMCYNBackendLoginState : uint8
{
    SignedOut,
    Authenticating,
    LoadingBootstrap,
    Ready,
    Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FWMCYNBackendLoginStateChanged,
    EWMCYNBackendLoginState,
    State,
    const FString&,
    Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FWMCYNBackendLoginReady,
    const FString&,
    Username,
    const FString&,
    DisplayName,
    const FString&,
    WorldHost,
    int32,
    WorldPort);

UCLASS()
class WMCYNRUNTIME_API UWMCYNBackendSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    void LoginAndLoadBootstrap(const FString& Identifier, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    void SignOut();

    UFUNCTION(BlueprintPure, Category = "WMCYN|Backend")
    FString GetWorldTravelURL() const;

    UFUNCTION(BlueprintPure, Category = "WMCYN|Backend")
    bool IsReadyToEnterWorld() const;

    /** Client-travels the local player to the canonical runtime with the join ticket. */
    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    bool TravelToFirstSignalWorld();

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    void SetBackendBaseUrl(const FString& InBaseUrl);

    UPROPERTY(BlueprintAssignable, Category = "WMCYN|Backend")
    FWMCYNBackendLoginStateChanged OnLoginStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "WMCYN|Backend")
    FWMCYNBackendLoginReady OnLoginReady;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    EWMCYNBackendLoginState LoginState = EWMCYNBackendLoginState::SignedOut;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString VerifiedUserId;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString VerifiedUsername;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString VerifiedDisplayName;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString WorldId;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString WorldRuntimeId;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString WorldHost;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    int32 WorldPort = 0;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString WorldBuildId;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    int32 WorldProtocolVersion = 0;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString PresenceMode;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    FString PresenceSlot;

    UPROPERTY(BlueprintReadOnly, Transient, Category = "WMCYN|Backend")
    TArray<FString> PresenceCapabilities;

private:
    void RequestBootstrap();
    void SetState(EWMCYNBackendLoginState NewState, const FString& Message);
    void Fail(const FString& ErrorCode);
    FString MakeUrl(const FString& Path) const;

    FString BackendBaseUrl = TEXT("https://us-central1-wmcyn-online-mobile.cloudfunctions.net/api");
    FString IdToken;
    FString RefreshToken;

    /** Opaque short-lived signed ticket; never replicated, never logged. */
    FString JoinTicket;
};
