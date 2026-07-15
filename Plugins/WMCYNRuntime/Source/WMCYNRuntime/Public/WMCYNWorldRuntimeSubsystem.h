#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WMCYNJoinTicket.h"
#include "WMCYNWorldRuntimeSubsystem.generated.h"

class AGameModeBase;
class APlayerController;

/**
 * Server-side implementation of Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md.
 *
 * When activated with -WMCYNRegisterRuntime on the authoritative process, it
 * registers this process as the singleton canonical Crib runtime, reports
 * `online` once L_WMCYNOnline is serving, heartbeats every 10 seconds, and
 * validates every remote login's short-lived signed join ticket before the
 * player may keep their connection. Inactive in normal clients and PIE.
 *
 * Configuration (server deployment only; never ship these values to Quest or
 * PCVR client builds):
 *  - WMCYN_RUNTIME_SERVER_KEY env var or -WMCYNServerKey=   registration auth
 *  - WMCYN_JOIN_TICKET_SECRET env var or -WMCYNTicketSecret= ticket HMAC secret
 *  - -WMCYNPublicHost=  internet-reachable DNS name or IP (required)
 *  - -WMCYNPublicPort=  published UDP port (default 7777)
 *  - -WMCYNRegion=      region label (default us-east)
 *  - -WMCYNBackendUrl=  backend base URL override
 *  - -WMCYNBuildId=     build id override (default first-signal-001)
 */
UCLASS()
class WMCYNRUNTIME_API UWMCYNWorldRuntimeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category = "WMCYN|WorldRuntime")
    bool IsRuntimeRegistrationActive() const { return bActive; }

    UFUNCTION(BlueprintPure, Category = "WMCYN|WorldRuntime")
    FString GetRuntimeId() const { return RuntimeId; }

private:
    bool ResolveConfiguration();
    void SendRegister();
    void SendHeartbeat(const FString& Status);
    FString ComputeStatus() const;
    int32 CountConnectedUsers() const;

    void HandlePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
    void HandleLogout(AGameModeBase* GameMode, AController* Exiting);
    void KickInvalidLogin(AGameModeBase* GameMode, APlayerController* Player, const FString& Reason);
    void PruneUsedTickets();

    bool bActive = false;
    bool bRegistered = false;
    bool bReportedOnline = false;

    FString RuntimeId;
    FString BackendBaseUrl = TEXT("https://us-central1-wmcyn-online-mobile.cloudfunctions.net/api");
    FString ServerKey;
    FString TicketSecret;
    FString PublicHost;
    int32 PublicPort = 7777;
    FString Region = TEXT("us-east");
    FString BuildId;

    FTSTicker::FDelegateHandle HeartbeatTickerHandle;
    FDelegateHandle PostLoginHandle;
    FDelegateHandle LogoutHandle;

    /** Replay resistance: ticket ids consumed this process, with expiry. */
    TMap<FString, int64> UsedTicketExpiry;

    /** slot -> uid for currently connected, ticket-validated users. */
    TMap<FString, FString> OccupiedSlots;

    /** Controller -> slot, so logout can release occupancy. */
    TMap<FObjectKey, FString> ControllerSlots;
};
