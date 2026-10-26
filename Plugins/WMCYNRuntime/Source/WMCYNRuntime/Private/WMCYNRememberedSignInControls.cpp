#include "WMCYNRememberedSignInControls.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "WMCYNFirstSignalLoginAsyncAction.h"

void UWMCYNRememberedSignInControls::Initialize(UUserWidget* InWidget)
{
    Widget = InWidget;
    Backend = InWidget && InWidget->GetGameInstance() ? InWidget->GetGameInstance()->GetSubsystem<UWMCYNBackendSubsystem>() : nullptr;
    if (!Backend || !Widget.IsValid()) return;
    if (UButton* Button = Cast<UButton>(InWidget->GetWidgetFromName(TEXT("BTN_ContinueSaved"))))
        Button->OnClicked.AddUniqueDynamic(this, &UWMCYNRememberedSignInControls::Continue);
    if (UButton* Button = Cast<UButton>(InWidget->GetWidgetFromName(TEXT("BTN_SwitchAccount"))))
        Button->OnClicked.AddUniqueDynamic(this, &UWMCYNRememberedSignInControls::SwitchAccount);
    if (UCheckBox* Check = Cast<UCheckBox>(InWidget->GetWidgetFromName(TEXT("CHK_RememberSignIn"))))
    {
        Check->SetIsChecked(false);
        Check->OnCheckStateChanged.AddUniqueDynamic(this, &UWMCYNRememberedSignInControls::RememberChanged);
    }
    Backend->OnLoginStateChanged.AddUniqueDynamic(this, &UWMCYNRememberedSignInControls::StateChanged);
    Refresh();
    if (Backend->bCredentialRemovalFailed)
        if (UTextBlock* Status = Cast<UTextBlock>(InWidget->GetWidgetFromName(TEXT("TXT_Status"))))
            Status->SetText(FText::FromString(TEXT("Signed out. Use Switch Account to retry forgetting saved sign-in before sharing this device.")));
#if !UE_BUILD_SHIPPING
    if (FParse::Param(FCommandLine::Get(), TEXT("WMCYNAutoContinue")) && Backend->HasRememberedSignIn()) Continue();
#endif
}

void UWMCYNRememberedSignInControls::Refresh()
{
    if (!Widget.IsValid() || !Backend) return;
    const bool bSaved = Backend->HasRememberedSignIn();
    const bool bBusy = Backend->LoginState == EWMCYNBackendLoginState::Authenticating || Backend->LoginState == EWMCYNBackendLoginState::LoadingBootstrap;
    if (UWidget* Row = Widget->GetWidgetFromName(TEXT("ROW_RememberedSignIn")))
        Row->SetVisibility(bSaved || Backend->bCredentialRemovalFailed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (UWidget* Button = Widget->GetWidgetFromName(TEXT("BTN_ContinueSaved"))) Button->SetIsEnabled(bSaved && !bBusy);
    if (UTextBlock* Label = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TXT_ContinueSaved"))))
        Label->SetText(bSaved ? FText::Format(NSLOCTEXT("WMCYN", "ContinueSaved", "Continue as {0}"), FText::FromString(Backend->GetRememberedDisplayLabel()))
            : FText::FromString(TEXT("Saved sign-in unavailable")));
    for (const TCHAR* Name : {TEXT("TXT_UsernameLabel"), TEXT("SizeBox_0"), TEXT("TXT_PasswordLabel"), TEXT("SizeBox_1"), TEXT("BTN_EnterWorld"), TEXT("WMCYN_PairingPanel")})
    {
        if (UWidget* Control = Widget->GetWidgetFromName(Name))
        {
            Control->SetVisibility(bSaved ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
            Control->SetIsEnabled(!bBusy);
        }
    }
    if (UWidget* Check = Widget->GetWidgetFromName(TEXT("CHK_RememberSignIn")))
    {
#if PLATFORM_WINDOWS
        Check->SetVisibility(bSaved ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
#else
        Check->SetVisibility(ESlateVisibility::Collapsed);
#endif
        Check->SetIsEnabled(!bBusy);
    }
}

void UWMCYNRememberedSignInControls::Continue()
{
    if (Widget.IsValid()) UWMCYNFirstSignalLoginAsyncAction::ContinueRememberedSignIn(Widget.Get(), Widget.Get());
}

void UWMCYNRememberedSignInControls::SwitchAccount()
{
    if (!Backend || !Widget.IsValid()) return;
    Backend->SignOut();
    for (const TCHAR* Name : {TEXT("IN_Username"), TEXT("IN_Password")})
        if (UEditableTextBox* Field = Cast<UEditableTextBox>(Widget->GetWidgetFromName(Name))) Field->SetText(FText::GetEmpty());
    if (UCheckBox* Check = Cast<UCheckBox>(Widget->GetWidgetFromName(TEXT("CHK_RememberSignIn")))) Check->SetIsChecked(false);
    if (UTextBlock* Status = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TXT_Status"))))
        Status->SetText(FText::FromString(Backend->bCredentialRemovalFailed
            ? TEXT("Signed out. Saved credential could not be removed from this device.")
            : TEXT("Sign in with another account.")));
    Refresh();
}

void UWMCYNRememberedSignInControls::RememberChanged(bool bChecked)
{
    if (Backend) Backend->SetRememberSignIn(bChecked);
}

void UWMCYNRememberedSignInControls::StateChanged(EWMCYNBackendLoginState State, const FString& Message)
{
    Refresh();
}
