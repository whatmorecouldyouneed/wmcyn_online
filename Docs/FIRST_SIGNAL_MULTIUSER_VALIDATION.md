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

## LAN Validation Phase (Interim Listen Host)

Before the remote run, prove the complete chain on one LAN with the PCVR recording PC as the registered listen host. The product model (one canonical runtime discovered through Firebase) is exercised end to end; only the host's network location is temporary.

1. Ensure the deployed backend has `WMCYN_ALLOW_PRIVATE_HOST=true` (LAN phase only).
2. On the PCVR PC, set `WMCYN_RUNTIME_SERVER_KEY` and `WMCYN_JOIN_TICKET_SECRET` (values from the Functions deploy env), then launch the packaged Win64 build:

```
wmcyn_online.exe /Game/Levels/L_WMCYNOnline?listen -WMCYNRegisterRuntime -WMCYNPublicHost=<PC LAN IP> -WMCYNPublicPort=7777
```

3. Confirm the log shows runtime registration and `runtime reported online`, and that `worldRuntimes/wmcyn-online` carries the LAN host with a current lease.
4. The host user logs in normally at the 3D gate (they are the PCVR_Recording participant); the registered host does not travel.
5. Each Quest launches the sideloaded APK, logs in, and should automatically travel into the host's world with a join ticket; the traveled pawn auto-enters with the verified identity and no second login gate.
6. Verify names, replicated body/head/hand presence, and two-way voice across all four users.
7. Known interim limitation: slots are assigned by join order, not by the ticket reservation; marker placement may not match slot names until ticket-slot assignment is enforced.
8. After the LAN pass: set `WMCYN_ALLOW_PRIVATE_HOST=false`, redeploy, forward UDP 7777 on the host's router, and relaunch with `-WMCYNPublicHost=<public IP or DNS>` for the remote run.

## Current Blocker

Local PIE proves the game-side replication model. The backend contract (registration, heartbeat, tickets) is deployed live; the interim PCVR listen host stands in for the future dedicated runtime. Do not mark the remote gate complete until all four devices enter the canonical runtime from separate locations and the evidence above is recorded with `WMCYN_ALLOW_PRIVATE_HOST` disabled.
