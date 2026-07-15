#include "WMCYNFirstSignalLoginAsyncAction.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "WMCYNFirstSignalBlueprintLibrary.h"

bool UWMCYNFirstSignalLoginAsyncAction::StartForLoginWidget(
    const UObject* WorldContextObject,
    UUserWidget* InLoginWidget,
    const FString& Identifier,
    const FString& Password,
    const bool bAllowPIEDevelopmentFallback)
{
    if (!WorldContextObject || !InLoginWidget)
    {
        return false;
    }

    UWMCYNFirstSignalLoginAsyncAction* Action = AuthenticateAndLoadFirstSignal(
        WorldContextObject,
        Identifier,
        Password,
        bAllowPIEDevelopmentFallback);
    if (!Action)
    {
        return false;
    }

    Action->LoginWidget = InLoginWidget;
    Action->Activate();
    return true;
}

UWMCYNFirstSignalLoginAsyncAction* UWMCYNFirstSignalLoginAsyncAction::AuthenticateAndLoadFirstSignal(
    const UObject* WorldContextObject,
    const FString& Identifier,
    const FString& Password,
    const bool bAllowPIEDevelopmentFallback)
{
    UWMCYNFirstSignalLoginAsyncAction* Action = NewObject<UWMCYNFirstSignalLoginAsyncAction>();
    Action->WorldContext = WorldContextObject;
    Action->LoginIdentifier = Identifier.TrimStartAndEnd();
    Action->LoginPassword = Password;
    Action->bAllowLocalPIEFallback = bAllowPIEDevelopmentFallback;
    if (WorldContextObject)
    {
        Action->RegisterWithGameInstance(WorldContextObject);
    }
    return Action;
}

void UWMCYNFirstSignalLoginAsyncAction::Activate()
{
    if (!WorldContext.IsValid() || LoginIdentifier.IsEmpty() || LoginPassword.Len() < 6)
    {
        const FString Message = TEXT("Enter a valid username or email and password.");
        SetLoginStatus(Message);
        OnFailed.Broadcast(Message);
        Finish();
        return;
    }

    if (ShouldUsePIEDevelopmentFallback())
    {
        SetLoginStatus(TEXT("Using local First Signal identity..."));
        OnProgress.Broadcast(TEXT("Using local First Signal identity."));
        FString LocalUsername = LoginIdentifier;
        LocalUsername.RemoveFromStart(TEXT("@"));
        FString LeftSide;
        FString RightSide;
        if (LocalUsername.Split(TEXT("@"), &LeftSide, &RightSide) && !LeftSide.IsEmpty())
        {
            LocalUsername = LeftSide;
        }
        if (!UWMCYNFirstSignalBlueprintLibrary::SubmitLocalFirstSignalIdentity(
                WorldContext.Get(),
                LocalUsername,
                LocalUsername))
        {
            const FString Message = TEXT("Player presence is not ready.");
            SetLoginStatus(Message);
            OnFailed.Broadcast(Message);
            Finish();
            return;
        }

        OnReady.Broadcast(LocalUsername, LocalUsername);
        SetLoginStatus(FString::Printf(TEXT("Entering WMCYN as %s"), *LocalUsername));
        CompleteLoginWidget();
        Finish();
        return;
    }

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

    BackendSubsystem->OnLoginStateChanged.AddDynamic(this, &UWMCYNFirstSignalLoginAsyncAction::HandleLoginStateChanged);
    BackendSubsystem->OnLoginReady.AddDynamic(this, &UWMCYNFirstSignalLoginAsyncAction::HandleLoginReady);
    BackendSubsystem->LoginAndLoadBootstrap(LoginIdentifier, LoginPassword);
}

void UWMCYNFirstSignalLoginAsyncAction::HandleLoginStateChanged(
    const EWMCYNBackendLoginState State,
    const FString& Message)
{
    if (bFinished)
    {
        return;
    }

    if (State == EWMCYNBackendLoginState::Failed)
    {
        const FString FailureMessage = FString::Printf(TEXT("Login failed: %s"), *Message);
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

void UWMCYNFirstSignalLoginAsyncAction::HandleLoginReady(
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

bool UWMCYNFirstSignalLoginAsyncAction::ShouldUsePIEDevelopmentFallback() const
{
    if (!bAllowLocalPIEFallback || FParse::Param(FCommandLine::Get(), TEXT("WMCYNForceBackendAuth")))
    {
        return false;
    }

    if (FParse::Param(FCommandLine::Get(), TEXT("WMCYNLocalIdentity")))
    {
        return true;
    }

#if WITH_EDITOR
    const UWorld* World = WorldContext.IsValid() ? WorldContext->GetWorld() : nullptr;
    return World && World->WorldType == EWorldType::PIE;
#else
    return false;
#endif
}

void UWMCYNFirstSignalLoginAsyncAction::SetLoginStatus(const FString& Message) const
{
    if (!LoginWidget)
    {
        return;
    }

    if (UTextBlock* StatusText = Cast<UTextBlock>(LoginWidget->GetWidgetFromName(TEXT("TXT_Status"))))
    {
        StatusText->SetText(FText::FromString(Message));
    }
}

void UWMCYNFirstSignalLoginAsyncAction::CompleteLoginWidget()
{
    if (!LoginWidget)
    {
        return;
    }

    if (UFunction* CloseFunction = LoginWidget->FindFunction(TEXT("CloseLoginGate")))
    {
        LoginWidget->ProcessEvent(CloseFunction, nullptr);
    }
    else
    {
        LoginWidget->RemoveFromParent();
    }
}

void UWMCYNFirstSignalLoginAsyncAction::Finish()
{
    if (bFinished)
    {
        return;
    }
    bFinished = true;

    if (BackendSubsystem)
    {
        BackendSubsystem->OnLoginStateChanged.RemoveDynamic(this, &UWMCYNFirstSignalLoginAsyncAction::HandleLoginStateChanged);
        BackendSubsystem->OnLoginReady.RemoveDynamic(this, &UWMCYNFirstSignalLoginAsyncAction::HandleLoginReady);
    }
    LoginPassword.Reset();
    SetReadyToDestroy();
}
