# First Signal Tasks

## Acceptance Target

Bring The WMCYN Crib online with two standalone VR users and one PCVR recording user in the same persistent world, with basic presence, username/display name, two-way voice, OBS-friendly capture, and one Verbatim marker.

## Completed Baseline

- [x] Preserve HumanCodeable AFCore / Advanced VR Framework as the runtime baseline.
- [x] Use `L_WMCYNOnline` and the Crib as the active First Signal world.
- [x] Create WMCYN-owned Content folder scaffolding.
- [x] Create deterministic markers for StandaloneVR_A, StandaloneVR_B, and PCVR_Recording.
- [x] Align the default PlayerStart and StandaloneVR_A at yaw `180` so the login-locked user spawns facing the intended direction.
- [x] Create `BP_WMCYN_QuestUserPawn` as a child of AFCore `BP_Pawn_VR_Char`.
- [x] Configure WMCYN Level Data, GameMode, PlayerController, and PlayerState wrappers.
- [x] Spawn and possess `BP_WMCYN_QuestUserPawn` in VR Preview.
- [x] Confirm headset tracking, controllers/hands, and analog locomotion work.
- [x] Remove the Jared MetaHuman preview from the active First Signal path.
- [x] Repair the active Crib floor collision path and remove the temporary floor guard.
- [x] Confirm hardware VR locomotion around the Crib without falling through the floor.

## Login and Identity

- [x] Reduce the visible login screen to username, password, and Enter World.
- [x] Keep username/password on stable native Unreal `EditableTextBox` controls.
- [x] Wire the native fields to the existing AFCore runtime keyboard overlay without editing AFCore.
- [x] Preserve the active AFCore `WidgetInteractionComponent` virtual-user focus when opening or switching login fields.
- [x] Separate keyboard text focus from pointer clicks with a WMCYN-owned, hit-testing-disabled interaction on its own virtual user.
- [x] Drive the thin cyan login ray from the tracked `RightAim` controller pose so it points naturally forward and is independent of hand/finger animation.
- [x] Lock locomotion and stick turning in the WMCYN native-pawn adapter until successful identity submission while preserving headset/controller tracking.
- [ ] Reconfirm continuous username/password typing, keyboard Enter/Close, controller-forward `RightAim` placement, mirror nameplate visibility, and Enter World movement unlock in the production native pawn headset path.
- [x] Move the keyboard fully beyond the login `Comp_Widget` hit-test rectangle using the root widget's lower edge.
- [x] Keep the login gate in `Game Only` while movement is locked so AFCore receives controller Select presses after text focus.
- [x] Set explicit dark text/hint colors for the white username and password fields.
- [x] Confirm in headset that AFCore keyboard clicks type visible text into both username and password.
- [x] Route `Enter World` and password keyboard Enter through one `SubmitLogin` path.
- [x] Close the AFCore keyboard overlay and owning 3D entry manager after successful submission.
- [x] Confirm a headset login submission stores identity, closes the menu, and restores world control.
- [x] Run a focused VR login regression check covering both fields, keyboard controls, identity storage, and login-gate closure.
- [x] Create replicated WMCYN PlayerState fields: `Username`, `DisplayName`, `PresenceMode`, and `Capabilities`.
- [x] Remove superseded access-code/session-selection and marker-relocation logic from the active login graph.
- [x] Mirror successful AFCore player-name updates into the authoritative WMCYN PlayerState identity fields.
- [x] Enter the persistent Crib world after login without a user-facing session picker.
- [x] Drive the inherited AFCore NameTag from `DisplayName` through `Comp_PlayerInfo_Basic`.
- [x] Confirm three-client PIE receives replicated identity state and shows both remote AFCore NameTags on each client.
- [ ] Confirm the same NameTag behavior with distinct login names on the physical three-device run.

## Voice Validation

- [x] Add WMCYN-owned `WMCYN_VOIPTalker` to the active Quest pawn.
- [x] Attach VOIP presence to the inherited AFCore head path and attenuation.
- [x] Add `BP_WMCYN_VoiceRegistrationComponent` without editing AFCore.
- [x] Create the hidden technical OnlineSubsystem session used to register local talker `0`.
- [x] Automatically run `ToggleSpeaking 1` for the current VR Preview proof.
- [x] Diagnose the UE 5.8 Windows startup crash caused by `MaxLocalTalkers=4` overflowing fixed one-entry local voice storage.
- [x] Set `MaxLocalTalkers=1` and keep `MaxRemoteTalkers=8`.
- [x] Confirm VR Preview starts without the `OnlineSubsystemNull` access violation.
- [x] Confirm the voice module and interface are available and enabled.
- [x] Confirm local talker `0` is registered, networked, recording, and talking.
- [x] Confirm `StartLocalProcessing(0) returned 0x00000000` after registration.
- [x] Confirm repeated non-zero compressed Oculus microphone frames flow.
- [x] Close the standalone capture lane and retain hybrid audio: in-game voice for presence/reference, external mics for backup/final clean recording.
- [x] Remove automatic voice-dump startup spam and suppress per-frame `LogVoiceEngine` packet logging by default.
- [x] Bind each replicated pawn's VOIPTalker to its own PlayerState while limiting capture/session setup to the locally controlled pawn.
- [x] Confirm one local capture path per world in three-client PIE without new invalid PlayerState/session warnings.
- [ ] Decide First Signal open mic plus mute versus push-to-talk.
- [ ] Confirm Quest user A hears Quest user B.
- [ ] Confirm Quest user B hears Quest user A.
- [ ] Confirm PCVR recording user hears both Quest users.
- [ ] Confirm OBS receives the intended voice mix.
- [ ] Confirm packaged Quest microphone permission and capture.
- [ ] Add minimal muted/speaking feedback.

## Multiplayer Presence

- [ ] Select the real persistent-world networking/backend path beyond local `OnlineSubsystemNull` validation.
- [x] Prove a three-client listen-server PIE topology with indexed StandaloneVR_A, StandaloneVR_B, and PCVR_Recording PlayerStates/pawns.
- [x] Confirm the three server-authoritative pawn transforms remain separated at the three indexed spawn markers.
- [x] Confirm each client hides its own AFCore NameTag and renders both remote AFCore NameTags.
- [ ] Run two standalone VR users in the same Crib runtime.
- [ ] Confirm each user sees the other's head/controller/hand presence.
- [ ] Confirm locomotion replicates acceptably.
- [ ] Join with the PCVR recording user.
- [ ] Confirm PCVR sees both standalone users and their display names.

## Player Body and Mirror

- [x] Audit the active WMCYN Quest pawn, AFCore inheritance chain, capsule, camera, and visual components.
- [x] Confirm the current visible avatar is a correctly scaled AFCore head-and-torso bust with no Skeletal Mesh assigned to `CharacterMesh0`.
- [x] Inspect AFCore's movement mannequin and Animation Blueprint for VR full-body IK.
- [x] Confirm the AFCore mannequin is a third-person idle/run/jump body, not a drop-in headset/controller-driven VR IK body.
- [x] Add WMCYN-owned `BP_WMCYN_AvatarMirror` and place `DEV_AvatarMirror_Tracking` near StandaloneVR_A without editing AFCore.
- [x] Add WMCYN-owned AFCore left/right hand wrappers and select them from `BP_WMCYN_QuestUserPawn` after AFCore initialization.
- [x] Verify the hand wrappers retain AFCore movement, interaction, laser, teleport, finger, and replication components.
- [x] Add a dedicated WMCYN mannequin mesh plus `CR_WMCYN_VRBody` without editing AFCore.
- [x] Drive head/hand targets from AFCore tracking and keep the lower body on AFCore's stable input pose instead of HMD-inferred pelvis/foot targets.
- [x] Bypass the head-driven full-body solve so the AFCore base pose keeps the torso upright and legs stable.
- [x] Replace fixed arm pole directions with WMCYN body-relative elbow targets derived from shoulder/hand midpoints.
- [x] Preserve the headset-confirmed right arm and mirror its solve-axis basis onto the left mannequin arm.
- [x] Raise only the visible AFCore head component from the WMCYN wrapper without changing HMD, capsule, camera proxy, or torso transforms.
- [x] Prove a second/unpossessed WMCYN pawn renders the full adult-scale tracked-body path in PIE.
- [x] Migrate Mimic Pro VR IK content under `/Game/FullBodyVRTemplate` and compile its UE 5.8 core assets.
- [x] Add `BP_WMCYN_QuestUserPawn_Mimic` as a true child of the stable WMCYN Quest pawn rather than replacing AFCore gameplay systems.
- [x] Add `BP_WMCYN_MimicBodyComponent` as a WMCYN-owned compatibility child with safe HMD velocity delta handling.
- [x] Attach Mimic to the inherited body mesh before BeginPlay and bind it to AFCore camera/controller tracking.
- [x] Confirm in PIE that the Mimic pawn preserves indexed spawn, stabilizer, local voice capture, login/identity inheritance, Manny mesh assignment, AnimBP startup, and component attachment.
- [x] Make the Mimic body owner-visible; use Manny's attached head, hide the separate AFCore bust, and keep only the duplicate Manny hand bones hidden.
- [x] Anchor the Mimic body to AFCore pawn ground plus the current 12 cm sole clearance without changing the AFCore capsule, tracking origin, or locomotion path.
- [x] Record and remove the failed detection-only seated pass after hardware showed a torso-height camera and folded knees.
- [x] Implement and remove the failed per-tick `CameraHandle` seated offset after hardware showed a badly displaced camera.
- [x] Compare the migrated sample's `VR Origin`, `Height Offset`, calibration marker, and head presentation directly against the WMCYN child.
- [x] Pass AFCore `CameraHandle` as Mimic's `VR Origin`, restore the untouched AFCore camera path, show Manny's attached head, and hide the separate AFCore head/torso bust.
- [x] Create an isolated native-Mimic A/B map, seated test child, and minimal GameMode without changing `L_WMCYNOnline`.
- [x] Replace only the native test child's one-centimeter custom physics capsule with the proven AFCore 26 cm `Pawn` collision settings; confirm stable Crib-floor contact in desktop PIE.
- [x] Run native Mimic VR Preview while seated and click the left thumbstick once to execute the package's own height calibration.
- [x] Confirm native Mimic VR Preview works flawlessly after calibration, including footstep audio and stable Crib-floor collision.
- [x] Promote the proven child to `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal` as the production player source of truth.
- [x] Audit the active controller, login, voice, NameTag, and replication dependencies before switching the main map.
- [x] Add generic indexed spawn/possession support for `BP_WMCYN_UserPawn_FirstSignal` without AFCore pawn casts.
- [x] Connect login submission to server-owned `Username` and `DisplayName` through the native pawn adapter.
- [x] Add a WMCYN-owned VOIPTalker/registration adapter and a nameplate visible to remote users and the local avatar mirror after login.
- [x] Anchor each nameplate above the replicated HMD position and drive its text from the entered username/replicated PlayerState identity.
- [x] Prove native HMD/controller pose replication and interpolation in three-client production PIE.
- [x] Switch `L_WMCYNOnline` to `BP_WMCYN_UserPawn_FirstSignal` after the adapter gates pass.
- [x] Restore native-pawn 3D menu interaction by normalizing both inherited widget rays to the UI `Visibility` trace channel in the WMCYN runtime adapter.
- [x] Remove the false `BAD SIZE` editor badges from the three indexed First Signal markers without changing their transforms, capsules, or AFCore source asset.
- [ ] Reconfirm the 3D login, AFCore keyboard, Enter World unlock, and typed identity on the production native pawn in VR Preview.
- [x] Confirm Mimic's existing AnimBP already drives procedural alternating foot targets from tracked world velocity.
- [x] Confirm the native Mimic hierarchy, attached Manny head, seated calibration, controller reach, locomotion, footsteps, and floor contact pass together in headset.
- [x] Retire further tuning of the AFCore-derived Mimic wrapper; keep it only as rollback evidence.
- [ ] Preserve the production native pawn's explicit left-thumbstick calibration behavior while adding WMCYN adapters.
- [ ] Measure Quest frame timing and decide whether the full Mimic solver is acceptable for two standalone users.
- [ ] Capture one mirror screenshot or short OBS clip.
- [ ] Confirm the local mirror shows body/hands and the direct first-person view shows chest/arms without head occlusion.
- [x] Confirm native headset/controller motion drives the Mimic body correctly in the successful hardware pass.

## PCVR and OBS

- [x] Use `BP_WMCYN_UserPawn_FirstSignal` as the shared default pawn for standalone VR and PCVR; inspect a camera-specific pawn only if OBS capture proves it necessary.
- [x] Assign local PIE slot `2` the replicated `PCVR` mode and `Recording` plus `CanTriggerVerbatimMarker` capabilities.
- [ ] Inspect `BP_Pawn_VR_Camera` only if the shared pawn cannot support clean capture.
- [ ] Establish an OBS-friendly spectator/camera output.
- [ ] Confirm framing, stable output, and usable audio/video capture.

## Verbatim Marker

- [ ] Add one WMCYN-owned trigger for a structured Verbatim world marker.
- [ ] Include timestamp, world/location, runtime ID, user identity, presence mode/slot, capabilities, map, and note.
- [ ] Log to Unreal and provide simple on-screen/debug confirmation first.
- [ ] Defer backend storage; optional local JSONL can follow the first proof.

## Build Pipeline

- [x] Recover substantial free disk space before cooking or packaging.
- [x] Attempt an isolated Android Development package smoke and capture the engine-installation blocker.
- [ ] Enable the Android optional download component for the installed UE 5.8 build in Epic Games Launcher.
- [ ] Configure the UE 5.8 Android SDK/NDK/JDK toolchain and verify it with Turnkey.
- [ ] Confirm Zen DDC no longer returns HTTP `507 Insufficient Storage` during the next sustained cook/cache run.
- [ ] Package the standalone Quest build.
- [ ] Sideload to both headsets.
- [ ] Package the Windows PCVR recording build.
- [ ] Run the repeatable three-device First Signal test.

## Current Next Steps

1. Reconfirm the existing 3D login and AFCore keyboard overlay on the production native pawn in VR Preview, including movement unlock after Enter World.
2. Enable UE 5.8 Android support, configure its SDK, and rerun the isolated Quest package smoke.
3. Run `Docs/FIRST_SIGNAL_MULTIUSER_VALIDATION.md` on Quest A, Quest B, and the PCVR/OBS machine using distinct login names.
4. Confirm cross-device native body/head/hand tracking, two-way voice, and OBS audio/video capture.
5. Measure Quest performance with two native Mimic users.
6. Add the first Verbatim marker.
7. Select the packaged-device persistent-world connection path beyond local `OnlineSubsystemNull` validation.

## Out of Scope

- public matchmaking
- stores, inventory, or economy
- mobile feed
- concerts or multiple rooms
- creator tools
- full backend authentication
- dedicated servers
- custom locomotion, grabbing, or base menu systems
- MetaHuman/avatar-fidelity work
