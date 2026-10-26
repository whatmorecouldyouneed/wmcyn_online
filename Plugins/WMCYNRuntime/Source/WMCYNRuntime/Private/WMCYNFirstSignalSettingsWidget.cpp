#include "WMCYNFirstSignalSettingsWidget.h"

#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "WMCYNFirstSignalBlueprintLibrary.h"
#include "WMCYNBackendSubsystem.h"
#include "WMCYNWorldRuntimeSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextBlock.h"
#include "WMCYNFirstSignalPresenceComponent.h"
#include "WMCYNLocalVoiceTalker.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "WMCYNCameraToolComponent.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/ProgressBar.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
    void StyleAudioButton(UUserWidget* Widget)
    {
        if (!Widget || !Widget->WidgetTree) return;
        Widget->WidgetTree->ForEachWidget([](UWidget* Child)
        {
            if (auto* Nested = Cast<UUserWidget>(Child)) StyleAudioButton(Nested);
            if (auto* Border = Cast<UBorder>(Child))
            {
                Border->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.022f, 0.022f, 0.022f), 8.0f));
                Border->SetBrushColor(FLinearColor::White);
            }
            if (auto* Text = Cast<UTextBlock>(Child))
            {
                FSlateFontInfo Font = Text->GetFont();
                Font.Size = 24;
                Font.TypefaceFontName = TEXT("Regular");
                Text->SetFont(Font);
                Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.92f, 0.92f)));
            }
        });
    }

    void StyleSlider(USlider* Slider)
    {
        if (!Slider) return;
        FSliderStyle Style = Slider->GetWidgetStyle();
        Style.BarThickness = 6.0f;
        Style.NormalThumbImage.ImageSize = FVector2D(24, 36);
        Style.HoveredThumbImage.ImageSize = FVector2D(28, 40);
        Style.DisabledThumbImage.ImageSize = FVector2D(24, 36);
        Slider->SetWidgetStyle(Style);
        Slider->SetSliderBarColor(FLinearColor(0.12f, 0.16f, 0.18f));
        Slider->SetSliderHandleColor(FLinearColor(0.1f, 0.8f, 0.55f));
    }

    UWMCYNFirstSignalPresenceComponent* LocalPresence(UUserWidget* Widget)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(Widget, 0);
        APawn* Pawn = PC ? PC->GetPawn() : nullptr;
        return Pawn ? Pawn->FindComponentByClass<UWMCYNFirstSignalPresenceComponent>() : nullptr;
    }
    UWMCYNLocalVoiceTalker* RemoteTalker(APlayerState* State)
    {
        APawn* Pawn = State ? State->GetPawn() : nullptr;
        return Pawn ? Pawn->FindComponentByClass<UWMCYNLocalVoiceTalker>() : nullptr;
    }
}

void UWMCYNFirstSignalSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RespawnButton)
    {
        RespawnButton->OnClicked.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked);
    }

    ConfigureAudioSettings();
    // AFCore initializes its decorative brushes after construction.
    GetWorld()->GetTimerManager().SetTimer(VisualStyleTimer, this, &ThisClass::ApplyAudioVisualStyle, 0.5f, false);
    StyleSlider(RemoteVoiceVolume);
    StyleSlider(CameraFOV);
    if (CameraShotSelector)
    {
        CameraShotSelector->ClearOptions();
        CameraShotSelector->AddOption(TEXT("Master"));
        CameraShotSelector->AddOption(TEXT("Interviewer"));
        CameraShotSelector->AddOption(TEXT("Guest"));
        CameraShotSelector->SetSelectedIndex(0);
    }
    if (CameraShotSaveButton) CameraShotSaveButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCameraShotSaveClicked);
    if (CameraShotSelectButton) CameraShotSelectButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCameraShotSelectClicked);
    if (CameraButton) CameraButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCameraClicked);
    if (CameraResetButton) CameraResetButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCameraResetClicked);
    if (CameraOutputButton) CameraOutputButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCameraOutputClicked);
    if (CameraFOV) CameraFOV->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleCameraFOVChanged);
#if !PLATFORM_WINDOWS
    if (CameraOutputButton) CameraOutputButton->SetVisibility(ESlateVisibility::Collapsed);
#endif
    if (MicButton) MicButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleMicClicked);
    if (MirrorButton) MirrorButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleMirrorClicked);
    if (ResetPanelButton) ResetPanelButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleResetPanelClicked);
    if (PinPanelButton) PinPanelButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePinPanelClicked);
    if (RemotePlayerSelector) RemotePlayerSelector->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleRemoteSelected);
    if (RemoteVoiceVolume) RemoteVoiceVolume->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleRemoteVolumeChanged);
    RefreshTools();
    GetWorld()->GetTimerManager().SetTimer(ToolsTimer, this, &ThisClass::RefreshTools, 1.0f, true);
    if (SignOutButton)
        SignOutButton->OnClicked.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleSignOutClicked);
}

void UWMCYNFirstSignalSettingsWidget::NativeDestruct()
{
    RemoveAudioBindings();
    if (CameraShotSaveButton) CameraShotSaveButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCameraShotSaveClicked);
    if (CameraShotSelectButton) CameraShotSelectButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCameraShotSelectClicked);
    if (CameraButton) CameraButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCameraClicked);
    if (CameraResetButton) CameraResetButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCameraResetClicked);
    if (CameraOutputButton) CameraOutputButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCameraOutputClicked);
    if (CameraFOV) CameraFOV->OnValueChanged.RemoveDynamic(this, &ThisClass::HandleCameraFOVChanged);
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(ToolsTimer);
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(VisualStyleTimer);
    if (MicButton) MicButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleMicClicked);
    if (MirrorButton) MirrorButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleMirrorClicked);
    if (ResetPanelButton) ResetPanelButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleResetPanelClicked);
    if (PinPanelButton) PinPanelButton->OnClicked.RemoveDynamic(this, &ThisClass::HandlePinPanelClicked);
    if (RemotePlayerSelector) RemotePlayerSelector->OnSelectionChanged.RemoveDynamic(this, &ThisClass::HandleRemoteSelected);
    if (RemoteVoiceVolume) RemoteVoiceVolume->OnValueChanged.RemoveDynamic(this, &ThisClass::HandleRemoteVolumeChanged);
    if (SignOutButton)
        SignOutButton->OnClicked.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleSignOutClicked);

    if (RespawnButton)
    {
        RespawnButton->OnClicked.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked);
    }

    Super::NativeDestruct();
}

void UWMCYNFirstSignalSettingsWidget::ApplyAudioVisualStyle()
{
    if (!AudioSettings) return;
    if (auto* Scaffold = Cast<UUserWidget>(AudioSettings->GetWidgetFromName(TEXT("Widget_Scaffold_Settings"))))
    {
        StyleAudioButton(Cast<UUserWidget>(Scaffold->GetWidgetFromName(TEXT("Widget_Button_Apply"))));
        StyleAudioButton(Cast<UUserWidget>(Scaffold->GetWidgetFromName(TEXT("Widget_Button_Reset"))));
    }
    for (const FName Name : {FName(TEXT("Widget_Slider_Master")), FName(TEXT("Widget_Slider_Music")),
        FName(TEXT("Widget_Slider_Voice")), FName(TEXT("Widget_Slider_SFX"))})
    {
        UUserWidget* Wrapper = Cast<UUserWidget>(AudioSettings->GetWidgetFromName(Name));
        if (!Wrapper) continue;
        if (auto* Bar = Cast<UProgressBar>(Wrapper->GetWidgetFromName(TEXT("MainProgressbar"))))
        {
            FProgressBarStyle Style = Bar->GetWidgetStyle();
            Style.BackgroundImage = FSlateRoundedBoxBrush(FLinearColor(0.055f, 0.055f, 0.055f), 3.0f);
            Style.FillImage = FSlateRoundedBoxBrush(FLinearColor(0.65f, 0.65f, 0.65f), 3.0f);
            Bar->SetWidgetStyle(Style);
            Bar->SetFillColorAndOpacity(FLinearColor::White);
        }
        if (auto* Track = Cast<USizeBox>(Wrapper->GetWidgetFromName(TEXT("SizeBox_0")))) Track->SetHeightOverride(6.0f);
        if (auto* Knob = Cast<USizeBox>(Wrapper->GetWidgetFromName(TEXT("SizeBox_1"))))
        {
            Knob->SetWidthOverride(28.0f);
            Knob->SetHeightOverride(28.0f);
        }
        if (auto* Image = Cast<UImage>(Wrapper->GetWidgetFromName(TEXT("KnobImage"))))
        {
            Image->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.92f, 0.92f, 0.92f), 14.0f));
            Image->SetColorAndOpacity(FLinearColor::White);
        }
    }
}

void UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked()
{
    UWMCYNFirstSignalBlueprintLibrary::RequestLocalFirstSignalRespawn(this);
}

void UWMCYNFirstSignalSettingsWidget::HandleMicClicked()
{
    if (auto* Presence = LocalPresence(this)) Presence->SetLocalMicrophoneMuted(!Presence->IsLocalMicrophoneMuted());
    RefreshTools();
}

UWMCYNCameraToolComponent* UWMCYNFirstSignalSettingsWidget::ResolveCameraTool()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn || !Pawn->IsLocallyControlled()) return nullptr;
    auto* Tool = Pawn->FindComponentByClass<UWMCYNCameraToolComponent>();
    if (!Tool)
    {
        Tool = NewObject<UWMCYNCameraToolComponent>(Pawn);
        Pawn->AddInstanceComponent(Tool);
        Tool->RegisterComponent();
    }
    return Tool;
}

void UWMCYNFirstSignalSettingsWidget::HandleCameraClicked()
{
    if (auto* Tool = ResolveCameraTool())
    {
        const bool bSuccess = Tool->SetCameraEnabled(!Tool->IsCameraEnabled());
        if (CameraPreview)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(Tool->GetPreview());
            Brush.ImageSize = FVector2D(640, 360);
            CameraPreview->SetBrush(Brush);
            CameraPreview->SetVisibility(Tool->IsCameraEnabled() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }
        if (CameraStatus) CameraStatus->SetText(FText::FromString(!bSuccess ? TEXT("Camera unavailable: PCVR pilot") : Tool->IsCameraEnabled() ? TEXT("Camera ready") : TEXT("Camera off")));
        if (CameraFOV) Tool->SetFieldOfView(30.0f + CameraFOV->GetValue() * 70.0f);
        RefreshTools();
    }
}

void UWMCYNFirstSignalSettingsWidget::HandleCameraResetClicked()
{
    if (auto* Tool = ResolveCameraTool()) Tool->ResetCameraPose();
}

void UWMCYNFirstSignalSettingsWidget::HandleCameraShotSaveClicked()
{
    auto* Tool = ResolveCameraTool();
    const int32 ShotIndex = CameraShotSelector ? CameraShotSelector->GetSelectedIndex() : INDEX_NONE;
    const bool bSaved = Tool && Tool->SaveShot(ShotIndex);
    if (CameraStatus) CameraStatus->SetText(FText::FromString(bSaved ? TEXT("Shot saved") : TEXT("Enable an authorized camera first")));
}

void UWMCYNFirstSignalSettingsWidget::HandleCameraShotSelectClicked()
{
    auto* Tool = ResolveCameraTool();
    const int32 ShotIndex = CameraShotSelector ? CameraShotSelector->GetSelectedIndex() : INDEX_NONE;
    const bool bSelected = Tool && Tool->SelectShot(ShotIndex);
    if (CameraStatus) CameraStatus->SetText(FText::FromString(bSelected ? TEXT("Shot selected") : TEXT("Shot unavailable or camera off")));
}

void UWMCYNFirstSignalSettingsWidget::HandleCameraOutputClicked()
{
    if (auto* Tool = ResolveCameraTool())
    {
        const bool bSuccess = Tool->SetSpectatorOutput(!Tool->IsSpectatorOutputEnabled());
        if (CameraStatus) CameraStatus->SetText(FText::FromString(!bSuccess ? TEXT("Output unavailable: enable camera and PCVR") :
            Tool->IsSpectatorOutputEnabled() ? TEXT("Desktop camera feed active; OBS independent") : TEXT("Desktop headset feed restored")));
    }
}

void UWMCYNFirstSignalSettingsWidget::HandleCameraFOVChanged(float Value)
{
    if (auto* Tool = ResolveCameraTool()) Tool->SetFieldOfView(30.0f + Value * 70.0f);
}

void UWMCYNFirstSignalSettingsWidget::HandleMirrorClicked()
{
    // Only the existing WMCYN mirror, never arbitrary scene captures or remote actors.
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (It->GetClass()->GetPathName() == TEXT("/Game/WMCYN/Dev/AvatarMirror/BP_WMCYN_AvatarMirror.BP_WMCYN_AvatarMirror_C"))
        {
            It->SetActorHiddenInGame(!It->IsHidden());
        }
    }
}

void UWMCYNFirstSignalSettingsWidget::HandleResetPanelClicked()
{
    if (auto* Presence = LocalPresence(this)) Presence->ResetRuntimeMenuPosition();
}

void UWMCYNFirstSignalSettingsWidget::HandlePinPanelClicked()
{
    if (auto* Presence = LocalPresence(this))
    {
        bPanelPinned = !bPanelPinned;
        Presence->SetRuntimeMenuPinned(bPanelPinned);
        PinPanelButton->SetToolTipText(FText::FromString(bPanelPinned ? TEXT("Unpin menu") : TEXT("Pin menu in world")));
    }
}

void UWMCYNFirstSignalSettingsWidget::RefreshTools()
{
    const APlayerController* CameraPC = UGameplayStatics::GetPlayerController(this, 0);
    const APawn* CameraPawn = CameraPC ? CameraPC->GetPawn() : nullptr;
    const auto* CameraTool = CameraPawn ? CameraPawn->FindComponentByClass<UWMCYNCameraToolComponent>() : nullptr;
    const bool bCameraActive = CameraTool && CameraTool->IsCameraEnabled();
    const ESlateVisibility CameraVisibility = bCameraActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
    if (CameraFOV) CameraFOV->SetVisibility(CameraVisibility);
    if (CameraResetButton) CameraResetButton->SetVisibility(CameraVisibility);
#if PLATFORM_WINDOWS
    if (CameraOutputButton) CameraOutputButton->SetVisibility(CameraVisibility);
#endif
    if (CameraPreview)
    {
        if (bCameraActive)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(CameraTool->GetPreview());
            Brush.ImageSize = FVector2D(640,360);
            CameraPreview->SetBrush(Brush);
        }
        CameraPreview->SetVisibility(bCameraActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        if (auto* Frame = Cast<USizeBox>(CameraPreview->GetParent())) Frame->SetVisibility(CameraVisibility);
    }
    if (CameraButton) CameraButton->SetToolTipText(FText::FromString(bCameraActive ? TEXT("Hide camera") : TEXT("Show camera")));
    if (CameraResetButton) CameraResetButton->SetToolTipText(FText::FromString(TEXT("Recall camera")));
    UWMCYNFirstSignalPresenceComponent* Presence = LocalPresence(this);
    if (MicrophoneStatus) MicrophoneStatus->SetText(FText::FromString(Presence ? Presence->GetLocalMicrophoneStatus() : TEXT("Mic unavailable")));
    if (MicButton) MicButton->SetIsEnabled(Presence != nullptr);
    if (ConnectionStatus)
    {
        const auto* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWMCYNBackendSubsystem>() : nullptr;
        const bool bVerified = Backend && !Backend->VerifiedUserId.IsEmpty();
        const auto* LocalController = UGameplayStatics::GetPlayerController(this, 0);
        const FString World = GetWorld()->GetNetMode() == NM_Client && LocalController && LocalController->GetNetConnection()
            ? TEXT("World connected") : TEXT("Local world");
        ConnectionStatus->SetText(FText::FromString((bVerified ? TEXT("Signed in | ") : TEXT("Local identity | ")) + World));
    }
    if (!RemotePlayerSelector || !GetWorld()->GetGameState()) return;
    TMap<FString, TWeakObjectPtr<APlayerState>> Current;
    const auto* PC = UGameplayStatics::GetPlayerController(this, 0);
    for (APlayerState* State : GetWorld()->GetGameState()->PlayerArray)
    {
        if (!State || (PC && State == PC->PlayerState)) continue;
        // PlayerId disambiguates equal display names; never show authentication UIDs.
        Current.Add(FString::Printf(TEXT("%s (#%d)"), *State->GetPlayerName(), State->GetPlayerId()), State);
    }
    bool bChanged = Current.Num() != RemotePlayers.Num();
    for (const auto& Entry : Current) bChanged |= RemotePlayers.FindRef(Entry.Key) != Entry.Value;
    if (bChanged || RemotePlayerSelector->GetOptionCount() == 0)
    {
        RemotePlayers = MoveTemp(Current);
        RemotePlayerSelector->ClearOptions();
        TArray<FString> Names;
        RemotePlayers.GetKeys(Names);
        Names.Sort();
        for (const FString& Name : Names) RemotePlayerSelector->AddOption(Name);
        SelectedRemotePlayer.Reset();
        if (Names.Num()) RemotePlayerSelector->SetSelectedOption(Names[0]);
        else
        {
            RemotePlayerSelector->AddOption(TEXT("No other users connected"));
            RemotePlayerSelector->SetSelectedOption(TEXT("No other users connected"));
        }
    }
    RemotePlayerSelector->SetIsEnabled(RemotePlayers.Num() > 0);
    if (RemoteVoiceVolume) RemoteVoiceVolume->SetIsEnabled(RemoteTalker(SelectedRemotePlayer.Get()) != nullptr);
}

void UWMCYNFirstSignalSettingsWidget::HandleRemoteSelected(FString Item, ESelectInfo::Type SelectionType)
{
    SelectedRemotePlayer = RemotePlayers.FindRef(Item);
    auto* Talker = RemoteTalker(SelectedRemotePlayer.Get());
    if (RemoteVoiceVolume)
    {
        RemoteVoiceVolume->SetIsEnabled(Talker != nullptr);
        RemoteVoiceVolume->SetValue(Talker ? Talker->GetListenerVolume() : 1.0f);
    }
}

void UWMCYNFirstSignalSettingsWidget::HandleRemoteVolumeChanged(float Value)
{
    if (auto* Talker = RemoteTalker(SelectedRemotePlayer.Get())) Talker->SetListenerVolume(Value);
}

void UWMCYNFirstSignalSettingsWidget::HandleSignOutClicked()
{
    UGameInstance* Instance = GetGameInstance();
    UWMCYNBackendSubsystem* Backend = Instance ? Instance->GetSubsystem<UWMCYNBackendSubsystem>() : nullptr;
    UWMCYNWorldRuntimeSubsystem* Runtime = Instance ? Instance->GetSubsystem<UWMCYNWorldRuntimeSubsystem>() : nullptr;
    APlayerController* Player = GetOwningPlayer();
    if (!Player && Instance) Player = Instance->GetFirstLocalPlayerController();
    if (!Backend || !Player || (Runtime && Runtime->IsRuntimeRegistrationActive())) return;
    Backend->SignOut();
    // Absolute local travel leaves the server; no cached identity may reopen the login gate.
    Player->ClientTravel(TEXT("/Game/Levels/L_WMCYNOnline"), TRAVEL_Absolute);
}

void UWMCYNFirstSignalSettingsWidget::HandleMasterVolumeChanged(float Value)
{
    ApplySoundClassVolume(MasterSoundClass, Value, true);
}

void UWMCYNFirstSignalSettingsWidget::HandleMusicVolumeChanged(float Value)
{
    ApplySoundClassVolume(MusicSoundClass, Value, true);
}

void UWMCYNFirstSignalSettingsWidget::HandleVoiceVolumeChanged(float Value)
{
    ApplySoundClassVolume(VoiceSoundClass, Value, true);
}

void UWMCYNFirstSignalSettingsWidget::HandleSFXVolumeChanged(float Value)
{
    ApplySoundClassVolume(SFXSoundClass, Value, true);
}

void UWMCYNFirstSignalSettingsWidget::ConfigureAudioSettings()
{
    if (!AudioSettings)
    {
        return;
    }

    if (UWidget* AudioQualityRow = AudioSettings->GetWidgetFromName(TEXT("Widget_ListedSlot_4")))
    {
        AudioQualityRow->SetVisibility(ESlateVisibility::Collapsed);
    }

    MasterSlider = ResolveNativeSlider(TEXT("Widget_Slider_Master"));
    MusicSlider = ResolveNativeSlider(TEXT("Widget_Slider_Music"));
    VoiceSlider = ResolveNativeSlider(TEXT("Widget_Slider_Voice"));
    SFXSlider = ResolveNativeSlider(TEXT("Widget_Slider_SFX"));
    StyleSlider(MasterSlider);
    StyleSlider(MusicSlider);
    StyleSlider(VoiceSlider);
    StyleSlider(SFXSlider);

    if (MasterSlider)
    {
        MasterSlider->OnValueChanged.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleMasterVolumeChanged);
    }
    if (MusicSlider)
    {
        MusicSlider->OnValueChanged.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleMusicVolumeChanged);
    }
    if (VoiceSlider)
    {
        VoiceSlider->OnValueChanged.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleVoiceVolumeChanged);
    }
    if (SFXSlider)
    {
        SFXSlider->OnValueChanged.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleSFXVolumeChanged);
    }

    DefaultSoundMix = LoadObject<USoundMix>(nullptr, TEXT("/Game/AFCore/Sound/Classes/SMix_Default.SMix_Default"));
    MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/AFCore/Sound/Classes/SC_Master.SC_Master"));
    MusicSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/AFCore/Sound/Classes/SC_Music.SC_Music"));
    VoiceSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/AFCore/Sound/Classes/SC_Voice.SC_Voice"));
    SFXSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/AFCore/Sound/Classes/SC_SFX.SC_SFX"));

    if (DefaultSoundMix)
    {
        UGameplayStatics::PushSoundMixModifier(this, DefaultSoundMix);
    }
}

void UWMCYNFirstSignalSettingsWidget::RemoveAudioBindings()
{
    if (MasterSlider)
    {
        MasterSlider->OnValueChanged.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleMasterVolumeChanged);
    }
    if (MusicSlider)
    {
        MusicSlider->OnValueChanged.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleMusicVolumeChanged);
    }
    if (VoiceSlider)
    {
        VoiceSlider->OnValueChanged.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleVoiceVolumeChanged);
    }
    if (SFXSlider)
    {
        SFXSlider->OnValueChanged.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleSFXVolumeChanged);
    }
}

USlider* UWMCYNFirstSignalSettingsWidget::ResolveNativeSlider(FName WidgetName) const
{
    UWidget* SliderWrapper = AudioSettings ? AudioSettings->GetWidgetFromName(WidgetName) : nullptr;
    UFunction* GetMainSlider = SliderWrapper ? SliderWrapper->FindFunction(TEXT("getMainSlider")) : nullptr;
    if (!GetMainSlider)
    {
        return nullptr;
    }

    struct FGetMainSliderParams
    {
        USlider* ReturnValue = nullptr;
    } Params;

    SliderWrapper->ProcessEvent(GetMainSlider, &Params);
    return Params.ReturnValue;
}

void UWMCYNFirstSignalSettingsWidget::ApplySoundClassVolume(
    USoundClass* SoundClass,
    float Value,
    bool bApplyToChildren)
{
    if (!DefaultSoundMix || !SoundClass)
    {
        return;
    }

    UGameplayStatics::SetSoundMixClassOverride(
        this,
        DefaultSoundMix,
        SoundClass,
        FMath::Clamp(Value, 0.0f, 1.0f),
        1.0f,
        0.0f,
        bApplyToChildren);
}
