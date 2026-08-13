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

USTRUCT()
struct FWMCYNAvatarManifest
{
    GENERATED_BODY()

    FString AvatarId;
    int32 AvatarVersion = 0;
    FString Platform;
    bool bIsDefaultAvatar = true;
    FString DisplayName;
    FString DeliveryType;
    FString DeliveryUrl;
    FString DeliveryAssetRoot;
    FString DeliverySkeletalMeshPath;
    FString DeliveryHeadSkeletalMeshPath;
    FString DeliveryAnimClassPath;
    FString CacheKey;
};

DECLARE_DELEGATE_ThreeParams(
    FWMCYNAvatarManifestCallback,
    bool,
    const FWMCYNAvatarManifest&,
    const FString&);

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
    void SignInWithCustomTokenAndLoadBootstrap(const FString& CustomToken);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    void SignOut();

    UFUNCTION(BlueprintPure, Category = "WMCYN|Backend")
    FString GetBackendBaseUrl() const;

    UFUNCTION(BlueprintPure, Category = "WMCYN|Backend")
    FString GetWorldTravelURL() const;

    UFUNCTION(BlueprintPure, Category = "WMCYN|Backend")
    bool IsReadyToEnterWorld() const;

    /** Client-travels the local player to the canonical runtime with the join ticket. */
    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    bool TravelToFirstSignalWorld();

    void RequestAvatarManifest(FWMCYNAvatarManifestCallback&& Callback);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    void SetBackendBaseUrl(const FString& InBaseUrl);

    UFUNCTION(BlueprintCallable, Category = "WMCYN|Backend")
    void SetFirebaseWebApiKey(const FString& InFirebaseWebApiKey);

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
    FString ResolveFirebaseWebApiKey() const;
    FString ResolveAvatarManifestPlatform() const;

    FString BackendBaseUrl = TEXT("https://api-rrm3u3yaba-uc.a.run.app");
    FString FirebaseWebApiKey;
    FString IdToken;
    FString RefreshToken;

    /** Opaque short-lived signed ticket; never replicated, never logged. */
    FString JoinTicket;
};
