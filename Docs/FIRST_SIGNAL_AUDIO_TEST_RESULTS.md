# First Signal Audio Test Results

Last updated: 2026-07-11

## Test 001 - VR Preview Audio / Voice Log Review

Context:

- Map: `L_WMCYNOnline`
- Runtime: VR Preview / PIE
- GameMode: `BP_WMCYN_GameMode_FirstSignal_C`
- Active path: WMCYN Quest pawn synced to `StandaloneVR_A`

Relevant observed log pattern:

- Audio device registers with `L_WMCYNOnline`.
- Audio bus subsystem initializes.
- `BP_WMCYN_GameMode_FirstSignal_C` loads.
- Enhanced Input local player subsystem initializes.
- Voice encoder initializes with `libopus`.
- `StopLocalVoiceProcessing(0)` returns `0xFFFFFFFF`.
- Networked voice stops for user `0`.
- HMD spectator mode is set.
- HMD recentering reports no valid head pose at startup.
- WMCYN stabilizer resets LocalFloor tracking.
- WMCYN controller syncs Quest pawn to `StandaloneVR_A`.
- WMCYN stabilizer snaps Quest pawn to `StandaloneVR_A`.
- OpenXR warns that no mapping context is configured in project settings.

## Interpretation

Working:

- The WMCYN level/runtime path is loading correctly.
- The WMCYN Quest pawn spawn/sync/stabilizer path is active.
- Audio output device registration is working.
- Unreal voice encoder is available.
- Locomotion and runtime startup are not blocked by the audio warnings.

Not proven:

- Microphone capture is not proven.
- Local voice processing is not proven.
- Network/user-to-user voice is not proven.
- Quest-to-Quest voice is not proven.
- PCVR monitoring and OBS audio capture are not proven.

## Test 002 - WMCYN VOIPTalker Registration Hook

Context:

- Runtime: MCP in-viewport PIE smoke test, not headset hardware.
- Active pawn: `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn`
- New WMCYN-owned component: `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent`
- No AFCore assets were edited.

Implementation:

- `BP_WMCYN_QuestUserPawn` has a WMCYN-owned `WMCYN_VOIPTalker` component.
- The inherited AFCore `vOIPComponent` reference now points to `WMCYN_VOIPTalker`.
- `WMCYN_VOIPTalker` uses AFCore `SMesh_Framework_Head` for attachment and AFCore `Atten_Voice_Normal` for voice attenuation.
- `BP_WMCYN_VoiceRegistrationComponent` is attached to `BP_WMCYN_QuestUserPawn`.
- On BeginPlay, the registration component waits briefly, gets the owning WMCYN Quest pawn, validates its VOIPTalker and PlayerState, then calls `RegisterWithPlayerState`.

Relevant observed log:

- `WMCYN Voice: registered VOIPTalker with PlayerState`

Result:

Status: Registration smoke pass.

The active WMCYN Quest pawn now creates and registers a VOIPTalker in PIE. This is necessary groundwork for in-game voice, but it is not proof of working microphone capture or user-to-user voice.

## Test 003 - Local Talker Creation Experiment

Context:

- User headset test after `ToggleSpeaking 1` still logged `StartLocalProcessing(0) returned 0xFFFFFFFF`.
- Unreal also logged `VoiceCapture device overridden by HMD`, so the headset mic route is visible.
- WMCYN voice registration already logs successfully.

Implementation:

- Updated `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent`.
- On BeginPlay it now:
  - sets mic threshold to `0.0`,
  - calls `CreateTalkerForPlayer` using the pawn PlayerState,
  - keeps the existing WMCYN VOIPTalker `RegisterWithPlayerState` call.

MCP PIE smoke result:

- `WMCYN Voice: CreateTalkerForPlayer called`
- `WMCYN Voice: registered VOIPTalker with PlayerState`

Status: Ready for headset retest.

This may or may not fix `StartLocalProcessing(0)`. If the same error remains after retesting, the blocker is likely below the Blueprint talker layer: local voice capture / OnlineSubsystem Null / editor VR Preview voice runtime.

Current blocker signal:

- `StopLocalVoiceProcessing(0) returned 0xFFFFFFFF` keeps appearing. Treat this as evidence that the current run is not establishing a valid local voice-processing path.

Non-blocking warnings for this pass:

- `AUDCLNT_E_RAW_MODE_UNSUPPORTED` is not currently blocking audio output because Unreal falls back to normal output.
- 10-bit swapchain fallback is not blocking this audio test.
- HMD recentering warning appears at startup but does not currently block locomotion.

Packaging warning:

- The OpenXR mapping-context warning should be cleaned up or explicitly accepted before repeatable Quest package validation. Current VR Preview locomotion works, but packaged builds should not depend on deprecated XR action/axis mappings forever.
- WMCYN tested a dedicated Enhanced Input action/context for the First Signal voice test. After restart, OpenXR consumed it, but controllers stopped working because the context did not include the full AFCore controller/locomotion bindings.
- The global default mapping context registration was rolled back. Keep AFCore legacy controller mappings active until a complete Enhanced Input migration exists.

## Result

Status: Partial pass with VOIP registration hook in place.

Audio output, runtime startup, and pawn VOIPTalker registration are acceptable. `ToggleSpeaking 1` now runs automatically after VOIPTalker registration for VR Preview testing. Voice input/transmission remains unproven.

## Test 004 - Auto ToggleSpeaking / Local Talker Diagnosis

Context:

- User headset VR Preview log after the automatic `ToggleSpeaking 1` pass.
- Controllers and locomotion were restored by removing the partial WMCYN Enhanced Input mapping context from global default mappings.

Relevant observed log:

- `WMCYN Voice: CreateTalkerForPlayer called`
- `WMCYN Voice: registered VOIPTalker with PlayerState`
- `Cmd: ToggleSpeaking 1`
- `StartLocalProcessing(0) returned 0xFFFFFFFF`
- `Starting networked voice for user: 0`
- `WMCYN Voice: auto ToggleSpeaking 1 for VR Preview`

Interpretation:

- The WMCYN VOIPTalker registration component is firing.
- The automatic speak toggle is firing.
- The remaining failure is below the VOIPTalker and button-input layer.
- UE 5.8 source inspection shows `StartLocalVoiceProcessing(0)` returns failure when local user `0` is not the voice engine's registered owning user.
- `OnlineSubsystemNull` normally registers local talkers when a technical online session registers local players. The current standalone VR Preview lane does not appear to create that local talker path before `ToggleSpeaking 1`.

Config cleanup:

- Added explicit `[OnlineSubsystem]` voice keys:
  - `MaxLocalTalkers=4`
  - `MaxRemoteTalkers=8`
  - `VoiceNotificationDelta=0.200000`
  - `bDuckingOptOut=true`

Status: local talker registration still needs a First Signal-owned solution or a hidden technical session path.

Next diagnostic command for headset/PIE logs:

```text
online voice dump
```

Expected useful output includes whether the voice module/interface are available, local/remote talker limits, push-to-talk state, and current voice debug state.

## Test 005 - Hidden Technical Voice Session Smoke Test

Context:

- Runtime: in-viewport PIE smoke test, not headset VR Preview.
- Asset changed: `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent`.
- AFCore assets were not edited.

Implementation:

- After VOIPTalker registration, the WMCYN voice component now creates a hidden technical OnlineSubsystem session with:
  - `PublicConnections=4`
  - `bUseLAN=true`
  - `bUseLobbiesIfAvailable=false`
- On session success, it waits briefly, runs `ToggleSpeaking 1`, then runs `online voice dump`.
- On session failure, it still runs `ToggleSpeaking 1` and `online voice dump` so the log remains diagnostic.
- This is not a user-facing session, access code, lobby, or picker. It is an Unreal voice/runtime registration helper for the persistent WMCYN world.

Relevant observed log:

- `RegisterLocalTalker(0) returned 0x00000000`
- `WMCYN Voice: hidden technical voice session created`
- `VOIP StartRecording`
- `StartLocalProcessing(0) returned 0x00000000`
- `WMCYN Voice: auto ToggleSpeaking 1 for VR Preview`
- `online voice dump`
- `Voice Module Available: true`
- `Voice Interface Available: true`
- `IsRecording: 1`
- Local talker `[0]` reports:
  - `Registered: 1`
  - `Networked: 1`
- `ReadLocalVoiceData` began returning non-zero available byte counts.
- Logs began triggering `Local 0 TALKING` / `Local 0 NOTTALKING`.

Result:

Status: local voice registration smoke pass in PIE.

The missing layer was local talker registration. The hidden technical session caused `OnlineSubsystemNull` to register local talker `0`, after which `StartLocalProcessing(0)` returned success.

Still not proven:

- headset VR Preview mic capture after editor restart,
- Quest packaged mic permission,
- Quest-to-Quest audio,
- PCVR monitoring,
- OBS capture.

Note:

- The current editor process still reported old voice config defaults in `online voice dump` (`Max Local Talkers: 1`, etc.). Restart Unreal before the next headset test so the new `DefaultEngine.ini` voice diagnostic keys load.

## Test 006 - Headset VR Preview Local Voice Capture

Context:

- Runtime: headset VR Preview / PIE in `L_WMCYNOnline`.
- Capture device observed: `Headset Microphone (Oculus Virtual Audio Device)`.
- AFCore assets were not edited.
- WMCYN still follows the AFCore documentation guidance to reuse framework-compatible multiplayer/session paths before building custom networking; the hidden technical session is only an internal Unreal voice registration helper.

Relevant observed log:

- `WMCYN Voice: CreateTalkerForPlayer called`
- `WMCYN Voice: registered VOIPTalker with PlayerState`
- `RegisterLocalTalker(0) returned 0x00000000`
- `WMCYN Voice: hidden technical voice session created`
- `VOIP StartRecording`
- `StartLocalProcessing(0) returned 0x00000000`
- `WMCYN Voice: auto ToggleSpeaking 1 for VR Preview`
- `online voice dump`
- `Voice Module Available: true`
- `Voice Interface Available: true`
- `IsRecording: 1`
- Local talker `[0]` reports:
  - `Registered: 1`
  - `Networked: 1`
- `ReadLocalVoiceData` returned non-zero available byte counts such as `866`, `461`, `434`, and `488`.
- `OpusEncode` encoded local voice frames.
- Logs triggered `Local 0 TALKING` and `Local 0 NOTTALKING`.

Result:

Status: local headset mic capture pass.

The WMCYN hidden technical session successfully registers local talker `0` in headset VR Preview. Local voice processing starts, the Oculus virtual headset microphone is selected, local audio data is captured/encoded, and Unreal emits local talking state events.

Still not proven:

- Quest packaged mic permission.
- Quest user A hearing Quest user B.
- Quest user B hearing Quest user A.
- PCVR recording user hearing all three Quest users.
- OBS capture of intended output.

Notes:

- The run used `Speakers (Realtek(R) Audio)` for render output while using the Oculus virtual headset microphone for capture. For the PCVR recording pass, confirm the intended output device/OBS source.
- `online voice dump` still reported `Max Local Talkers: 1`, `Max Remote Talkers: 16`, and `Ducking Opt Out Enabled: false`. This does not block the one-local-user capture proof, but the config mismatch should be rechecked before multi-user packaging.

## Test 007 - VR Preview Crash Isolation

Context:

- After the successful headset local mic capture proof, VR Preview began crashing on startup.
- Latest crash context shows an access violation with the call stack passing through `OnlineSubsystemUtils` and `OnlineSubsystemNull`.
- The crash log reaches voice module startup and HMD voice capture selection, then stops before normal WMCYN voice registration messages complete.

Action:

- Parked the automatic hidden technical session startup in `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent`.
- Parked automatic `ToggleSpeaking 1` and `online voice dump` on `BeginPlay`.
- A follow-up in-viewport PIE smoke run still crashed through the same `OnlineSubsystemUtils` / `OnlineSubsystemNull` stack before WMCYN voice registration messages appeared.
- Fully parked the WMCYN voice component startup. It now only prints `WMCYN Voice: registration component parked for OnlineSubsystemNull crash isolation`.
- Temporarily disabled Unreal voice in `Config/DefaultEngine.ini`:
  - `bHasVoiceEnabled=false`
  - `[Voice] bEnabled=false`
  - `bAndroidVoiceEnabled=False`
- AFCore assets were not edited.

Expected next VR Preview log:

- `WMCYN Voice: registration component parked for OnlineSubsystemNull crash isolation`
- No `CreateTalkerForPlayer`
- No `RegisterWithPlayerState`
- No hidden technical session
- No `ToggleSpeaking 1`

Status: superseded.

Follow-up:

- The crash was later determined to be unrelated to the WMCYN voice implementation and tied to a `DefaultEngine.ini` issue.
- The working voice path should be restored rather than kept parked.

Status: superseded by Test 008.

## Test 008 - Voice Implementation Restored

Context:

- The unrelated config issue has been fixed.
- The previously proven WMCYN voice path is being restored because it worked as expected in headset VR Preview.
- AFCore assets were not edited.

Implementation restored:

- `Config/DefaultEngine.ini` voice flags are enabled again:
  - `bHasVoiceEnabled=true`
  - `[Voice] bEnabled=true`
  - `bAndroidVoiceEnabled=True`
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` is active again.
- On BeginPlay it:
  - waits briefly,
  - sets mic threshold to `0.0`,
  - casts its owner to `BP_WMCYN_QuestUserPawn`,
  - gets PlayerState and the inherited AFCore `VOIPComponent`,
  - calls `CreateTalkerForPlayer`,
  - calls `RegisterWithPlayerState`,
  - creates the hidden technical OnlineSubsystem session,
  - runs `ToggleSpeaking 1`,
  - runs `online voice dump`.

Expected restored logs:

- `WMCYN Voice: CreateTalkerForPlayer called`
- `WMCYN Voice: registered VOIPTalker with PlayerState`
- `WMCYN Voice: hidden technical voice session created`
- `StartLocalProcessing(0) returned 0x00000000`
- `WMCYN Voice: auto ToggleSpeaking 1 for VR Preview`
- `WMCYN Voice: requested online voice dump`

Status: implementation restored.

## Test 009 - Restored Voice Retest / Config Cache

Context:

- Runtime: VR Preview / PIE in `L_WMCYNOnline`.
- The restored WMCYN voice Blueprint path fired successfully.
- On disk, `Config/DefaultEngine.ini` has `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.

Relevant observed log:

- `WMCYN Voice: CreateTalkerForPlayer called`
- `WMCYN Voice: registered VOIPTalker with PlayerState`
- `WMCYN Voice: hidden technical voice session created`
- `WMCYN Voice: auto ToggleSpeaking 1 for VR Preview`
- `WMCYN Voice: requested online voice dump`
- `LogOnlineVoice: OSS: Voice interface disabled by config [OnlineSubsystem].bHasVoiceEnabled`
- `Voice Module Available: false`
- `Voice Interface Available: false`
- `Max Local Talkers: 4`
- `Max Remote Talkers: 8`
- `Ducking Opt Out Enabled: true`

Interpretation:

- The WMCYN voice implementation is restored and executing.
- The editor/runtime config cache still has `bHasVoiceEnabled=false` from the earlier crash-isolation pass.
- The project config file is already correct on disk; no further Blueprint change is indicated by this log.

Status: restart required.

## Test 010 - UE 5.8 Windows Local-Talker Overflow Diagnosis

Context:

- After the editor reloaded voice-enabled config, VR Preview crashed before WMCYN Blueprint BeginPlay.
- The same crash reproduced across four reports.

Observed evidence:

- Access violation: `0xffffffffffffffff`.
- Call stack: `OnlineSubsystemUtils` -> `OnlineSubsystemNull` during world startup.
- The log ended immediately after `LogVoiceEncode: EncoderVersion: libopus unknown`.
- No WMCYN voice registration messages appeared, so `BP_WMCYN_VoiceRegistrationComponent` had not executed.
- UE 5.8 source defines `MAX_SPLITSCREEN_TALKERS` as `1` on Windows.
- `FVoiceEngineImpl` stores `PlayerVoiceData[MAX_SPLITSCREEN_TALKERS]`, but its initialization loop uses configured `MaxLocalTalkers` as the upper bound.
- Project config had `MaxLocalTalkers=4`, causing three out-of-bounds writes during voice-engine initialization.

Correction:

- Set `[OnlineSubsystem] MaxLocalTalkers=1`.
- Keep `MaxRemoteTalkers=8`.
- Preserve the restored WMCYN voice Blueprint implementation.
- AFCore assets were not edited.

Status: passed by Test 011.

## Test 011 - Restored Voice Startup and Local Capture Pass

Observed:

- VR Preview reached WMCYN BeginPlay without crashing.
- `RegisterLocalTalker(0) returned 0x00000000`.
- After registration, `StartLocalProcessing(0) returned 0x00000000`.
- `Voice Module Available: true` and `Voice Interface Available: true`.
- `Max Local Talkers: 1` and `Max Remote Talkers: 8`.
- Local talker `0` reported `Registered: 1`, `Networked: 1`, and `Talking: 1`.
- `IsRecording: 1`.
- Repeated `ReadLocalVoiceData` calls returned non-zero compressed data, primarily 29- and 56-byte packets.
- PIE shut down normally and destroyed the online subsystem cleanly.

Interpretation:

- The `MaxLocalTalkers=1` correction resolved the startup access violation.
- The restored WMCYN voice implementation is active and local Oculus microphone capture is working.
- The first `StartLocalProcessing(0) returned 0xFFFFFFFF` is expected ordering noise before the hidden technical session registers user `0`; the post-registration call succeeds.
- Attempts to register users `1..3` are harmless warnings for unavailable local slots. First Signal uses one local user per device.
- Zen DDC separately reported HTTP `507 Insufficient Storage`; this is a disk/cache capacity issue, not a voice failure.

Status: local VR Preview voice capture passed. Multi-device transmission remains unproven.

## Next Test

Run a restored voice retest:

1. Free or relocate enough disk/cache storage to stop Zen DDC `507` errors.
2. Run Quest user A and Quest user B in the same networked world runtime.
3. Confirm each user hears the other.
4. Join from PCVR and confirm the recording user hears all three Quest users.
5. Verify the intended voice mix reaches OBS.
