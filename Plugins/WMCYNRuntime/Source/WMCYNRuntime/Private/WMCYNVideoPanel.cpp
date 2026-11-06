#include "WMCYNVideoPanel.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Kismet/GameplayStatics.h"
#include "WMCYNFirstSignalPresenceComponent.h"
#include "WMCYNVideoChannel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

AWMCYNVideoPanel::AWMCYNVideoPanel()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;
    if (!RootComponent) SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

bool AWMCYNVideoPanel::BindWidget()
{
    if (!IsValid(Channel)) return false;
    if (Widget) return true;
    TInlineComponentArray<UWidgetComponent*> Hosts(this);
    UWidgetComponent* Form = nullptr;
    for (UWidgetComponent* Host : Hosts)
    {
        if (Host->GetWidget()) { Widget = Host->GetWidget(); Form = Host; break; }
    }
    if (!Widget || !Channel) return false;
    if (auto* Button = Cast<UButton>(Widget->GetWidgetFromName(TEXT("BTN_Submit"))))
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSubmit);
    if (auto* Button = Cast<UButton>(Widget->GetWidgetFromName(TEXT("BTN_Reset"))))
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleReset);
    if (auto* Field = Cast<UEditableTextBox>(Widget->GetWidgetFromName(TEXT("IN_Link"))))
        Field->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::HandleCommitted);
    if (auto* Volume = Cast<USlider>(Widget->GetWidgetFromName(TEXT("SLD_Volume"))))
    {
        Volume->SetValue(Channel->GetLocalVolume());
        Volume->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleVolume);
    }
    if (auto* Paste = Cast<UButton>(Widget->GetWidgetFromName(TEXT("BTN_Paste"))))
    {
#if PLATFORM_DESKTOP
        Paste->OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePaste);
#else
        // Quest has no desktop clipboard to paste from.
        Paste->SetVisibility(ESlateVisibility::Collapsed);
#endif
    }
    if (auto* Label = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TXT_Title")))) Label->SetText(Title);
    if (ActorHasTag(TEXT("DJLaptop"))) InstallLaptopToggle(Form);
    return true;
}

void AWMCYNVideoPanel::InstallLaptopToggle(UWidgetComponent* Form)
{
    // The existing form covers the top 82% only while editing. The strip below it
    // stays interactive without blocking the booth's video material or its UVs.
    auto* Strip = NewObject<UWidgetComponent>(this);
    AddInstanceComponent(Strip);
    Strip->SetupAttachment(GetRootComponent());
    Strip->SetRelativeScale3D(Form->GetRelativeScale3D());
    Strip->SetRelativeLocation(FVector(.1, 0, -530 * Form->GetRelativeScale3D().Z));
    Strip->SetDrawSize(FVector2D(1400,160));
    Strip->SetCollisionProfileName(Form->GetCollisionProfileName());
    Strip->SetCollisionResponseToChannels(Form->GetCollisionResponseToChannels());
    Strip->RegisterComponent();
    Strip->SetSlateWidget(SNew(SButton)
        .HAlign(HAlign_Center).VAlign(VAlign_Center)
        .OnClicked_Lambda([Form]()
        {
            const bool Open = !Form->IsVisible();
            Form->SetVisibility(Open);
            Form->SetCollisionEnabled(Open ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
            return FReply::Handled();
        })
        [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",64))
            .Text_Lambda([Form](){ return FText::FromString(Form->IsVisible() ? TEXT("Back to video") : TEXT("Edit video link")); })]);
    Form->SetVisibility(false);
    Form->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWMCYNVideoPanel::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (GetNetMode() != NM_DedicatedServer && BindWidget()) Refresh();
}

void AWMCYNVideoPanel::Refresh()
{
    if (auto* Now = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TXT_NowPlaying"))))
    {
        const FString Url = Channel->GetEffectiveUrl();
        Now->SetText(FText::FromString(Url.IsEmpty() ? TEXT("Nothing playing") : Url.Left(64)));
    }
    if (auto* Status = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("TXT_Status"))))
    {
        const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
        const auto* Presence = Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
        const FString Response = Presence ? Presence->VideoResponse : FString();
        Status->SetText(FText::FromString(Response.IsEmpty() ? Channel->GetStatus() : Response + TEXT(" - ") + Channel->GetStatus()));
    }
}

void AWMCYNVideoPanel::Send(const FString& Url, const bool bReset)
{
    const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    auto* Presence = Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
    if (Presence && Channel) Presence->RequestVideoChannel(Channel, Url, bReset);
}

void AWMCYNVideoPanel::HandleSubmit()
{
    const auto* Field = Cast<UEditableTextBox>(Widget->GetWidgetFromName(TEXT("IN_Link")));
    if (Field) Send(Field->GetText().ToString().TrimStartAndEnd(), false);
}

void AWMCYNVideoPanel::HandleReset()
{
    Send(FString(), true);
}

void AWMCYNVideoPanel::HandlePaste()
{
    FString Text;
    FPlatformApplicationMisc::ClipboardPaste(Text);
    if (auto* Field = Cast<UEditableTextBox>(Widget->GetWidgetFromName(TEXT("IN_Link"))))
        Field->SetText(FText::FromString(Text.TrimStartAndEnd()));
}

void AWMCYNVideoPanel::HandleVolume(const float Value)
{
    if (Channel) Channel->SetLocalVolume(Value);
}

void AWMCYNVideoPanel::HandleCommitted(const FText& Text, const ETextCommit::Type Method)
{
    if (Method == ETextCommit::OnEnter) Send(Text.ToString().TrimStartAndEnd(), false);
}
