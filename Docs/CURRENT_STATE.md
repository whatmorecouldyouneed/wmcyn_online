# Current State

## Project

- WMCYN Online is an Unreal Engine 5.8 project.
- Current target: First Signal Build inside The WMCYN Crib.
- Canonical outcome: two standalone VR users and one PCVR recording user in the same persistent world, with basic presence, display names, voice, OBS-friendly capture, and one Verbatim marker.
- Active map: `/Game/Levels/L_WMCYNOnline` using the WMCYN Crib environment.
- Product flow: username/password login -> persistent WMCYN world -> user identity appears in-world. There is no user-facing host/guest flow, session picker, or access code.

## AFCore Baseline

- HumanCodeable AFCore / Advanced VR Framework is present under `/Game/AFCore` and has already been tested.
- AFCore remains the baseline for locomotion, hands/controllers, grabbing, VR menus, replication helpers, and multiplayer-compatible pawn behavior.
- Do not edit `/Game/AFCore/*` by default. Use WMCYN children, wrappers, or duplicates first.
- AFCore documentation is indexed in `Docs/AFCORE_DOCUMENTATION_INDEX.md`.

## Active Runtime Path

- `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` is the active First Signal VR pawn. It is a WMCYN-owned child of `/Game/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Char`.
- `/Game/WMCYN/Core/BP_WMCYN_GameMode_FirstSignal` is active in `L_WMCYNOnline`.
- `/Game/WMCYN/Core/BP_WMCYN_PlayerController_FirstSignal` forces the VR pawn path and syncs the spawned pawn into AFCore's `playingPawn` reference.
- `/Game/WMCYN/Data/DA_WMCYN_FirstSignal_Level` points at `L_WMCYNOnline` and selects `BP_WMCYN_QuestUserPawn` as the default VR pawn.
- Spawn markers remain:
  - `SPAWN_FirstSignal_StandaloneVR_A`
  - `SPAWN_FirstSignal_StandaloneVR_B`
  - `SPAWN_FirstSignal_PCVR_Recording`
- `PLAYERSTART_FirstSignal_Default` provides deterministic local VR Preview entry at StandaloneVR_A.
- `/Game/WMCYN/Core/BP_WMCYN_VRPreviewStabilizer` sets LocalFloor tracking, resets HMD orientation/position, and snaps the local test pawn to StandaloneVR_A.
- Hardware VR Preview confirms locomotion, headset tracking, controllers/hands, and the active Crib floor path work without the old floor guard.
- Imported Crib collision currently uses repaired complex-as-simple collision on the relevant floor components. `Plane_003` / `/Game/Environments/WMCYN_Crib/Scene_058` is the verified active walkable floor near spawn.
- Jared MetaHuman assets are outside the active First Signal path. Basic AFCore/default VR presence is the accepted avatar baseline.

## Identity and Login

- `/Game/WMCYN/UI/WBP_WMCYN_LoginJoin` visually presents username, password, and Enter World.
- Its older prototype graph still needs to be connected cleanly to WMCYN identity and persistent-world entry.
- `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal` owns replicated First Signal fields:
  - `Username`
  - `DisplayName`
  - `PresenceMode`
  - `Capabilities`
- The inherited AFCore NameTag or a small WMCYN wrapper remains the preferred display-name path.

## Voice State

- Project voice is enabled with `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.
- `[OnlineSubsystem]` uses `MaxLocalTalkers=1`, `MaxRemoteTalkers=8`, `VoiceNotificationDelta=0.200000`, and `bDuckingOptOut=true`.
- `MaxLocalTalkers` must remain `1` on Windows UE 5.8. Windows compiles `MAX_SPLITSCREEN_TALKERS` as one; the previous value of four caused `FVoiceEngineImpl::Init` to write beyond its fixed local voice array and crash VR Preview during `OnlineSubsystemNull` startup.
- `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` owns `WMCYN_VOIPTalker`, attached to the inherited AFCore head path and using AFCore voice attenuation.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` is active and WMCYN-owned. It sets mic threshold, creates/registers the talker, creates the hidden technical OnlineSubsystem session, runs `ToggleSpeaking 1`, and requests `online voice dump`.
- The hidden technical session is Unreal voice plumbing only. It does not change the persistent-world product model.
- Latest VR Preview test passes:
  - no startup crash
  - voice module/interface available and enabled
  - local talker `0` registered and networked
  - `StartLocalProcessing(0) returned 0x00000000` after registration
  - `IsRecording: 1`
  - TALKING event fired
  - repeated non-zero compressed Oculus microphone packets flowed
- The initial `StartLocalProcessing(0) returned 0xFFFFFFFF` happens before local talker registration; the automatic post-registration call succeeds.
- `Invalid user specified in RegisterLocalTalker(1..3)` is harmless extra-slot diagnostic noise. First Signal uses one local user per device.
- Local headset microphone capture is proven. Quest-to-Quest hearing, PCVR monitoring, OBS audio capture, and packaged Quest microphone permission remain unproven.

## Input and Avatar Notes

- AFCore legacy VR controller mappings currently preserve working locomotion/controllers.
- The partial WMCYN Enhanced Input push-to-talk context is parked because registering it globally displaced the working AFCore input path after restart.
- The visible AFCore avatar appeared roughly 1/4 to 1/6 expected size in one test. Actor/component scale and `worldToMeters` inspected as normal, so this remains a visual-avatar issue rather than a global world-scale change.

## MCP

- Unreal MCP is reachable at `http://127.0.0.1:8000/mcp` when the editor server is running.
- Editor toolsets expose actor, scene, asset, Blueprint, object, material, and related inspection/editing capabilities.
- Inspect before mutating, and do not modify maps/assets unless the active task authorizes it.

## Operational Blocker

- `C:` reached zero free bytes during the latest run.
- Zen Local DDC returned HTTP `507 Insufficient Storage`.
- Old generated crash reports and an unused temporary WMCYN voice autosave copy were removed to recover minimal working space; no project or AFCore assets were deleted.
- Free or relocate substantial storage before packaging, cooking, shader/texture cache growth, or sustained editor work.

## Next Gate

1. Free disk space and confirm Zen DDC writes normally.
2. Run Quest user A and Quest user B in the same networked world runtime and prove two-way voice/presence.
3. Join with the PCVR recording user and prove both Quest users are visible and audible.
4. Verify OBS capture.
5. Then connect login identity/nameplate and implement one Verbatim marker.
