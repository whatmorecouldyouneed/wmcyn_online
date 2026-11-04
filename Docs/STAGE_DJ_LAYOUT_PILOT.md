# Stage and DJ Layout Pilot

## Complete Assembly Follow-up - 2026-11-04

The founder requested the omitted stage structure. All 89 source mesh objects
(24,908 source polygons) are now exported intact as StageComplete.fbx and
imported as SM_StageComplete. STAGE_Pilot_CompleteAssembly uses uniform scale
0.257879: 500 cm wide, approximately 344 cm deep. This preserves relative
source geometry, not the original 19.39 m world dimensions. Aleph remains full-size.

Original trusses, rails, light housings, deck and support frame are present.
Six source material slots map to WMCYN preview materials; Unity shader parity,
emission and functional stage lighting are not claimed. No scene/prefab reference
to the source FBX GUID was found, so original Unity placement is unverified.

The old deck is now hidden visually and resized to match, retaining its simple
collision at Z32. The complete assembly is visual-only: rails/posts do not yet
block players. The step was moved to meet the enlarged front edge. Nine support
samples pass across the expanded floor. Rendered editor screenshot reviewed;
headset traversal and rail/post collision remain pending.

Use prepare_complete_stage.py in Blender, then restore_complete_stage.py in
Unreal. Do not rerun the older pilot placement/validation scripts: they describe
the superseded slab-only layout below. No source assets or frameworks modified.

## Scope and Status - 2026-11-04

Founder approved layout first in the grass alcove by the spiral staircase,
facing the pool. Camera grip is founder-confirmed; seating is deferred.
The local L_WMCYNOnline layout is implemented and saved. No playback,
audio routing, replicated media, interactive mixer, or production rig is added.

## Reuse

- Source: `C:/Users/jvred/WMCYN v2.0/Assets/stage.fbx`.
  Extracted its Cube.001 deck, centered it, and resized only that slab to
  400 x 300 x 24 cm. The full 19.39 m stage/truss assembly does not fit this pilot.
- Source: `Assets/aleph 9/VDJ_R2100/Model/Booth_StandardSet.fbx`.
  Kept equipment at source scale, approximately 1.9 m wide and 1.12 m tall.
  Used the standard booth instead of the 6.28 m full set.
- Source color/normal/AO textures imported. Unity material GUIDs point to
  PSD color originals; adjacent supplied PNG versions are used for this preview.
  Original Unity scripts/probes are not imported. Glass is deliberately opaque
  for this static pilot; emission/display shader fidelity remains a polish item.
- No explicit redistribution license was found locally. Local reuse was approved;
  confirm distribution rights before shipping these source-derived assets.

## Layout

Assets: `/Game/WMCYN/Environment/Crib/StagePilot/`.
Outliner folder: `WMCYN_Production/StagePilot`.

| Actor | Position cm | Purpose |
|---|---|---|
| STAGE_Pilot_Deck | -1850, -2050, 8 | Deck top Z32; faces +X/pool |
| STAGE_Pilot_FrontStep | -1660, -2050, 8 | 140 x 80 cm; top Z20 |
| DJ_Pilot_AlephStandard | -1890, -2050, 32 | Controls face rear/operator; yaw270 |
| LGT_StagePilot_Preview | -1770, -2050, 320 | Temporary 350-lumen non-shadowing light |

All four actors carry WMCYN_StagePilot. Remove only these actors to roll back
the placement. Imported source files and existing architecture were not modified.
AFCore, Mimic, login, voice, camera and player code/assets remain untouched.

## Repeatable Scripts

1. Blender background: `Tools/Lighting/prepare_stage_preview.py` extracts FBX files
   into `SourceArt/StagePilot` and writes provenance.
2. Unreal outside PIE: `Tools/Lighting/audit_stage_alcove.py` records geometry.
3. Unreal: `Tools/Lighting/import_stage_preview.py` imports missing assets,
   assigns WMCYN materials, and creates the three mesh actors without duplicates.
4. Unreal: `Tools/Lighting/validate_stage_preview.py` sets the final booth pose,
   adds the temporary light, checks support and saves the current level.

Reports: `Saved/Diagnostics/Lighting/stage_*.json`.
Do not rerun as a general cleanup tool: the placement scripts own these pilot
actors, and validation deliberately resets the booth to the approved pilot pose.

## Evidence and Next Gate

- Editor, 2026-11-04: imported two meshes, eight materials, five textures.
- Editor: 26/26 downward simple-collision samples hit the deck/step at expected
  heights with upward normals. Stair bounds separated by 372.9 cm along Y.
- Editor: screenshots inspected before and after orientation/light corrections;
  deck is horizontal, no visible rock/stair overlap, booth rear-facing controls.
- Original lawn did not respond to component collision traces in the survey.
  Added collision only to the pilot meshes, not the original lawn. The complete
  approach route must be tested in headset before declaring safe stage access.
- Headset, packaged Quest, packaged PCVR and remote multiplayer: NOT tested.

Next: approve scale in headset, walk the approach/step/deck and behind the booth.
Then scope one approved prepared track with synchronized playback and operator
controls. Static imported DJ hardware does not itself provide any audio transport.
