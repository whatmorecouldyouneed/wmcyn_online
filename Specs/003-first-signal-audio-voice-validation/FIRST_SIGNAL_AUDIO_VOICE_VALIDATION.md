# First Signal Audio / Voice Validation

Status: In progress  
Scope: First Signal Build only  
Last updated: 2026-07-11

## Goal

Prove that two Quest users can speak and hear each other inside the persistent WMCYN Crib world, and that one PCVR recording user can hear them and capture usable audio through OBS.

This spec is the next validation gate after Quest locomotion and Crib collision became usable.

## Current Known State

- `L_WMCYNOnline` loads `BP_WMCYN_GameMode_FirstSignal`.
- `BP_WMCYN_QuestUserPawn` spawns and locomotion works in headset VR Preview.
- The pawn no longer falls through the current Crib floor lane.
- Audio output initializes through `Headphones (Oculus Virtual Audio Device)`.
- WASAPI raw mode is unsupported, but Unreal falls back successfully to 48 kHz stereo output.
- Voice encoder initialization appears in logs.
- Local headset voice capture is proven in VR Preview logs.
- User-to-user voice is not proven.
- Earlier `ToggleSpeaking 1` tests returned `StartLocalProcessing(0) returned 0xFFFFFFFF`; UE 5.8 source inspection indicates that path fails when local user `0` is not the voice engine owning user/local talker.
- `OnlineSubsystemNull` normally registers local talkers when a technical session registers local players. The product remains one persistent world with no user-facing session/access-code flow, but the implementation may need an invisible technical session or WMCYN-owned local-talker bridge.
- `[OnlineSubsystem]` now explicitly sets voice diagnostic keys: `MaxLocalTalkers=1`, `MaxRemoteTalkers=8`, `VoiceNotificationDelta=0.200000`, and `bDuckingOptOut=true`. Keep `MaxLocalTalkers=1` on Windows because UE 5.8 compiles the local voice storage for one Windows talker; the two other First Signal users are remote talkers on separate devices.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` is active again after the crash was determined to be unrelated to the WMCYN voice implementation.
- The hidden technical OnlineSubsystem session path confirmed local talker `0` registration and local voice processing success in PIE and headset VR Preview.
- The restored component calls mic threshold setup, `CreateTalkerForPlayer`, `RegisterWithPlayerState`, hidden technical session creation, `ToggleSpeaking 1`, and `online voice dump`.
- Project voice is enabled for the restored voice lane: `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.
- OpenXR reports no mapping context in project settings. Locomotion works now, but packaged Quest input should not rely on deprecated action/axis mappings forever.
- `BP_WMCYN_QuestUserPawn` inherits an AFCore `vOIPComponent` variable. Earlier inspection found it unset; it now points to WMCYN-owned `WMCYN_VOIPTalker`.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` is attached to `BP_WMCYN_QuestUserPawn` and PIE logs confirm `WMCYN Voice: registered VOIPTalker with PlayerState`.

## Product Flow For This Gate

1. User launches WMCYN Online on Quest.
2. User logs in with username/password.
3. User enters the persistent WMCYN Crib world.
4. User can move normally.
5. User speaks.
6. Other Quest user hears them.
7. PCVR recording user hears both Quest users.
8. OBS captures usable world audio from the PCVR machine.

## In Scope

- Inspect AFCore voice-related assets and active pawn voice setup.
- Confirm microphone permission path for packaged Quest builds.
- Confirm local mic capture.
- Confirm push-to-talk behavior or switch to open mic plus mute if required.
- Add the smallest WMCYN-owned voice wrapper only if AFCore does not activate voice automatically.
- Add temporary debug feedback for mic/speaking/mute state if needed.
- Confirm PCVR audio monitoring and OBS capture path.
- Document results in `Docs/FIRST_SIGNAL_AUDIO_TEST_RESULTS.md`.

## Out Of Scope

- Do not edit AFCore assets directly.
- Do not choose a final production voice backend yet.
- Do not add public matchmaking, dedicated servers, or full backend auth.
- Do not build a polished audio settings UI.
- Do not add avatar fidelity, MetaHumans, or face animation.
- Do not add Verbatim, nameplate, or identity polish until voice is no longer blocked.

## Preferred Technical Direction

Start with AFCore and Unreal voice paths.

First inspect:

- `BP_WMCYN_QuestUserPawn`
- inherited AFCore `vOIPComponent`
- AFCore VOIP / voice assets
- `SC_Voice`
- `Atten_Voice_Normal`
- `Atten_Voice_Mute`
- `Atten_Voice_UI`
- project voice config
- Android microphone permission config
- current push-to-talk mappings

If AFCore already registers VOIP correctly, use it.

If not, create a WMCYN-owned wrapper around the active WMCYN pawn or controller. Do not modify AFCore.

Suggested wrapper names if needed:

- `/Game/WMCYN/Core/BP_WMCYN_VoiceRuntimeComponent`
- `/Game/WMCYN/Core/BP_WMCYN_VoiceDebugState`
- `/Game/WMCYN/UI/WBP_WMCYN_VoiceDebug`

Only create these if inspection proves they are needed.

## Implementation Plan

### 1. Inspect Voice Runtime

- Confirmed in PIE: the active WMCYN Quest pawn registers a `VOIPTalker` during runtime.
- Confirmed in PIE: inherited `vOIPComponent` now points to WMCYN-owned `WMCYN_VOIPTalker`.
- Confirm whether local voice processing starts, stops, or never starts.
- Run `online voice dump` and capture the voice debug state.
- Confirm whether local talker `0` is registered before `ToggleSpeaking 1`.
- Confirm VR Preview starts cleanly with the restored voice implementation enabled.
- Reintroduce local talker registration only through a dedicated delayed/manual pass after startup stability is confirmed.
- Capture the exact relevant logs.

### 2. Confirm Quest Microphone Permission

- Check packaged Android/Quest config for microphone permission.
- Package a Quest build only after config is understood.
- On headset, confirm OS-level mic permission prompt or granted permission state.

### 3. Prove Local Mic Capture

- First target: local mic capture proof on one Quest.
- If headset voice testing again shows local talker `0` missing, do not re-enable immediate BeginPlay hidden session creation blindly. Use a delayed/manual WMCYN-owned activation path or a small local-talker bridge.
- Acceptable proof:
  - clear Unreal log evidence,
  - speaking indicator,
  - local loopback test,
  - or audible monitored voice path.

### 4. Decide Push-To-Talk vs Open Mic

Current config requires push-to-talk.

Test current push-to-talk first. If it blocks natural Quest conversation or conflicts with controller usage, move to:

- open mic by default,
- simple mute toggle,
- visible muted/speaking state.

Do not polish the UI yet.

### 5. Two-Quest Voice Test

Acceptance test:

- Quest A enters the Crib.
- Quest B enters the same Crib world.
- Quest A speaks and Quest B hears them.
- Quest B speaks and Quest A hears them.
- Locomotion remains stable while speaking.

### 6. PCVR Recording Audio Test

Acceptance test:

- PCVR recording user enters the same world.
- PCVR user hears Quest A and Quest B.
- OBS captures the intended output device.
- Captured audio is intelligible enough for First Signal review.

### 7. OpenXR Mapping Warning

Do not let this derail voice validation, but before repeatable Quest builds:

- identify the intended OpenXR / Enhanced Input mapping context,
- decide whether current AFCore input can remain for this pass,
- document the risk if it remains unchanged.

## Acceptance Criteria

This spec is complete when:

- Quest A mic capture is confirmed.
- Quest B mic capture is confirmed.
- Quest A and Quest B can hear each other.
- PCVR recording user can hear both Quest users.
- OBS can capture usable audio from the PCVR machine.
- The team knows whether First Signal uses push-to-talk or open mic plus mute.
- Any WMCYN voice wrapper is WMCYN-owned and does not modify AFCore.
- Results are written to `Docs/FIRST_SIGNAL_AUDIO_TEST_RESULTS.md`.

## Failure Path

If in-game voice does not work quickly:

- keep external/real microphones as the temporary recording fallback,
- document the exact Unreal/Quest voice blocker,
- continue First Signal with basic presence and OBS capture,
- defer final native voice backend selection.

Do not let native voice block the entire First Signal build unless conversation itself cannot be recorded by any path.

## Next Exact Task

Restart Unreal after the `MaxLocalTalkers=1` correction, run VR Preview with WMCYN voice restored, and confirm startup no longer crashes during `OnlineSubsystemNull` voice initialization. Expected WMCYN logs include `WMCYN Voice: hidden technical voice session created`, `WMCYN Voice: auto ToggleSpeaking 1 for VR Preview`, and `WMCYN Voice: requested online voice dump`. If local mic capture still passes, proceed to Quest-to-Quest and PCVR monitoring tests.
