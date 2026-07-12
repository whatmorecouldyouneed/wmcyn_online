# Crib Collision Stabilization - 2026-07-09

## Purpose

Replace the temporary First Signal floor guard with real Crib collision where possible, without touching AFCore or adding new gameplay systems.

## What Was Found

- The headset path works with `BP_WMCYN_QuestUserPawn`; earlier tests needed `BP_WMCYN_VRPreviewStabilizer` to log `WMCYN FloorGuard: rescued Quest pawn below floor`.
- The old debug floor proved the pawn could be kept usable, but it was not the final Crib collision solution.
- Follow-up headset testing proved the pawn can now stand on Crib collision without the old debug floor, but the old stabilizer height made the pawn appear to hover above the visible floor.
- Several imported Crib meshes had this broken pattern:
  - no simple collision shapes
  - `collisionTraceFlag = CTF_UseSimpleAsComplex`
  - placed component set to block gameplay
- That means the component can look collision-enabled in the Details panel while still giving the pawn no usable simple floor collision.

## Assets Changed

- `/Game/Environments/WMCYN_Crib/Scene_096`
  - Used by `Plane_102`, the closest identified spawn-floor surface around `SPAWN_FirstSignal_StandaloneVR_A`.
  - Initially received one simple convex collision hull.
  - The convex hull created an invisible raised-floor feel, so it was removed.
  - BodySetup now uses `CTF_UseComplexAsSimple`.
  - Kept BodySetup collision enabled.

- `/Game/Environments/WMCYN_Crib/Scene_002`
  - Used by imported component `collider`.
  - Set BodySetup `collisionTraceFlag` to `CTF_UseComplexAsSimple`.
  - Enabled double-sided geometry.

- `/Game/Environments/WMCYN_Crib/Scene_058`
  - Used by `Plane_003`, previously selected as a candidate floor surface.
  - Set BodySetup `collisionTraceFlag` to `CTF_UseComplexAsSimple`.
  - Enabled double-sided geometry.

## What Was Not Changed

- No AFCore assets were intentionally edited.
- The temporary debug floor actor/folder is no longer present in the current level.
- `L_WMCYNOnline` was not saved during this pass.
- The temporary `WMCYN FloorGuard` height rescue was removed from `BP_WMCYN_VRPreviewStabilizer`.

## Current Best Hypothesis

The original fall-through was caused by imported Crib collision not being CharacterMovement-ready. The later hover was caused by the old WMCYN stabilizer still using the debug-floor root height (`Z 649.15`) after the real Crib floor started working.

Latest MCP PIE result: `BP_WMCYN_QuestUserPawn0` starts around `X 14.1, Y -700, Z 553.94`, reports `MOVE_Walking`, `floorBlocking=true`, `walkable=true`, and stands on imported Crib component `Plane_003` at an impact point around `Z 461.79`.

## Next Headset Test

1. Restart or reload the level if Unreal still has stale physics state.
2. Run VR Preview in `L_WMCYNOnline`.
3. Confirm the pawn starts near `SPAWN_FirstSignal_StandaloneVR_A`.
4. Confirm the pawn no longer falls through the Crib floor.
5. Confirm the pawn no longer visibly hovers above the floor.
6. Confirm Quest stick locomotion still feels stable.

Superseded old test plan:

- Keep the debug floor and floor guard active.
- Watch the Output Log for `WMCYN FloorGuard`.
- Remove the guard only after the pawn can spawn and move on Crib collision without rescue logs.

## Done Criteria

- `BP_WMCYN_QuestUserPawn` starts near `SPAWN_FirstSignal_StandaloneVR_A`.
- Quest stick locomotion works.
- The pawn remains on the Crib floor without falling through.
- The pawn does not visibly hover above the floor.
- Output Log does not show new `WMCYN FloorGuard` messages.
