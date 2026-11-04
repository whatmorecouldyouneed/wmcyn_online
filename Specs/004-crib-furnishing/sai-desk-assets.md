# Sai PC/Desk Setup — VRChat to UE5 Conversion

The PC and desk setup from the VRChat world, converted for the Crib. Source is
`C:\Users\jvred\WMCYN v2.0\Assets\Sai` — a Japanese VRChat room asset,
"Sai's Room v2.0".

## What the source actually is

Not a desk asset. **One FBX containing a whole room**: 368 mesh objects, 31
materials, ~340k triangles, covering bed, closet, walls, door, curtains, heater
and aircon alongside the desk. Authored in Blender 4.2.6.

There is nothing to select by name. Objects are named as generic Blender
primitives — `立方体.047`, `円柱.019`, `NURBSパス.012` — and the Unity scene
file lists only colliders and lights, not the room's contents. What *does*
identify the desk is **the material an object uses and where it sits in the
room**, and that is what the extraction classifies on.

## Pipeline

Two steps, then the Unreal import.

### 1. `Tools/Models/extract_sai_desk.py` (Blender, headless)

    blender -b --factory-startup --python Tools/Models/extract_sai_desk.py

Classifies every mesh object into `desk` / `chair` / dropped, and writes
`SM_Crib_SaiDesk.fbx`, `SM_Crib_SaiChair.fbx` and `sai_material_map.json` to
`Intermediate/WMCYN/SaiDesk/`. Nothing in the source folder is modified; the
material renames happen in memory only.

Three rules, and the order matters:

- **Deny by material** — floor, wall, frame, door ×2, bed, the
  curtain/aircon/chest/bin/lighting set, both closet/heater/fan/drawers sets.
- **`Chair` material** → its own group.
- **Spatial box**, world bbox centre inside `X ∈ [-1.05, 1.05]`,
  `Y ∈ [0.70, 2.78]`, `Z ∈ [0, 1.75]` metres → desk group.

Two tuning passes were needed and both are worth knowing about:

- **VR gear was 39% of the first extraction.** `mic,hmd,tracker,usbhub,bs,…`
  plus the controller came to 63k triangles of wall-mounted base stations and
  HMD stands. `INCLUDE_GEAR` is off; turning it on readmits only pieces inside
  the box and below `GEAR_MAX_Z`, which is roughly the desk-top hardware rather
  than the wall rig.
- **Cables need full containment, not a centre test.** They run the length of
  the room, so any run balanced over the desk passed a centre test and then
  trailed metres out of the set and up to the ceiling — clearly visible in the
  first import. Objects using a cable material must have their **whole bounding
  box** inside the box. That alone took the group from 2.25 × 2.26 × 1.91 m
  down to 2.02 × 2.18 × 1.57 m.

Material names are renamed to ASCII slugs before export (`机,プリンタ,…` →
`Sai_Desk`), because the originals are Japanese and contain commas, which are
unusable as Unreal asset names. `sai_material_map.json` records slug → original
so the next step can still find the Unity material.

**Both groups share one origin**, taken from the desk: centred in XY, sitting on
the floor. The chair therefore keeps its true position relative to the desk
while remaining a separate actor, so dropping both at the same transform
reassembles the setup.

### 2. `Tools/Models/stage_sai_textures.py`

Resolves each material to its texture and stages it under an ASCII name.
Unity keeps the albedo as a GUID in the `.mat`; the filename only appears in the
`.meta` sidecar beside the texture, so this builds a GUID → filename map from
`tex/*.meta` and reads `_MainTex` through it. Materials with no `_MainTex` are
flat colours, and Unity serialises `_Color` in gamma space — converted to linear
**once** here, the same rule the rug and couch follow. Do not convert again when
authoring the Unreal material.

### 3. Unreal import

Done through the editor's MCP bridge rather than a committed headless script, so
there is no `import_sai_desk.py`. The parameters, for repeating it:

- `StaticMeshTools.import_file` into
  `/Game/WMCYN/Environment/Crib/Furniture/SaiDesk/Meshes`, with
  `import_materials=False`, `import_textures=False`, `combine_meshes=True`.
  Materials are built from the Unity data, not from the FBX.
- Textures into `…/Textures` as `T_Sai_*`, `sRGB` on,
  `maxTextureSize` 2048 (1024 where the source is already 1K),
  `TEXTUREGROUP_World`.
- One material per slug as `M_Sai_*`: texture → `MP_BaseColor`, or a linear
  `BaseColour` vector parameter for the flat ones, plus a `Roughness` parameter
  at 0.72.
- `generate_lods([0.5, 0.25])` → 3 LODs, and
  `generate_convex_collisions` (14 hulls desk, 8 chair). The Crib shell has no
  collision at all, so without this they would be walk-through.

## Cleanup pass

Two more rules were needed once the set was placed and inspected.

**Wall-mounted fixtures floated.** Outlets, switches, faceplates, power-strip
sockets and the HMD wall mount are fixed to the room's walls. The walls are
dropped, so these hung in mid air — clearly visible as a headset and two plates
suspended beside the desk. `wall_mounted()` drops anything thin (<= 20 cm deep)
and flush against a wall plane: the back wall at `Y 2.62`, the side wall at
`X -0.98`. The desk touches the same back wall but is far too deep to trip it.

**Small props needed full containment.** A 12-triangle rail passed the centre
test while poking 18 cm outside the region, and whatever it was fixed to was
gone, so it hung in space too. Anything whose largest dimension is <= 60 cm must
now be wholly inside the box. Large pieces stay exempt, since the desk itself
legitimately spans the whole area. That took the group's Y extent from 2.17 m
to 1.96 m.

Together these dropped 28 objects and 12,652 triangles.

**Note on re-extraction:** the shared origin is the desk group's bounding-box
centre, so it moves whenever the group's membership changes. This pass shifted
it 10.4 cm, and the placed actors were nudged the opposite way to compensate.
Check this after any future tuning, or hand-placed actors will drift.

## Floating props: settle them, do not delete them

Phones were left hanging in mid air above the desk. Three attempts at detecting
this, two of them wrong, are worth recording so they are not retried:

- **"Nothing within 25 cm below"** flagged almost nothing, because a prop
  floating 5 cm over the desk does have the desk below it.
- **"Gap to the nearest thing below > 1.5 cm"** flagged 33 objects, nearly all
  false positives. Plenty of parts legitimately have nothing beneath them
  because they are cantilevered: a USB stick in the PC's side, every
  monitor-arm segment, a dongle hanging on a cable.
- **Connectivity to the floor** works. Build a touch graph over the bounding
  boxes (12 mm tolerance), take connected components, and treat any component
  that never reaches floor level as adrift. Cantilevered parts survive because
  they touch a host that is itself grounded.

But detection was only half of it. The first version **deleted** adrift
components, and that was the wrong action: these phones come in pairs, one was
adrift and its twin was not, so deleting left a lone phone where there had been
two. Removing scenery to fix its position is worse than the original bug.

`settle_or_drop()` now lowers each adrift component onto the highest grounded
surface beneath its footprint, with 2 mm of overlap so it seats rather than
hovers, and only deletes a component when there is nothing under it at all.
Two settled: `立方体.059`, a 15 x 8 x 1 cm phone, down 3.3 cm, and `円柱.033`
down 8.3 cm. Nothing was deleted, so the set stayed at 117 objects.

Note that a small cube hanging in space over the rug is **correct** and is kept
- it touches a cable that runs to the desk, which is exactly what a dongle on a
wire looks like.

## Winding: the PC rendered inside-out

The PC looked like it had no case — you could see its intake grille, fan and
motherboard through the shell.

**Cause: six objects carry a negative-determinant (mirrored) transform**, the PC
among them, along with `ドック`, two monitor arms and two smaller parts. Their
faces are wound correctly in local space, but mirroring reverses them in world
space. Neither Blender's solid view nor Unity culls backfaces by default, so
this was invisible in both; Unreal culls, so the outward faces disappeared and
the interior showed through.

`unmirror()` reverses the faces of those objects, cancelling the flip. The
transforms are left alone — baking them out would be tidier but would disturb
the parenting the hierarchy relies on.

A separate `fix_normals()` pass recalculates outward winding generally; it
corrected 11 objects whose normals were merely inconsistent (`NURBSパス.017` had
all 470 faces inverted). That is a different fault from the mirroring and both
were present.

**Two diagnostic lessons.** First, a Blender render is not a check for this
class of bug, because it does not cull — I rendered the PC from source and from
the export, saw a closed case both times, and wrongly concluded nothing was
wrong. Only the Unreal viewport shows it. Second, the face-direction count was
the tell and I under-weighted it: 1,445 of the PC's 3,115 faces reported as
pointing inward.

## Result

| | triangles | LODs | size (cm) | slots |
|---|---|---|---|---|
| `SM_Crib_SaiDesk` | 76,194 | 3 | 197.7 × 196.3 × 145.0 | 13 |
| `SM_Crib_SaiChair` | 26,704 | 3 | 97.7 × 97.6 × 124.1 | 1 |

Scale check passed: Blender reported 2.02 × 2.18 × 1.57 m and 0.98 × 0.98 ×
1.24 m, so the metre → centimetre conversion landed. Every slot resolves to an
`M_Sai_*` material; none left on `WorldGridMaterial`.

Seven textures (`Chair`, `Clutter`, `Desk`, `Keyboard`, `Misc`, `Monitors`,
`PC`) and eight flat-colour materials.

Both actors are placed at `-350, -481, 470` with yaw 90, on clear tile in front
of the rug. **That is a review position, not a decision** — move or delete them.

## Removing individual props

The desk is one merged static mesh, so clicking a prop in the level selects the
whole assembly. There is no per-object handle to delete. Two ways round it.

**By name, through the extractor.** Every run writes
`Intermediate/WMCYN/SaiDesk/sai_desk_contents.md`: 142 rows listing each
object's name, group, material, size and centre in centimetres from the mesh
pivot (which sits centred on the desk at floor level, +X toward the room,
+Y toward the back wall, +Z up). Find the thing, add its name to
`DENY_OBJECTS` at the top of `Tools/Models/extract_sai_desk.py`, then:

    blender -b --factory-startup --python Tools/Models/extract_sai_desk.py

and reimport `SM_Crib_SaiDesk`. Reimport may clear the material slot
assignments, since they are set after import rather than coming from the FBX -
check the slots afterwards and reassign the `M_Sai_*` materials if so.

This is the route to prefer. It is reproducible, it survives a future
re-extraction, and the reasoning stays in the script.

**By hand, in Modeling Mode.** Unreal's Modeling Mode can select and delete
triangles on a static mesh directly, no code involved. It edits the asset in
place, so the change is invisible to the extractor and will be lost the next
time the mesh is re-imported. Fine for a one-off, bad as a habit.

## Open

- **Licensing is unresolved.** This is third-party Japanese VRChat content and
  there is no license, readme or terms anywhere in the asset folder. This
  repository is public and already carries the same open question about paid
  marketplace content. The scripts here can be committed safely; the converted
  content under `Content/WMCYN/.../SaiDesk/` is the part to decide on before
  pushing.
- **Budget.** 115k triangles for desk and chair together, against 14k for the
  whole couch. It is by far the heaviest thing in the Crib and this is a Quest
  target. Faithful conversion was the brief, and LODs are in; whether it earns
  that share of the frame is a separate call once it can be measured on device.
  The cables alone are 25k of it and are the obvious first cut.
- `Sai_ScreenBotan` has no Unity `.mat` — it is FBX-only — so it is a neutral
  grey placeholder rather than its real screen content.
- Lightmap UVs were generated on import, but the level has not been rebuilt
  since, so these are lit dynamically for now.
