#include "WMCYNBackendSubsystem.h"

#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

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
}

void UWMCYNBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString CommandLineUrl;
    if (FParse::Value(FCommandLine::Get(), TEXT("WMCYNBackendUrl="), CommandLineUrl))
    {
        SetBackendBaseUrl(CommandLineUrl);
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

void UWMCYNBackendSubsystem::RequestBootstrap()
{
    SetState(EWMCYNBackendLoginState::LoadingBootstrap, TEXT("loading_world"));

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(MakeUrl(TEXT("/v1/vr/bootstrap")));
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
            const FString WorldStatus = ReadString(*World, TEXT("status"));
            if (WorldStatus != TEXT("available") || WorldHost.IsEmpty() || WorldPort <= 0)
            {
                Fail(TEXT("world_offline"));
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
    SetState(EWMCYNBackendLoginState::SignedOut, TEXT("signed_out"));
}

FString UWMCYNBackendSubsystem::GetWorldTravelURL() const
{
    return IsReadyToEnterWorld() ? FString::Printf(TEXT("%s:%d"), *WorldHost, WorldPort) : FString();
}

bool UWMCYNBackendSubsystem::IsReadyToEnterWorld() const
{
    return LoginState == EWMCYNBackendLoginState::Ready && !WorldHost.IsEmpty() && WorldPort > 0;
}

void UWMCYNBackendSubsystem::SetBackendBaseUrl(const FString& InBaseUrl)
{
    BackendBaseUrl = InBaseUrl.TrimStartAndEnd();
    while (BackendBaseUrl.EndsWith(TEXT("/")))
    {
        BackendBaseUrl.LeftChopInline(1);
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
