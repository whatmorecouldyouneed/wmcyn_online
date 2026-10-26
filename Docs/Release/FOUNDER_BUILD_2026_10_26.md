# Founder build checkpoint - October 26, 2026

## Source checkpoint

Unreal main pushed through `dcc98d1`:
- `0feef5b`: isolated PCVR/Quest workflow, explicit ALPHA upload, signing gate.
- `7367113`: existing native identity, avatar, login, voice and camera source.
- `f08d612`: founder-approved existing AFCore/Mimic asset state, unchanged;
  read-only inventory in FRAMEWORK_INVENTORY_2026_10_26.json. This is not a
  comprehensive binary graph diff or new hardware acceptance.
- `43f83c8`: existing WMCYN login/menu/pawn assets and fonts.
- `5b61123`: existing Jared assets, PCVR cook inclusion and Quest exclusion.
- `7ce0a59`: existing furnished Crib/interview room, materials and collision assets.
- `dcc98d1`: matching Launcher editor modules (BuildId 55116800).

Backend main pushed through `1be0443`: signed profile identity (`66c78b1`) and
shared account-owned manifest resolution for user/server routes (`1be0443`).
Backend build and all 58 tests pass. No Firebase deployment performed this pass.
Unrelated backend local tools/docs remain untouched.

## Build and live state

First automatic artifact-only run:
https://github.com/whatmorecouldyouneed/wmcyn_online/actions/runs/40408026391

Runner online; Meta CLI installed and APK/OBB flags verified. Release key
validated without replacement. Existing old APK is debug-signed, not for ALPHA.
Developer Windows sign-in persistence does not imply Android persistence;
Quest secure store currently returns unsupported and requires normal login.

Read-only Firestore inspection confirms the canonical runtime has a fresh
heartbeat and protocolVersion=1. New source requires protocolVersion=2.
Do NOT distribute the new clients until matching server and additive backend
identity changes are deployed. VM/service were already running; not restarted.
Do not disable cost controls or replace the world during an active test.

## Pending acceptance

- Clean checkout cook/package/signature verification and any resulting fixes.
- Reproducible Linux build from same source; coordinate protocol-2 replacement.
- Backend API deployment from a reviewed clean source snapshot.
- Fresh/update installs, correct names, headset audio/input, remote peers.
- ALPHA upload, processing confirmation and founder channel access.

Native five-test rerun from October 23 passed; current packages remain unverified.
No new avatar fidelity, camera, seating, or room edits were made in this pass.
Remaining local historical docs/scripts/source art are not erased or represented
as a clean worktree. Review and preserve them separately from release contents.
