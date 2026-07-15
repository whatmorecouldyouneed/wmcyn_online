# Current State

## Project

- WMCYN Online is an Unreal Engine 5.8 project.
- Current target: First Signal Build inside The WMCYN Crib.
- Canonical outcome: three standalone Quest users and one PCVR recording user in the same always-on world, with basic presence, display names, voice, and OBS-friendly capture.
- Active map: `/Game/Levels/L_WMCYNOnline` using the WMCYN Crib environment.
- Product flow: username or email + password -> persistent WMCYN world -> verified user identity appears in-world. There is no user-facing host/guest flow, session picker, or access code.

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
- `/Game/WMCYN/Core/BP_WMCYN_GameMode_FirstSignal` owns the runtime presence counter. The active four-slot assignment is `StandaloneVR_A = 0`, `StandaloneVR_B = 1`, `StandaloneVR_C = 2`, and `PCVR_Recording = 3`.
- `/Game/WMCYN/Data/DA_WMCYN_FirstSignal_Level` retains the previous AFCore pawn selection as rollback data. Production spawning now comes from `BP_WMCYN_GameMode_FirstSignal.DefaultPawnClass`.
- Spawn markers remain:
  - `SPAWN_FirstSignal_StandaloneVR_A`
  - `SPAWN_FirstSignal_StandaloneVR_B`
  - `SPAWN_FirstSignal_StandaloneVR_C`
  - `SPAWN_FirstSignal_PCVR_Recording`
- `PLAYERSTART_FirstSignal_Default` provides deterministic local VR Preview entry at StandaloneVR_A. The approved indexed runtime marker `SPAWN_FirstSignal_StandaloneVR_A` is yaw `0`; the delayed stabilizer reapplies that complete marker transform after HMD reset. The default PlayerStart still carries yaw `180` and should be reconciled with the approved marker before packaging.
- `/Game/WMCYN/Core/BP_WMCYN_VRPreviewStabilizer` sets LocalFloor tracking and resets local HMD orientation/position. After native pawn initialization, its authority-only correction reapplies both the indexed marker location and rotation so HMD/body startup cannot discard the intended entry facing. Network clients do not overwrite the server-assigned slot before PlayerState replication arrives.
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
- `Plugins/WMCYNRuntime` supplies the smallest WMCYN-owned adapter around the native pawn. `WMCYN_FirstSignalPresence` handles reliable login identity submission, dynamic VOIPTalker registration, AFCore's pawn `Widget_NameTag` for remote views and the local mirror, and 20 Hz replicated head/left-hand/right-hand transforms with interpolation.
- The same adapter normalizes the native pawn's two inherited `WidgetInteractionComponent` rays to the `Visibility` UI trace channel at 750 cm while preserving the package's existing left/right trigger `PressPointerKey` and `ReleasePointerKey` graphs. Production PIE confirms both rays are configured and hit testing is enabled; no imported Mimic or AFCore asset is edited.
- The earlier three-client listen-server PIE remains valid evidence for indexed spawn, ownership, remote-only nameplates, voice registration, and tracked-pose transport. The implementation now adds Quest slot `2` and moves PCVR Recording to slot `3`; a four-client PIE regression and four-device internet proof remain pending.
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

- `/Game/WMCYN/UI/WBP_WMCYN_LoginJoin` is the only active login surface and contains identifier, password, Enter World, and status UI. The identifier accepts username or email.
- `/Game/WMCYN/UI/WBP_WMCYN_BootLogin` had zero referencers and was deleted on 2026-07-14. Do not restore a parallel boot-login path.
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
- After promotion to the native production pawn, its inherited rays were found to trace `Pawn` instead of the UI `Visibility` channel. The WMCYN runtime adapter corrects that mismatch and attaches the local pointer at zero offset to the pawn's tracked `RightAim` motion-controller component. The cyan ray now follows the standard controller-forward aim pose instead of finger or hand animation. It remains visible for the entire login gate and hides immediately after successful login.
- The native pawn can open the AFCore keyboard, but it does not participate in AFCore's controller registration path. A single interaction component also lost text focus whenever it clicked a keyboard key. The adapter now creates a second invisible `WMCYN_KeyboardInputInteraction` on virtual user 1 with hit testing disabled. `WBP_WMCYN_LoginJoin` registers that focus-only interaction with the AFCore widget host before keyboard spawn or field refocus, while the visible fingertip pointer on virtual user 0 only clicks buttons. The production native-pawn continuous-type/Enter/Close headset regression remains the next hardware confirmation.
- The WMCYN presence adapter now reinforces the login gate on the production native pawn: locomotion and stick turning are ignored and current movement is stopped until identity submission succeeds. HMD and controller tracking remain active for menu use. Successful login resets both movement and look-input locks before the 3D gate closes.
- A 2026-07-14 headset pass confirms login completes and post-login locomotion is smooth. That pass exposed the awkward finger-driven pointer direction and the locally hidden mirror label; both runtime paths are corrected above and await the next headset confirmation.
- The superseded access-code selector, presence-slot selector, spawn-marker relocation, and fallback pawn-spawn logic are removed from the active widget path.
- `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal` owns replicated First Signal fields:
  - `Username`
  - `DisplayName`
  - `PresenceMode`
  - `Capabilities`
- `WBP_WMCYN_LoginJoin.SubmitLogin` now calls WMCYN-owned `Submit First Signal Login`. The native bridge runs `UWMCYNBackendSubsystem` asynchronously, reports progress through the existing status text, submits verified identity into the replicated presence layer, and closes the existing 3D gate only after success. Password text and backend tokens are never stored in replicated state.
- PIE uses an explicit local identity fallback so VR/editor iteration does not depend on Firebase billing. `-WMCYNForceBackendAuth` exercises the real service in PIE, while packaged clients use backend authentication unless explicitly launched with the development-only `-WMCYNLocalIdentity` switch.
- `PresenceMode`, `Capabilities`, and indexed slot assignment remain server-owned. Login cannot grant itself recording or marker capabilities.
- The native pawn's WMCYN adapter hosts AFCore `/Game/AFCore/Blueprints/Widgets/Pawn/Widget_NameTag` in runtime component `WMCYN_AFCoreNameTag_Runtime`. This is AFCore's intended player identity widget: the VR, mobile, and desktop character pawns reference it directly, and it binds to `Comp_PlayerInfo_Basic.Updated_PlayerName`. The host is AFCore `/Game/AFCore/Blueprints/Components/UI/Comp_Widget`, not a plain Unreal `WidgetComponent`; this preserves AFCore theme initialization through `FL_UI` and `/Game/AFCore/Materials/UI/UI/M_UIMaster`. It uses `DA_Theme_Default`, matching AFCore's pawn template and preventing the white-on-white fallback seen with the plain host. Successful login mirrors the server-authoritative display name into the AFCore component as well as WMCYN/Unreal PlayerState fields. The desired-size widget is constrained to a `25 cm` world width, follows the replicated HMD with a fixed `30 cm` world-up offset, and billboards around world yaw only. `Owner No See` keeps it out of the user's direct HMD view while allowing the local mirror and remote users to render it. No AFCore asset is modified.
- A two-client identity probe proves both the listen-server path and client-to-server RPC path. Both PlayerStates replicated `NetworkProbe`, and each opposite client displayed that value on the remote nameplate. The probe actor was removed from the active test map after validation.
- No AFCore asset was edited. The AFCore runtime keyboard remains the input mechanism; WMCYN owns identity storage and display for the native pawn.

## Backend and World Entry

- The existing backend is the Firebase project `wmcyn-online-mobile`, documented locally at `C:/Users/jvred/Documents/WMCYN/wmcyn-backend-infra` and hosted through Firebase Cloud Functions/Cloud Run when billing is active.
- The local backend checkout was repaired from upstream `main` at `b2f260b` while preserving its existing local CORS work. Active backend work is on `codex/first-signal-backend-bootstrap`.
- The deployed service is intentionally unavailable while Firebase billing is inactive. Current HTTP `500/503` responses are an infrastructure state, not evidence that the upstream source fails to build.
- `POST /v1/auth/login` accepts one `identifier` field plus password. It supports email, username, or `@username`; usernames resolve through server-only `loginHandles/{handleNormalized}` before Firebase email/password exchange.
- `GET /v1/vr/bootstrap` returns verified identity and the configured Crib runtime endpoint behind Firebase Bearer authentication. The world remains offline until a valid host/port record exists.
- Initial users are provisioned manually through Firebase Authentication plus `users/{uid}` and `loginHandles/{handleNormalized}` as documented in `Docs/FIRST_SIGNAL_USER_PROVISIONING.md`.
- Existing headset pairing-code endpoints are not part of the WMCYN Online First Signal product flow. First Signal remains login -> automatic world entry, with no visible access code or session picker.
- The backend does not currently register or discover an Unreal world runtime. It has AR world/scene and livestream records, but no authoritative Unreal server host, port, build ID, heartbeat, or join-ticket contract.
- Packaging Quest and Windows clients alone does not create an online world. The product target is one internet-reachable authoritative Unreal runtime, registered and heartbeating through Firebase, with authenticated automatic bootstrap. LAN/listen-server proof is no longer an acceptance gate.
- The backend hardening pass removed public `/debug/env`, limits development `x-uid` auth to the Auth emulator plus an explicit flag, restored rate limiting, restricted user/profile reads, removed anonymous admin mutation access, and replaced direct live-route `x-uid` trust.
- `UWMCYNBackendSubsystem` implements asynchronous login and bootstrap in WMCYN-owned C++ with private token storage and a command-line backend URL override. MSVC `14.50.35717` is installed, UnrealBuildTool selects toolchain `14.50.35737`, and `wmcyn_onlineEditor` now compiles and links `UnrealEditor-WMCYNRuntime.dll` successfully under UE 5.8.
- AFCore reuse wrappers exist at `/Game/WMCYN/UI/Multiplayer/WBP_WMCYN_WhosHere` and `/Game/WMCYN/UI/Settings/WBP_WMCYN_Settings_Audio`. `/Game/WMCYN/UI/Settings/WBP_WMCYN_Settings_FirstSignal` composes the AFCore audio page with a WMCYN-owned `RESPAWN TO ENTRY` action, and `/Game/WMCYN/UI/Menu/WBP_WMCYN_RuntimeMenu` exposes the roster and Settings through an AFCore scaffold, vertical tabs, and widget switcher hosted by the native pawn adapter.
- The runtime menu binds non-destructively to AFCore's existing `FaceButton02Right` action for Quest right `B`; desktop `M` toggles the menu and `Escape` closes it. Headset testing confirms right `B` now opens and closes the menu and the pointer can interact with it.
- Respawn is server-authoritative and reuses the controller's proven indexed `SyncPresencePawn` path. It stops movement, returns the current possessed pawn to its assigned presence slot, preserves login/identity/voice, closes the menu, and does not reload the level or replace the pawn. A 2026-07-15 headset pass confirmed respawn returns the user to the assigned entry slot while preserving login, voice, body tracking, and movement.
- Settings audio is simplified to Master, Music, Voice, and SFX. Audio Quality is hidden. The WMCYN wrapper applies AFCore's sound-mix class overrides live, WMCYN-owned copies of the Mimic footstep cues route through AFCore `SC_SFX`, and project VOIP routes through AFCore `SC_Voice`. Master controls both paths while SFX and Voice remain independent; no AFCore or Mimic source asset is edited.
- An engine-driven PIE smoke submitted `CodexSmoke` through the production `SubmitLogin` function, unlocked the gate, created `WMCYN_AFCoreRuntimeMenu`, and opened it with `M`. The captured runtime menu showed `CodexSmoke` in the AFCore Players roster and exposed the Audio tab. The temporary auto-submit event was removed before the widget was compiled and saved.
- Verbatim is a stretch feature and is not part of the First Signal acceptance gate.

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
- Local headset microphone capture is proven, so the standalone audio-capture lane is closed. Quest-to-Quest hearing across three Quest users, PCVR monitoring, OBS audio capture, and packaged Quest microphone permission remain part of the integrated four-user test.
- The earlier three-client PIE produced one native local capture path per world. `OnlineSubsystemNull` logs from fixed split-screen probes are local PIE noise, not proof of separate-device hearing.
- First Signal keeps a hybrid audio plan: in-game voice for world presence/reference, with external/real microphones as backup and the final clean recording source.

## Input and Avatar Notes

- AFCore legacy VR controller mappings currently preserve working locomotion/controllers.
- The partial WMCYN Enhanced Input push-to-talk context is parked because registering it globally displaced the working AFCore input path after restart.
- The earlier 1/4-to-1/6-size impression came from AFCore's roughly 90 cm head-and-torso bust, not from world scale. Actor scale, capsule, camera, AFCore components, and `worldToMeters` remain normal.
- The superseded AFCore-derived Mimic wrapper used a separate `WMCYN_VRBodyMesh`, AFCore hand actors, and AFCore ground anchoring. Those transforms and visibility rules do not define the production native pawn.
- The production native pawn keeps Mimic's own Manny body, attached head, controller hierarchy, Stage tracking, `Height Offset`, and explicit left-thumbstick calibration path.
- Headset testing is authoritative: after calibration, the native pawn has correct body/camera alignment, locomotion, footsteps, and stable Crib-floor collision. Desktop multi-client PIE now proves the WMCYN adapter and cross-user pose transport; remaining checks are packaged-device interaction, Quest performance, separate-device voice, and OBS capture.
- A downward collision trace at `SPAWN_FirstSignal_StandaloneVR_A` and eight surrounding samples all hit the Crib floor near `Z 461.8`. The capsule starts near `Z 557`, leaving its bottom about 5 cm above the floor. Any remaining fall-through needs a specific location outside this validated spawn patch.
- The four indexed AFCore `BP_PlayerPosition` actors are logical WMCYN post-possession anchors, so Unreal's built-in `PlayerStart` `BAD SIZE` sprite is a false-positive editor badge for this path. Their transforms and collision capsules remain unchanged; only the instance editor sprites in `L_WMCYNOnline` show the normal player marker.

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
- The UE 5.8 installation is missing its Android target binaries and AutomationTool/UnrealBuildTool platform assemblies, so Turnkey currently discovers only Win64. The machine also has no discoverable Android SDK/ADB.
- In Epic Games Launcher, open UE 5.8 **Options**, enable the **Android target platform**, and apply the installation change. Then install/configure the UE 5.8 toolchain (`platforms;android-34`, `build-tools;35.0.1`, `cmake;3.22.1`, and NDK `27.2.12479018` / r27c) through `SetupAndroid.bat` before retrying the Quest package smoke.
- No project config was changed in response. Existing Android settings already target arm64, SDK 35, ES3.1/Vulkan, and Android voice.

## Windows PCVR Build Readiness

- A production-map-only Win64 Development `BuildCookRun` now succeeds and archives to `Builds/PCVR/FirstSignal-Development/Windows`.
- The archive contains the runnable `wmcyn_online.exe`, a 557 MB pak, and roughly 1.61 GB total staged content.
- Packaging now cooks only `/Game/Levels/L_WMCYNOnline`; the inherited AFCore demo/template map list is no longer part of the WMCYN shipping manifest.
- `/Game/AFCore/AnimBP/Hand` is excluded from WMCYN packages because its legacy Epic-hand AnimBPs contain removed SteamVR Input structs. The production native Mimic pawn does not depend on those assets, and no AFCore asset was modified.
- A packaged `-nohmd` startup smoke stayed responsive, loaded `L_WMCYNOnline`, created `BP_WMCYN_GameMode_FirstSignal`, assigned/synced slot `0`, locked movement behind login, and initialized the native voice path.
- Packaged startup reports a handled `Comp_Widget` custom-property-list ensure, then continues into the world. Treat this as an AFCore UE 5.8 compatibility follow-up; it is not the current startup-crash path and was not "fixed" by editing AFCore.

## Next Gate

1. Implement and deploy the singleton runtime registration, heartbeat lease, join-ticket, and reconnect contract in `Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md`.
2. Enable the UE 5.8 Android optional component, configure its Android SDK, and rerun the isolated Quest package smoke.
3. Restore Firebase billing and run one real `-WMCYNForceBackendAuth` login/bootstrap proof against the hardened backend.
4. Prove Quest A, Quest B, Quest C, and PCVR Recording enter the same runtime from separate internet locations with names, tracking, voice, and OBS capture.
5. Measure Quest frame timing with three native Mimic users, then specify the handheld camera feature.
