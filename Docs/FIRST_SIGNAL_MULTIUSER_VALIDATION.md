# First Signal Multi-User Validation

## Purpose

Prove three standalone Quest users and one PCVR recording user can enter the same canonical WMCYN Crib world over the public internet with visible identity, replicated presence, voice, and OBS-friendly capture.

## Desktop Proof And Four-Slot Regression

The earlier three-client listen-server PIE in `L_WMCYNOnline` proved the underlying identity, nameplate, pose, and voice-ownership paths. The active implementation now defines four slots:

- Slot `0`: `StandaloneVR_A`, Quest mode, marker capability, location approximately `(14, -700, 554)`.
- Slot `1`: `StandaloneVR_B`, Quest mode, marker capability, location approximately `(14, -520, 555)`.
- Slot `2`: `StandaloneVR_C`, Quest mode, location approximately `(14, -340, 557)`.
- Slot `3`: `PCVR_Recording`, PCVR mode, Recording capability, location approximately `(14, -1000, 557)`.
- A four-client PIE regression must confirm four production `BP_WMCYN_UserPawn_FirstSignal` instances bind to distinct PlayerStates and markers.
- Each local pawn must hide its own inherited AFCore NameTag and render three remote NameTags.
- Each pawn's VOIPTalker binds to its own PlayerState; only the locally controlled pawn starts local capture.
- Each PIE world reports one local capture-active path without new invalid PlayerState/session warnings.

The automated extra PIE windows used AFCore's default `AP-1743` name because their login forms were not manually completed. Distinct identity must be verified in the physical run.

## Remote Four-Device Run

Use four distinct test identities from separate physical locations:

- Quest A: `Jared-Quest-A`
- Quest B: `Noah-Quest-B`
- Quest C: `Quest-C`
- PCVR/OBS: `WMCYN-Camera`

1. Confirm the canonical server registered `worldRuntimes/wmcyn-online`, has a current heartbeat lease, and is serving `L_WMCYNOnline` through an internet-reachable endpoint.
2. Launch the packaged Quest build on Quest A, Quest B, and Quest C in their remote locations.
3. Launch the Windows PCVR recording build on the OBS machine.
4. On each device, enter username or email plus password and select `Enter World`. There is no access code or session picker.
5. Confirm Quest A/B/C occupy slots A/B/C and the PCVR user occupies the recording start.
6. From every device, confirm the other three users' body/head/controller presence and display names are visible.
7. Move all three Quest users independently and confirm motion remains smooth and distinct on the PCVR view.
8. Speak from each Quest in turn. Confirm the other two Quests and PCVR user hear each speaker.
9. In OBS, capture the PCVR spectator/game output and the intended in-game reference mix. Keep external microphones running as the clean-recording backup.
10. Record a 30-second Crib shot with the PCVR view stable and all intended names readable.
11. Disconnect and reconnect one Quest; confirm automatic bootstrap restores verified identity without exposing a host/session UI.

## Pass Evidence

- Quest A/B/C mutually see and hear one another: `[ ]`
- PCVR sees all three names and moving VR presences: `[ ]`
- PCVR hears all three in-game voice paths: `[ ]`
- OBS records a stable, correctly framed video: `[ ]`
- OBS records usable reference audio; external mic backup is clean: `[ ]`
- Reconnect returns the verified user to the canonical world: `[ ]`
- No user-facing host/session/access-code flow appears: `[ ]`

## Current Blocker

Local PIE proves the game-side replication model, but packaged devices still need the runtime registration, heartbeat, ticket validation, and public deployment defined in `Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md`. Do not mark the remote gate complete until all four devices enter the canonical runtime from separate locations and the evidence above is recorded.
