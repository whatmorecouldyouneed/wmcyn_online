# Crib Furnishing: Reference-Inspired WMCYN Redesign

Date: 2026-10-17. Status: Layout A approved; gate 4 pilot placed and saved.
Live floor measurements and neutral asset previews are recorded in layout-approval.md.
The floor-support gate is resolved by a validated WMCYN-owned collision repair, and
the sectional plus coffee table are placed on the measured floor with convex
collision, clearance and door checks recorded in pilot-status.md. Headset
walkthrough and Quest performance measurement remain open. The remaining eleven
approved Layout A meshes are imported, materialed and collision-ready in
Furniture/Batch01 for manual placement; see batch01-assets.md.

## Intent and Boundaries

Furnish the main lounge, dining and bar as a substantial first batch, using purchased
Hidden Heights furniture in a new WMCYN arrangement. Static visual dressing only.
Do not recreate the public VRChat world's games or execute its scripts.
Preserve the interview/bathroom remodel, doors, water, lighting, floor repairs,
player calibration, AFCore, Mimic and all multiplayer/login behavior.

The interview room is a separate measured set, not the destination for the mansion
sectional. Its reference is Jared's living room: black three-seat couch, centered
WMCYN banner, green rug, dark cushions, colorful flower pillows and warm side lamp.
Measured: couch 210.82 cm overall, usable seating 182.88 cm, ceiling 274.32 cm,
banner 182.88 x 109.22 cm. Seat height/depth and physical tracking anchors remain
unmeasured. Do not infer these from a perspective photograph or claim 1:1 alignment.

## Evidence and Source Priority

1. Current Unreal geometry/collision and protected remodeled areas govern fit.
2. `C:/Users/jvred/WMCYN v2.0/Assets/Hidden Heights SGB` supplies local prefabs,
   material assignments and scene references. Main scene differs from the original;
   exact placement differences have not yet been classified.
3. Purchased `Hidden_Heights_VRS_v1.1.unitypackage` is the recovery/comparison baseline.
4. Booth gallery and `Docs/CRIB_SOURCE_RESTORATION_AUDIT.md` establish visual intent.
5. `C:/Users/jvred/iCloudDrive/WMCYN/LOGOS` supplies branded art; leave originals intact.

Audit tools: `Tools/Lighting/audit_furnishing_source.py` and
`Tools/Lighting/measure_furnishing_fbx.py`. Reports are under ignored
`Saved/Diagnostics/Lighting/`. Never commit the paid Unity project/package wholesale.

## First Batch Asset Schedule

Names below refer to `Hidden Heights SGB/prefabs/`. These are candidates, not an
approved placement manifest. Original scene counts are references, not target counts.

| Zone | Source prefab | Target count | Treatment |
| --- | --- | ---: | --- |
| Lounge | sectional Sofa | 1 | Main room only; verify 4.62 x 5.09 m footprint |
| Lounge | coffee table | 1 | Conversation center; preserve walk-around route |
| Lounge | accent chair | 2 | Face conversation group, not circulation aisle |
| Lounge | rug 1 | 1 | Scale after layout; no collision |
| Lounge | side table | 2 | Along seating ends |
| Lounge | floor lamp | 1 | Decorative; no new light in furniture batch |
| Lounge | house plant 4 | 2 | Corners outside door/tracking clearances |
| Dining | kitchen table | 1 | Verify tabletop height before scaling |
| Dining | dining chair | 4 | Match table height; preserve chair pull-out space |
| Dining | sideboard | 1 | Flush to approved wall with slight clearance |
| Dining | prop vase | 1 | Small centerpiece, no collision |
| Bar | bar stool 1 | 4 | Match existing counter height and access |
| Bar | prop coffee cup | 2 | No interaction or collision |
| Walls | WMCYN pattern and chrome wordmark images | 2 | Framed opaque art, preserve proportions |

The HQ banner is reserved for the interview-set spec. Five JPG/PNG logo assets
were visually inspected. Two HEIC files remain unreviewed and are excluded from
this batch until decoded and reviewed. The HQ raster is not the physical banner's
aspect ratio: compose on the measured canvas without stretching logo/QR content.

## Batch Editing Contract

Use WMCYN-owned furniture meshes/materials/textures and Outliner folders per zone.
No raw Unity behaviors, VRCChair scripts, shaders or collision are blindly imported.
Resolve prefab material overrides by GUID; ignore misleading FBX material names
and unused slots. Preserve texture UV0; verify normal-map convention and generate
non-overlapping lightmap UVs on new meshes. Reuse verified existing Unreal assets.

Placement manifest must contain batch ID, stable item ID, source prefab GUID/path,
Unreal asset path, zone, calibrated transform in cm/degrees, material overrides,
collision policy and expected bounds. Unresolved transforms remain null and apply
must reject them. No guessed Unity-to-Unreal coordinate conversion.

Dry-run reports creates/updates/conflicts without mutation. Apply only owns actors
with matching batch/item tags; stable labels alone are insufficient. Repeated apply
is idempotent. Manual edits since the last apply are conflicts, never overwritten.
Record pre-change transforms/materials and new actors for batch-only rollback.
Rollback does not delete reused assets or affect unrelated actors. Save only the
approved assets/current map; no SaveAll, automatic commit, push or deployment.

Major furniture gets simple collision. Rugs, wall art and small decor do not block
Pawn. Static meshes only: no simulation, ticks, seating, grabbing, video or net RPCs.
Use 2K maximum imported texture size initially; smaller decor should use 1K or less.
Measure actual Quest cost; these caps alone are not proof of optimization.

## Execution Gates

1. Resolve source prefab hierarchy and material dependencies; measure candidate FBXs.
2. Inspect existing Unreal equivalents and main lounge/dining/bar bounds. Calibrate
   source-to-world transforms with at least three non-collinear architecture anchors.
3. Present a top-down annotated proposal, exact item choices and measured bounds.
   User approves this layout before importing/applying to the active level.
4. Pilot sectional/table/material setup; screenshot scale and finish. Apply remaining
   approved items only after the pilot passes.
5. Compare fixed-camera before/after views, validate and hand off for VR walkthrough.

## Acceptance

- No missing materials, flipped normals, floating feet, duplicate actors or z-fighting.
- Primary paths retain at least 100 cm clearance, with actual pawn capsule sweeps.
- Door swing envelopes, stairs, spawn, menu and mirror space remain clear.
- Rerun produces no duplicates; dry-run detects intentional manual transform changes.
- Rollback restores only the selected batch.
- Record triangles, material slots, draw calls, texture memory and frame time before
  and after. Quest packaging/hardware performance and multiplayer regression remain
  explicit gates, not inferred from editor screenshots.
- No furniture implementation begins in the reserved interview room in this batch.

## Subsequent Specs

Pool terrace; secondary lounges/bedrooms; bathroom fixtures; measured interview set;
then interactive seating/video/props as separate systems. First batch completion
does not authorize those later features.
