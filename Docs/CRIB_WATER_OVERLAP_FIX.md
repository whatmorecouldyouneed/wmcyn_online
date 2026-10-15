# Crib Water Overlap Fix

## Diagnosis

The imported actor contains two upper-pool surfaces with the same XY bounds:

- `Plane_037` / `Scene_081`: 290 triangles; world Z approximately -13 to -6 cm.
- `Plane_099` / `Scene_094`, section/material slot 1: 637 triangles; Z -5.5 to
  +2.7 cm, with upward, downward, and zero vertex normals.

The latter draws over the first and produced large irregular triangular patches.
This is overlapping surface geometry, not evidence of coplanar Z-fighting alone.

## Applied Repair

`Tools/Lighting/repair_crib_water_overlap.py` applies a fully masked WMCYN-owned
material to **Plane_099 slot 1 only**. This is a reversible section-visibility
repair, not deletion or mesh cleanup. The original imported mesh stays intact.

- `Plane_037` remains the visible upper pool.
- `Plane_099` slot 0 retains the lower pool.
- `Plane_101` retains the waterfall geometry.
- No transforms, collision, walkable floor, pawn, or framework assets change.
- Original triangles remain stored and may still incur vertex processing cost.
  A geometry-stripped WMCYN duplicate is an optional later optimization.

The material is
`/Game/WMCYN/Environment/Crib/Materials/SourceRestoration/M_Crib_Water_DuplicateSection_Hidden`.

## Verification and Rollback

Terrace and overhead editor screenshots show the large triangular patches gone.
The terrace screenshot is `Saved/Diagnostics/Lighting/Water_Overlap_Fixed.png`.
Hardware/mobile performance has not been tested for this change.

The script validates the expected source mesh and section count, preserves lower
pool material and component collision state, and writes rollback/result records
to `Saved/Diagnostics/Lighting/water_overlap_*.json` before saving the level.
To undo this repair, restore `Plane_099` material slot 1 to
`/Game/WMCYN/Environment/Crib/Materials/SourceRestoration/M_Crib_Water_Mobile`
and rebuild static lighting. Do not hide the entire component: it owns lower water too.
