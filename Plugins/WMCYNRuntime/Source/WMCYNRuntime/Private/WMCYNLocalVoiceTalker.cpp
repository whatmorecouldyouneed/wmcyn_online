#include "WMCYNLocalVoiceTalker.h"
#include "Components/AudioComponent.h"

void UWMCYNLocalVoiceTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
    Super::OnTalkingBegin(AudioComponent);
    Playback = AudioComponent;
    if (Playback.IsValid()) Playback->SetVolumeMultiplier(ListenerVolume);
}

void UWMCYNLocalVoiceTalker::OnTalkingEnd()
{
    Playback.Reset();
    Super::OnTalkingEnd();
}

void UWMCYNLocalVoiceTalker::SetListenerVolume(float Value)
{
    if (!FMath::IsFinite(Value)) return;
    ListenerVolume = FMath::Clamp(Value, 0.0f, 1.0f);
    if (Playback.IsValid()) Playback->SetVolumeMultiplier(ListenerVolume);
}
