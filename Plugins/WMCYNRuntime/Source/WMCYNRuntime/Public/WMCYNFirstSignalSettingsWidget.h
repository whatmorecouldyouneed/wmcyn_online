#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WMCYNFirstSignalSettingsWidget.generated.h"

class UButton;
class USlider;
class USoundClass;
class USoundMix;
class UUserWidget;

UCLASS()
class WMCYNRUNTIME_API UWMCYNFirstSignalSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleRespawnClicked();

    UFUNCTION()
    void HandleMasterVolumeChanged(float Value);

    UFUNCTION()
    void HandleMusicVolumeChanged(float Value);

    UFUNCTION()
    void HandleVoiceVolumeChanged(float Value);

    UFUNCTION()
    void HandleSFXVolumeChanged(float Value);

    void ConfigureAudioSettings();
    void RemoveAudioBindings();
    USlider* ResolveNativeSlider(FName WidgetName) const;
    void ApplySoundClassVolume(USoundClass* SoundClass, float Value, bool bApplyToChildren);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUserWidget> AudioSettings;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> RespawnButton;

    UPROPERTY(Transient)
    TObjectPtr<USlider> MasterSlider;

    UPROPERTY(Transient)
    TObjectPtr<USlider> MusicSlider;

    UPROPERTY(Transient)
    TObjectPtr<USlider> VoiceSlider;

    UPROPERTY(Transient)
    TObjectPtr<USlider> SFXSlider;

    UPROPERTY(Transient)
    TObjectPtr<USoundMix> DefaultSoundMix;

    UPROPERTY(Transient)
    TObjectPtr<USoundClass> MasterSoundClass;

    UPROPERTY(Transient)
    TObjectPtr<USoundClass> MusicSoundClass;

    UPROPERTY(Transient)
    TObjectPtr<USoundClass> VoiceSoundClass;

    UPROPERTY(Transient)
    TObjectPtr<USoundClass> SFXSoundClass;
};
