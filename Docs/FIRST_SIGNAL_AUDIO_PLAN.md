# First Signal Audio Plan

## Goal

Make sure two standalone Quest users can speak and hear each other in the persistent WMCYN Crib world, while the PCVR recording user can monitor the conversation.

For First Signal, audio is a capability gate, not a polish pass.

## Current Project State

- Project voice config is already partially enabled:
  - `[OnlineSubsystem] bHasVoiceEnabled=true`
  - `[Voice] bEnabled=true`
  - `bAndroidVoiceEnabled=True`
  - `VoiPSampleRate=Normal24000Hz`
  - `VoipBufferingDelay=0.200000`
- Current online subsystem is `Null`.
- `[OnlineSubsystem]` explicitly sets `MaxLocalTalkers=1`, `MaxRemoteTalkers=8`, `VoiceNotificationDelta=0.200000`, and `bDuckingOptOut=true` for the First Signal voice diagnostic lane. One local talker is correct per headset/PC; the other users are remote talkers.
- `DefaultGame.ini` currently has `bRequiresPushToTalk=true`.
- `DefaultInput.ini` maps `PushToTalk` to:
  - `Tab`
  - `Gamepad_LeftShoulder`
  - `OculusTouch_Left_Thumbstick_Click`
  - `Gamepad_LeftThumbstick`
  - `OculusTouch_Right_Thumbstick_Click`
  - `Gamepad_RightThumbstick`
- `DefaultInput.ini` maps `VOIPLoop` to `L`, which is useful for PC/editor diagnostics but not Quest hardware.
- AFCore has audio assets that look directly relevant:
  - `/Game/AFCore/Sound/Classes/SC_Voice`
  - `/Game/AFCore/Sound/Attenuation/Atten_Voice_Normal`
  - `/Game/AFCore/Sound/Attenuation/Atten_Voice_Mute`
  - `/Game/AFCore/Sound/Attenuation/Atten_Voice_UI`
  - `/Game/AFCore/Blueprints/Widgets/Settings/Pages/Widget_Settings_Audio`
- `BP_WMCYN_QuestUserPawn` inherits an AFCore `vOIPComponent` variable typed as `VOIPTalker`. Earlier MCP inspection found it unset; it now points to WMCYN-owned `WMCYN_VOIPTalker`.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` is attached to `BP_WMCYN_QuestUserPawn` and PIE logs confirm `WMCYN Voice: registered VOIPTalker with PlayerState`.

## Latest VR Preview Audio Log Read

2026-07-10 headset VR Preview in `L_WMCYNOnline` showed:

- Audio output initialized successfully through `Headphones (Oculus Virtual Audio Device)`.
- WASAPI raw mode was not supported, but Unreal fell back successfully to a normal 48 kHz stereo render stream.
- The voice encoder loaded: `LogVoiceEncode: EncoderVersion: libopus unknown`.
- Networked voice is not proven yet. The log shows `StopLocalVoiceProcessing(0) returned 0xFFFFFFFF`, which means the current run did not establish a confirmed local voice-processing path.
- After adding automatic `ToggleSpeaking 1`, the headset log confirmed the command fires, but `StartLocalProcessing(0)` still returns `0xFFFFFFFF`.
- UE 5.8 source inspection indicates that return path means local user `0` is not registered as the voice engine's owning local talker.
- `OnlineSubsystemNull` normally registers local talkers when a technical online session registers local players. WMCYN can keep the user-facing model as one persistent world, but implementation may still need an invisible technical session or a WMCYN-owned registration bridge.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` proved the hidden technical OnlineSubsystem session path can register local talker `0` and change `StartLocalProcessing(0)` from failure to success.
- After the headset mic proof, VR Preview began crashing on startup through `OnlineSubsystemUtils` / `OnlineSubsystemNull`. Source and crash-stack inspection later identified `MaxLocalTalkers=4` as the cause: UE 5.8 Windows has fixed storage for one local talker, so initialization wrote out of bounds. Config now uses `MaxLocalTalkers=1`, and the complete WMCYN voice component remains active.
- The run also logged `No mapping context provided in the OpenXR Input project settings`; locomotion still works right now, but packaged Quest input should not depend on deprecated action/axis mappings forever. A WMCYN-only Enhanced Input push-to-talk context was tested as a default mapping context, but after restart it caused controller input to stop working because it did not include the full AFCore controller/locomotion action set. That global registration has been rolled back.

Read: audio output and local Oculus microphone capture are proven. Startup must be retested after the one-local-talker correction; push-to-talk/open-mic behavior and user-to-user voice transmission are still unverified.

## First Signal Audio Direction

Use a hybrid MVP approach:

- In-game voice proves presence and basic conversation in the world.
- External/real mics remain the backup/final recording path until in-game audio quality is proven.
- Do not block PCVR/OBS capture on perfect native voice yet.

The desired user experience is:

1. User launches WMCYN Online on Quest.
2. User logs in with username/password.
3. User enters the persistent WMCYN Crib world.
4. User's display name appears over/near their avatar.
5. User can speak in-world.
6. Nearby users can hear them.
7. PCVR recording user can hear/monitor them.
8. A mute or voice-state indicator is available before recording use.

## Important Decisions To Make

- Push-to-talk vs open mic:
  - Current config requires push-to-talk.
  - For a small Crib conversation, open mic plus mute is probably better.
  - Do not change this until the first Quest mic test proves the current behavior.
- Voice backend:
  - `Null` subsystem may be enough for local/editor/LAN-style tests.
  - A persistent online world will eventually need a real voice/network backend.
  - Do not choose EOS/Vivox/Meta voice until basic local voice behavior and build constraints are known.
- Spatial voice:
  - First Signal can start with simple proximity/spatial voice if AFCore's attenuation path works.
  - If spatial voice blocks progress, use non-spatial group voice for the first hardware proof.

## Risks

- Quest may need explicit microphone permission handling in the Android build.
- `PushToTalk` now includes left and right Quest thumbstick click/hold via legacy input mappings, with left/right gamepad thumbstick fallbacks.
- The voice registration component is currently fully parked. It does not register VOIPTalker or call Unreal voice/talker APIs. This pass should first prove VR Preview startup stability.
- WMCYN-owned Enhanced Input assets exist but are parked until a full input migration is planned:
  - `/Game/WMCYN/Input/IA_WMCYN_PushToTalk`
  - `/Game/WMCYN/Input/IMC_WMCYN_FirstSignal_VR`
- The WMCYN First Signal PlayerController has an enhanced-action branch, but this should not be relied on while AFCore controller input remains on the legacy mapping path.
- `VoiPSoundClass` currently points at `SC_SFX`, while AFCore also has `SC_Voice`; this may be intentional, but should be inspected before changing.
- The active pawn now has WMCYN-owned VOIPTalker registration. This removes the first runtime setup blocker, but it does not prove mic capture or user-to-user voice.
- OnlineSubsystem `Null` is not a final persistent-world backend.

## Implementation Steps

1. Confirm the packaged Quest app requests/has microphone permission.
2. Confirm local Quest microphone capture is possible.
3. Restart Unreal and run VR Preview with WMCYN voice fully parked and project voice disabled.
4. Confirm VR Preview no longer crashes on startup.
5. If stable, keep voice parked while First Signal presence/spawn work continues, then design a safer delayed/manual voice activation path in a dedicated voice pass.
6. Confirm push-to-talk behavior in headset, or switch to open mic plus mute if push-to-talk blocks natural conversation.
7. Confirm Quest A can hear Quest B in the same world.
8. Confirm Quest B can hear Quest A in the same world.
9. Confirm PCVR recording user can hear both Quest users.
10. Confirm OBS captures the intended audio path.
11. Add a small WMCYN voice state surface:
   - muted/unmuted
   - speaking indicator
   - local mic permission/status
12. Document the final recording workflow for First Signal.

## Acceptance Gate

- Quest user A speaks and Quest user B hears them.
- Quest user B speaks and Quest user A hears them.
- PCVR recording user can hear/monitor both Quest users.
- The voice workflow is understandable to the users.
- OBS has a usable recording path.
- No AFCore assets are edited directly.
