#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "WMCYNLocalVoiceTalker.generated.h"

class UAudioComponent;

/** Listener-local gain; never replicated and independent of microphone capture. */
UCLASS()
class WMCYNRUNTIME_API UWMCYNLocalVoiceTalker : public UVOIPTalker
{
    GENERATED_BODY()
public:
    virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;
    virtual void OnTalkingEnd() override;
    void SetListenerVolume(float Value);
    float GetListenerVolume() const { return ListenerVolume; }
private:
    float ListenerVolume = 1.0f;
    TWeakObjectPtr<UAudioComponent> Playback;
};
