# Interview Flower Pillows

Implemented 2026-10-19 as decorative photo-reference recreations, not downloaded
or official licensed manufacturer meshes. No third-party model was incorporated.

- Large: 22 inches / 55.88 cm petal-to-petal; estimated depth 13.7 cm.
- Small: 12 inches / 30.48 cm; estimated depth 7.47 cm.
- Large black/turquoise face centered on the interview couch; smaller yellow/red
  face forward-left. Existing checked end pillows and source sectional preserved.
- Assets: `/Game/WMCYN/Environment/Crib/Furniture/InterviewFlowers/`.
- Actors: `FURN_Interview_Flower_Large` and `FURN_Interview_Flower_Small`, under
  `WMCYN_Environment/Furniture/InterviewSet` in `L_WMCYNOnline`.
- One shared 1024px color/fabric atlas and matte material; one material slot per
  mesh. Each base mesh has 9,152 triangles. Generated 50% and 25% LODs use
  screen thresholds 0.18 and 0.08. No simulated fur, rig, grab, or physics.
- Large transform: location (-1200,-481,543), pitch -8, yaw 0, roll -3.
- Small transform: location (-1178,-451,530), pitch -8, yaw 0, roll 8.
- Collision disabled on these decorative actors/components; simple mesh collision removed.

## Reproduction

Run `Tools/Models/build_interview_flower_pillows.py` with Blender in background
mode. FBX files, shared PNG and dimension/triangle report are generated under
`SourceArt/InterviewFlowers`. The source script uses metres with FBX unit metadata.

`Tools/Models/reimport_interview_flowers.py` runs inside Unreal Python and replaces
only these two mesh imports, preserving the shared WMCYN material. It does not
change actor placement or save the level. Reapply generated LODs after reimport.

## Verification

Unreal mesh bounds match measured diameters. Front and oblique editor captures
checked placement, face direction, material and cushion contact. Reimport completed
without reported validation errors. These are first-pass approximations of the
physical plushes; no headset performance or packaged-device acceptance claimed.
No pawn, AFCore, Mimic, camera, room shell, or existing furniture edits in this pass.
