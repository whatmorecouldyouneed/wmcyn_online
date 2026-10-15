# Crib Door 01: First Real Leaf Conversion

Date: 2026-10-14. Scope: the open left door in the user's reference view.

## Confirmed Findings

- User confirmed the orange pilot supports right-hand grip, push and pull in VR.
- The apparent warped texture on the adjacent closed door was primarily the
  imported `collider` component being rendered with AFCore `M_Glass`. Hiding the
  actual door mesh left the reflection artifact visible; hiding only the collision
  shell removed it. Material replacement is not needed for that artifact.
- The shell now has visibility off, HiddenInGame on, and CastShadow off. Its
  mesh, BlockAllDynamic profile, and QueryAndPhysics collision remain unchanged.
- Source-normal and weighted-normal experiments did not resolve the artifact.
  Their diagnostic mesh is not used by the level. Do not run
  `repair_crib_door_normals.py` as the production fix.
- Both visible doors, frames and additional doors are combined in
  `house parts low poly_009` / `/Game/Environments/WMCYN_Crib/Scene_036`.

## Implemented

- `DOOR_Crib_01_Grip` uses WMCYN Blueprint `BP_WMCYN_CribDoor_01` in
  `/Game/WMCYN/Environment/Crib/Doors`, adapted from the verified grip pilot.
- Separated 184 triangles for the open door leaf, window insert and both handles
  into `SM_Crib_OpenDoor01_Leaf`. Preserved source UV0, normals and atlas material.
- The remaining 826 triangles become `SM_Crib_Doors_Without_OpenDoor01`, placed
  as `ARCH_Crib_Doors_Remainder_01`. All 1,010 source triangles are accounted for.
- Original combined component is hidden, not deleted; its collision is unchanged.
  Original imported assets and AFCore assets are not modified.
- Actor folder: `WMCYN_Environment/InteractiveDoors`. Hinge is approximately
  (-747.5, 85, 460.6) cm. Initial pose matches the original open leaf; AFCore
  Comp_Drag AngleYaw runs from -100 (closed) to 0 (original open).
- Right-hand grip anchor is (23.5, -87, 110.4) cm relative to the extracted leaf.
  The same 35 cm acquisition / 120 cm release limits apply as the pilot.
- This remains a standalone-only mechanical test. Moving leaf has no blocking
  collision until obstruction/player safety is validated. It is not a shipped
  replicated door or a closed-door access barrier.
- The neighboring right door now displays its original finish but is still static.
  Its shared-shell collision must be separated safely before converting it.

## Validation and Next Test

- Open and closed editor screenshots confirm visual fit within the retained frame.
- Production lighting rebuild succeeded and the map/build data were saved. The
  stale static door shadow is removed. Remainder uses regenerated lightmap UV1;
  original texture UV0 is unchanged. Before/after screenshots are in
  `Saved/Diagnostics/Lighting/Crib_Doors_Before_Conversion.png` and
  `Crib_Doors_After_Conversion.png`.
- PIE synthetic AFCore hand trajectory: 0 -> -99.994 -> -0.006 degrees, PASS.
  This verifies the extracted leaf, not physical controller input on this doorway.
- Run VR Preview, log in, approach the original open left door, hold right side
  grip near the handle, and push it closed / pull it open. Check handle alignment,
  reach, swing direction and passage. Report the hardware result before rollout.
- Next: owned-player/server latch routing, obstruction-safe collision, then the
  adjacent closed leaf. Do not mass-convert or remove the Crib collision shell.

## Reproduction and Rollback

Scripts in `Tools/Lighting`: `inspect_view_door_surfaces.py`,
`hide_crib_collision_visuals.py`, `export_crib_door_geometry.py`,
`analyze_crib_door_islands.py` (external Python), `convert_first_crib_door.py`.
Apply `crib_door_01.dsl` to the new Blueprint through BlueprintTools.write_graph_dsl.
`test_crib_door_01.py` runs only in PIE. Geometry selection is source-specific and
asserts the expected triangle counts; it is not a generic bulk door converter.

Rollback records in `Saved/Diagnostics/Lighting`:
`collision_visual_rollback.json`, `door_01_conversion_rollback.json`.
To undo the conversion, hide the two new actors and restore visibility/shadow
settings on the original combined component. Keep the proven floor collision.

The unused `SM_Crib_Doors_SourceNormals` is retained as a diagnostic artifact only.
