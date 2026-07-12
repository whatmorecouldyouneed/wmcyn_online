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
- [x] Create replicated WMCYN PlayerState fields: `Username`, `DisplayName`, `PresenceMode`, and `Capabilities`.
- [ ] Remove superseded access-code/session-selection logic from the login graph.
- [ ] Write successful login values into the WMCYN PlayerState.
- [ ] Enter the persistent Crib world after login without a user-facing session picker.
- [ ] Drive the inherited AFCore NameTag or a small WMCYN nameplate from `DisplayName`.
- [ ] Confirm remote users see replicated identity/nameplate state.

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
- [ ] Decide First Signal open mic plus mute versus push-to-talk.
- [ ] Confirm Quest user A hears Quest user B.
- [ ] Confirm Quest user B hears Quest user A.
- [ ] Confirm PCVR recording user hears both Quest users.
- [ ] Confirm OBS receives the intended voice mix.
- [ ] Confirm packaged Quest microphone permission and capture.
- [ ] Add minimal muted/speaking feedback.

## Multiplayer Presence

- [ ] Select the real persistent-world networking/backend path beyond local `OnlineSubsystemNull` validation.
- [ ] Run two standalone VR users in the same Crib runtime.
- [ ] Confirm each user sees the other's head/controller/hand presence.
- [ ] Confirm locomotion replicates acceptably.
- [ ] Join with the PCVR recording user.
- [ ] Confirm PCVR sees both standalone users and their display names.

## PCVR and OBS

- [ ] Keep PCVR on the shared AFCore VR pawn family unless capture proves it insufficient.
- [ ] Inspect `BP_Pawn_VR_Camera` only if the shared pawn cannot support clean capture.
- [ ] Establish an OBS-friendly spectator/camera output.
- [ ] Confirm framing, stable output, and usable audio/video capture.

## Verbatim Marker

- [ ] Add one WMCYN-owned trigger for a structured Verbatim world marker.
- [ ] Include timestamp, world/location, runtime ID, user identity, presence mode/slot, capabilities, map, and note.
- [ ] Log to Unreal and provide simple on-screen/debug confirmation first.
- [ ] Defer backend storage; optional local JSONL can follow the first proof.

## Build Pipeline

- [ ] Recover substantial free disk space before cooking or packaging.
- [ ] Confirm Zen DDC no longer returns HTTP `507 Insufficient Storage`.
- [ ] Package the standalone Quest build.
- [ ] Sideload to both headsets.
- [ ] Package the Windows PCVR recording build.
- [ ] Run the repeatable three-device First Signal test.

## Current Next Steps

1. Free more disk space and verify DDC stability.
2. Prove Quest A <-> Quest B voice and replicated presence.
3. Add the PCVR recording user and verify OBS capture.
4. Connect login identity and nameplates.
5. Add the first Verbatim marker.

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
