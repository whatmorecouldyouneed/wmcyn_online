#include "WMCYNWorldRuntimeSubsystem.h"

#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"
#include "Misc/Parse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "WMCYNFirstSignalWorldContract.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNWorldRuntime, Log, All);

namespace
{
FString ReadSecret(const TCHAR* EnvironmentName, const TCHAR* CommandLineKey)
{
    FString Value;
    if (FParse::Value(FCommandLine::Get(), CommandLineKey, Value) && !Value.IsEmpty())
    {
        return Value;
    }
    return FPlatformMisc::GetEnvironmentVariable(EnvironmentName);
}

FString ProductionMapName()
{
    return TEXT("L_WMCYNOnline");
}
}

namespace WMCYNFirstSignalWorldContract
{
FString GetBuildId()
{
    FString Override;
    if (FParse::Value(FCommandLine::Get(), TEXT("WMCYNBuildId="), Override) && !Override.IsEmpty())
    {
        return Override;
    }
    return DefaultBuildId;
}

FString GetPresenceModeHint()
{
    FString Override;
    if (FParse::Value(FCommandLine::Get(), TEXT("WMCYNPresenceMode="), Override))
    {
        if (Override == TEXT("Quest") || Override == TEXT("PCVR"))
        {
            return Override;
        }
    }
#if PLATFORM_ANDROID
    return TEXT("Quest");
#else
    return TEXT("PCVR");
#endif
}
}

void UWMCYNWorldRuntimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (!FParse::Param(FCommandLine::Get(), TEXT("WMCYNRegisterRuntime")))
    {
        return;
    }
    if (!ResolveConfiguration())
    {
        UE_LOG(LogWMCYNWorldRuntime, Error,
            TEXT("WMCYN WorldRuntime: -WMCYNRegisterRuntime is set but configuration is incomplete. ")
            TEXT("The runtime will NOT register and remote logins will NOT be accepted."));
        return;
    }

    bActive = true;
    RuntimeId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
    UE_LOG(LogWMCYNWorldRuntime, Display,
        TEXT("WMCYN WorldRuntime: registering runtime %s for %s at %s:%d (build %s, protocol %d)"),
        *RuntimeId, WMCYNFirstSignalWorldContract::WorldId, *PublicHost, PublicPort,
        *BuildId, WMCYNFirstSignalWorldContract::ProtocolVersion);

    SendRegister();

    HeartbeatTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateWeakLambda(this, [this](float)
        {
            SendHeartbeat(ComputeStatus());
            PruneUsedTickets();
            return true;
        }),
        WMCYNFirstSignalWorldContract::HeartbeatIntervalSeconds);

    PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(
        this, &UWMCYNWorldRuntimeSubsystem::HandlePostLogin);
    LogoutHandle = FGameModeEvents::GameModeLogoutEvent.AddUObject(
        this, &UWMCYNWorldRuntimeSubsystem::HandleLogout);
}

void UWMCYNWorldRuntimeSubsystem::Deinitialize()
{
    if (bActive)
    {
        if (HeartbeatTickerHandle.IsValid())
        {
            FTSTicker::GetCoreTicker().RemoveTicker(HeartbeatTickerHandle);
        }
        FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);
        FGameModeEvents::GameModeLogoutEvent.Remove(LogoutHandle);

        // Best-effort graceful shutdown; a crash is covered by lease expiry.
        SendHeartbeat(TEXT("offline"));
        bActive = false;
    }

    Super::Deinitialize();
}

bool UWMCYNWorldRuntimeSubsystem::ResolveConfiguration()
{
    FString UrlOverride;
    if (FParse::Value(FCommandLine::Get(), TEXT("WMCYNBackendUrl="), UrlOverride) && !UrlOverride.IsEmpty())
    {
        BackendBaseUrl = UrlOverride;
        while (BackendBaseUrl.EndsWith(TEXT("/")))
        {
            BackendBaseUrl.LeftChopInline(1);
        }
    }

    ServerKey = ReadSecret(TEXT("WMCYN_RUNTIME_SERVER_KEY"), TEXT("WMCYNServerKey="));
    TicketSecret = ReadSecret(TEXT("WMCYN_JOIN_TICKET_SECRET"), TEXT("WMCYNTicketSecret="));
    FParse::Value(FCommandLine::Get(), TEXT("WMCYNPublicHost="), PublicHost);
    FParse::Value(FCommandLine::Get(), TEXT("WMCYNPublicPort="), PublicPort);
    FParse::Value(FCommandLine::Get(), TEXT("WMCYNRegion="), Region);
    BuildId = WMCYNFirstSignalWorldContract::GetBuildId();

    if (ServerKey.Len() < 32)
    {
        UE_LOG(LogWMCYNWorldRuntime, Error, TEXT("WMCYN WorldRuntime: server key missing or under 32 chars."));
        return false;
    }
    if (TicketSecret.Len() < 32)
    {
        UE_LOG(LogWMCYNWorldRuntime, Error, TEXT("WMCYN WorldRuntime: join ticket secret missing or under 32 chars."));
        return false;
    }
    if (PublicHost.IsEmpty())
    {
        UE_LOG(LogWMCYNWorldRuntime, Error, TEXT("WMCYN WorldRuntime: -WMCYNPublicHost= is required."));
        return false;
    }
    if (PublicPort <= 0 || PublicPort > 65535)
    {
        UE_LOG(LogWMCYNWorldRuntime, Error, TEXT("WMCYN WorldRuntime: invalid -WMCYNPublicPort=%d."), PublicPort);
        return false;
    }
    return true;
}

FString UWMCYNWorldRuntimeSubsystem::ComputeStatus() const
{
    const UGameInstance* GameInstance = GetGameInstance();
    const UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;
    if (World && World->HasBegunPlay() &&
        UGameplayStatics::GetCurrentLevelName(World) == ProductionMapName())
    {
        return TEXT("online");
    }
    return TEXT("starting");
}

int32 UWMCYNWorldRuntimeSubsystem::CountConnectedUsers() const
{
    const UGameInstance* GameInstance = GetGameInstance();
    const UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;
    const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
    if (!GameState)
    {
        return 0;
    }

    int32 Count = 0;
    for (const APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (PlayerState && !PlayerState->IsABot() && !PlayerState->IsInactive())
        {
            ++Count;
        }
    }
    return Count;
}

void UWMCYNWorldRuntimeSubsystem::SendRegister()
{
    const TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetStringField(TEXT("worldId"), WMCYNFirstSignalWorldContract::WorldId);
    Payload->SetStringField(TEXT("locationId"), WMCYNFirstSignalWorldContract::LocationId);
    Payload->SetStringField(TEXT("runtimeId"), RuntimeId);
    Payload->SetStringField(TEXT("host"), PublicHost);
    Payload->SetNumberField(TEXT("port"), PublicPort);
    Payload->SetStringField(TEXT("region"), Region);
    Payload->SetStringField(TEXT("buildId"), BuildId);
    Payload->SetNumberField(TEXT("protocolVersion"), WMCYNFirstSignalWorldContract::ProtocolVersion);
    Payload->SetNumberField(TEXT("capacity"), WMCYNFirstSignalWorldContract::Capacity);
    Payload->SetStringField(TEXT("status"), ComputeStatus());

    FString Body;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Payload, Writer);

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendBaseUrl + TEXT("/v1/vr/runtime/register"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("x-wmcyn-server-key"), ServerKey);
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            const int32 Code = bSucceeded && Response.IsValid() ? Response->GetResponseCode() : 0;
            bRegistered = Code >= 200 && Code < 300;
            UE_LOG(LogWMCYNWorldRuntime, Display,
                TEXT("WMCYN WorldRuntime: register response %d%s"), Code,
                bRegistered ? TEXT("") : TEXT(" (will retry on next heartbeat)"));
        });
    Request->ProcessRequest();
}

void UWMCYNWorldRuntimeSubsystem::SendHeartbeat(const FString& Status)
{
    if (!bActive)
    {
        return;
    }
    // Registration may have failed transiently; re-register instead of
    // heartbeating a record that does not carry this runtimeId.
    if (!bRegistered)
    {
        SendRegister();
        return;
    }

    const TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetStringField(TEXT("runtimeId"), RuntimeId);
    Payload->SetStringField(TEXT("status"), Status);
    Payload->SetNumberField(TEXT("connectedUsers"), CountConnectedUsers());

    const TSharedRef<FJsonObject> Occupied = MakeShared<FJsonObject>();
    for (const TPair<FString, FString>& Pair : OccupiedSlots)
    {
        Occupied->SetStringField(Pair.Key, Pair.Value);
    }
    Payload->SetObjectField(TEXT("occupiedSlots"), Occupied);

    FString Body;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Payload, Writer);

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendBaseUrl + TEXT("/v1/vr/runtime/heartbeat"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("x-wmcyn-server-key"), ServerKey);
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this, Status](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            const int32 Code = bSucceeded && Response.IsValid() ? Response->GetResponseCode() : 0;
            if (Code == 409)
            {
                // A newer process owns the singleton record. Do not fight it.
                UE_LOG(LogWMCYNWorldRuntime, Warning,
                    TEXT("WMCYN WorldRuntime: heartbeat rejected as stale; this runtime has been superseded."));
                bRegistered = false;
            }
            else if (Code < 200 || Code >= 300)
            {
                UE_LOG(LogWMCYNWorldRuntime, Warning,
                    TEXT("WMCYN WorldRuntime: heartbeat failed with %d."), Code);
            }
            else if (!bReportedOnline && Status == TEXT("online"))
            {
                bReportedOnline = true;
                UE_LOG(LogWMCYNWorldRuntime, Display, TEXT("WMCYN WorldRuntime: runtime reported online."));
            }
        });
    Request->ProcessRequest();
}

void UWMCYNWorldRuntimeSubsystem::HandlePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
    if (!bActive || !GameMode || !NewPlayer || !GameMode->HasAuthority())
    {
        return;
    }

    UNetConnection* Connection = NewPlayer->GetNetConnection();
    if (!Connection)
    {
        // Local player on a listen host; the deployed runtime is expected to
        // run headless, so only network entrants carry tickets.
        return;
    }

    const FString Ticket = UGameplayStatics::ParseOption(
        Connection->RequestURL, WMCYNFirstSignalWorldContract::JoinTicketOption);
    if (Ticket.IsEmpty())
    {
        KickInvalidLogin(GameMode, NewPlayer, TEXT("missing join ticket"));
        return;
    }

    FWMCYNJoinTicketPayload Payload;
    if (!WMCYNJoinTicket::VerifyJoinTicket(TicketSecret, Ticket, Payload))
    {
        KickInvalidLogin(GameMode, NewPlayer, TEXT("invalid or expired join ticket"));
        return;
    }
    if (Payload.RuntimeId != RuntimeId ||
        Payload.WorldId != WMCYNFirstSignalWorldContract::WorldId ||
        Payload.BuildId != BuildId ||
        Payload.ProtocolVersion != WMCYNFirstSignalWorldContract::ProtocolVersion)
    {
        KickInvalidLogin(GameMode, NewPlayer, TEXT("join ticket does not match this runtime"));
        return;
    }
    if (UsedTicketExpiry.Contains(Payload.TicketId))
    {
        KickInvalidLogin(GameMode, NewPlayer, TEXT("join ticket already used"));
        return;
    }

    UsedTicketExpiry.Add(Payload.TicketId, Payload.ExpiresAtUnix);
    OccupiedSlots.Add(Payload.Slot, Payload.Uid);
    ControllerSlots.Add(FObjectKey(NewPlayer), Payload.Slot);

    UE_LOG(LogWMCYNWorldRuntime, Display,
        TEXT("WMCYN WorldRuntime: validated join ticket for slot %s (mode %s)."),
        *Payload.Slot, *Payload.Mode);
}

void UWMCYNWorldRuntimeSubsystem::HandleLogout(AGameModeBase* GameMode, AController* Exiting)
{
    if (!bActive || !Exiting)
    {
        return;
    }

    FString Slot;
    if (ControllerSlots.RemoveAndCopyValue(FObjectKey(Exiting), Slot))
    {
        OccupiedSlots.Remove(Slot);
        UE_LOG(LogWMCYNWorldRuntime, Display,
            TEXT("WMCYN WorldRuntime: released slot %s on logout."), *Slot);
    }
}

void UWMCYNWorldRuntimeSubsystem::KickInvalidLogin(
    AGameModeBase* GameMode, APlayerController* Player, const FString& Reason)
{
    UE_LOG(LogWMCYNWorldRuntime, Warning,
        TEXT("WMCYN WorldRuntime: rejecting login (%s)."), *Reason);

    if (GameMode->GameSession)
    {
        GameMode->GameSession->KickPlayer(Player, FText::FromString(TEXT("WMCYN: entry not authorized.")));
    }
    else if (UNetConnection* Connection = Player->GetNetConnection())
    {
        Connection->Close();
    }
}

void UWMCYNWorldRuntimeSubsystem::PruneUsedTickets()
{
    const int64 Now = FDateTime::UtcNow().ToUnixTimestamp();
    for (auto It = UsedTicketExpiry.CreateIterator(); It; ++It)
    {
        // Keep entries a minute past expiry; an expired ticket already fails
        // verification, so the replay set only needs to cover the live window.
        if (It->Value + 60 < Now)
        {
            It.RemoveCurrent();
        }
    }
}
