#include "WMCYNWorldRuntimeSubsystem.h"
#include "WMCYNAccountAvatarCatalog.h"
#include "WMCYNIdentityPolicy.h"

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
#include "GenericPlatform/GenericPlatformHttp.h"
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

    const FString& RequestUrl = Connection->RequestURL;
    FString Ticket;
    if (const int32 OptionsIndex = RequestUrl.Find(TEXT("?")); OptionsIndex != INDEX_NONE)
    {
        Ticket = UGameplayStatics::ParseOption(
            RequestUrl.Mid(OptionsIndex), WMCYNFirstSignalWorldContract::JoinTicketOption);
    }

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

    if (!WMCYNIdentityPolicy::IsUsableName(Payload.Username) ||
        !WMCYNIdentityPolicy::IsUsableName(Payload.DisplayName))
    {
        KickInvalidLogin(GameMode, NewPlayer, TEXT("signed profile identity missing or invalid"));
        return;
    }
    UsedTicketExpiry.Add(Payload.TicketId, Payload.ExpiresAtUnix);
    OccupiedSlots.Add(Payload.Slot, Payload.Uid);
    ControllerSlots.Add(FObjectKey(NewPlayer), Payload.Slot);
    VerifiedControllerUsers.Add(FObjectKey(NewPlayer), Payload.Uid);
    VerifiedControllerIdentities.Add(FObjectKey(NewPlayer), Payload);
    if (NewPlayer->PlayerState)
    {
        NewPlayer->PlayerState->SetPlayerName(Payload.DisplayName);
        NewPlayer->PlayerState->ForceNetUpdate();
    }
    VerifiedControllerPlatforms.Add(FObjectKey(NewPlayer),
        Payload.Mode == TEXT("PCVR") ? TEXT("pcvr") : TEXT("quest"));

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
    VerifiedControllerUsers.Remove(FObjectKey(Exiting));
    VerifiedControllerPlatforms.Remove(FObjectKey(Exiting));
    VerifiedControllerIdentities.Remove(FObjectKey(Exiting));
    if (ControllerSlots.RemoveAndCopyValue(FObjectKey(Exiting), Slot))
    {
        OccupiedSlots.Remove(Slot);
        UE_LOG(LogWMCYNWorldRuntime, Display,
            TEXT("WMCYN WorldRuntime: released slot %s on logout."), *Slot);
    }
}

const FWMCYNJoinTicketPayload* UWMCYNWorldRuntimeSubsystem::GetVerifiedIdentity(const AController* Controller) const
{
    return bActive && IsValid(Controller) && Controller->HasAuthority()
        ? VerifiedControllerIdentities.Find(FObjectKey(Controller)) : nullptr;
}

void UWMCYNWorldRuntimeSubsystem::RequestAccountAvatar(
    APlayerController* Controller, FWMCYNAvatarManifestCallback&& Callback)
{
    const FObjectKey Key(Controller);
    const FString* User = VerifiedControllerUsers.Find(Key);
    const FString* Platform = VerifiedControllerPlatforms.Find(Key);
    if (!bActive || !IsValid(Controller) || !Controller->HasAuthority() || !User || !Platform)
    {
        Callback.ExecuteIfBound(false, FWMCYNAvatarManifest(), TEXT("ticket_identity_unavailable"));
        return;
    }

    const FString RequestedUser = *User;
    const FString RequestedPlatform = *Platform;
    const TWeakObjectPtr<APlayerController> WeakController(Controller);
    const auto Completion = MakeShared<FWMCYNAvatarManifestCallback>(MoveTemp(Callback));
    const auto Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendBaseUrl + TEXT("/v1/vr/runtime/avatar?uid=") +
        FGenericPlatformHttp::UrlEncode(RequestedUser) + TEXT("&platform=") + RequestedPlatform);
    Request->SetVerb(TEXT("GET"));
    Request->SetTimeout(15.0f);
    Request->SetHeader(TEXT("x-wmcyn-server-key"), ServerKey);
    Request->OnProcessRequestComplete().BindWeakLambda(this,
        [this, Key, WeakController, RequestedUser, RequestedPlatform, Completion]
        (FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            FWMCYNAvatarManifest Manifest;
            const FString* CurrentUser = VerifiedControllerUsers.Find(Key);
            if (!bActive || !WeakController.IsValid() || !CurrentUser || *CurrentUser != RequestedUser)
            {
                Completion->ExecuteIfBound(false, Manifest, TEXT("ticket_identity_changed"));
                return;
            }
            TSharedPtr<FJsonObject> Root;
            if (!bSucceeded || !Response.IsValid() || Response->GetResponseCode() != 200 ||
                !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Root) ||
                !Root.IsValid())
            {
                UE_LOG(LogWMCYNWorldRuntime, Warning,
                    TEXT("WMCYN_AVATAR stage=server_manifest result=failure status=%d"),
                    Response.IsValid() ? Response->GetResponseCode() : 0);
                Completion->ExecuteIfBound(false, Manifest, TEXT("server_manifest_failed"));
                return;
            }
            Root->TryGetStringField(TEXT("platform"), Manifest.Platform);
            if (Manifest.Platform != RequestedPlatform ||
                !Root->TryGetBoolField(TEXT("isDefaultAvatar"), Manifest.bIsDefaultAvatar))
            {
                Completion->ExecuteIfBound(false, Manifest, TEXT("manifest_contract_invalid"));
                return;
            }
            Root->TryGetStringField(TEXT("cacheKey"), Manifest.CacheKey);
            const TSharedPtr<FJsonObject>* Delivery = nullptr;
            if (Root->TryGetObjectField(TEXT("delivery"), Delivery) && Delivery && Delivery->IsValid())
            {
                (*Delivery)->TryGetStringField(TEXT("type"), Manifest.DeliveryType);
                (*Delivery)->TryGetStringField(TEXT("skeletalMeshPath"), Manifest.DeliverySkeletalMeshPath);
                (*Delivery)->TryGetStringField(TEXT("headSkeletalMeshPath"), Manifest.DeliveryHeadSkeletalMeshPath);
                (*Delivery)->TryGetStringField(TEXT("animClassPath"), Manifest.DeliveryAnimClassPath);
            }
            if (!Manifest.bIsDefaultAvatar && (Manifest.DeliveryType != TEXT("runtime_mesh") ||
                Manifest.DeliverySkeletalMeshPath.IsEmpty() || Manifest.CacheKey.IsEmpty()))
            {
                Completion->ExecuteIfBound(false, Manifest, TEXT("manifest_contract_invalid"));
                return;
            }
            if (Manifest.bIsDefaultAvatar &&
                WMCYNAccountAvatarCatalog::Resolve(RequestedUser, RequestedPlatform, Manifest))
            {
                UE_LOG(LogWMCYNWorldRuntime, Display,
                    TEXT("WMCYN_AVATAR stage=account_catalog result=selected authority=verified_ticket"));
            }
            UE_LOG(LogWMCYNWorldRuntime, Display,
                TEXT("WMCYN_AVATAR stage=server_manifest result=success source=%s"),
                Manifest.bIsDefaultAvatar ? TEXT("default") : TEXT("published"));
            Completion->ExecuteIfBound(true, Manifest, FString());
        });
    if (!Request->ProcessRequest())
    {
        Completion->ExecuteIfBound(false, FWMCYNAvatarManifest(), TEXT("request_start_failed"));
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
