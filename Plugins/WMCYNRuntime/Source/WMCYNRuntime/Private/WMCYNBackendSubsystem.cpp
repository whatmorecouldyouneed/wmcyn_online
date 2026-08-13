#include "WMCYNBackendSubsystem.h"

#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HAL/PlatformMisc.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Parse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "WMCYNFirstSignalWorldContract.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNBackend, Log, All);

namespace
{
bool ParseJsonObject(const FString& Body, TSharedPtr<FJsonObject>& OutObject)
{
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
    return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
}

FString ReadString(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
{
    FString Value;
    return Object.IsValid() && Object->TryGetStringField(Field, Value) ? Value : FString();
}

int32 ReadInt32(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, const int32 DefaultValue = 0)
{
    double Value = 0.0;
    return Object.IsValid() && Object->TryGetNumberField(Field, Value)
        ? FMath::RoundToInt(Value)
        : DefaultValue;
}
}

void UWMCYNBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString CommandLineUrl;
    if (FParse::Value(FCommandLine::Get(), TEXT("WMCYNBackendUrl="), CommandLineUrl))
    {
        SetBackendBaseUrl(CommandLineUrl);
    }

    FString CommandLineWebApiKey;
    if (FParse::Value(FCommandLine::Get(), TEXT("WMCYNWebApiKey="), CommandLineWebApiKey))
    {
        SetFirebaseWebApiKey(CommandLineWebApiKey);
    }
    else
    {
        FString ConfiguredWebApiKey;
        if (GConfig && GConfig->GetString(TEXT("WMCYN"), TEXT("FirebaseWebApiKey"), ConfiguredWebApiKey, GGameIni))
        {
            SetFirebaseWebApiKey(ConfiguredWebApiKey);
        }
    }
}

void UWMCYNBackendSubsystem::LoginAndLoadBootstrap(const FString& Identifier, const FString& Password)
{
    if (LoginState == EWMCYNBackendLoginState::Authenticating ||
        LoginState == EWMCYNBackendLoginState::LoadingBootstrap)
    {
        return;
    }

    const FString CleanIdentifier = Identifier.TrimStartAndEnd();
    if (CleanIdentifier.IsEmpty() || Password.Len() < 6)
    {
        Fail(TEXT("invalid_credentials"));
        return;
    }

    SignOut();
    SetState(EWMCYNBackendLoginState::Authenticating, TEXT("authenticating"));

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetStringField(TEXT("identifier"), CleanIdentifier);
    Payload->SetStringField(TEXT("password"), Password);

    FString Body;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Payload, Writer);

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(MakeUrl(TEXT("/v1/auth/login")));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (!bSucceeded || !Response.IsValid())
            {
                Fail(TEXT("backend_unreachable"));
                return;
            }

            TSharedPtr<FJsonObject> Root;
            if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300 ||
                !ParseJsonObject(Response->GetContentAsString(), Root))
            {
                TSharedPtr<FJsonObject> ErrorRoot;
                ParseJsonObject(Response->GetContentAsString(), ErrorRoot);
                const FString Error = ReadString(ErrorRoot, TEXT("error"));
                Fail(Error.IsEmpty() ? TEXT("login_failed") : Error);
                return;
            }

            const TSharedPtr<FJsonObject>* Auth = nullptr;
            const TSharedPtr<FJsonObject>* Identity = nullptr;
            if (!Root->TryGetObjectField(TEXT("auth"), Auth) || !Auth ||
                !Root->TryGetObjectField(TEXT("identity"), Identity) || !Identity)
            {
                Fail(TEXT("invalid_login_response"));
                return;
            }

            IdToken = ReadString(*Auth, TEXT("idToken"));
            RefreshToken = ReadString(*Auth, TEXT("refreshToken"));
            VerifiedUserId = ReadString(*Identity, TEXT("uid"));
            VerifiedUsername = ReadString(*Identity, TEXT("username"));
            VerifiedDisplayName = ReadString(*Identity, TEXT("displayName"));
            if (IdToken.IsEmpty() || VerifiedUserId.IsEmpty())
            {
                Fail(TEXT("invalid_login_response"));
                return;
            }

            RequestBootstrap();
        });

    if (!Request->ProcessRequest())
    {
        Fail(TEXT("request_start_failed"));
    }
}

void UWMCYNBackendSubsystem::SignInWithCustomTokenAndLoadBootstrap(const FString& CustomToken)
{
    if (LoginState == EWMCYNBackendLoginState::Authenticating ||
        LoginState == EWMCYNBackendLoginState::LoadingBootstrap)
    {
        return;
    }

    const FString CleanCustomToken = CustomToken.TrimStartAndEnd();
    if (CleanCustomToken.IsEmpty())
    {
        Fail(TEXT("invalid_pairing_token"));
        return;
    }

    const FString WebApiKey = ResolveFirebaseWebApiKey();
    if (WebApiKey.IsEmpty())
    {
        Fail(TEXT("firebase_web_api_key_missing"));
        return;
    }

    SignOut();
    SetState(EWMCYNBackendLoginState::Authenticating, TEXT("exchanging_pairing_code"));

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetStringField(TEXT("token"), CleanCustomToken);
    Payload->SetBoolField(TEXT("returnSecureToken"), true);

    FString Body;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Payload, Writer);

    const FString Url = FString::Printf(
        TEXT("https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key=%s"),
        *FGenericPlatformHttp::UrlEncode(WebApiKey));

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (!bSucceeded || !Response.IsValid())
            {
                Fail(TEXT("backend_unreachable"));
                return;
            }

            TSharedPtr<FJsonObject> Root;
            if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300 ||
                !ParseJsonObject(Response->GetContentAsString(), Root))
            {
                TSharedPtr<FJsonObject> ErrorRoot;
                ParseJsonObject(Response->GetContentAsString(), ErrorRoot);
                const FString Error = ReadString(ErrorRoot, TEXT("error"));
                Fail(Error.IsEmpty() ? TEXT("pairing_sign_in_failed") : Error);
                return;
            }

            IdToken = ReadString(Root, TEXT("idToken"));
            RefreshToken = ReadString(Root, TEXT("refreshToken"));
            VerifiedUserId = ReadString(Root, TEXT("localId"));
            if (IdToken.IsEmpty() || VerifiedUserId.IsEmpty())
            {
                Fail(TEXT("invalid_pairing_response"));
                return;
            }

            RequestBootstrap();
        });

    if (!Request->ProcessRequest())
    {
        Fail(TEXT("request_start_failed"));
    }
}

void UWMCYNBackendSubsystem::RequestBootstrap()
{
    SetState(EWMCYNBackendLoginState::LoadingBootstrap, TEXT("loading_world"));

    const FString Query = FString::Printf(
        TEXT("/v1/vr/bootstrap?buildId=%s&protocolVersion=%d&mode=%s"),
        *FGenericPlatformHttp::UrlEncode(WMCYNFirstSignalWorldContract::GetBuildId()),
        WMCYNFirstSignalWorldContract::ProtocolVersion,
        *WMCYNFirstSignalWorldContract::GetPresenceModeHint());

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(MakeUrl(Query));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *IdToken));
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (!bSucceeded || !Response.IsValid())
            {
                Fail(TEXT("backend_unreachable"));
                return;
            }

            TSharedPtr<FJsonObject> Root;
            if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300 ||
                !ParseJsonObject(Response->GetContentAsString(), Root))
            {
                TSharedPtr<FJsonObject> ErrorRoot;
                ParseJsonObject(Response->GetContentAsString(), ErrorRoot);
                const FString Error = ReadString(ErrorRoot, TEXT("error"));
                Fail(Error.IsEmpty() ? TEXT("bootstrap_failed") : Error);
                return;
            }

            const TSharedPtr<FJsonObject>* Identity = nullptr;
            if (Root->TryGetObjectField(TEXT("identity"), Identity) && Identity)
            {
                VerifiedUserId = ReadString(*Identity, TEXT("uid"));
                VerifiedUsername = ReadString(*Identity, TEXT("username"));
                VerifiedDisplayName = ReadString(*Identity, TEXT("displayName"));
            }

            const TSharedPtr<FJsonObject>* World = nullptr;
            if (!Root->TryGetObjectField(TEXT("world"), World) || !World)
            {
                Fail(TEXT("invalid_bootstrap_response"));
                return;
            }

            WorldId = ReadString(*World, TEXT("worldId"));
            WorldRuntimeId = ReadString(*World, TEXT("runtimeId"));
            WorldHost = ReadString(*World, TEXT("host"));
            WorldBuildId = ReadString(*World, TEXT("buildId"));
            double PortValue = 0.0;
            WorldPort = (*World)->TryGetNumberField(TEXT("port"), PortValue) ?
                FMath::RoundToInt(PortValue) : 0;
            double ProtocolValue = 0.0;
            WorldProtocolVersion = (*World)->TryGetNumberField(TEXT("protocolVersion"), ProtocolValue) ?
                FMath::RoundToInt(ProtocolValue) : 0;
            if (WorldHost.IsEmpty() || WorldPort <= 0 || WorldRuntimeId.IsEmpty())
            {
                Fail(TEXT("world_offline"));
                return;
            }

            const TSharedPtr<FJsonObject>* Presence = nullptr;
            PresenceCapabilities.Reset();
            if (Root->TryGetObjectField(TEXT("presence"), Presence) && Presence)
            {
                PresenceMode = ReadString(*Presence, TEXT("mode"));
                PresenceSlot = ReadString(*Presence, TEXT("slot"));
                const TArray<TSharedPtr<FJsonValue>>* Capabilities = nullptr;
                if ((*Presence)->TryGetArrayField(TEXT("capabilities"), Capabilities) && Capabilities)
                {
                    for (const TSharedPtr<FJsonValue>& Value : *Capabilities)
                    {
                        FString Capability;
                        if (Value.IsValid() && Value->TryGetString(Capability))
                        {
                            PresenceCapabilities.Add(Capability);
                        }
                    }
                }
            }

            JoinTicket = ReadString(Root, TEXT("joinTicket"));
            if (JoinTicket.IsEmpty())
            {
                Fail(TEXT("missing_join_ticket"));
                return;
            }

            SetState(EWMCYNBackendLoginState::Ready, TEXT("ready"));
            OnLoginReady.Broadcast(VerifiedUsername, VerifiedDisplayName, WorldHost, WorldPort);
        });

    if (!Request->ProcessRequest())
    {
        Fail(TEXT("request_start_failed"));
    }
}

void UWMCYNBackendSubsystem::SignOut()
{
    IdToken.Reset();
    RefreshToken.Reset();
    VerifiedUserId.Reset();
    VerifiedUsername.Reset();
    VerifiedDisplayName.Reset();
    WorldId.Reset();
    WorldRuntimeId.Reset();
    WorldHost.Reset();
    WorldPort = 0;
    WorldBuildId.Reset();
    WorldProtocolVersion = 0;
    PresenceMode.Reset();
    PresenceSlot.Reset();
    PresenceCapabilities.Reset();
    JoinTicket.Reset();
    SetState(EWMCYNBackendLoginState::SignedOut, TEXT("signed_out"));
}

FString UWMCYNBackendSubsystem::GetBackendBaseUrl() const
{
    return BackendBaseUrl;
}

FString UWMCYNBackendSubsystem::GetWorldTravelURL() const
{
    if (!IsReadyToEnterWorld())
    {
        return FString();
    }
    FString Url = FString::Printf(TEXT("%s:%d"), *WorldHost, WorldPort);
    if (!JoinTicket.IsEmpty())
    {
        Url += FString::Printf(TEXT("?%s=%s"),
            WMCYNFirstSignalWorldContract::JoinTicketOption, *JoinTicket);
    }
    return Url;
}

bool UWMCYNBackendSubsystem::IsReadyToEnterWorld() const
{
    return LoginState == EWMCYNBackendLoginState::Ready && !WorldHost.IsEmpty() && WorldPort > 0;
}

bool UWMCYNBackendSubsystem::TravelToFirstSignalWorld()
{
    const FString Url = GetWorldTravelURL();
    if (Url.IsEmpty())
    {
        return false;
    }

    UGameInstance* GameInstance = GetGameInstance();
    APlayerController* PlayerController = GameInstance ? GameInstance->GetFirstLocalPlayerController() : nullptr;
    if (!PlayerController)
    {
        return false;
    }

    PlayerController->ClientTravel(Url, ETravelType::TRAVEL_Absolute);

    // The ticket is single-use: clear it so a later retry goes back through
    // bootstrap for a fresh one instead of replaying this ticket.
    JoinTicket.Reset();
    return true;
}

void UWMCYNBackendSubsystem::RequestAvatarManifest(FWMCYNAvatarManifestCallback&& Callback)
{
    FWMCYNAvatarManifest EmptyManifest;
    if (IdToken.IsEmpty())
    {
        Callback.ExecuteIfBound(false, EmptyManifest, TEXT("missing_auth_token"));
        return;
    }

    TSharedRef<FWMCYNAvatarManifestCallback> Completion = MakeShared<FWMCYNAvatarManifestCallback>(MoveTemp(Callback));

    const FString Platform = ResolveAvatarManifestPlatform();
    const FString Query = FString::Printf(
        TEXT("/v1/avatar/manifest?platform=%s"),
        *FGenericPlatformHttp::UrlEncode(Platform));

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(MakeUrl(Query));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *IdToken));
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [Completion](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            FWMCYNAvatarManifest Manifest;
            if (!bSucceeded || !Response.IsValid())
            {
                Completion->ExecuteIfBound(false, Manifest, TEXT("backend_unreachable"));
                return;
            }

            TSharedPtr<FJsonObject> Root;
            if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300 ||
                !ParseJsonObject(Response->GetContentAsString(), Root))
            {
                TSharedPtr<FJsonObject> ErrorRoot;
                ParseJsonObject(Response->GetContentAsString(), ErrorRoot);
                Completion->ExecuteIfBound(false, Manifest, ReadString(ErrorRoot, TEXT("error")));
                return;
            }

            Manifest.AvatarId = ReadString(Root, TEXT("avatarId"));
            Manifest.AvatarVersion = ReadInt32(Root, TEXT("avatarVersion"), 1);
            Manifest.Platform = ReadString(Root, TEXT("platform"));
            Manifest.bIsDefaultAvatar = Root->GetBoolField(TEXT("isDefaultAvatar"));
            Manifest.DisplayName = ReadString(Root, TEXT("displayName"));
            Manifest.CacheKey = ReadString(Root, TEXT("cacheKey"));

            const TSharedPtr<FJsonObject>* Delivery = nullptr;
            if (Root->TryGetObjectField(TEXT("delivery"), Delivery) && Delivery)
            {
                Manifest.DeliveryType = ReadString(*Delivery, TEXT("type"));
                Manifest.DeliveryUrl = ReadString(*Delivery, TEXT("url"));
                Manifest.DeliveryAssetRoot = ReadString(*Delivery, TEXT("assetRoot"));
                Manifest.DeliverySkeletalMeshPath = ReadString(*Delivery, TEXT("skeletalMeshPath"));
                Manifest.DeliveryHeadSkeletalMeshPath = ReadString(*Delivery, TEXT("headSkeletalMeshPath"));
                Manifest.DeliveryAnimClassPath = ReadString(*Delivery, TEXT("animClassPath"));
            }

            Completion->ExecuteIfBound(true, Manifest, FString());
        });

    if (!Request->ProcessRequest())
    {
        Completion->ExecuteIfBound(false, EmptyManifest, TEXT("request_start_failed"));
    }
}

void UWMCYNBackendSubsystem::SetBackendBaseUrl(const FString& InBaseUrl)
{
    BackendBaseUrl = InBaseUrl.TrimStartAndEnd();
    while (BackendBaseUrl.EndsWith(TEXT("/")))
    {
        BackendBaseUrl.LeftChopInline(1);
    }
}

void UWMCYNBackendSubsystem::SetFirebaseWebApiKey(const FString& InFirebaseWebApiKey)
{
    FirebaseWebApiKey = InFirebaseWebApiKey.TrimStartAndEnd();
}

void UWMCYNBackendSubsystem::SetState(EWMCYNBackendLoginState NewState, const FString& Message)
{
    LoginState = NewState;
    UE_LOG(LogWMCYNBackend, Display, TEXT("WMCYN Backend: %s"), *Message);
    OnLoginStateChanged.Broadcast(LoginState, Message);
}

void UWMCYNBackendSubsystem::Fail(const FString& ErrorCode)
{
    UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN Backend: %s"), *ErrorCode);
    LoginState = EWMCYNBackendLoginState::Failed;
    OnLoginStateChanged.Broadcast(LoginState, ErrorCode);
}

FString UWMCYNBackendSubsystem::MakeUrl(const FString& Path) const
{
    return BackendBaseUrl + Path;
}

FString UWMCYNBackendSubsystem::ResolveFirebaseWebApiKey() const
{
    if (!FirebaseWebApiKey.IsEmpty())
    {
        return FirebaseWebApiKey;
    }

    const FString EnvironmentValue = FPlatformMisc::GetEnvironmentVariable(TEXT("WMCYN_WEB_API_KEY"));
    if (!EnvironmentValue.IsEmpty())
    {
        return EnvironmentValue;
    }

    return FString();
}

FString UWMCYNBackendSubsystem::ResolveAvatarManifestPlatform() const
{
    if (PresenceMode.Equals(TEXT("Quest"), ESearchCase::IgnoreCase))
    {
        return TEXT("quest");
    }

    if (PresenceMode.Equals(TEXT("PCVR"), ESearchCase::IgnoreCase))
    {
        return TEXT("pcvr");
    }

#if PLATFORM_ANDROID
    return TEXT("quest");
#else
    return TEXT("pcvr");
#endif
}
