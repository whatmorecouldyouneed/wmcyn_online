# Crib Source Restoration Audit

Date: 2026-10-14. Status: reference/source inspection complete; restoration not applied.

## Reference and Intent

Reviewed every distinct image in the [Hidden Heights Booth gallery](https://booth.pm/ja/items/3801299): 12 images, with the hero also repeated as a thumbnail. In gallery order:

1. Exterior corner: clear floor-to-ceiling glazing, wood soffits, stone columns, planted beds and low exterior lights.
2. Double-height living room: pale sectional, rug, bar, pale plank/tile floor, transparent upper railing.
3. Bedroom: wood ceiling, stone accent wall, warm bedside lights and dark door.
4. Upper billiards room: stone panels, pale floor, ring pendant, thin cool ceiling accent.
5. Passage: irregular stone cladding, wood ceiling/wall, artwork and blue-lit opening at the far end.
6. Upper dining landing: dark doors, pale floor, stone columns and restrained furniture.
7. Kitchen: white counters/cabinets, darker lower cabinetry, stone/wood accents and glazed wine storage.
8. Lower billiards/bar: pale floor, wood/stone walls, blue pool-facing windows and cool ceiling accents.
9. Pool-side curved facade: tiled blue pool edge, pale terrace, clear balustrade and furnished upper balcony.
10. Enclosed exterior recreation area: green surface, rocks, timber deck, spiral stair and inflatables.
11. Frontal glazed facade: warm wood soffit, chandelier, planted lower bed and step/wall lighting.
12. Unity package folder layout.

Preserve the architecture's intentional curves. Restore material separation before decorating or raising light intensity again. Our interview area remains right of the driveway-side entrance; these references do not change that decision.

The listing describes a Unity 2019.4.31f1 package with Unity-generated lightmap UVs and baked lighting. It also distinguishes included assets from public-demo extras. Do not assume VRChat/Udon behaviors or Unity shaders transfer with FBX. Do not copy the public demo's optional games into the MVP.

## Local Evidence

- Original package: `C:/Users/jvred/Downloads/Hidden_Heights_VRS_v1.1/Hidden_Heights_VRS_v1.1.unitypackage` (669,635,197 bytes).
- Script inventory: 709 entries, including 110 FBXs, 105 prefabs, 74 materials, 123 PNGs, 78 JPGs, four TIFs, 29 EXRs, three Unity scenes and two shaders. Counts include all package subfolders, not only the house.
- Main original model: `Assets/Hidden Heights SGB/fbx/Hidden_Heights.fbx`. A background Blender 4.4 import successfully found 107 mesh objects. This is a diagnostic import only, not an Unreal reimport or proof of identical geometry.
- Unreal import metadata points to `D:/3D Files/wmcyn_crib_default_unity_transfer.fbx`, an intermediate export, not the original package FBX.
- Live read-only Unreal audit found 107 static mesh components under the imported actor and 40 distinct imported Crib materials. Of those, 25 have a constant/vector base-color input; none has a roughness input connected. Some solid colors are intentional, so this is not a claim that all 25 are broken.
- `house_atlas_1_` and its duplicate have gray base color and no normal/metallic/roughness connection, although the original material resolves to all those maps. The emissive map did survive.
- `wood_floor`, `wood_floor_001`, `stone_wall`, `pool`, `pool_tiles` and other important finishes have flat base colors. Source texture files are available.
- Imported `glass` is opaque with no opacity input. Some level components already override it with AFCore glass; preserve those until a WMCYN replacement is verified, and never edit AFCore itself.
- Imported `water` and `water_001` are opaque blue with a static normal input, no explicit roughness or opacity. The source has custom QuestWater shaders with two scrolling normal layers, reflection cubemap and interior-color parameters, including transparent variants. An opaque variant also exists, so opacity alone is not the entire water defect.
- Current water users: `Plane_037` / `Scene_081`, `Plane_099` / `Scene_094`, `Plane_101` / `Scene_095`. Verify their separate surfaces before assigning one shared replacement.

## Exact Recovery Sources

Paths below are relative to `Assets/Hidden Heights SGB/` inside the package:

| Target | Verified source |
| --- | --- |
| Architectural atlas | `textures/house atlas 1/house_atlas1_Base_color.jpg`, `house_atlas1_Normal_OpenGL.jpg`, `house_atlas1_Roughness.png`, `house_atlas1_Metallic.png`, `house_atlas1_Emissive.png` |
| Stone wall | `textures/stone wall/TexturesCom_Brick_StoneCladding3_2K_albedo.jpg`, corresponding `_normal.jpg` and `_roughness.jpg` |
| Wood variant 1 | `textures/wood floor/woodfloor_seamless.jpg` and `woodfloor_seamless_normal.jpg` |
| Wood variant 2 | `textures/wood floor/base wood.jpg` and `base wood normal.jpg` |
| Pool shell/tiles | `textures/pool/pool_*` and `textures/pool/pool tiles/pool_tiles_*` maps |
| Water | `shaders/water/Water_N_A.png`, `Water_N_B.png`, `QuestWater.shader`, `QuestWaterTransparent.shader` and their material parameters |

The two wood variants must be matched to original Unity renderer assignments and existing UV0, not guessed from Unreal numeric suffixes. Inspect OpenGL normal conventions and convert only on WMCYN-owned texture copies. Use actual roughness maps rather than blindly interpreting Unity smoothness/gloss settings as Unreal roughness.

## Scripted Restoration Order

1. Create an explicit source-material-to-WMCYN-material manifest. Import only required maps into `/Game/WMCYN/Environment/Crib/Materials/SourceRestoration`; cap texture sizes deliberately for Quest after comparison. Do not auto-match ambiguous names.
2. Pilot the architectural atlas and one stone surface using level-component material overrides. Compare fixed-camera before/after views, UV placement and material compile results. Then batch only verified material slots. Keep originals intact and record rollback assignments.
3. Diagnose the distorted upper doors under neutral/unlit, wireframe and normal visualization. Match the exact component to original FBX geometry. If normals are wrong, repair a WMCYN mesh duplicate; if geometry differs, restore only the affected mesh with transform/UV/collision checks. The present audit does NOT establish the door deformation's cause.
4. Restore pool shell and tile maps before judging the water. Build a small WMCYN mobile-friendly water material reproducing the source's scrolling normals, tint and controlled reflections. Evaluate transparency cost and underside views on Quest. No fluid simulation, swimming system or blanket engine-renderer changes.
5. Re-bake static lighting after material changes affect bounced light; compare the three existing lighting viewpoints plus doors and pool. Then restore selected furniture and the measured interview set.

## Tools and Reproduction

```powershell
python Tools/Lighting/audit_unity_source.py "C:/Users/jvred/Downloads/Hidden_Heights_VRS_v1.1/Hidden_Heights_VRS_v1.1.unitypackage" --output Saved/Diagnostics/Lighting/unity_source_audit.json
python Tools/Lighting/extract_unity_reference.py Saved/Diagnostics/Lighting/unity_source_audit.json --output Saved/Diagnostics/Lighting/UnityReference --asset "Assets/Hidden Heights SGB/fbx/Hidden_Heights.fbx"
```

In the Unreal Cmd console:

```text
py "C:/Users/jvred/OneDrive/Documents/Unreal Projects/wmcyn_online/Tools/Lighting/audit_crib_materials.py"
```

The package audit resolves Unity GUID references without running package scripts. Extraction permits explicit paths only, rejects path traversal and refuses to overwrite differing files. Raw source copies and generated JSON stay under ignored `Saved/Diagnostics/Lighting`; no paid source package is added to Git.

Current pass changed inspection scripts and documentation only. No `.uasset`, `.umap`, configuration, collision, light, pawn, spawn, AFCore or Mimic changes. No deployment or device test. Preserve the completed lighting pass and its rollback baseline.
