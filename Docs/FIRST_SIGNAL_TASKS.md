# First Signal Tasks

## Acceptance Target

Bring The WMCYN Crib online with two standalone VR users and one PCVR recording user in the same persistent world, with basic presence, username/display name, two-way voice, OBS-friendly capture, and one Verbatim marker.

## Completed Baseline

- [x] Preserve HumanCodeable AFCore / Advanced VR Framework as the runtime baseline.
- [x] Use `L_WMCYNOnline` and the Crib as the active First Signal world.
- [x] Create WMCYN-owned Content folder scaffolding.
- [x] Create deterministic markers for StandaloneVR_A, StandaloneVR_B, and PCVR_Recording.
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

## PCVR and OBS

- [x] Keep PCVR on the shared AFCore VR pawn family unless capture proves it insufficient.
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
- [ ] Confirm Zen DDC no longer returns HTTP `507 Insufficient Storage` during the next sustained cook/cache run.
- [ ] Package the standalone Quest build.
- [ ] Sideload to both headsets.
- [ ] Package the Windows PCVR recording build.
- [ ] Run the repeatable three-device First Signal test.

## Current Next Steps

1. Run `Docs/FIRST_SIGNAL_MULTIUSER_VALIDATION.md` on Quest A, Quest B, and the PCVR/OBS machine using distinct login names.
2. Select and implement the packaged-device persistent-world connection path beyond local `OnlineSubsystemNull` validation.
3. Close Quest A <-> Quest B voice and PCVR monitoring/OBS checks with hardware evidence.
4. Add the first Verbatim marker.

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
