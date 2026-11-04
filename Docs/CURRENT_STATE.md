# Current State

## Active Priority - 2026-11-04

- Shared video: three channels (living room, stage plus DJ mirror, basement) and in-world panels
  replace the autoplay actor. PCVR resolves YouTube and Twitch per client with pinned yt-dlp; Quest
  plays direct links and reports those two as unavailable until its dependency is approved.
  Simulate and 17/17 synthetic tests pass; Win64, Android and Linux compile. No real YouTube or
  Twitch link has been resolved yet, and panels are untested in a headset. See
  `CRIB_DISPLAY_STREAM.md` and `SHARED_VIDEO_QUEST_GATE.md`.

- Release hold: the tray and video work is on `feature/protocol3-tray-and-video`, not `main`. On
  2026-11-04 the founder chose to push it while the repo stays public for now, which publishes the
  tray art. Protocol 3 still needs a matched server cutover.

- Shared video Quest-first gate started: isolated VideoProbe map/native diagnostic
  added; Win64 Editor build and Android ARM64 native compile/link pass. Packaged
  playback is NOT verified. GPL Android wrapper is blocked; the MIT alternative is
  audited but not adopted. See `SHARED_VIDEO_QUEST_GATE.md`.

- Tray session (Joint Roller port) in progress. Native code and content are in the working tree;
  11/11 synthetic WMCYN tests pass. Local `ProtocolVersion` is now 3, so local builds cannot join the
  live protocol 2 world until a matched release is approved. See `TRAY_SESSION.md`.

- Stage follow-up: full 89-piece FBX visual assembly restored at a proportional
  5 m width; old slab retained as hidden simple support. Editor-rendered review
  and nine expanded support checks pass. Rails/posts are visual-only; functional
  stage lights, Unity material parity and headset traversal remain pending.

- Founder confirms camera is grabbable and defers seating; stage/DJ layout is next.
- Original stage deck and Aleph Standard booth reused in the approved pool-side
  grass alcove. Four WMCYN-only pilot actors saved in L_WMCYNOnline.
- Editor screenshots reviewed; 26 simple deck/step support checks pass. Lawn
  approach, headset scale and remote acceptance remain open; no playback added.
- See `STAGE_DJ_LAYOUT_PILOT.md` for source provenance, scripts and next checks.

## Active Priority - 2026-11-02

- Founder confirms distribution is closed and Gio/Cole are green. This supersedes
  older distribution blockers below; exact installed artifact hashes were not
  re-audited in this pass. Do not reopen distribution as the primary work lane.
- Next: recordable interview camera, two-person seating, then short OBS rehearsal.
- Camera reliability pass rebuilt successfully with Launcher UE 5.8 (55116800).
  Expanded `WMCYN.UI.HandheldCamera` automation passes, zero errors/warnings.
  This is synthetic NullRHI evidence, not a rendered/headset/remote recording pass.
- See `INTERVIEW_CAMERA_REHEARSAL.md` for capture settings and remaining gates.

## First Interview Priority (2026-10-23)

`FIRST_INTERVIEW_ROADMAP.md` is the canonical delivery plan; `AGENT_HANDOFF.md`
records the source/build checkpoint. Older sections below are dated history,
not evidence that today's build passed device testing. Prioritize a recordable
episode in the approved couch set: default avatar fallback, three switched
shots, headset microphones/VOIP mix and prepared music. External microphones
and isolated speaker tracks are deferred per founder correction. Jared's local tracking passed
October 23; garments and grooms are implemented and asset-verified, but no headset/PIE visual confirmation,
  and packaged/remote acceptance is open.
Protocol-2 signed identity and three camera shot slots are source changes only.
Backend tests pass (58/58); native UHT/compile-only passes (20 actions).
Link/restart, native test execution and hardware acceptance are pending.

## Project

- Jared avatar follow-up (2026-10-20): remembered login succeeds, but the live
  account-owned job is still `preview_ready` without runtime mesh paths, so its
  PCVR manifest returns default. WMCYN preview skeleton links are valid.
  Ticket-bound server resolution is implemented in source; publication and
  matching runtime builds remain pending. See `Docs/JARED_ACCOUNT_AVATAR_2026_10_20.md`.

- Furnishing spec started (2026-10-17): `Specs/004-crib-furnishing/spec.md` defines the main living/dining/bar batch, reference-inspired redesign and static dressing only. Local Unity/package comparison, 87 prefab dependency inspections and ten FBX measurements are recorded in its source audit. Measured Unreal layout/approval remains pending; no furniture imported. Interview-room living-room replica is a separate measured set.

- Adaptive UI pilot (2026-10-16): priorities 1-4 approved and implementation underway. WMCYN login/settings styling, local mic/mirror controls, panel reset/pin, per-person volume and a local camera preview/spectator-output pilot exist. Editor native build passes; headset and remote-peer acceptance remain open. See `Docs/ADAPTIVE_UI_IMPLEMENTATION.md`. Persistent-sign-in acceptance remains separate.

- Development workflow priority (2026-10-16): remembered sign-in first, then explicit offline testing and developer diagnostics, then adaptive UI styling. Windows secure storage/refresh and Designer-visible Remember Me, Continue, Switch Account and Settings Sign Out are implemented. Credential-store automation and local UI smoke checks pass; real-account restore and Quest secure storage remain pending. See `Docs/PERSISTENT_SIGN_IN.md`. Do not treat this as a shipped or headset-validated feature.

- Avatar audit (2026-10-15): the current login test account has no verified account-matched PCVR runtime output. User chose native default, not rebinding the older test-account MetaHuman. Runtime source now hardens fallback, retries, pre-cooked path checks and client avatar RPC handling; published online avatars require a trusted server-side account resolver. No Firestore/backend deployment changes. See `Docs/AVATAR_LOGIN_AUDIT_2026_10_15.md` for evidence, build status and acceptance steps.

- WMCYN Online is an Unreal Engine 5.8 project.
- Current target: First Signal Build inside The WMCYN Crib.
- Canonical outcome: three standalone Quest users and one PCVR recording user in the same always-on world, with basic presence, display names, voice, and OBS-friendly capture.
- Active map: `/Game/Levels/L_WMCYNOnline` using the WMCYN Crib environment.
- Product flow: username or email + password -> persistent WMCYN world -> verified user identity appears in-world. There is no user-facing host/guest flow, session picker, or access code.

## Milestones

- **Rough First Signal: complete (2026-10-14).** Two distinct Quest users have each entered the WMCYN runtime on separate headset runs, exercised the login/world-entry path, and confirmed the working native VR presence and locomotion lane. This earns a smoke-level milestone checkmark.
- This is not yet the full First Signal acceptance gate. Simultaneous three-Quest plus PCVR presence, remote voice, packaged-device validation, OBS capture, and reconnect behavior remain open.
- The next production lane is now the Crib interview set: establish room/couch anchors, add intentional furniture and lighting, validate the iPhone VCam path, then add video players and other presentation features.

## Crib Lighting (2026-10-14)

- Bathroom corner follow-up (2026-10-17): extended Door 03 bathroom south into the unused recess, replacing the internal cross-wall with a continuous west wall and a south closure at Y44. Original window remains on the interview side. Eight capsule checks pass, including entry into the enlarged corner. Existing floor finish and final light bake remain unfinished.

- Bathroom layout revision (2026-10-17): Door 03 (left closed door in the approved view) now accesses a separate bathroom shell. Three WMCYN partitions close its connection to the interview space; Door 04 remains the interview entrance and the window alcove remains outside the bathroom. Six synthetic capsule checks pass. Temporary bathroom light only; fixtures, final lighting bake and headset traversal pending. See `Tools/Lighting/build_interview_bathroom_partition.py`.

- Interview-room window correction (2026-10-17): reopened the original window alcove by disabling the new south return and extending the enclosure around it. Original window retained and screenshot-verified; 120 floor samples and four passage sweeps pass. Existing rock intrusion, final lighting and headset validation remain pending.

- Interview-room blockout (2026-10-17): spaces behind Doors 03/04 joined using WMCYN architecture/collision copies, with rear/side enclosure and ceiling. 96/96 floor samples, three capsule passage sweeps, and preservation of 811 shell horizontal faces pass. Headset traversal, threshold polish, set dressing and lighting rebake remain pending. See `Docs/INTERVIEW_ROOM_DOOR03_SCOPE.md`.

- Walking collision repair (2026-10-17): three WMCYN floor duplicates outside spawn had no usable simple collision. UseComplexAsSimple plus double-sided geometry now passes 355/355 surface trace samples (previously 0/355 on the same meshes). No pawn/calibration or source-framework changes. Respawn moved to the top of Settings. Headset traversal and remote/package acceptance remain pending; see `Docs/CRIB_WALKABLE_COLLISION_2026_10_17.md`.

- Collision polish remains open after the successful headset door pass. Read-only pawn-default inspection found a 26 cm WMCYN capsule radius, 45 cm step height, and both capsule and skeletal mesh blocking Pawn. Verify runtime behavior before changing the WMCYN wrapper; do not alter the native Mimic baseline. Doorway threshold-to-floor discrepancies also need visual-floor comparison. See the collision polish follow-up in `Docs/FIRST_SIGNAL_TASKS.md`.

- Door rollout: 15 identified wooden leaves across six imported mesh groups now use WMCYN grip-door Blueprints, preserving frames and atlas materials. A WMCYN collision-shell duplicate opens 12 previously blocked doorways; all 811 floor/ceiling trace samples match the original. All 15 synthetic hinge tests pass. Jared reports headset validation passed (2026-10-14); exhaustive individual door coverage was not recorded. This is standalone VR Preview interaction: multiplayer synchronization implementation and multi-headset validation remain open. See `Docs/CRIB_DOOR_BATCH_ROLLOUT.md`.

- User confirmed the orange door grip/push/pull pilot in headset. The real open left door is now separated into `DOOR_Crib_01_Grip`, with retained frame/UVs and standalone grip control. Adjacent closed-door warped reflections were caused by a visible glass collision shell; its rendering is disabled while collision is preserved. Synthetic real-door swing passes; real-door hardware, blocking safety and multiplayer remain open. See `Docs/CRIB_DOOR_01_CONVERSION.md`.

- Door interaction pilot: one freestanding orange `DEV_CribDoor_GripPilot_RightHand` is saved near spawn, using AFCore Comp_Drag and a WMCYN standalone right-grip adapter. Compile, synthetic swing, and editor startup smoke checks pass; headset grip acceptance and multiplayer routing remain open. Pilot ignores Pawn collision; no imported doorway is replaced. See `Docs/CRIB_INTERACTIVE_DOORS_SPIKE.md`.

- Water overlap repair: `Plane_099` slot 1 is now suppressed with a reversible WMCYN masked material, leaving `Plane_037` as the upper water surface. Lower pool, waterfall, and collision are preserved. Terrace/overhead visual checks show the large jagged patches resolved; packaged/mobile testing remains open. See `Docs/CRIB_WATER_OVERLAP_FIX.md`.

- Restoration follow-up: architectural material overrides have been applied in WMCYN-owned assets. Entrance inspection found Static fixture values did not match the setup script (14.1 lumens / 1000 cm observed). The script now writes intensity/radius/cones through editor properties. The 220/100-lumen pass remained overbright; ceiling lights are now 55 lumens and pendants 25 lumens, production-rebaked and screenshot-checked at the entrance and pool-side interior. Validation checks exact intensity and radius. The user identifies the dark entrance area as the missing chandelier location, not a reason to raise all fixture intensities. Water artifacts remain open; this is not final art-lighting acceptance.

- Basic fixture lighting is now implemented in `L_WMCYNOnline`: 73 recessed and ten pendant Static lights, a Lightmass Importance Volume, and three reflection captures. Production-quality bake and editor save/reload validation pass.
- Six WMCYN mesh duplicates repair demonstrated black-floor lightmap problems without changing source geometry, texture UV0, or collision. AFCore, Mimic, pawn/spawn transforms, and renderer configuration are unchanged.
- Repeatable batch: `Tools/Lighting/run_crib_lighting.py`. Results and screenshots: `Docs/CRIB_BASIC_LIGHTING_PLAN.md`.
- Eighteen other imported mesh UV warnings remain. Packaged Quest/PCVR readability, moving-avatar lighting, and performance are pending; no new clients or server were deployed for this pass.
- Next: inspect the now-lit driveway-side entrance/right-hand area for the measured interview set. Basic lighting review comes before more decoration.
- Source/reference inspection is complete: `Docs/CRIB_SOURCE_RESTORATION_AUDIT.md`. All 12 distinct Booth gallery images were reviewed; the original Unity package and material GUIDs confirm recoverable missing finishes. The live import has 40 Crib materials, 25 with flat base-color inputs and none with roughness connected. Restore verified atlas/wood/stone/pool materials before further light-intensity tuning; door deformation still requires a targeted geometry/normals diagnosis. No restoration assets have been applied yet.

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
- The world runtime contract in `Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md` is now implemented in code on both sides; see that doc's Implementation Map. The backend adds `/v1/vr/runtime/register` and `/v1/vr/runtime/heartbeat` behind the `x-wmcyn-server-key` deployment credential, and `/v1/vr/bootstrap` now enforces runtime health/lease, build/protocol compatibility (`409`), mode/slot capacity with transactional reservations, `503` while offline or heartbeat-stale, and issues 60-second HMAC-signed join tickets.
- `UWMCYNWorldRuntimeSubsystem` (WMCYNRuntime) is the server-side implementation: `-WMCYNRegisterRuntime` plus env-var secrets activate registration, 10-second heartbeats with connected-user count and validated slot occupancy, `online` reporting after `L_WMCYNOnline` begins play, PostLogin join-ticket validation with replay rejection, and kick of unauthorized entrants. `UWMCYNBackendSubsystem` sends build/protocol/mode to bootstrap and travels with `host:port?WMCYNTicket=<ticket>` via `TravelToFirstSignalWorld()`.
- Both Win64 targets compile the new runtime code cleanly (the editor target needed a `NetCore` module dependency for `UNetConnection::Close`). The `WMCYN.FirstSignal.JoinTicket.*` automation tests pass headlessly: the plugin's HMAC-SHA256 matches the RFC 4231 vector and verifies a Node-issued fixture ticket, proving backend/Unreal ticket parity.
- The backend is deployed live on `wmcyn-online-mobile` with both runtime secrets configured (gitignored deploy env in the backend repo). Live curl proofs: register/heartbeat reject bad keys (401), accept the real key, reject private hosts unless the LAN-phase `WMCYN_ALLOW_PRIVATE_HOST=true` flag is deployed, and bootstrap enforces auth.
- First Signal uses an interim topology documented in `Docs/DECISIONS.md`: the PCVR recording PC runs the packaged Win64 build as a registered listen host (`?listen -WMCYNRegisterRuntime`), because the Launcher-distributed UE 5.8 cannot build dedicated-server targets. The one-canonical-runtime product model and all contracts are unchanged; a source-built dedicated server replaces the host later.
- Verified clients client-travel to the bootstrap endpoint with their join ticket; after travel the pawn auto-submits the persisted verified identity and skips the second login gate. The registered host does not travel. Right-stick turning now works during the login gate (locomotion stays locked); the failed auto-facing experiments are removed.
- Known interim gap: GameMode assigns presence slots by connection order; ticket-reserved slots are not yet enforced in pawn placement. Fix before remote acceptance.
- 2026-07-15 milestone: the interim listen host registered as runtime `98b6d7a9` at `10.10.0.174:7777`, reported `online`, and heartbeats. A real Firebase login with a provisioned account followed by bootstrap returned that live runtime, a `StandaloneVR_A` reservation, and a signed join ticket — the real backend login/bootstrap proof is closed, including world discovery.
- Quest test accounts exist in Firebase Auth for AA (`vantageforever8@gmail.com`), ghost (`mbatagio123@gmail.com`), and Cole (`cdixonmoore2002@gmail.com`); their `users/{uid}` and `loginHandles` Firestore docs are pending manual console creation, so bootstrap currently returns null usernames for them.
- The interim host now registers under the public IP with `WMCYN_ALLOW_PRIVATE_HOST=false` redeployed; a host restart correctly published a new runtime ID for the same logical world. Router UDP 7777 port-forward to the host PC is the remaining network step for remote entry.
- The Quest distribution zip (APK + install README) is at `Builds/Distribution/WMCYN_FirstSignal_Quest.zip` for Drive-sharing to the remote users.

## Canonical Runtime and CI (2026-07-20)

- UE 5.8 is built from source at `C:\UE58` (Launcher builds cannot produce dedicated servers). The Linux cross-toolchain `v26_clang-20.1.8-rockylinux8` is installed; `LINUX_MULTIARCH_ROOT` is a machine env var.
- The canonical runtime is a Linux dedicated server on GCP: VM `wmcyn-crib-runtime` (e2-standard-4, us-east1-b, project `wmcyn-online-mobile`), static IP `35.185.72.104`, UDP 7777 open, systemd unit `wmcyn-crib` with the shared secrets, `Restart=always`, boot-start. Deployed proof: `register response 200` then `runtime reported online`; a live bootstrap returned that runtime with a `StandaloneVR_A` reservation and signed ticket.
- The interim PCVR listen host is retired. The dedicated server has no local player, so the menu-widget crash class does not apply to it.
- `WMCYNRuntime.uplugin` `PlatformAllowList` includes Linux (the first server build shipped without `/Script/WMCYNRuntime`), and `ServerDefaultMap` boots `L_WMCYNOnline` on dedicated servers.
- GitHub Actions on self-hosted runner `wmcyn-buildbox` (this PC; logon autostart task installed) provides button-driven ops, all `workflow_dispatch` only: `World Control` (VM up/down), `Build Linux Server` (+optional deploy to the VM), `Build Quest APK` (artifact upload). The full cycle — world up, build+deploy, runtime online, world down — is CI-proven.
- The world is kept OFF between test sessions (~$9/month idle: disk + reserved IP; ~$105/month running). Spin up via the Actions button before sessions.
- Workflow steps use Windows PowerShell 5.1 semantics: never redirect `gcloud` stderr, set `$ErrorActionPreference = 'Continue'` in steps that call it.
- Open item: the GitHub repo is public and contains paid marketplace content (AFCore, Mimic Pro, Crib art); flipping it private is recommended and pending owner decision.
- On-device Quest findings closed on 2026-07-15: `bPackageForMetaQuest=True` (was dead legacy key; app launched 2D), Quest renderer config `MobileHDR=False`/`MobileMultiView=True` (missing array MSAA resolve shader crashed startup), in-app `RECORD_AUDIO` request via AndroidPermission, and a real owner-only mesh pointer beam replacing debug-draw (never renders on Quest). Packaged Quest startup, world load, login gate, keyboard typing, and validation are all headset-confirmed.
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

- Crib display stream audio (2026-10-19): BP_Display_C_1 silence healed. Root causes: MediaSound component lacked the `MediaSound` tag AFCore binds by (fixed, persisted in the umap), and the autoplay bind sourced a player from `MT_Video` whose player is `None`. `BP_WMCYN_DisplayAutoplay` now takes its player from the cast `Comp_Video` and binds sound at ~1s/4s after BeginPlay; engine-verified. Known hazard: the adultswim HLS stream corrupt frame asserted Electra (`ElectraUtilsMPEGVideo.cpp:96`) and crashed the editor once; stable feed or local file recommended for long sessions. See `Docs/CRIB_DISPLAY_STREAM.md`.

## MCP

- Unreal MCP is reachable at `http://127.0.0.1:8000/mcp` when the editor server is running.
- Editor toolsets expose actor, scene, asset, Blueprint, object, material, and related inspection/editing capabilities.
- Inspect before mutating, and do not modify maps/assets unless the active task authorizes it.

## Storage

- The earlier zero-space / Zen DDC `507` blocker is cleared. `C:` currently has roughly 446 GB free.
- Recheck free space before large Quest/Windows cooks, but storage is not the active implementation blocker.

## Quest Build Readiness

- The Quest package gate closed on 2026-07-15. An Android Development `BuildCookRun` (build, ASTC cook of `L_WMCYNOnline`, pak, package, archive) succeeded in 10m19s and archived `wmcyn_online-arm64.apk` (390 MB, `com.wmcyn.wmcyn_online`) plus adb install/uninstall scripts to `Saved/StagedBuilds/FirstSignalQuestSmoke/Android_ASTC`.
- The UE 5.8 Android engine component is installed through Epic Games Launcher, and Turnkey reports `Status=Valid, Current_Sdk=r27c, Sdk_HasBestVersion`.
- The machine toolchain lives at `%LOCALAPPDATA%\Android\Sdk` (NDK `27.2.12479018`/r27c, `platforms;android-34`, `build-tools;35.0.1`, `cmake;3.22.1`, platform-tools/adb) with Android Studio's bundled OpenJDK 21 as `JAVA_HOME`. `ANDROID_HOME`, `JAVA_HOME`, and `NDKROOT` are persisted user environment variables; shells opened before 2026-07-15 must re-read them.
- The `SteamController` plugin reference in the `.uproject` is now scoped to Win64. Its unrestricted reference pulled `SteamShared` into the Android target, which has no arm64 Steam binaries and failed UBT rules resolution.
- Existing Android project settings were otherwise untouched: arm64, SDK 35, ES3.1/Vulkan, and Android voice.
- Remaining Quest steps are device-side: sideload the APK to the three headsets and confirm packaged startup, login, and microphone permission.

## Mobile Companion Alignment (2026-10-14)

- The companion app lives at `C:/Users/jvred/Documents/WMCYN/wmcyn-mobile-app/wmcyn-mobile-app` and uses the same Firebase project and production API base as WMCYN Online.
- The local mobile checkout is detached at handoff commit `e0be1ee` with modified lockfiles. It is not a clean branch checkout and must not be reset blindly.
- Mobile `origin/main` has intentionally moved to the native Swift/SwiftUI client. Commit `5fd5f14` removed the Expo/React Native app after native feature parity; `0b271d9` is the current native TestFlight build-number follow-up. Native iOS is now the mobile source of truth.
- The shared pairing backend contract is live: `POST /v1/auth/code.create`, authenticated `POST /v1/auth/code.attach`, and polling `POST /v1/auth/code.exchange`. The authenticated device attaches a short-lived code, the VR client exchanges it for a Firebase custom token, and the existing ID-token/bootstrap path continues from there.
- The native iOS client owns the current pairing and avatar flows. The old Expo pairing wording is historical; verify final display/attach semantics against `native-ios/Sources/Pairing/` before changing the Unreal widget.
- The native iOS avatar client models upload URL -> scan create -> status polling -> preview -> publish -> delete, and sends a Firebase ID token for avatar requests. The backend `origin/main` now has the corresponding avatar routes plus a platform-aware manifest with a default-avatar response, but the Unreal runtime still needs a real published non-default manifest/apply proof.
- Alignment decision: the mobile app is an account/avatar companion and never hosts or selects the persistent Crib world. Unreal remains responsible for world entry, presence, locomotion, and runtime avatar application after Firebase identity/bootstrap.

## Windows PCVR Build Readiness

- A production-map-only Win64 Development `BuildCookRun` now succeeds and archives to `Builds/PCVR/FirstSignal-Development/Windows`.
- The archive contains the runnable `wmcyn_online.exe`, a 557 MB pak, and roughly 1.61 GB total staged content.
- Packaging now cooks only `/Game/Levels/L_WMCYNOnline`; the inherited AFCore demo/template map list is no longer part of the WMCYN shipping manifest.
- `/Game/AFCore/AnimBP/Hand` is excluded from WMCYN packages because its legacy Epic-hand AnimBPs contain removed SteamVR Input structs. The production native Mimic pawn does not depend on those assets, and no AFCore asset was modified.
- A packaged `-nohmd` startup smoke stayed responsive, loaded `L_WMCYNOnline`, created `BP_WMCYN_GameMode_FirstSignal`, assigned/synced slot `0`, locked movement behind login, and initialized the native voice path.
- Packaged startup reports a handled `Comp_Widget` custom-property-list ensure, then continues into the world. Treat this as an AFCore UE 5.8 compatibility follow-up; it is not the current startup-crash path and was not "fixed" by editing AFCore.

## Avatar Pipeline Prep

- The phone-side avatar capture/upload/preview/publish flow and backend avatar job endpoints exist outside this repo and are reported as verified in production.
- A real uploaded avatar capture was inspected on 2026-07-29. The payload is a Live Link Face style archive, not a ready-made Unreal mesh: `.mov`, `depth_data.bin`, `depth_metadata.mhaical`, `frame_log.csv`, `take.json`, and `thumbnail.jpg`.
- The Unreal avatar lane now has its first WMCYN-owned presentation boundary, but is not closed end to end:
  - `UWMCYNBackendSubsystem` can request `GET /v1/avatar/manifest?platform=quest|pcvr`
  - `UWMCYNFirstSignalPresenceComponent` reads `skeletalMeshPath`, optional `headSkeletalMeshPath`, and `animClassPath` from a runtime-mesh manifest
  - WMCYN runtime paths render through transient WMCYN-owned body/head skeletal components; the native Mimic mesh remains instantiated as the tracking source of truth
  - the presence component carries soft cook references for the verified WMCYN body, head, and presentation AnimBlueprint so the first published avatar is retained in packaged builds
  - non-WMCYN/default placeholder paths are ignored by the wrapper and leave the proven native Mimic presentation intact
  - backend manifest responses now support default avatar, placeholder published avatar, `runtime_mesh`, and `pak`
  - the workstation generation/output-selection lane now produces WMCYN-owned mesh/material assets and a compatible presentation AnimBlueprint; remaining product steps are verified manifest publication, locomotion behavior, and device validation
- Avatar docs now live in:
  - `Docs/METAHUMAN_AVATAR_SPEC.md`
  - `Docs/BACKEND_HANDOFF_AVATAR.md`
  - `Docs/AVATAR_IMPLEMENTATION_PLAN.md`
  - `Docs/AVATAR_WORKSTATION_RUNBOOK.md`
- The agreed Unreal direction is:
  - pairing code becomes the preferred in-headset sign-in path for avatar-era UX
  - `GET /v1/avatar/manifest?platform=quest|pcvr` should return one resolved platform-specific manifest
  - the endpoint should return `200` with a default avatar manifest when the user has no published avatar yet
  - runtime avatar delivery should support both `runtime_mesh` and `pak`, with runtime mesh acceptable as the first proof and cooked bundles as the ship-quality target
- `Plugins/WMCYNRuntime` now includes the first Unreal-side pairing runtime support:
  - `UWMCYNBackendSubsystem::SignInWithCustomTokenAndLoadBootstrap`
  - `UWMCYNFirstSignalPairingAsyncAction` for `code.create` + polling `code.exchange`
  - `UWMCYNFirstSignalBlueprintLibrary::BeginFirstSignalPairing`
- The pairing path now compiles in the editor and can drive the same verified identity/bootstrap/world-entry path as credential login once a login widget binds to it.
- The existing `WBP_WMCYN_LoginJoin` asset has not yet been updated in-editor to expose the pairing-code UX. The runtime plumbing exists; the widget hookup is still pending.
- WMCYN-owned workstation wrappers now exist in-repo:
  - `Tools/AvatarPipeline/wmcyn_metahuman_pipeline.py`
  - `Tools/AvatarPipeline/Run-WMCYNAvatarPipeline.ps1`
  - `Tools/AvatarPipeline/Publish-WMCYNAvatarRuntimeMesh.ps1`
- The first successful runtime extraction pass completed on 2026-07-31 for `WMCYNAvatar_52b64e54`:
  - body mesh: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Body.WMCYNAvatar_52b64e54_Body`
  - head mesh: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Head.WMCYNAvatar_52b64e54_Head`
  - generated face/body materials and textures are persisted under the same WMCYN runtime root
  - Quest and PCVR currently share these verified paths pending a real Quest optimization pass
  - the exported body uses the MetaHuman base skeleton and now has a WMCYN-owned compatible AnimBlueprint:
    `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C`
- Do not publish the old Quinn/`ABP_VRBody` animation class against the MetaHuman mesh. The new WMCYN class is a
  reference-pose compatibility wrapper for this gate; its locomotion graph still needs to be authored and tested.

### Jared MetaHuman First Signal presentation family (2026-10-19)

- The newer `/Game/MetaHumans/Jared` import (chosen over the older broken import) now backs a WMCYN-owned
  First Signal presentation family at `/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview/`:
  - `SKM_Jared_Body_Preview` / `SKM_Jared_Head_Preview` (duplicates of the official `SKM_Jared_BodyMesh` / `SKM_Jared_FaceMesh`)
  - `SKM_Jared_Outfit_Preview` / `SKM_Jared_Shoes_Preview` (duplicates of the import's `RiggedFit` / `Shoes`)
  - `Grooms/{Hair_S_360Waves,Eyebrows_M_Wide,Goatee_L_Wavy}` plus a retargeted `_Binding` for each
  - five baked map materials `M_Jared_{Body,Head,Head_LOD2,Head_LOD5to7,Teeth}_BakedPreview` that route official baked
    textures (`T_*_BC/N`) around the import's missing material-function references (roughness 0.65 / specular 0.3)
- Repair state:
  - official import arrived with **empty skeleton references** on all four meshes. The skeletons are now assigned
    directly on the preview assets by `UWMCYNRetargetEditorLibrary::AssignSkeletalMeshSkeleton`
    (`Tools/AvatarPipeline/assign_jared_skeletons.py`), which merges the bone tree in C++ because the property is
    read-only to Python and to the MCP bridge. The earlier `CoreRedirects` approach was removed: the redirects
    could never fire. Targets are the plugin skeletons `metahuman_base_skel` (342 bones) and
    `Face_Archetype_Skeleton` (875 bones); the script asserts their bone counts are unchanged by the merge, since
    those assets live in engine-plugin content under Program Files.
  - material graphs carry the baked basecolor/normal map wiring; the normal sample feeds the material Normal pin.
  - garment textures: `RiggedFit`'s ORM had been imported as sRGB colour, which is what made the jacket and pants
    look like wet plastic. Fixed to `TC_Masks`/sRGB off, with the normal map on `TC_Normalmap`
    (`Tools/AvatarPipeline/fix_jared_appearance.py`).
  - eye occlusion shell and lacrimal edge (`eyeshell_shader_shader`, `eyeEdge_shader_shader`) are hidden behind
    `M_WMCYN_Hidden`, a translucent unlit material at opacity 0.
  - eyes use `MI_Jared_Eye{L,R}_Preview`, flat DefaultLit stand-ins sampling the baked `T_EyeSclera{L,R}_BC` at
    roughness 0.15. **This is a judgement call, not a derivation.** The MetaHuman unified eye shader is present
    but does not resolve here, and its baked-texture path is behind a static switch unreachable from script. If
    the eyes still look wrong, the next suspects are that the sclera bake omits the iris, or that the eyeball UVs
    are not what this assumes - not further material swapping.
  - grooms render against `MID_*` instances duplicated from the source import. Slot-to-material is *derived*, by
    matching each slot name to the instance's parent material name, not hardcoded. Where several instances share
    a parent the lowest-numbered wins and the alternatives are recorded in
    `Saved/Diagnostics/Avatars/jared_groom_materials.json`, so a wrong hair colour is a swap to a named
    alternative rather than another guess. Every groom material slot was `None` **in the source assets**, not as
    an artefact of duplication - that is why hair and goatee first rendered white.
  - eyelashes are deliberately groom-free: the head mesh carries eyelash geometry in its own
    `eyelashes_shader_shader` section, so adding the groom would double them.
  - **unverified**: the above is confirmed only by the two diagnostic JSON reports. No headset or PIE visual
    confirmation of groom colour or the eye stand-in has been recorded.
  - **pending editor validation**: PIE run, plugin rebuild (Live Coding or restart), visual/bone binding check for
    both meshes, and an AnimBP compatibility probe for the shared presentation AnimBP.
- Runtime changes (C++, requires rebuild):
  - `WMCYNFirstSignalPresenceComponent` now accepts a family table that includes both the historical
    `WMCYNAvatar_52b64e54` catalog and the new Jared Preview family; manifest body/head paths must match one
    family exactly, and the shared presentation AnimBP path must match.
  - **Garments and grooms travel with the family, not over the wire.** `FRuntimeAvatarAcceptFamily` carries
    optional `Outfit`/`Shoes` paths and a groom list; the 52b64e54 family leaves them null. The manifest contract,
    the replicated fields and the trust boundary are all unchanged, so the server must still log
    `stage=authority result=rejected reason=unverified_client_selection` for client-supplied paths. Asset paths
    originate only in C++. Do not move them into the manifest to "simplify" this.
  - Garments are driven by `SetLeaderPoseComponent(AvatarBodyVisual)`, not a second AnimBP - they share the body
    skeleton, so they copy its pose rather than evaluate another retarget. Grooms attach to `AvatarHeadVisual`
    with `SetBindingAsset` *before* `SetGroomAsset`; that order matters.
  - All assets load before any visual swap. A family that names an asset which fails to load aborts the whole
    swap, so a missing garment can never leave a half-dressed avatar.
  - `WMCYNRuntime.uplugin` now declares the `HairStrands` plugin dependency. It compiled without it, but
    packaged builds would have failed.
  - `WMCYNBackendSubsystem::RequestAvatarManifest` gained a dev-only `-WMCYNDevAvatarJared` (editor/unpackaged
    builds only) synthetic `runtime_mesh` manifest so the Crib can login-validate Jared without a backend
    publication. Packaged/hosted builds keep the trusted backend as the only published-manifest authority.
- Pending for hosted-runtime use: a real server-side published `runtime_mesh` manifest for `[account email]`
  (ownership was verified unresolved in `Docs/AVATAR_LOGIN_AUDIT_2026_10_15.md`); the client accepts it
  identically through the family table.
- Offline packaging review on 2026-07-31 confirmed the cooked output contains the published body mesh, head mesh, and
  `ABP_WMCYN_MetaHuman_Presentation`. Explicit WMCYN Asset Manager entries now retain those three manifest targets
  without cooking the discarded one-gigabyte export folder. A full isolated Windows stage was attempted but stopped
  when the workstation ran out of disk space copying debug symbols; no source or content assets were changed by that
  failure.
- Those wrappers intentionally lean on Epic's official MetaHuman Python helpers for take ingest, identity creation, conform-from-identity, auto-rig, texture download, and optimized assembly rather than rebuilding the MetaHuman editor flow from scratch.
- This avatar lane is a planned next phase after the current First Signal shared-world gates pass; it is not the active blocker today.

### Mimic to MetaHuman pose bridge (2026-07-31)

- The first WMCYN-owned Mimic-to-MetaHuman pose/IK bridge is now saved and compiled.
- The native Mimic `BP_WMCYN_UserPawn_FirstSignal` body remains authoritative for tracking, locomotion, calibration, footsteps, collision, and floor behavior.
- The WMCYN MetaHuman presentation AnimBlueprint now contains `Retarget Pose From Mesh -> AnimGraph Root` and uses the native Mimic BodyMesh as its parent source.
- WMCYN-owned source/target IK Rigs and `RTG_WMCYN_Mimic_To_MetaHuman` are saved under the avatar runtime folder. AFCore assets remain untouched.
- The Windows stage-only archive is available at `D:\WMCYN_Packages\WMCYN_MimicMetaHumanBridge_20260731_StageOnly` and its pak contains the bridge assets.
- Headset validation is still required. The full fresh cook exceeded the local 20-minute window while processing MetaHuman shader/texture dependencies; the stage-only archive used the existing cooked output that already contained the bridge assets.

## Next Gate

1. Reconcile the default PlayerStart yaw with the approved `StandaloneVR_A` marker before packaging.
2. Enforce ticket-reserved slots in GameMode assignment instead of connection-order indexing.
3. Run the four-client PIE regression for indexed spawn, identity, nameplates, roster, tracking, and voice ownership.
4. Start the GCP runtime through the existing workflow and run the remote acceptance with three Quest users plus one PCVR/OBS user.
5. Confirm packaged Quest microphone permission, voice, menu, and performance on all three headsets.
6. Resolve the mobile/Unreal pairing direction, then bind the in-headset pairing UI.
7. Publish one real avatar manifest and prove the non-default avatar applies on PCVR before attempting Quest optimization.
