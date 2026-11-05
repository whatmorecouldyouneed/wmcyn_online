#include "WMCYNKeyboardInteractionComponent.h"

#include "InputCoreTypes.h"

FString WMCYNKeyboard::TextForKey(const FKey& Key)
{
    if (Key == EKeys::BackSpace) return TEXT("\b");
    if (Key == EKeys::SpaceBar) return TEXT(" ");
    if (Key == EKeys::Tab) return TEXT("\t");
    if (Key == EKeys::Enter) return TEXT("\n");
    return FString();
}

bool UWMCYNKeyboardInteractionComponent::PressKey(FKey Key, bool bRepeat)
{
#if PLATFORM_ANDROID
    const FString Text = WMCYNKeyboard::TextForKey(Key);
    return Text.IsEmpty() ? true : SendKeyChar(Text, bRepeat);
#else
    return Super::PressKey(Key, bRepeat);
#endif
}

bool UWMCYNKeyboardInteractionComponent::ReleaseKey(FKey Key)
{
#if PLATFORM_ANDROID
    return true;
#else
    return Super::ReleaseKey(Key);
#endif
}
