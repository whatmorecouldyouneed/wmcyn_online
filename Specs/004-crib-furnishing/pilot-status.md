# Lounge Pilot Status

2026-10-17. Floor-support gate cleared; two-piece pilot placed and saved in
`L_WMCYNOnline`. Headset walkthrough and Quest performance remain open.

## Floor-Support Gate: Resolved

The earlier block ("no floor hits between Z60 and Z-30") was a mesh collision
defect, not missing geometry. `Plane_049` rendered horizontal triangles while its
BodySetup produced no queryable surface, the same imported-Crib failure mode seen
during the 2026-07 walkable-floor repair.

`repair_lounge_floor.py` duplicated the source mesh to WMCYN-owned
`/Game/WMCYN/Environment/Crib/Meshes/SM_Crib_Walkable_Scene_086`, set
`CTF_USE_COMPLEX_AS_SIMPLE` plus double-sided geometry, and reassigned the
component. Geometry, texture UV0, material slots and the source asset are
unchanged. The script validates before it keeps the change and restores the
original mesh if any sample fails.

- 574 of 574 horizontal triangle centroids return a component hit.
- The 90-point candidate-A grid returns a floor at every position, measured
  Z 10.36-10.63 cm, and all 90 pass the capsule corridor test.
- Live re-verification: `Plane_049` is on the repaired mesh and traces at both
  pilot points hit at Z 10.529 and Z 10.487.

The projected approval band of "Z ~6" was a render estimate. The measured
collision surface is ~10.5 cm, and placement uses the measured value.

## Placed

Batch `WMCYNBatch_LoungePilot`, Outliner folder
`WMCYN_Environment/Furniture/LoungePilot`, both Static with convex collision.

| Item | Location cm | Measured floor Z | Floor gap | Tris | Slots |
| --- | --- | ---: | ---: | ---: | --- |
| FURN_Lounge_Sectional_01 | -810, -600, 10.998 | 10.529 | 0.000 cm | 23,836 | 1 |
| FURN_Lounge_CoffeeTable_01 | -720, -480, 10.487 | 10.487 | 0.000 cm | 228 | 1 |

Each Z is derived per item from a live floor trace minus the mesh's local
bounding-box minimum, so the lowest geometry rests exactly on the measured
surface. World bounds minimum Z equals the traced floor Z on both pieces: no
floating feet and no sinking.

## Acceptance Evidence

- Materials resolve; pale upholstery and marble top render correctly, no missing
  or default materials, normals correct. See `Lounge_Pilot_After_Seating.png`.
- The coffee table sits inside the sectional's open corner; no mesh interpenetration.
- Collision is functional and measured, not inferred: the 90-point grid went from
  90 clear before placement to 63 clear and 27 blocked, and the blocked cells
  match the L footprint plus the table.
- Circulation: a continuous clear ring remains around the group. The west strip
  (X -1200, -1100), south row (Y -900), east strip (X -400, -300) and north row
  (Y -100) are fully clear, each cell tested as a 100 x 100 cm free box, so
  primary routes keep at least 100 cm.
- Doors: every door within 400 cm in plan sits on another storey (Z -470.5 or
  Z +460.6). No lounge-floor door swing is affected.
- Player systems untouched: all `SPAWN_FirstSignal_*` markers, the default
  PlayerStart and `DEV_AvatarMirror_Tracking` are on the Z 557 floor.
- No duplicates: the batch folder contains exactly the two actors.

## Not Claimed

- No headset walkthrough. Editor screenshots are not VR acceptance.
- No draw-call, texture-memory or frame-time capture yet, and no Quest hardware
  measurement. Triangles and material slots above are counts, not a performance
  result.
- No lighting rebuild after placement; the pieces currently use existing bakes.
- Nothing committed, packaged or deployed.

## Rollback

Delete the two actors carrying tag `WMCYNBatch_LoungePilot`. That affects only
this batch and leaves the imported assets and the floor repair in place. The
floor repair rolls back separately by reassigning `Plane_049` to
`/Game/Environments/WMCYN_Crib/Scene_086`.

## Next

Headset walkthrough of the seating group, then the remaining approved Layout A
living/dining/bar items. Reports: `lounge_pilot_placement.json`,
`lounge_floor_repair.json`, `lounge_pilot_clearance.json`, `lounge_pilot_import.json`,
and the fixed before/after views in `Saved/Diagnostics/Lighting`.
