# First Signal Tasks

## Acceptance Target

Bring The WMCYN Crib online with three standalone Quest users and one PCVR recording user in the same always-on world, with basic presence, username/display name, two-way voice, and OBS-friendly capture.

## Completed Baseline

- [x] Preserve HumanCodeable AFCore / Advanced VR Framework as the runtime baseline.
- [x] Use `L_WMCYNOnline` and the Crib as the active First Signal world.
- [x] Create WMCYN-owned Content folder scaffolding.
- [x] Create deterministic markers for StandaloneVR_A, StandaloneVR_B, StandaloneVR_C, and PCVR_Recording.
- [x] Confirm yaw `0` on StandaloneVR_A as the correct runtime entry facing and reapply it through the delayed stabilizer.
- [ ] Reconcile the default PlayerStart yaw with the approved StandaloneVR_A marker before packaging.
- [x] Reapply the indexed marker rotation in the delayed VR stabilizer so native HMD/body initialization cannot reverse the approved entry facing.
- [x] Create `BP_WMCYN_QuestUserPawn` as a child of AFCore `BP_Pawn_VR_Char`.
- [x] Configure WMCYN Level Data, GameMode, PlayerController, and PlayerState wrappers.
- [x] Spawn and possess `BP_WMCYN_QuestUserPawn` in VR Preview.
- [x] Confirm headset tracking, controllers/hands, and analog locomotion work.
- [x] Remove the Jared MetaHuman preview from the active First Signal path.
- [x] Repair the active Crib floor collision path and remove the temporary floor guard.
- [x] Confirm hardware VR locomotion around the Crib without falling through the floor.

## Login and Identity

- [x] Reduce the visible login screen to username-or-email, password, and Enter World.
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
- [x] Retire the unreferenced `WBP_WMCYN_BootLogin`; keep one active login surface.
- [x] Run a focused VR login regression check covering both fields, keyboard controls, identity storage, and login-gate closure.
- [x] Create replicated WMCYN PlayerState fields: `Username`, `DisplayName`, `PresenceMode`, and `Capabilities`.
- [x] Remove superseded access-code/session-selection and marker-relocation logic from the active login graph.
- [x] Mirror successful AFCore player-name updates into the authoritative WMCYN PlayerState identity fields.
- [x] Enter the persistent Crib world after login without a user-facing session picker.
- [x] Drive the inherited AFCore NameTag from `DisplayName` through `Comp_PlayerInfo_Basic`.
- [x] Confirm the earlier three-client PIE receives replicated identity state and shows remote AFCore NameTags on each client.
- [ ] Confirm four-client PIE shows three remote AFCore NameTags per client.
- [ ] Confirm the same NameTag behavior with distinct login names on the physical four-device internet run.

## Runtime Menu and AFCore Reuse

- [x] Create `WBP_WMCYN_RuntimeMenu` as a WMCYN-owned composition of AFCore scaffold, vertical tabs, and widget switcher behavior.
- [x] Expose `WBP_WMCYN_WhosHere` and `WBP_WMCYN_Settings_Audio` from the post-login runtime menu without editing AFCore.
- [x] Verify the runtime menu opens and closes in-headset through AFCore's `FaceButton02Right` mapping on Quest right `B`; desktop `M` remains the toggle fallback and `Escape` closes it.
- [x] Prove in PIE that successful login creates the menu and the AFCore Players roster displays the submitted identity.
- [ ] Headset-confirm runtime menu pointer interaction and four-user roster refresh.
- [x] Add `WBP_WMCYN_Settings_FirstSignal`, composing the AFCore Audio page with a WMCYN-owned Respawn button.
- [x] Remove Audio Quality from the active Settings page and expose only Master, Music, Voice, and SFX.
- [x] Route WMCYN-owned footstep cues through AFCore `SC_SFX` so Master/SFX can affect the current footsteps.
- [ ] Headset-confirm Master and SFX mute/reduce footsteps; confirm Voice remains independently adjustable.
- [x] Implement server-authoritative respawn by reusing the current indexed `SyncPresencePawn` path without reloading the level or replacing the pawn.
- [x] Headset-confirm Respawn returns the current possessed user to the assigned slot and preserves login, voice, tracking, and movement.

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
- [ ] Confirm Quest user C hears and is heard by Quest users A and B.
- [ ] Confirm PCVR recording user hears all three Quest users.
- [ ] Confirm OBS receives the intended voice mix.
- [ ] Confirm packaged Quest microphone permission and capture.
- [ ] Add minimal muted/speaking feedback.

## Multiplayer Presence

- [x] Audit the existing Firebase backend and identify its reusable authentication/profile contracts.
- [x] Confirm upstream backend `main` builds and lints from a clean clone.
- [x] Repair the local `wmcyn-backend-infra` checkout from upstream while preserving existing local work.
- [x] Remove public debug config output and production authentication bypasses before reactivating Firebase.
- [x] Support one identifier field accepting username, `@username`, or email plus password.
- [x] Add an authenticated VR bootstrap response containing verified identity and the current Crib runtime endpoint.
- [x] Add the WMCYN Unreal HTTP authentication/profile subsystem source without storing or replicating the password/token.
- [x] Install UE 5.8's preferred MSVC `14.50.35717`, rebuild `WMCYNRuntime`, and connect the active login Blueprint to the asynchronous subsystem.
- [x] Keep locomotion locked until authentication and world entry succeed; retain an explicit local fallback for PIE development only.
- [x] Select one authoritative internet-hosted Unreal runtime as the persistent-world networking model beyond local `OnlineSubsystemNull` validation.
- [x] Define Unreal world-runtime registration, heartbeat lease, build compatibility, reconnect, Firebase bootstrap, and short-lived join-ticket contracts in `Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md`.
- [x] Implement server-only runtime registration and 10-second heartbeat against the singleton `worldRuntimes/wmcyn-online` record.
- [x] Implement stale-runtime rejection, build/protocol compatibility, and signed join-ticket issue/validation.
- [ ] Rebuild the editor target and run the `WMCYN.FirstSignal.JoinTicket.*` automation tests after the editor closes (Live Coding blocked the editor-target link; the Game target compiles clean).
- [ ] Deploy the updated Firebase Functions with `WMCYN_RUNTIME_SERVER_KEY` and `WMCYN_JOIN_TICKET_SECRET` configured once billing is restored.
- [ ] Deploy the canonical `L_WMCYNOnline` runtime to an internet-reachable server with `-WMCYNRegisterRuntime`, the shared secrets, and `-WMCYNPublicHost=`.
- [x] Prove a three-client listen-server PIE topology with indexed StandaloneVR_A, StandaloneVR_B, and PCVR_Recording PlayerStates/pawns.
- [x] Expand indexed assignment to StandaloneVR_A=`0`, StandaloneVR_B=`1`, StandaloneVR_C=`2`, and PCVR_Recording=`3`.
- [x] Confirm all four server-authoritative marker transforms and metadata remain distinct in `L_WMCYNOnline`.
- [ ] Run a four-client PIE regression for indexed spawn, identity, nameplates, tracking, roster, and voice ownership.
- [x] Confirm the earlier three-client proof hides each local AFCore NameTag and renders both remote NameTags; repeat with three remotes in four-client PIE.
- [ ] Run three standalone Quest users in the same Crib runtime over the internet.
- [ ] Confirm each user sees the other three users' head/controller/hand presence and names.
- [ ] Confirm locomotion replicates acceptably.
- [ ] Join with the PCVR recording user.
- [ ] Confirm PCVR sees all three standalone users and their display names.
- [ ] Prove disconnect/reconnect restores verified identity and slot without a session picker.
- [ ] Prove server restart publishes a new runtime ID while clients return to the same logical Crib world.

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
- [x] Add a WMCYN-owned VOIPTalker/registration adapter and host AFCore pawn `Widget_NameTag` for remote users and the local avatar mirror after login.
- [x] Anchor each AFCore NameTag above the replicated HMD and bridge entered/replicated identity into `Comp_PlayerInfo_Basic.Updated_PlayerName`.
- [x] Headset-validate the upright yaw-only NameTag and settle its final First Signal width at `25 cm`.
- [x] Replace the plain runtime widget host with AFCore `Comp_Widget` + `DA_Theme_Default` so `M_UIMaster` receives the intended theme parameters.
- [ ] Headset-confirm the AFCore-themed NameTag no longer renders white-on-white after login.
- [x] Prove native HMD/controller pose replication and interpolation in three-client production PIE.
- [x] Switch `L_WMCYNOnline` to `BP_WMCYN_UserPawn_FirstSignal` after the adapter gates pass.
- [x] Restore native-pawn 3D menu interaction by normalizing both inherited widget rays to the UI `Visibility` trace channel in the WMCYN runtime adapter.
- [x] Remove the false `BAD SIZE` editor badges from the original three indexed First Signal markers without changing their transforms, capsules, or AFCore source asset; the new C marker uses the same logical-anchor pattern.
- [ ] Reconfirm the 3D login, AFCore keyboard, Enter World unlock, and typed identity on the production native pawn in VR Preview.
- [x] Confirm Mimic's existing AnimBP already drives procedural alternating foot targets from tracked world velocity.
- [x] Confirm the native Mimic hierarchy, attached Manny head, seated calibration, controller reach, locomotion, footsteps, and floor contact pass together in headset.
- [x] Retire further tuning of the AFCore-derived Mimic wrapper; keep it only as rollback evidence.
- [ ] Preserve the production native pawn's explicit left-thumbstick calibration behavior while adding WMCYN adapters.
- [ ] Measure Quest frame timing and decide whether the full Mimic solver is acceptable for three standalone users.
- [ ] Capture one mirror screenshot or short OBS clip.
- [ ] Confirm the local mirror shows body/hands and the direct first-person view shows chest/arms without head occlusion.
- [x] Confirm native headset/controller motion drives the Mimic body correctly in the successful hardware pass.

## PCVR and OBS

- [x] Use `BP_WMCYN_UserPawn_FirstSignal` as the shared default pawn for standalone VR and PCVR; inspect a camera-specific pawn only if OBS capture proves it necessary.
- [x] Assign slot `3` the replicated `PCVR` mode and `Recording` capability; slots `0..2` remain Quest.
- [ ] Inspect `BP_Pawn_VR_Camera` only if the shared pawn cannot support clean capture.
- [ ] Establish an OBS-friendly spectator/camera output.
- [ ] Confirm framing, stable output, and usable audio/video capture.

## Stretch: Verbatim Marker

- [ ] Do not schedule this work until the First Signal shared-world, voice, OBS, and camera gates pass.
- [ ] Add one WMCYN-owned trigger for a structured Verbatim world marker.
- [ ] Include timestamp, world/location, runtime ID, user identity, presence mode/slot, capabilities, map, and note.
- [ ] Log to Unreal and provide simple on-screen/debug confirmation first.
- [ ] Defer backend storage; optional local JSONL can follow the first proof.

## Build Pipeline

- [x] Recover substantial free disk space before cooking or packaging.
- [x] Attempt an isolated Android Development package smoke and capture the engine-installation blocker.
- [ ] Enable the Android optional download component for the installed UE 5.8 build in Epic Games Launcher.
- [ ] Configure the UE 5.8 Android SDK/NDK/JDK toolchain and verify it with Turnkey.
- [x] Confirm Zen DDC no longer returns HTTP `507 Insufficient Storage` during a full Windows cook/cache run.
- [ ] Package the standalone Quest build.
- [ ] Sideload to all three Quest headsets.
- [x] Package and archive the Windows PCVR Development build from `L_WMCYNOnline`.
- [x] Smoke-test packaged Windows startup through map load, indexed pawn sync, login gate, and native voice initialization.
- [ ] Run the packaged Windows build with the PCVR headset and OBS.
- [ ] Review the nonfatal AFCore `Comp_Widget` custom-property-list ensure without editing AFCore by default.
- [ ] Run the repeatable four-device remote First Signal test.

## Current Next Steps

1. Implement and deploy the always-on runtime registration, heartbeat, ticket, and reconnect contract.
2. Enable UE 5.8 Android support, configure its SDK, and rerun the isolated Quest package smoke.
3. Restore Firebase billing and run one real backend login/bootstrap proof.
4. Prove Quest A, Quest B, Quest C, and PCVR Recording over the public internet with tracking, names, voice, and OBS capture.
5. Measure Quest performance with three native Mimic users, then specify the handheld camera feature.

## Out of Scope

- public matchmaking
- stores, inventory, or economy
- mobile feed
- concerts or multiple rooms
- creator tools
- full account administration, password recovery, or public registration UI
- production-scale dedicated-server fleet orchestration
- custom locomotion, grabbing, or base menu systems
- MetaHuman/avatar-fidelity work
- Verbatim marker before the primary First Signal gates pass
