#include "WMCYNBackendSubsystem.h"
#include "WMCYNAccountAvatarCatalog.h"

#include "Dom/JsonObject.h"
#include "WMCYNJsonHelpers.h"
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
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "WMCYNFirstSignalWorldContract.h"
#include "WMCYNSecureSignInStore.h"

DEFINE_LOG_CATEGORY_STATIC(LogWMCYNBackend, Log, All);

using namespace WMCYNJson;

namespace
{
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

void UWMCYNBackendSubsystem::Deinitialize()
{
    ++AuthenticationGeneration;
    IdToken.Reset();
    RefreshToken.Reset();
    JoinTicket.Reset();
    Super::Deinitialize();
}

FString UWMCYNBackendSubsystem::GetCredentialScope() const
{
    return TEXT("WMCYN/SignIn/v1/") + FMD5::HashAnsiString(*(BackendBaseUrl + TEXT("|") + ResolveFirebaseWebApiKey()));
}

bool UWMCYNBackendSubsystem::IsForceLoginTesting() const
{
    return FParse::Param(FCommandLine::Get(), TEXT("WMCYNForceLogin"));
}

bool UWMCYNBackendSubsystem::HasRememberedSignIn() const
{
    if (IsRunningDedicatedServer() || IsForceLoginTesting() || bSuppressRememberedSignIn) return false;
    FString Token, Label;
    return WMCYNSecureSignInStore::Read(GetCredentialScope(), Token, Label);
}

FString UWMCYNBackendSubsystem::GetRememberedDisplayLabel() const
{
    if (!HasRememberedSignIn()) return FString();
    FString Token, Label;
    WMCYNSecureSignInStore::Read(GetCredentialScope(), Token, Label);
    return Label;
}

void UWMCYNBackendSubsystem::SetRememberSignIn(const bool bRemember)
{
    bRememberSignIn = bRemember;
    if (!bRemember)
    {
        bSuppressRememberedSignIn = true;
        bCredentialRemovalFailed = !WMCYNSecureSignInStore::Remove(GetCredentialScope());
        if (bCredentialRemovalFailed)
            UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AUTH stage=credential_delete result=failed"));
    }
    else
    {
        bSuppressRememberedSignIn = false;
        if (LoginState == EWMCYNBackendLoginState::Ready) PersistSignIn();
    }
}

void UWMCYNBackendSubsystem::PersistSignIn()
{
    if (!bRememberSignIn || IsForceLoginTesting() || IsRunningDedicatedServer() || RefreshToken.IsEmpty()) return;
    const bool bSaved = WMCYNSecureSignInStore::Write(GetCredentialScope(), RefreshToken,
        VerifiedDisplayName.IsEmpty() ? VerifiedUsername : VerifiedDisplayName);
    bSuppressRememberedSignIn = !bSaved;
    UE_LOG(LogWMCYNBackend, Display, TEXT("WMCYN_AUTH stage=credential_save result=%s"), bSaved ? TEXT("success") : TEXT("unavailable"));
}

void UWMCYNBackendSubsystem::RestoreSignInAndLoadBootstrap()
{
    if (LoginState == EWMCYNBackendLoginState::Authenticating || LoginState == EWMCYNBackendLoginState::LoadingBootstrap) return;
    FString SavedToken, SavedLabel;
    if (!HasRememberedSignIn() || !WMCYNSecureSignInStore::Read(GetCredentialScope(), SavedToken, SavedLabel))
    {
        Fail(TEXT("remembered_sign_in_unavailable"));
        return;
    }
    const FString ApiKey = ResolveFirebaseWebApiKey();
    if (ApiKey.IsEmpty()) { Fail(TEXT("firebase_web_api_key_missing")); return; }
    const uint64 Generation = ++AuthenticationGeneration;
    IdToken.Reset();
    JoinTicket.Reset();
    VerifiedUserId.Reset();
    VerifiedUsername.Reset();
    VerifiedDisplayName.Reset();
    bRememberSignIn = true;
    SetState(EWMCYNBackendLoginState::Authenticating, TEXT("restoring_sign_in"));
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://securetoken.googleapis.com/v1/token?key=") + FGenericPlatformHttp::UrlEncode(ApiKey));
    Request->SetVerb(TEXT("POST"));
    Request->SetTimeout(15.0f);
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
    Request->SetContentAsString(TEXT("grant_type=refresh_token&refresh_token=") + FGenericPlatformHttp::UrlEncode(SavedToken));
    Request->OnProcessRequestComplete().BindWeakLambda(this,
        [this, Generation, SavedLabel](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (Generation != AuthenticationGeneration) return;
            if (!bSucceeded || !Response.IsValid()) { Fail(TEXT("refresh_unreachable")); return; }
            TSharedPtr<FJsonObject> Root;
            const bool bJson = ParseJsonObject(Response->GetContentAsString(), Root);
            if (Response->GetResponseCode() != 200 || !bJson)
            {
                const TSharedPtr<FJsonObject>* Error = nullptr;
                const FString Code = bJson && Root->TryGetObjectField(TEXT("error"), Error)
                    ? ReadString(*Error, TEXT("message")) : FString();
                if (Code == TEXT("TOKEN_EXPIRED") || Code == TEXT("INVALID_REFRESH_TOKEN") ||
                    Code == TEXT("USER_DISABLED") || Code == TEXT("USER_NOT_FOUND"))
                {
                    SignOut();
                    Fail(TEXT("sign_in_required"));
                }
                else Fail(TEXT("refresh_failed_retry_available"));
                return;
            }
            IdToken = ReadString(Root, TEXT("id_token"));
            RefreshToken = ReadString(Root, TEXT("refresh_token"));
            VerifiedUserId = ReadString(Root, TEXT("user_id"));
            if (IdToken.IsEmpty() || RefreshToken.IsEmpty() || VerifiedUserId.IsEmpty())
            { Fail(TEXT("invalid_refresh_response")); return; }
            // Save rotation before bootstrap: an offline world must not lose the new credential.
            if (bRememberSignIn && !bSuppressRememberedSignIn &&
                !WMCYNSecureSignInStore::Write(GetCredentialScope(), RefreshToken, SavedLabel))
                UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AUTH stage=credential_rotation result=failed"));
            UE_LOG(LogWMCYNBackend, Display, TEXT("WMCYN_AUTH stage=refresh result=success"));
            RequestBootstrap();
        });
    if (!Request->ProcessRequest()) Fail(TEXT("request_start_failed"));
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

    ResetAuthentication(!IsForceLoginTesting());
    SetState(EWMCYNBackendLoginState::Authenticating, TEXT("authenticating"));
    const uint64 Generation = AuthenticationGeneration;

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
        [this, Generation](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (Generation != AuthenticationGeneration) return;
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

    ResetAuthentication(!IsForceLoginTesting());
    SetState(EWMCYNBackendLoginState::Authenticating, TEXT("exchanging_pairing_code"));
    const uint64 Generation = AuthenticationGeneration;

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
        [this, Generation](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (Generation != AuthenticationGeneration) return;
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
    const uint64 Generation = AuthenticationGeneration;
    SetState(EWMCYNBackendLoginState::LoadingBootstrap, TEXT("loading_world"));

    const FString Query = FString::Printf(
        TEXT("/v1/vr/bootstrap?buildId=%s&protocolVersion=%d&mode=%s"),
        *FGenericPlatformHttp::UrlEncode(WMCYNFirstSignalWorldContract::GetBuildId()),
        WMCYNFirstSignalWorldContract::ProtocolVersion,
        *WMCYNFirstSignalWorldContract::GetPresenceModeHint());

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(MakeUrl(Query));
    Request->SetTimeout(15.0f);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *IdToken));
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this, Generation](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (Generation != AuthenticationGeneration) return;
            if (!bSucceeded || !Response.IsValid())
            {
                Fail(TEXT("backend_unreachable"));
                return;
            }

            if (Response->GetResponseCode() == 401)
            {
                SignOut();
                Fail(TEXT("sign_in_required"));
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
            if (!Root->TryGetObjectField(TEXT("identity"), Identity) || !Identity ||
                ReadString(*Identity, TEXT("uid")).IsEmpty() ||
                ReadString(*Identity, TEXT("uid")) != VerifiedUserId)
            {
                Fail(TEXT("bootstrap_identity_mismatch"));
                return;
            }
            VerifiedUsername = ReadString(*Identity, TEXT("username"));
            VerifiedDisplayName = ReadString(*Identity, TEXT("displayName"));

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

            PersistSignIn();
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
    bRememberSignIn = false;
    ResetAuthentication(true);
}

void UWMCYNBackendSubsystem::ResetAuthentication(const bool bForgetSavedCredential)
{
    ++AuthenticationGeneration;
    bSuppressRememberedSignIn = true;
    bCredentialRemovalFailed = bForgetSavedCredential && !WMCYNSecureSignInStore::Remove(GetCredentialScope());
    if (bCredentialRemovalFailed)
        UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AUTH stage=credential_delete result=failed"));
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

    // Dev-only synthetic manifest for workstation validation of the Crib's
    // presentation avatar copies (-WMCYNDevAvatarJared). Packaged and hosted
    // builds always keep the trusted backend as the owner of published
    // account resolution; this override never answers there.
    // RequiresCookedData() is true for any cooked target, so this is false only
    // in an uncooked editor build. FPlatformMisc::IsPackaged() does not exist in
    // UE 5.8 and was failing the whole module build, which is why the override
    // never reached the DLL.
    if (!FPlatformProperties::RequiresCookedData() &&
        FParse::Param(FCommandLine::Get(), TEXT("WMCYNDevAvatarJared")))
    {
        FWMCYNAvatarManifest DevManifest;
        DevManifest.bIsDefaultAvatar = false;
        DevManifest.AvatarId = TEXT("dev-jared-preview");
        DevManifest.AvatarVersion = 1;
        DevManifest.Platform = Platform;
        DevManifest.DisplayName = TEXT("Jared");
        DevManifest.CacheKey = TEXT("dev-jared-preview");
        DevManifest.DeliveryType = TEXT("runtime_mesh");
        DevManifest.DeliverySkeletalMeshPath = TEXT("/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview/SKM_Jared_Body_Preview.SKM_Jared_Body_Preview");
        DevManifest.DeliveryHeadSkeletalMeshPath = TEXT("/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview/SKM_Jared_Head_Preview.SKM_Jared_Head_Preview");
        DevManifest.DeliveryAnimClassPath = TEXT("/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C");
        UE_LOG(LogWMCYNBackend, Display, TEXT("WMCYN_AVATAR stage=manifest_http result=success platform=%s source=dev_override"), *Platform);
        Completion->ExecuteIfBound(true, DevManifest, FString());
        return;
    }

    const FString Query = FString::Printf(
        TEXT("/v1/avatar/manifest?platform=%s"),
        *FGenericPlatformHttp::UrlEncode(Platform));

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(MakeUrl(Query));
    Request->SetTimeout(15.0f);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *IdToken));
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [Completion, Platform, RequestedUid = VerifiedUserId](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            FWMCYNAvatarManifest Manifest;
            if (!bSucceeded || !Response.IsValid())
            {
                UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AVATAR stage=manifest_http result=failure reason=transport"));
                Completion->ExecuteIfBound(false, Manifest, TEXT("backend_unreachable"));
                return;
            }

            TSharedPtr<FJsonObject> Root;
            if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300 ||
                !ParseJsonObject(Response->GetContentAsString(), Root))
            {
                TSharedPtr<FJsonObject> ErrorRoot;
                ParseJsonObject(Response->GetContentAsString(), ErrorRoot);
                UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AVATAR stage=manifest_http result=failure status=%d"), Response->GetResponseCode());
                Completion->ExecuteIfBound(false, Manifest, TEXT("manifest_http_or_json_error"));
                return;
            }

            Manifest.AvatarId = ReadString(Root, TEXT("avatarId"));
            Manifest.AvatarVersion = ReadInt32(Root, TEXT("avatarVersion"), 1);
            Manifest.Platform = ReadString(Root, TEXT("platform"));
            if (!Root->TryGetBoolField(TEXT("isDefaultAvatar"), Manifest.bIsDefaultAvatar) || Manifest.Platform != Platform)
            {
                UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AVATAR stage=manifest_http result=failure reason=invalid_contract"));
                Completion->ExecuteIfBound(false, Manifest, TEXT("manifest_contract_invalid"));
                return;
            }
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

            if (!Manifest.bIsDefaultAvatar && (Manifest.DeliveryType != TEXT("runtime_mesh") || Manifest.DeliverySkeletalMeshPath.IsEmpty()))
            {
                UE_LOG(LogWMCYNBackend, Warning, TEXT("WMCYN_AVATAR stage=manifest_http result=failure reason=precooked_delivery_required"));
                Completion->ExecuteIfBound(false, Manifest, TEXT("precooked_delivery_required"));
                return;
            }
            if (Manifest.bIsDefaultAvatar && WMCYNAccountAvatarCatalog::Resolve(RequestedUid, Platform, Manifest))
            {
                UE_LOG(LogWMCYNBackend, Display, TEXT("WMCYN_AVATAR stage=account_catalog result=selected authority=verified_login"));
            }
            UE_LOG(LogWMCYNBackend, Display, TEXT("WMCYN_AVATAR stage=manifest_http result=success platform=%s source=%s"),
                *Platform, Manifest.bIsDefaultAvatar ? TEXT("default") : TEXT("published"));
            Completion->ExecuteIfBound(true, Manifest, FString());
        });

    if (!Request->ProcessRequest())
    {
        Completion->ExecuteIfBound(false, EmptyManifest, TEXT("request_start_failed"));
    }
}

void UWMCYNBackendSubsystem::SetBackendBaseUrl(const FString& InBaseUrl)
{
    FString NewBaseUrl = InBaseUrl.TrimStartAndEnd();
    while (NewBaseUrl.EndsWith(TEXT("/")))
    {
        NewBaseUrl.LeftChopInline(1);
    }
    if (BackendBaseUrl != NewBaseUrl)
    {
        ResetAuthentication(false);
        BackendBaseUrl = NewBaseUrl;
        bSuppressRememberedSignIn = false;
        bRememberSignIn = false;
    }
}

void UWMCYNBackendSubsystem::SetFirebaseWebApiKey(const FString& InFirebaseWebApiKey)
{
    const FString NewKey = InFirebaseWebApiKey.TrimStartAndEnd();
    if (FirebaseWebApiKey != NewKey)
    {
        ResetAuthentication(false);
        FirebaseWebApiKey = NewKey;
        bSuppressRememberedSignIn = false;
        bRememberSignIn = false;
    }
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
