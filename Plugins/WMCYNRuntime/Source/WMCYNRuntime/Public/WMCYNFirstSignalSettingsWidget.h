#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "WMCYNFirstSignalSettingsWidget.generated.h"

class UButton;
class USlider;
class USoundClass;
class USoundMix;
class UUserWidget;
class UTextBlock;
class APlayerState;
class UImage;
class UWMCYNCameraToolComponent;

UCLASS()
class WMCYNRUNTIME_API UWMCYNFirstSignalSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION() void HandleCameraShotSaveClicked();
    UFUNCTION() void HandleCameraShotSelectClicked();
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UComboBoxString> CameraShotSelector;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CameraShotSaveButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CameraShotSelectButton;
    UFUNCTION() void HandleCameraClicked();
    UFUNCTION() void HandleCameraResetClicked();
    UFUNCTION() void HandleCameraOutputClicked();
    UFUNCTION() void HandleCameraFOVChanged(float Value);
    UWMCYNCameraToolComponent* ResolveCameraTool();
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CameraButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CameraResetButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CameraOutputButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> CameraStatus;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UImage> CameraPreview;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> CameraFOV;
    UFUNCTION() void HandleMicClicked();
    UFUNCTION() void HandleMirrorClicked();
    UFUNCTION() void HandleResetPanelClicked();
    UFUNCTION() void HandlePinPanelClicked();
    UFUNCTION() void HandleRemoteSelected(FString Item, ESelectInfo::Type SelectionType);
    UFUNCTION() void HandleRemoteVolumeChanged(float Value);
    void RefreshTools();
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> MicButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> MirrorButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ResetPanelButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> PinPanelButton;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> ConnectionStatus;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> MicrophoneStatus;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UComboBoxString> RemotePlayerSelector;
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USlider> RemoteVoiceVolume;
    TMap<FString, TWeakObjectPtr<APlayerState>> RemotePlayers;
    TWeakObjectPtr<APlayerState> SelectedRemotePlayer;
    FTimerHandle ToolsTimer;
    FTimerHandle VisualStyleTimer;
    void ApplyAudioVisualStyle();
    bool bPanelPinned = false;

    UFUNCTION()
    void HandleSignOutClicked();

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SignOutButton;

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
