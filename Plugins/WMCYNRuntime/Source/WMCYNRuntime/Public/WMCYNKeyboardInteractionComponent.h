#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "WMCYNKeyboardInteractionComponent.generated.h"

namespace WMCYNKeyboard
{
    /** The text a function key should type. Empty means the key types nothing. */
    WMCYNRUNTIME_API FString TextForKey(const FKey& Key);
}

/**
 * Carries AFCore's in-world keyboard input. On Android, Slate receives a function key's platform
 * keycode as a character, so Delete and friends type letters. Sending text directly avoids that
 * without editing AFCore.
 */
UCLASS()
class WMCYNRUNTIME_API UWMCYNKeyboardInteractionComponent : public UWidgetInteractionComponent
{
    GENERATED_BODY()
public:
    virtual bool PressKey(FKey Key, bool bRepeat = false) override;
    virtual bool ReleaseKey(FKey Key) override;
};
