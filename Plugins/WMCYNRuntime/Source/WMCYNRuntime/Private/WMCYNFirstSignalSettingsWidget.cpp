#include "WMCYNFirstSignalSettingsWidget.h"

#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "WMCYNFirstSignalBlueprintLibrary.h"

void UWMCYNFirstSignalSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RespawnButton)
    {
        RespawnButton->OnClicked.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked);
    }

    ConfigureAudioSettings();
}

void UWMCYNFirstSignalSettingsWidget::NativeDestruct()
{
    RemoveAudioBindings();

    if (RespawnButton)
    {
        RespawnButton->OnClicked.RemoveDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked);
    }

    Super::NativeDestruct();
}

void UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked()
{
    UWMCYNFirstSignalBlueprintLibrary::RequestLocalFirstSignalRespawn(this);
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
