# Crib Basic Lighting Pass

Status: basic lighting implemented and production-quality Lightmass bake
completed on 2026-10-14. Editor visual checks and save/reload validation pass.
Mobile preview, packaged Quest/PCVR, moving-avatar illumination, and headset
frame-time acceptance remain open. Basic visibility precedes interview-set
placement and decoration; this is not final art lighting.

## Implemented

- `Tools/Lighting/run_crib_lighting.py` is the one-command editor workflow:
  geometry survey -> WMCYN lightmap repair -> fixture lights -> bake -> validation.
  See `Tools/Lighting/README.md` for the command. No individual fixture placement
  is necessary, and reruns reuse stable actor labels.
- 73 recessed fixtures were identified from their actual disconnected mesh
  islands across five imported components. Each now has a downward Static
  spotlight six centimeters below its lower face: 2,200 lm, 4,000 K, 850 cm
  attenuation, 35/75-degree inner/outer cone. Ten pendant faces use 650 lm,
  3,500 K, 450 cm attenuation, and 30/65-degree cones.
- All 83 lights live in `WMCYN_02_Crib_Lighting/Ceiling`. Labels are
  `LGT_Crib_<Upper|Main|Lower>_Ceiling_NN` and `LGT_Crib_Pendant_NN`.
- Added `LMIV_Crib_Occupied` and three sphere reflection captures
  (`RC_Crib_Upper`, `RC_Crib_Main`, `RC_Crib_Lower`) in `BakeSupport`.
  No extra movable lights, SkyLight, Lumen, or renderer-config changes.
  Existing imported and standalone point lights remain unchanged.
- The pilot exposed black floor layers caused by overlapping lightmap UVs.
  Increased resolution alone did not fix them. WMCYN-owned duplicates of
  `Scene_059`, `060`, `085`, `090`, `091`, and `097` now use box-projected source
  UV1 and regenerated lightmap UV2 at resolution 512. Their components are
  `Plane_004`, `005`, `048`, `084`, `086`, and `124`. Original assets remain
  untouched; no geometry or collision reconstruction was performed.
- `Plane_003` and `Plane_004` also have per-instance 512 lightmap resolution.
  The verified walkable `Scene_058` source mesh remains unchanged.
- Validation compares oriented triangle geometry, material UV0, material slots,
  collision complexity, and simple/convex collision counts against each source.
  All six pass, including after map reload. All 83 lights are unique, Static,
  and downward-facing. The map and its BuiltData are saved together.

## Validation and Remaining Work

- Production bake succeeded; zero unbuilt light interactions and zero unbuilt
  primitive interactions. Map Check reports zero errors/warnings. Lightmass UV
  warnings are a separate report and are **not** all resolved.
- The six repaired meshes no longer report overlaps. Eighteen other imported
  meshes still do: `Scene_004`, `005`, `006`, `011`, `021`, `039`, `040`, `051`,
  `062`, `064`, `065`, `070`, `074`, `076`, `084`, `086`, `092`, and `099`.
  These include glass, exterior/pool/scenery and other interior pieces. Address
  visible residual artifacts in a scoped follow-up; do not blanket-reimport or
  rebuild Crib collision.
- The bake reported approximately 8.1 MB lightmaps and 5.3 MB shadowmaps in the
  editor. These are not measured Quest package or GPU-memory figures.
- The installed engine emits a D3D11 `FDebugViewModePS` compilation warning for
  `/Engine/BufferVisualization/CustomDepthWorldUnits` during screenshot/build
  work. Normal Lit rendering and the bake complete; the engine debug-view shader
  issue is not claimed fixed and no engine assets were modified.
- Upper room, main bar/stairs, and lower bar were visually inspected in Lit mode.
  Floors and walls are now readable. The existing mostly neutral/white imported
  materials are preserved; this pass does not restore missing decorative textures.
- A short in-viewport PIE startup reached the existing 3D login gate and native
  pawn indexed-slot-0 synchronization, then stopped cleanly. No headset was
  available: OpenXR system lookup and Oculus voice capture initialization failed,
  so this is not a VR, voice, login-submission, or locomotion regression pass.
- Next gate: walk the same route in PCVR and a fresh packaged Quest client,
  checking moving avatars, stairs, light leaks, dark corners, and frame time.
  Mobile preview and device acceptance have not been performed in this pass.
- No server deployment or new APK was produced. Existing installed clients will
  not receive these map/lightmap changes until repackaged and redistributed.

Editor captures (not headset captures):

![Main bar after bake](Images/CribLighting/Main_After.png)
![Upper room after bake](Images/CribLighting/Upper_After.png)
![Lower bar after bake](Images/CribLighting/Lower_After.png)

## Original Survey

- Open map: `/Game/Levels/L_WMCYNOnline`; PIE stopped.
- Four standalone PointLight actors are each Stationary, approximately 100.53
  lumens, with 1,000 cm attenuation and shadows enabled. This is sparse coverage
  for the Crib; an intensity increase alone will not establish room coverage.
- The imported Crib actor also contains a Static point-light component named
  `Light`, at 5,000,000 unitless intensity with 16,384 cm attenuation. Review its
  contribution during the test bake rather than stacking new lights blindly.
- No standalone SkyLight, DirectionalLight, reflection capture, or Lightmass
  Importance Volume appears in the current actor inventory. Nested components
  outside the inspected Crib actor have not been exhaustively audited.
- Ceiling downlights are visible in Lit view while the surfaces below remain
  very dark. Fixture geometry is not equivalent to a working light source.
- Imported components `Overhead bar lights` and `overhead bar lights 2` reference
  `Scene_053` and `Scene_054`. Their shared `lights` material has a Base Color
  connection and no Emissive Color connection. These grouped meshes are not a
  verified inventory of every individual ceiling fixture; placements still need
  a room-by-room visual survey.
- Live `r.AllowStaticLighting = 1`, `r.DynamicGlobalIlluminationMethod = 0`, and
  World Settings `bForceNoPrecomputedLighting = false` permit a baked approach.
- Live `r.MobileHDR = 0`. Config disables movable spotlights and their shadows;
  auto exposure is also disabled. Preserve these settings for the first proof.
- Sampled fixture meshes and walkable-floor mesh `Scene_058` are Static with
  lightmap coordinate index 2 and resolution 64. UV presence is not proof of
  non-overlapping UVs or adequate lightmap density at the placed scale.

## Original Implementation Plan

1. Survey the existing ceiling fixtures by room and record their world positions.
   Start with the driveway entry and adjacent interview-area route, then halls,
   stairs, remaining interiors, and covered exterior transitions. Use the actual
   fixture layout; do not place a generic grid through walls or floors.
2. Prove one small cluster first. Put WMCYN-owned Static spotlights just below
   recessed fixture faces, aimed down into the room. Use Static point lights for
   fittings that should spread light in all directions. Tune radius and output
   to the actual room dimensions. Start around neutral-warm 3,500-4,000 K.
3. Inspect lightmap UVs, density, normals, and wall thickness on the test area's
   receiving meshes. Correct only demonstrated baking problems. Prefer instance
   lightmap-resolution overrides; mesh UV changes need a separately scoped fix.
4. Bound the occupied house with a Lightmass Importance Volume, excluding the
   distant scenery. Run a Preview-quality bake and inspect errors, leaks, and
   avatar illumination. Add suitable reflection captures and modest ambient/daylight
   support if needed after evaluating the existing imported light.
5. Compare Lit editor, mobile preview, PCVR, and a packaged standalone Quest.
   Check moving avatars as well as walls: baked surfaces alone do not prove good
   player lighting. Do not enable Mobile HDR or Lumen as an incidental fix.
6. Repeat the proven fixture pattern across the Crib, then bake and validate the
   full occupied route. Add any fixture glow through WMCYN material overrides;
   glow is visual treatment, not the room's primary illumination.

Suggested Outliner root: `WMCYN_02_Crib_Lighting`, with `Ceiling`, `Ambient`, and
`BakeSupport` subfolders. Labels: `LGT_Crib_<Room>_Ceiling_01`,
`LGT_Crib_<Room>_Fill_01`, `LMIV_Crib_Occupied`, `RC_Crib_<Room>_01`.

## Full Acceptance Criteria (Pending Device Checks)

- Floors, doorways, stairs, walls, and other players are readable throughout the
  occupied Crib in Lit mode and in the packaged Quest build.
- Light appears to come from the existing fittings without broad light leaks,
  major black patches, or blown-out surfaces.
- Player tracking, collision, scale, login, and locomotion remain unchanged.
- Compare headset frame time against the current baseline in the same views;
  desktop screenshots alone do not establish Quest performance.
- Save matched before/after views and resolve relevant lighting-build errors.

Placement is straightforward. The uncertain effort is the imported meshes'
lightmap quality and the mobile bake result. A small test bake determines that
before expanding the work. No custom Blueprint system is needed.

AFCore, Mimic, spawn markers, and the original Crib geometry remain outside the
lighting edits. A completed lighting pass will require new client packages;
lighting is client-rendered and does not require a new networking system.

References: [Epic static lighting](https://dev.epicgames.com/documentation/en-us/unreal-engine/static-light-mobility-in-unreal-engine),
[volumetric lightmaps for movable objects](https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-lightmaps-in-unreal-engine),
[mobile performance guidance](https://dev.epicgames.com/documentation/en-us/unreal-engine/performance-guidelines-for-mobile-devices-in-unreal-engine).
