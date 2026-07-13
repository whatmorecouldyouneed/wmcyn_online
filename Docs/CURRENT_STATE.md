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
- `/Game/WMCYN/Core/BP_WMCYN_PlayerController_FirstSignal` forces the VR pawn path, syncs the spawned pawn into AFCore's `playingPawn` reference, and assigns indexed presence slots on authority.
- `/Game/WMCYN/Core/BP_WMCYN_GameMode_FirstSignal` owns the runtime presence counter. Local three-client PIE assigns `StandaloneVR_A = 0`, `StandaloneVR_B = 1`, and `PCVR_Recording = 2` without overlap.
- `/Game/WMCYN/Data/DA_WMCYN_FirstSignal_Level` points at `L_WMCYNOnline` and selects `BP_WMCYN_QuestUserPawn` as the default VR pawn.
- Spawn markers remain:
  - `SPAWN_FirstSignal_StandaloneVR_A`
  - `SPAWN_FirstSignal_StandaloneVR_B`
  - `SPAWN_FirstSignal_PCVR_Recording`
- `PLAYERSTART_FirstSignal_Default` provides deterministic local VR Preview entry at StandaloneVR_A.
- `/Game/WMCYN/Core/BP_WMCYN_VRPreviewStabilizer` sets LocalFloor tracking and resets local HMD orientation/position. Its spawn correction is authority-only so network clients do not overwrite the server-assigned slot before PlayerState replication arrives.
- Hardware VR Preview confirms locomotion, headset tracking, controllers/hands, and the active Crib floor path work without the old floor guard.
- Imported Crib collision currently uses repaired complex-as-simple collision on the relevant floor components. `Plane_003` / `/Game/Environments/WMCYN_Crib/Scene_058` is the verified active walkable floor near spawn.
- Jared MetaHuman assets are outside the active First Signal path. Basic AFCore/default VR presence is the accepted avatar baseline.

## Identity and Login

- `/Game/WMCYN/UI/WBP_WMCYN_LoginJoin` now contains only username, password, Enter World, and status UI in its active entry path.
- `WBP_WMCYN_LoginJoin` keeps native Unreal `EditableTextBox` fields and inherits AFCore `Widget_Base` so it can reuse AFCore overlay plumbing without embedding AFCore's unstable input wrapper.
- Selecting username or password spawns the existing AFCore `BP_Overlay_Widget_Keyboard` with `Widget_Keyboard_US`. The WMCYN wrapper stores one overlay instance, reuses it while switching fields, and explicitly focuses the selected field through AFCore's active `WidgetInteractionComponent` virtual user.
- Headset testing showed that the first lowered keyboard was visible but non-interactive: it remained coplanar with and partly inside the login `Comp_Widget` component's invisible 1600x1000 hit-test rectangle, so the login panel won the controller trace before the keyboard.
- The keyboard now uses the login root geometry and AFCore's canonical below-widget placement rule (`X=0.5`, `Y=1.05`, top pivot). This puts the keyboard fully beyond the login component's bottom collision edge while retaining the AFCore overlay, keyboard layout, source `WidgetInteractionComponent`, and selected-field focus.
- A second headset test proved keyboard hover but not press. The login manager had been using `Game and UI` input mode; once a native text box gained focus, that mode could consume the controller trigger before AFCore's laser Select path dispatched `PressPointerKey`. The login gate now stays in `Game Only` while movement remains locked, which preserves AFCore controller input and world-space widget interaction.
- Username and password now use explicit near-black normal/focused/read-only text colors on the white field background. Their `Username` and `Password` hint text no longer inherits the white parent foreground.
- Do not place `/Game/AFCore/Blueprints/Widgets/Core/Input/Widget_Input_TextBox` in WMCYN Designer trees. In UE 5.8 its AFCore design-time theme/`AFCore_Border` preview path causes a repeatable Slate/UMGEditor access violation during Designer insertion or thumbnail/autosave generation.
- Latest headset proof confirms the `Game Only` login gate receives controller clicks, selects both native fields, and types visible username/password text through the AFCore keyboard.
- `WBP_WMCYN_LoginJoin` now routes both `BTN_EnterWorld.OnClicked` and password `OnTextCommitted(OnEnter)` through one WMCYN-owned `SubmitLogin` function. Successful submission closes the AFCore keyboard overlay, restores game input and locomotion, and destroys the owning world-space entry manager so the menu exits cleanly.
- A headset regression test on 2026-07-13 confirms username and password selection, AFCore keyboard entry, login submission, identity storage, and login-gate closure all work. Field selection now compares the controller pointer against each field's exact geometry, checking password first, so stale username hover cannot steal password clicks.
- The superseded access-code selector, presence-slot selector, spawn-marker relocation, and fallback pawn-spawn logic are removed from the active widget path.
- `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal` owns replicated First Signal fields:
  - `Username`
  - `DisplayName`
  - `PresenceMode`
  - `Capabilities`
- Successful entry sends the username through inherited AFCore `Comp_PlayerInfo_Basic.SetPlayerName`, which already performs the server name update and drives the world-space NameTag.
- `BP_WMCYN_PlayerState_FirstSignal` binds to AFCore `Updated_PlayerName`; on authority it mirrors that value into `Username` and `DisplayName`, sets the current Quest-first `PresenceMode`, and stores `CanTriggerVerbatimMarker` as the temporary capability.
- No AFCore asset was edited and no duplicate WMCYN nameplate was needed.
- `BP_WMCYN_QuestUserPawn` applies one delayed WMCYN-owned visibility refresh after possession/replication settles. It still renders AFCore `Widget_NameTag` from AFCore `Comp_PlayerInfo_Basic`; the refresh only enforces local-hidden and remote-visible state on each machine.
- Three-client PIE confirms each local pawn hides its own label while both remote AFCore NameTags remain visible. All three PlayerStates carry replicated identity fields; the automated run used AFCore's default `AP-1743` name because no manual login was entered in the extra PIE windows.

## Voice State

- Project voice is enabled with `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.
- `[OnlineSubsystem]` uses `MaxLocalTalkers=1`, `MaxRemoteTalkers=8`, `VoiceNotificationDelta=0.200000`, and `bDuckingOptOut=true`.
- `MaxLocalTalkers` must remain `1` on Windows UE 5.8. Windows compiles `MAX_SPLITSCREEN_TALKERS` as one; the previous value of four caused `FVoiceEngineImpl::Init` to write beyond its fixed local voice array and crash VR Preview during `OnlineSubsystemNull` startup.
- `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` owns `WMCYN_VOIPTalker`, attached to the inherited AFCore head path and using AFCore voice attenuation.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` is active and WMCYN-owned. After the same two-second replication settle used by AFCore NameTag, each pawn's talker registers with that pawn's own PlayerState. Hidden technical session creation, mic setup, and `ToggleSpeaking 1` run only for the locally controlled pawn.
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
- Automatic `online voice dump` diagnostics were removed from normal startup, and `LogVoiceEngine=Error` suppresses per-frame packets plus recurring single-user PIE drop warnings while preserving errors and failures.
- Local headset microphone capture is proven, so the standalone audio-capture lane is closed. Quest-to-Quest hearing, PCVR monitoring, OBS audio capture, and packaged Quest microphone permission remain part of the integrated three-user test.
- Three-client PIE now produces exactly one `WMCYN Voice: local capture active` path per world and no new invalid-PlayerState/session warnings. This validates ownership and registration flow, not acoustic user-to-user hearing on separate hardware.
- First Signal keeps a hybrid audio plan: in-game voice for world presence/reference, with external/real microphones as backup and the final clean recording source.

## Input and Avatar Notes

- AFCore legacy VR controller mappings currently preserve working locomotion/controllers.
- The partial WMCYN Enhanced Input push-to-talk context is parked because registering it globally displaced the working AFCore input path after restart.
- The visible AFCore avatar appeared roughly 1/4 to 1/6 expected size in one test. Actor/component scale and `worldToMeters` inspected as normal, so this remains a visual-avatar issue rather than a global world-scale change.

## MCP

- Unreal MCP is reachable at `http://127.0.0.1:8000/mcp` when the editor server is running.
- Editor toolsets expose actor, scene, asset, Blueprint, object, material, and related inspection/editing capabilities.
- Inspect before mutating, and do not modify maps/assets unless the active task authorizes it.

## Storage

- The earlier zero-space / Zen DDC `507` blocker is cleared. `C:` currently has roughly 415 GB free.
- Recheck free space before large Quest/Windows cooks, but storage is not the active implementation blocker.

## Next Gate

1. Run the physical three-device checklist in `Docs/FIRST_SIGNAL_MULTIUSER_VALIDATION.md` with distinct login names.
2. Prove Quest user A and Quest user B see moving head/controller presence and hear each other.
3. Prove the PCVR recording user sees both names/presences, monitors both voices, and yields a clean OBS frame and audio reference.
4. Select the post-local networking path needed for packaged devices to enter the same persistent world runtime.
5. Add one structured Verbatim marker with Unreal-log and debug confirmation.
