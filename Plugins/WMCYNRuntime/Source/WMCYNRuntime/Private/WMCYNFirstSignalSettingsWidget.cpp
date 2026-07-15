#include "WMCYNFirstSignalSettingsWidget.h"

#include "Components/Button.h"
#include "WMCYNFirstSignalBlueprintLibrary.h"

void UWMCYNFirstSignalSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RespawnButton)
    {
        RespawnButton->OnClicked.AddUniqueDynamic(this, &UWMCYNFirstSignalSettingsWidget::HandleRespawnClicked);
    }
}

void UWMCYNFirstSignalSettingsWidget::NativeDestruct()
{
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
