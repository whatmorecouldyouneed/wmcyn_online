#include "WMCYNFirstSignalPairingAsyncAction.h"

#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "WMCYNFirstSignalBlueprintLibrary.h"
#include "WMCYNWorldRuntimeSubsystem.h"

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

void UWMCYNFirstSignalPairingAsyncAction::PrimeLoginWidget(UUserWidget* InLoginWidget)
{
    if (!InLoginWidget)
    {
        return;
    }

    UWMCYNFirstSignalPairingAsyncAction* PreviewAction = NewObject<UWMCYNFirstSignalPairingAsyncAction>();
    if (!PreviewAction)
    {
        return;
    }

    PreviewAction->LoginWidget = InLoginWidget;
    PreviewAction->EnsurePairingWidget();
    if (PreviewAction->PairingPanel)
    {
        PreviewAction->PairingPanel->SetVisibility(ESlateVisibility::Visible);
    }
}

bool UWMCYNFirstSignalPairingAsyncAction::StartForLoginWidget(
    const UObject* WorldContextObject,
    UUserWidget* InLoginWidget)
{
    if (!WorldContextObject || !InLoginWidget)
    {
        return false;
    }

    UWMCYNFirstSignalPairingAsyncAction* Action = BeginFirstSignalPairing(WorldContextObject);
    if (!Action)
    {
        return false;
    }

    Action->LoginWidget = InLoginWidget;
    Action->Activate();
    return true;
}

UWMCYNFirstSignalPairingAsyncAction* UWMCYNFirstSignalPairingAsyncAction::BeginFirstSignalPairing(
    const UObject* WorldContextObject)
{
    UWMCYNFirstSignalPairingAsyncAction* Action = NewObject<UWMCYNFirstSignalPairingAsyncAction>();
    Action->WorldContext = WorldContextObject;
    if (WorldContextObject)
    {
        Action->RegisterWithGameInstance(WorldContextObject);
    }
    return Action;
}

void UWMCYNFirstSignalPairingAsyncAction::Activate()
{
    if (!WorldContext.IsValid())
    {
        const FString Message = TEXT("WMCYN pairing context is not available.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
        return;
    }

    ApplyPairingWidgetPresentation();

    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext.Get());
    BackendSubsystem = GameInstance ? GameInstance->GetSubsystem<UWMCYNBackendSubsystem>() : nullptr;
    if (!BackendSubsystem)
    {
        const FString Message = TEXT("WMCYN login service is not available.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
        return;
    }

    BackendSubsystem->OnLoginStateChanged.AddDynamic(this, &UWMCYNFirstSignalPairingAsyncAction::HandleLoginStateChanged);
    BackendSubsystem->OnLoginReady.AddDynamic(this, &UWMCYNFirstSignalPairingAsyncAction::HandleLoginReady);
    RequestPairingCode();
}

void UWMCYNFirstSignalPairingAsyncAction::RequestPairingCode()
{
    if (!BackendSubsystem)
    {
        const FString Message = TEXT("WMCYN login service is not available.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
        return;
    }

    SetLoginStatus(TEXT("Requesting pairing code..."));
    OnProgress.Broadcast(TEXT("Requesting pairing code..."));

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendSubsystem->GetBackendBaseUrl() + TEXT("/v1/auth/code.create"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(TEXT("{}"));
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (bFinished)
            {
                return;
            }

            if (!bSucceeded || !Response.IsValid())
            {
                const FString Message = TEXT("Pairing service is unavailable.");
                SetLoginStatus(Message);
                OnFailed.Broadcast(Message);
                Finish();
                return;
            }

            TSharedPtr<FJsonObject> Root;
            if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300 ||
                !ParseJsonObject(Response->GetContentAsString(), Root))
            {
                const FString Message = TEXT("Could not create pairing code.");
                SetLoginStatus(Message);
                OnFailed.Broadcast(Message);
                Finish();
                return;
            }

            PairingCode = ReadString(Root, TEXT("code"));
            PairingCodeExpiresAt = ReadString(Root, TEXT("expiresAt"));
            if (PairingCode.IsEmpty())
            {
                const FString Message = TEXT("Could not create pairing code.");
                SetLoginStatus(Message);
                OnFailed.Broadcast(Message);
                Finish();
                return;
            }

            SetPairingCodeText(PairingCode);
            SetLoginStatus(FString::Printf(TEXT("Pair on your phone using code %s"), *PairingCode));
            OnCodeReady.Broadcast(PairingCode, PairingCodeExpiresAt);
            OnProgress.Broadcast(TEXT("Waiting for phone approval..."));

            if (UWorld* World = WorldContext.IsValid() ? WorldContext->GetWorld() : nullptr)
            {
                World->GetTimerManager().SetTimer(
                    PollTimerHandle,
                    this,
                    &UWMCYNFirstSignalPairingAsyncAction::PollPairingExchange,
                    2.0f,
                    true,
                    2.0f);
            }
        });

    if (!Request->ProcessRequest())
    {
        const FString Message = TEXT("Could not start pairing request.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
    }
}

void UWMCYNFirstSignalPairingAsyncAction::PollPairingExchange()
{
    if (bFinished || PairingCode.IsEmpty() || !BackendSubsystem)
    {
        return;
    }

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetStringField(TEXT("code"), PairingCode);

    FString Body;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Payload, Writer);

    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendSubsystem->GetBackendBaseUrl() + TEXT("/v1/auth/code.exchange"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindWeakLambda(
        this,
        [this](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded)
        {
            if (bFinished)
            {
                return;
            }

            if (!bSucceeded || !Response.IsValid())
            {
                const FString Message = TEXT("Pairing service is unavailable.");
                SetLoginStatus(Message);
                OnFailed.Broadcast(Message);
                Finish();
                return;
            }

            TSharedPtr<FJsonObject> Root;
            ParseJsonObject(Response->GetContentAsString(), Root);

            if (Response->GetResponseCode() == 202)
            {
                const FString Status = ReadString(Root, TEXT("status"));
                if (Status.Equals(TEXT("attached"), ESearchCase::IgnoreCase))
                {
                    SetLoginStatus(TEXT("Phone approved. Completing sign in..."));
                    OnProgress.Broadcast(TEXT("Phone approved. Completing sign in..."));
                }
                else
                {
                    SetLoginStatus(FString::Printf(TEXT("Waiting for phone approval (%s)..."),
                        Status.IsEmpty() ? TEXT("pending") : *Status));
                    OnProgress.Broadcast(TEXT("Waiting for phone approval..."));
                }
                return;
            }

            if (Response->GetResponseCode() == 200)
            {
                const FString CustomToken = ReadString(Root, TEXT("customToken"));
                if (CustomToken.IsEmpty())
                {
                    const FString Message = TEXT("Pairing completed without a valid token.");
                    SetLoginStatus(Message);
                    OnFailed.Broadcast(Message);
                    Finish();
                    return;
                }

                StopPolling();
                BackendSubsystem->SignInWithCustomTokenAndLoadBootstrap(CustomToken);
                return;
            }

            const FString Error = ReadString(Root, TEXT("error"));
            FString Message = TEXT("Pairing failed.");
            if (Response->GetResponseCode() == 404)
            {
                Message = TEXT("That pairing code is invalid.");
            }
            else if (Response->GetResponseCode() == 410)
            {
                Message = TEXT("That pairing code expired. Request a new one.");
            }
            else if (!Error.IsEmpty())
            {
                Message = FString::Printf(TEXT("Pairing failed: %s"), *Error);
            }

            SetLoginStatus(Message);
            OnFailed.Broadcast(Message);
            Finish();
        });

    if (!Request->ProcessRequest())
    {
        const FString Message = TEXT("Could not start pairing exchange.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
    }
}

void UWMCYNFirstSignalPairingAsyncAction::HandleLoginStateChanged(
    const EWMCYNBackendLoginState State,
    const FString& Message)
{
    if (bFinished)
    {
        return;
    }

    if (State == EWMCYNBackendLoginState::Failed)
    {
        const FString FailureMessage = FString::Printf(TEXT("Pairing failed: %s"), *Message);
        SetLoginStatus(FailureMessage);
        OnFailed.Broadcast(FailureMessage);
        Finish();
        return;
    }

    if (State == EWMCYNBackendLoginState::Authenticating)
    {
        SetLoginStatus(TEXT("Signing in..."));
        OnProgress.Broadcast(TEXT("Signing in..."));
    }
    else if (State == EWMCYNBackendLoginState::LoadingBootstrap)
    {
        SetLoginStatus(TEXT("Loading WMCYN world..."));
        OnProgress.Broadcast(TEXT("Loading WMCYN world..."));
    }
}

void UWMCYNFirstSignalPairingAsyncAction::HandleLoginReady(
    const FString& Username,
    const FString& DisplayName,
    const FString&,
    const int32)
{
    if (bFinished)
    {
        return;
    }

    const FString ResolvedDisplayName = DisplayName.IsEmpty() ? Username : DisplayName;

    const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext.Get());
    const UWMCYNWorldRuntimeSubsystem* RuntimeSubsystem = GameInstance ?
        GameInstance->GetSubsystem<UWMCYNWorldRuntimeSubsystem>() : nullptr;
    const bool bIsRuntimeHost = RuntimeSubsystem && RuntimeSubsystem->IsRuntimeRegistrationActive();

    if (!bIsRuntimeHost && BackendSubsystem && BackendSubsystem->IsReadyToEnterWorld())
    {
        OnReady.Broadcast(Username, ResolvedDisplayName);
        SetLoginStatus(FString::Printf(TEXT("Entering WMCYN as %s..."), *ResolvedDisplayName));
        if (BackendSubsystem->TravelToFirstSignalWorld())
        {
            Finish();
            return;
        }
    }

    if (!UWMCYNFirstSignalBlueprintLibrary::SubmitLocalFirstSignalIdentity(
            WorldContext.Get(),
            Username,
            ResolvedDisplayName))
    {
        const FString Message = TEXT("Player presence is not ready.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
        return;
    }

    OnReady.Broadcast(Username, ResolvedDisplayName);
    SetLoginStatus(FString::Printf(TEXT("Entering WMCYN as %s"), *ResolvedDisplayName));
    CompleteLoginWidget();
    Finish();
}

void UWMCYNFirstSignalPairingAsyncAction::ApplyPairingWidgetPresentation()
{
    if (!LoginWidget)
    {
        return;
    }

    EnsurePairingWidget();
    if (PairingPanel)
    {
        PairingPanel->SetVisibility(ESlateVisibility::Visible);
    }
    if (PairingTitleText)
    {
        PairingTitleText->SetText(FText::FromString(TEXT("PAIR WITH PHONE")));
    }
}

void UWMCYNFirstSignalPairingAsyncAction::EnsurePairingWidget()
{
    if (!LoginWidget)
    {
        return;
    }

    PairingPanel = Cast<UBorder>(LoginWidget->GetWidgetFromName(TEXT("WMCYN_PairingPanel")));
    PairingTitleText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_PairingTitle_Runtime")));
    if (!PairingTitleText)
    {
        PairingTitleText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_PairingTitle")));
    }

    PairingCodeText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_PairingCode_Runtime")));
    if (!PairingCodeText)
    {
        PairingCodeText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_PairingCode")));
    }

    PairingStatusText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_PairingStatus_Runtime")));
    if (!PairingStatusText)
    {
        PairingStatusText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_PairingStatus")));
    }
}

void UWMCYNFirstSignalPairingAsyncAction::SetLoginStatus(const FString& Message) const
{
    if (!LoginWidget)
    {
        return;
    }

    if (PairingStatusText)
    {
        PairingStatusText->SetText(FText::FromString(Message));
    }

    SetWidgetText(TEXT("TXT_Status"), Message);
}

void UWMCYNFirstSignalPairingAsyncAction::SetPairingCodeText(const FString& Code) const
{
    if (!LoginWidget)
    {
        return;
    }

    if (PairingCodeText)
    {
        PairingCodeText->SetText(FText::FromString(Code));
        return;
    }

    const TCHAR* CandidateNames[] = {
        TEXT("TXT_PairCode"),
        TEXT("TXT_PairingCode"),
        TEXT("TXT_Code"),
        TEXT("EDT_PairCode"),
        TEXT("EDT_PairingCode")
    };

    for (const TCHAR* CandidateName : CandidateNames)
    {
        if (UWidget* Widget = LoginWidget->GetWidgetFromName(CandidateName))
        {
            if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
            {
                TextBlock->SetText(FText::FromString(Code));
                return;
            }
            if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(Widget))
            {
                EditableTextBox->SetText(FText::FromString(Code));
                return;
            }
        }
    }
}

void UWMCYNFirstSignalPairingAsyncAction::SetWidgetText(const TCHAR* WidgetName, const FString& Text) const
{
    if (!LoginWidget || !WidgetName)
    {
        return;
    }

    if (UTextBlock* TextBlock = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(WidgetName)))
    {
        TextBlock->SetText(FText::FromString(Text));
        return;
    }

    if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(LoginWidget->GetWidgetFromName(WidgetName)))
    {
        EditableTextBox->SetText(FText::FromString(Text));
    }
}

void UWMCYNFirstSignalPairingAsyncAction::SetWidgetVisibility(
    const TCHAR* WidgetName,
    const ESlateVisibility Visibility) const
{
    if (!LoginWidget || !WidgetName)
    {
        return;
    }

    if (UWidget* Widget = LoginWidget->GetWidgetFromName(WidgetName))
    {
        Widget->SetVisibility(Visibility);
    }
}

void UWMCYNFirstSignalPairingAsyncAction::CompleteLoginWidget()
{
    if (!LoginWidget)
    {
        return;
    }

    const TWeakObjectPtr<UUserWidget> WeakLoginWidget = LoginWidget;
    if (UWorld* World = LoginWidget->GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(
            [WeakLoginWidget]()
            {
                UUserWidget* ResolvedLoginWidget = WeakLoginWidget.Get();
                if (!ResolvedLoginWidget)
                {
                    return;
                }

                if (UFunction* CloseFunction = ResolvedLoginWidget->FindFunction(TEXT("CloseLoginGate")))
                {
                    ResolvedLoginWidget->ProcessEvent(CloseFunction, nullptr);
                }
                else
                {
                    ResolvedLoginWidget->RemoveFromParent();
                }
            }));
        return;
    }

    LoginWidget->RemoveFromParent();
}

void UWMCYNFirstSignalPairingAsyncAction::StopPolling()
{
    if (UWorld* World = WorldContext.IsValid() ? WorldContext->GetWorld() : nullptr)
    {
        World->GetTimerManager().ClearTimer(PollTimerHandle);
    }
}

void UWMCYNFirstSignalPairingAsyncAction::Finish()
{
    if (bFinished)
    {
        return;
    }
    bFinished = true;

    StopPolling();

    if (BackendSubsystem)
    {
        BackendSubsystem->OnLoginStateChanged.RemoveDynamic(this, &UWMCYNFirstSignalPairingAsyncAction::HandleLoginStateChanged);
        BackendSubsystem->OnLoginReady.RemoveDynamic(this, &UWMCYNFirstSignalPairingAsyncAction::HandleLoginReady);
    }

    SetReadyToDestroy();
}
