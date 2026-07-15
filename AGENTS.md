# AGENTS.md

This repo is WMCYN Online, an Unreal Engine 5.8 project.

Use HumanCodeable AFCore / Advanced VR Framework for compatible reusable systems and do not edit its assets by default. The First Signal player-body source of truth is `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal`, a WMCYN child of Mimic Pro's native `BP_VRBodyCharacer`. Preserve its proven body, tracking hierarchy, seated calibration, locomotion, footsteps, and floor behavior. Adapt WMCYN login, identity, voice, nameplate, spawn, and replication around that pawn; do not transplant Mimic IK back into the AFCore pawn.

Current goal: First Signal Build inside The WMCYN Crib.

Canonical First Signal target:

Bring The WMCYN Crib online with three standalone Quest users and one PCVR recording user, all present in the same always-on world, with basic presence, voice, and OBS-friendly capture from the PCVR machine.

Prioritize:
- `L_WMCYNOnline` / `L_crib` integration
- three standalone Quest user presences
- one PCVR recording user presence
- display names
- user presence and temporary capabilities
- avatar placeholders
- username-or-email/password login into the persistent WMCYN world
- OBS-friendly PCVR capture
- existing Firebase identity/profile integration
- Firebase-authenticated bootstrap into one canonical internet-hosted Crib runtime
- always-on server registration, heartbeat, reconnect, and build compatibility

Do not prioritize:
- public matchmaking
- stores
- inventory
- mobile feed
- concerts
- multiple rooms
- creator tools
- economy
- full public account administration
- production-scale dedicated-server fleet orchestration
- Verbatim marker work before the primary First Signal gates pass

Read `Docs/CURRENT_STATE.md` first. Read `Docs/HC_AVF_ASSET_AUDIT.md` before touching framework assets. Read `Docs/WMCYN_ONLINE_PRD.md` when scope or product behavior is unclear.

Do not feed full chat transcripts into project docs. Turn discoveries into current state, decisions, and tasks.
