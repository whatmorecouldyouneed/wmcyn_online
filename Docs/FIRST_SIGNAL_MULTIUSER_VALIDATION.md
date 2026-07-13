# First Signal Multi-User Validation

## Purpose

Prove two standalone VR users and one PCVR recording user can enter the same persistent WMCYN Crib world with visible identity, basic replicated presence, voice, and OBS-friendly capture.

## Desktop Proof Completed

Three-client listen-server PIE in `L_WMCYNOnline` passes the runtime topology check:

- Slot `0`: `StandaloneVR_A`, Quest mode, marker capability, location approximately `(14, -700, 554)`.
- Slot `1`: `StandaloneVR_B`, Quest mode, marker capability, location approximately `(14, -520, 555)`.
- Slot `2`: `PCVR_Recording`, PCVR mode, Recording plus marker capabilities, location approximately `(14, -1000, 555)`.
- Each world has three replicated `BP_WMCYN_QuestUserPawn` instances tied to the correct PlayerState.
- Each local pawn hides its own inherited AFCore NameTag and renders both remote NameTags.
- Each pawn's VOIPTalker binds to its own PlayerState; only the locally controlled pawn starts local capture.
- Each PIE world reports one local capture-active path without new invalid PlayerState/session warnings.

The automated extra PIE windows used AFCore's default `AP-1743` name because their login forms were not manually completed. Distinct identity must be verified in the physical run.

## Physical Three-Device Run

Use three distinct test identities:

- Quest A: `Jared-Quest-A`
- Quest B: `Noah-Quest-B`
- PCVR/OBS: `WMCYN-Camera`

1. Start the shared WMCYN world runtime and confirm `L_WMCYNOnline` is active.
2. Launch the packaged Quest build on Quest A and Quest B.
3. Launch the Windows PCVR recording build on the OBS machine.
4. On each device, enter username/password and select `Enter World`. There is no access code or session picker.
5. Confirm Quest A and Quest B occupy separate Crib floor starts and the PCVR user occupies the recording start.
6. From every device, confirm both other users' head/controller presence and display names are visible.
7. Move both Quest users independently and confirm motion remains smooth and distinct on the PCVR view.
8. Speak from Quest A, then Quest B. Confirm the other Quest and PCVR user hear each speaker.
9. In OBS, capture the PCVR spectator/game output and the intended in-game reference mix. Keep external microphones running as the clean-recording backup.
10. Record a 30-second two-person Crib shot with the PCVR view stable and both names readable.

## Pass Evidence

- Quest A sees/hears Quest B: `[ ]`
- Quest B sees/hears Quest A: `[ ]`
- PCVR sees both names and moving VR presence: `[ ]`
- PCVR hears both in-game voice paths: `[ ]`
- OBS records a stable, correctly framed video: `[ ]`
- OBS records usable reference audio; external mic backup is clean: `[ ]`
- No user-facing host/session/access-code flow appears: `[ ]`

## Current Blocker

Local PIE proves the game-side replication model, but packaged devices still need a selected persistent-world network connection path beyond local `OnlineSubsystemNull` validation. Do not mark the hardware gate complete until all three devices enter the same runtime and the evidence above is recorded.
