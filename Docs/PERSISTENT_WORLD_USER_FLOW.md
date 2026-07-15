# Persistent World User Flow

Status: First Signal product model  
Last updated: 2026-07-15

## Core Model

The WMCYN Crib is the world. First Signal enters that world through username-or-email/password login.

The intended first-run flow is:

1. Build WMCYN Online in Unreal Engine 5.8.
2. Package the standalone headset build.
3. Sideload the build to Quest hardware.
4. User opens WMCYN Online on the headset or PCVR machine.
5. User sees a simple login screen.
6. User enters username or email and password.
7. User selects Enter World.
8. Login verifies identity, requests the current Firebase bootstrap, and automatically connects to the one canonical Crib runtime.
9. Server validates the join ticket and assigns a server-authoritative presence slot.
10. User appears in `L_WMCYNOnline` / The WMCYN Crib.
11. Username/display name appears above or near the user's avatar.
12. Three Quest users and one PCVR recording user enter the same always-on Crib world from their separate locations.
13. Quest users move, speak, and see each other's basic body/head/hand/controller presence.
14. PCVR recording user sees the three Quest users and captures the world through OBS.
15. On disconnect, the client automatically re-bootstraps and reconnects to the same logical world.

## Login Scope

First Signal login is intentionally small:

- username
- password
- Enter World

Do not add extra entry codes, world pickers, role selectors, or alternate room flows for First Signal.

## Identity Stored After Login

The first identity layer now lives in `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal`, a WMCYN-owned child of AFCore `BP_PlayerState_Main`.

The first identity layer should store:

- `DisplayName` / username
- `PresenceMode`: `Quest` or `PCVR`
- `Capabilities`: First Signal requires `Recording` where appropriate; additional capabilities can be added later.

The data should remain WMCYN-owned and replicated. Do not edit AFCore PlayerState directly.

## First Signal Acceptance Path

- Quest user A logs in and appears in the Crib.
- Quest user B logs in and appears in the Crib.
- Quest user C logs in and appears in the Crib.
- All three Quest users can move with analog-stick locomotion.
- All three Quest users see each other's basic VR presence and name.
- PCVR recording user logs in, appears in the same world, sees all three Quest users, and can frame OBS capture.
- All four clients connect through the canonical internet runtime; LAN and player-hosted sessions are not acceptance paths.
- Quest build can be packaged, sideloaded, launched, and retested consistently.

## Stretch Features

- Verbatim world markers are deferred until shared-world presence, voice, OBS capture, and the initial camera path are proven.
