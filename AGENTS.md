# AGENTS.md

This repo is WMCYN Online, an Unreal Engine 5.8 project.

Use HumanCodeable AFCore / Advanced VR Framework as the baseline. The framework has already been tested and works. Do not rebuild framework systems unless AFCore cannot support the need.

Current goal: First Signal Build inside The WMCYN Crib.

Canonical First Signal target:

Bring The WMCYN Crib online with two standalone VR users and one PCVR recording user, all present in the same world, with basic presence, OBS-friendly capture from the PCVR machine, and one Verbatim world marker.

Prioritize:
- `L_WMCYNOnline` / `L_crib` integration
- two standalone VR user presences
- one PCVR recording user presence
- display names
- user presence and temporary capabilities
- avatar placeholders
- username/password login into the persistent WMCYN world
- OBS-friendly PCVR capture
- Verbatim world marker event

Do not prioritize:
- public matchmaking
- stores
- inventory
- mobile feed
- concerts
- multiple rooms
- creator tools
- economy
- full backend auth
- dedicated servers

Read `Docs/CURRENT_STATE.md` first. Read `Docs/HC_AVF_ASSET_AUDIT.md` before touching framework assets. Read `Docs/WMCYN_ONLINE_PRD.md` when scope or product behavior is unclear.

Do not feed full chat transcripts into project docs. Turn discoveries into current state, decisions, and tasks.
