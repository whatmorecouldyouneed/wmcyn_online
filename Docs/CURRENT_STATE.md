# Current State

## Project

- WMCYN Online is an Unreal Engine 5.8 project.
- Current target: First Signal Build inside The WMCYN Crib.
- Canonical outcome: two standalone VR users and one PCVR recording user in the same persistent world, with basic presence, display names, voice, OBS-friendly capture, and one Verbatim marker.
- Active map: `/Game/Levels/L_WMCYNOnline` using the WMCYN Crib environment.
- Product flow: username/password login -> persistent WMCYN world -> user identity appears in-world. There is no user-facing host/guest flow, session picker, or access code.

## AFCore Baseline

- HumanCodeable AFCore / Advanced VR Framework is present under `/Game/AFCore` and has already been tested.
- AFCore remains the baseline for compatible reusable framework systems and reference behavior, but it is no longer the player-body/locomotion source of truth for the First Signal integration lane.
- `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal`, a WMCYN child of Mimic Pro's native `BP_VRBodyCharacer`, is the player source of truth for body, tracking hierarchy, seated calibration, native locomotion, footsteps, and floor behavior.
- Do not edit `/Game/AFCore/*` by default. Use WMCYN children, wrappers, or duplicates first.
- AFCore documentation is indexed in `Docs/AFCORE_DOCUMENTATION_INDEX.md`.

## Active Runtime Path

- `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal` is the production player source of truth. It is a WMCYN child of Mimic Pro's native `/Game/FullBodyVRTemplate/BluePrints/BP_VRBodyCharacer`.
- Preserve the native pawn's body hierarchy, Stage tracking, seated height calibration, locomotion, footsteps, controller/body solve, and floor behavior. Add WMCYN product systems around it through WMCYN-owned adapters.
- `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` and `/Game/WMCYN/Pawns/Experimental/BP_WMCYN_QuestUserPawn_Mimic` are rollback/diagnostic assets only. Do not use either as the basis for further First Signal body tuning.
- `L_WMCYNOnline` now spawns `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal` through `BP_WMCYN_GameMode_FirstSignal`. The prior AFCore-derived pawns remain rollback/diagnostic assets only.
- `/Game/WMCYN/Core/BP_WMCYN_GameMode_FirstSignal` is active in `L_WMCYNOnline`.
- `/Game/WMCYN/Core/BP_WMCYN_PlayerController_FirstSignal` assigns presence slots on authority and moves any possessed pawn to its indexed marker through generic Pawn/Character APIs. It no longer requires an AFCore `BP_Pawn_Base` cast or `playingPawn` assignment.
- `/Game/WMCYN/Core/BP_WMCYN_GameMode_FirstSignal` owns the runtime presence counter. Three-client production PIE assigns `StandaloneVR_A = 0`, `StandaloneVR_B = 1`, and `PCVR_Recording = 2` without overlap.
- `/Game/WMCYN/Data/DA_WMCYN_FirstSignal_Level` retains the previous AFCore pawn selection as rollback data. Production spawning now comes from `BP_WMCYN_GameMode_FirstSignal.DefaultPawnClass`.
- Spawn markers remain:
  - `SPAWN_FirstSignal_StandaloneVR_A`
  - `SPAWN_FirstSignal_StandaloneVR_B`
  - `SPAWN_FirstSignal_PCVR_Recording`
- `PLAYERSTART_FirstSignal_Default` provides deterministic local VR Preview entry at StandaloneVR_A.
- `/Game/WMCYN/Core/BP_WMCYN_VRPreviewStabilizer` sets LocalFloor tracking and resets local HMD orientation/position. Its spawn correction is authority-only so network clients do not overwrite the server-assigned slot before PlayerState replication arrives.
- Earlier AFCore-derived VR Preview confirmed the Crib floor path and framework controls. The authoritative player-body hardware result is now the native Mimic test below.
- Imported Crib collision currently uses repaired complex-as-simple collision on the relevant floor components. `Plane_003` / `/Game/Environments/WMCYN_Crib/Scene_058` is the verified active walkable floor near spawn.
- Jared MetaHuman assets are outside the active First Signal path. The proven native Mimic pawn is the accepted First Signal avatar and movement baseline for standalone VR and PCVR users.
- `/Game/WMCYN/Dev/AvatarMirror/BP_WMCYN_AvatarMirror` is a WMCYN-owned duplicate of AFCore's render-target mirror pattern. `DEV_AvatarMirror_Tracking` is placed behind StandaloneVR_A in `WMCYN_99_Dev_AvatarMirror` for headset tracking and proportion checks.
- `/Game/WMCYN/Pawns/Controllers/BP_WMCYN_Hand_Left` and `BP_WMCYN_Hand_Right` are WMCYN-owned children of AFCore's left/right hand controller classes. The active Quest pawn selects them after AFCore controller initialization, preserving AFCore movement, interaction, laser, teleport, finger, and replication components.
- The migrated Mimic Pro package is present under `/Game/FullBodyVRTemplate`. Its UE 5.8 core assets `ABP_VRBody`, `CR_VRBody`, and `BP_VRBodyComponent` compile successfully.

### Superseded AFCore-Derived Body Lane

- The following AFCore-derived Mimic integration records are retained only as rollback history. They are not the production player path and must not be used to override the proven native Mimic hierarchy.
- `BP_WMCYN_QuestUserPawn_Mimic` reuses the inherited `WMCYN_VRBodyMesh`, assigns Mimic's `SKM_Manny_Simple` plus `ABP_VRBody`, and attaches `/Game/WMCYN/Pawns/Experimental/BP_WMCYN_MimicBodyComponent` before BeginPlay.
- `BP_WMCYN_MimicBodyComponent` is a WMCYN-owned child of the migrated Mimic component. It guards HMD velocity against zero frame delta and leaves the imported package unchanged.
- Mimic receives AFCore `MainCamera`, `MotionControllerLeft`, and `MotionControllerRight` references plus AFCore `CameraHandle` as the expected `VR Origin`. Mimic does not own AFCore locomotion, input, camera-height correction, or capsule behavior.
- The active child anchors `WMCYN_VRBodyMesh` to AFCore's reported pawn-ground location every tick plus 12 cm sole clearance. This avoids Mimic's sample pawn `Dynamic Adjustments`, which also modify capsule, actor, VR-origin, speed, and input state.
- The first seated pass only detected a low HMD and left the tracked camera/controllers at seated height. Hardware testing failed: the camera remained in the hips/torso and Mimic folded the knees upward. That detection-only path has been removed.
- A second seated candidate rewrote AFCore's shared `CameraHandle` every tick using capsule half-height and a 170 cm target. Hardware testing also failed because the camera was far from the correct view. That override has been removed; AFCore owns `CameraHandle` again without WMCYN height writes.
- Direct comparison with `/Game/FullBodyVRTemplate/BluePrints/BP_VRBodyCharacer` found that Mimic expects a dedicated `Height Offset` below its `VR Origin`, calibrates against the rig's `FullBody_Component` marker at about 168.06 cm, and triggers calibration explicitly from left-thumbstick press. The sample does not derive its offset from capsule half-height and does not overwrite another framework's tracking handle every tick.
- The Mimic component now receives inherited AFCore `CameraHandle` as its `VR Origin`, matching the sample's tracking-origin semantics. The prior integration incorrectly supplied the body mesh.
- Manny's attached skeletal head is now visible, matching the sample. The separate AFCore static head/torso bust is hidden by the WMCYN child, and the obsolete WMCYN AFCore-head offset function has been removed.

### Native Mimic Source Of Truth

- An isolated native-package A/B lane now exists at `/Game/WMCYN/Dev/MimicNativeTest/L_WMCYN_MimicNativeTest`. Its GameMode spawns `BP_WMCYN_MimicNativeTestPawn`, a child of the imported `BP_VRBodyCharacer` with only `PlayMode = Seated` plus Crib-compatible capsule settings.
- The duplicate test map removes `BP_MapInfo`, `BP_WMCYN_FirstSignalEntryManager`, and `BP_WMCYN_VRPreviewStabilizer` so AFCore pawn selection, login gating, and LocalFloor forcing cannot contaminate Mimic's native `VR Origin`, `Height Offset`, Stage tracking, input, or calibration behavior. `L_WMCYNOnline` is unchanged by this test harness.
- The imported sample's one-centimeter custom physics capsule fell through the Crib immediately. The WMCYN test child copies the working AFCore capsule compatibility values: 26 cm radius, `Pawn` profile, `QueryAndPhysics`, and physics simulation disabled. Desktop PIE confirms the test pawn remains stable on the Crib floor.
- Headset validation passes the native lane: after left-thumbstick calibration, the pawn has correct camera/body alignment, full-body presentation, working locomotion and footstep audio, and stable Crib-floor collision.
- The proven test child has been promoted to `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal`. It keeps `PlayMode = Seated`, the proven capsule compatibility values, native Mimic behavior, `bReplicates = true`, movement replication, and 100 Hz net update frequency. It is now the production default in `L_WMCYNOnline`.
- `Plugins/WMCYNRuntime` supplies the smallest WMCYN-owned adapter around the native pawn. `WMCYN_FirstSignalPresence` handles reliable login identity submission, dynamic VOIPTalker registration, a remote-only runtime nameplate, and 20 Hz replicated head/left-hand/right-hand transforms with interpolation.
- The same adapter normalizes the native pawn's two inherited `WidgetInteractionComponent` rays to the `Visibility` UI trace channel at 750 cm while preserving the package's existing left/right trigger `PressPointerKey` and `ReleasePointerKey` graphs. Production PIE confirms both rays are configured and hit testing is enabled; no imported Mimic or AFCore asset is edited.
- Three-client listen-server PIE on production `L_WMCYNOnline` proves distinct indexed spawns, ownership, remote-only nameplate visibility, voice registration, and live tracked-pose transport. Every client hides its local label and shows two remote labels; remote pawns received and applied hundreds of pose updates. Slot `2` carries `PCVR`, `Recording`, and `CanTriggerVerbatimMarker`.
- The production `L_WMCYNOnline` smoke run confirms the native pawn spawns exactly at `StandaloneVR_A`, the 3D login gate starts with locomotion locked, voice registers, pose capture advances, and no new Blueprint runtime error occurs.
- Mimic already supplies procedural walking: `ABP_VRBody` calls `CalculateFeetTargets` every animation update, and the component drives alternating foot targets from smoothed HMD/world velocity. This is the active walking layer; no separate AFCore walk-cycle state machine was added.
- The solver stays idle when no HMD is active. This prevents ordinary in-viewport PIE from feeding zero tracking/height data into Mimic while preserving the full solver for VR Preview and PCVR.

### Retained Rollback Evidence

- The following arm-only and AFCore-derived body notes describe retained rollback assets, not the production pawn.
- `/Game/WMCYN/Pawns/Body/CR_WMCYN_VRBody` and the previous WMCYN arm-only solve remain on the stable base pawn as a rollback path. They are not the active solver while the Mimic child is selected.
- The full-body PBIK solve is bypassed for First Signal because its head-to-pinned-pelvis pull distorted the torso. AFCore's input pose now owns the torso and legs; only the two WMCYN arm solvers run procedurally.
- Each arm uses a WMCYN two-bone solve with a body-relative elbow pole calculated from the shoulder/hand midpoint.
- The right arm is the calibration source of truth. The left solver now mirrors its skeleton-axis basis (`PrimaryAxis +X`, `SecondaryAxis -Y`) while the right remains unchanged (`PrimaryAxis -X`, `SecondaryAxis +Y`).
- `BP_WMCYN_QuestUserPawn` raises only the inherited visible AFCore head mesh to relative location `(-4, 0, 10)` during body-rig initialization. The HMD camera, camera proxy, capsule, torso, and tracking transforms are unchanged.
- A disposable second-pawn PIE proof confirmed the complete WMCYN pawn renders a full adult-scale mannequin for another user's view. The test actor was removed after verification.
- The Mimic child disables `Owner No See` on `WMCYN_VRBodyMesh` at runtime. Manny's attached head is visible; only its hand bones remain hidden to prevent duplicate AFCore/WMCYN hand geometry.

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
- After promotion to the native production pawn, its inherited rays were found to trace `Pawn` instead of the UI `Visibility` channel. The WMCYN runtime adapter now corrects that mismatch. The production native-pawn VR Preview click/keyboard/login regression remains the next hardware confirmation.
- The superseded access-code selector, presence-slot selector, spawn-marker relocation, and fallback pawn-spawn logic are removed from the active widget path.
- `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal` owns replicated First Signal fields:
  - `Username`
  - `DisplayName`
  - `PresenceMode`
  - `Capabilities`
- Successful entry calls `WMCYN|Identity|SubmitLocalFirstSignalIdentity`. The pawn-owned runtime component sends a reliable server RPC, sanitizes the username/display name, writes `Username` and `DisplayName` on `BP_WMCYN_PlayerState_FirstSignal`, updates Unreal's standard player name, and forces replication. Password text is not stored.
- `PresenceMode`, `Capabilities`, and indexed slot assignment remain server-owned. Login cannot grant itself recording or marker capabilities.
- The native pawn uses `WMCYN_Nameplate_Runtime`, a lightweight WMCYN-owned text component attached above the head. It is hidden from the owning user, visible to remote users, and reads the replicated `DisplayName` with standard PlayerName as fallback.
- A two-client identity probe proves both the listen-server path and client-to-server RPC path. Both PlayerStates replicated `NetworkProbe`, and each opposite client displayed that value on the remote nameplate. The probe actor was removed from the active test map after validation.
- No AFCore asset was edited. The AFCore runtime keyboard remains the input mechanism; WMCYN owns identity storage and display for the native pawn.

## Voice State

- Project voice is enabled with `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.
- `[OnlineSubsystem]` uses `MaxLocalTalkers=1`, `MaxRemoteTalkers=8`, `VoiceNotificationDelta=0.200000`, and `bDuckingOptOut=true`.
- `MaxLocalTalkers` must remain `1` on Windows UE 5.8. Windows compiles `MAX_SPLITSCREEN_TALKERS` as one; the previous value of four caused `FVoiceEngineImpl::Init` to write beyond its fixed local voice array and crash VR Preview during `OnlineSubsystemNull` startup.
- The production native pawn dynamically owns `WMCYN_VOIPTalker_Runtime`, attached to its head/camera path by `WMCYN_FirstSignalPresence`. Each pawn registers against its own PlayerState; hidden technical session creation, mic setup, and `ToggleSpeaking 1` run only for the locally controlled pawn.
- `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` remains only for the superseded AFCore-derived pawn lane and is not the production native-pawn voice path.
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
- Three-client PIE produces one native local capture path per world. `OnlineSubsystemNull` still logs `Invalid user specified in RegisterLocalTalker(1..3)` while probing fixed split-screen indices; real local talker `0` registers and captures normally. This is local PIE noise, not proof of separate-device hearing.
- First Signal keeps a hybrid audio plan: in-game voice for world presence/reference, with external/real microphones as backup and the final clean recording source.

## Input and Avatar Notes

- AFCore legacy VR controller mappings currently preserve working locomotion/controllers.
- The partial WMCYN Enhanced Input push-to-talk context is parked because registering it globally displaced the working AFCore input path after restart.
- The earlier 1/4-to-1/6-size impression came from AFCore's roughly 90 cm head-and-torso bust, not from world scale. Actor scale, capsule, camera, AFCore components, and `worldToMeters` remain normal.
- The superseded AFCore-derived Mimic wrapper used a separate `WMCYN_VRBodyMesh`, AFCore hand actors, and AFCore ground anchoring. Those transforms and visibility rules do not define the production native pawn.
- The production native pawn keeps Mimic's own Manny body, attached head, controller hierarchy, Stage tracking, `Height Offset`, and explicit left-thumbstick calibration path.
- Headset testing is authoritative: after calibration, the native pawn has correct body/camera alignment, locomotion, footsteps, and stable Crib-floor collision. Desktop multi-client PIE now proves the WMCYN adapter and cross-user pose transport; remaining checks are packaged-device interaction, Quest performance, separate-device voice, and OBS capture.
- A downward collision trace at `SPAWN_FirstSignal_StandaloneVR_A` and eight surrounding samples all hit the Crib floor near `Z 461.8`. The capsule starts near `Z 557`, leaving its bottom about 5 cm above the floor. Any remaining fall-through needs a specific location outside this validated spawn patch.
- The three indexed AFCore `BP_PlayerPosition` actors are logical WMCYN post-possession anchors, so Unreal's built-in `PlayerStart` `BAD SIZE` sprite is a false-positive editor badge for this path. Their transforms and collision capsules remain unchanged; only the instance editor sprites in `L_WMCYNOnline` now show the normal player marker. Runtime spawn/possession behavior is untouched.

## MCP

- Unreal MCP is reachable at `http://127.0.0.1:8000/mcp` when the editor server is running.
- Editor toolsets expose actor, scene, asset, Blueprint, object, material, and related inspection/editing capabilities.
- Inspect before mutating, and do not modify maps/assets unless the active task authorizes it.

## Storage

- The earlier zero-space / Zen DDC `507` blocker is cleared. `C:` currently has roughly 446 GB free.
- Recheck free space before large Quest/Windows cooks, but storage is not the active implementation blocker.

## Quest Build Readiness

- A real Android Development `BuildCookRun` was attempted on 2026-07-13 using the production project and an isolated archive directory under `Saved/StagedBuilds/FirstSignalQuestSmoke`.
- Unreal stopped before compilation or cooking with: `Missing files required to build Android targets. Enable Android as an optional download component in the Epic Games Launcher.`
- The machine currently has a JDK but no discoverable Android SDK/ADB. Install the UE 5.8 Android optional component first, then run UE 5.8 `SetupAndroid.bat` or otherwise configure the SDK versions required by the engine before retrying the package smoke.
- No project config was changed in response. Existing Android settings already target arm64, SDK 35, ES3.1/Vulkan, and Android voice.

## Next Gate

1. Run the existing 3D login and AFCore keyboard on the production native pawn in VR Preview; confirm Enter World unlocks locomotion and the typed name reaches the runtime nameplate path.
2. Enable the UE 5.8 Android optional component, configure its Android SDK, and rerun the isolated Quest package smoke.
3. Run the physical three-device checklist with distinct names and confirm native body/head/hand motion is visible across Quest A, Quest B, and PCVR.
4. Close separate-device voice hearing and PCVR/OBS audio/video capture.
5. Measure Quest frame timing with two native Mimic users and tune only WMCYN-owned adapter rates if needed.
6. Add the first structured Verbatim marker.
7. Select and validate the packaged persistent-world connection path beyond local `OnlineSubsystemNull` PIE.
