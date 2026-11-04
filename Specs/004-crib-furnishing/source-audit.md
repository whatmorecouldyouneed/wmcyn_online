# Furnishing Source Audit

2026-10-17. Read-only source comparison; no Unreal assets modified.

## Findings

- Compared all 626 original package asset payloads against the local WMCYN v2.0
  project: 585 byte-identical, 41 changed, none missing. This compares asset content,
  not `.meta` import settings, and does not classify newly added project files.
- Main `Hidden Heights World.unity` differs; `World Props.unity` is identical.
  A byte difference is not proof that furniture placement was customized.
- Parsed 87 top-level prefabs using safe YAML loading. Recorded dependencies,
  material texture references, local transforms and collider definitions.
- Main scene: 3,047 serialized documents; includes one sectional, one kitchen table,
  four dining chairs, eleven bar-stool-1 instances and six bar-stool-2 instances.
  Counts describe serialized references, not confirmed active visible objects.
- World Props: 96 documents, generally one instance per prop. Use as a catalog.
- Four unresolved source-prefab GUIDs remain in the main scene. External game
  prefabs also exist. Exclude unresolved/external content from batch one.
- The sectional child uses 100x scale and -90-degree X quaternion rotation.
  Preserve composed hierarchy/import semantics; do not multiply Blender metre
  measurements by this child scale again.

## Geometry Measurements

Blender 4.4 background FBX imports, factory-startup scene, no source saves.
Dimensions are source FBX world bounds in metres, not final scene-instance dimensions.
Material slots in the FBX may include unrelated unused materials; prefab overrides
are the binding authority. Mesh object names are not reliable identifiers.

| FBX | X x Y x Z (m) | Triangles |
| --- | --- | ---: |
| sectional_sofa01 | 4.616 x 5.091 x 1.040 | 23,836 |
| accent_chair01 | 1.449 x 1.186 x 0.958 | 4,588 |
| coffee_table | 0.927 x 1.909 x 0.427 | 228 |
| dining_chair01 | 0.566 x 0.636 x 1.288 | 1,154 |
| floor_lamp01 | 0.882 x 1.009 x 2.301 | 2,652 |
| kitchen_table | 1.357 x 1.357 x 0.968 | 416 |
| rug01 | 6.848 x 7.893 x 0.007 | 40 |
| sideboard01 | 1.335 x 0.388 x 0.681 | 140 |
| bar_stool01 | 0.565 x 0.645 x 1.362 | 2,828 |
| side_table01 | 0.774 x 0.774 x 0.635 | 284 |

## Verified Material Families

- Sectional: `textures/large sectional sofa/large sofa.mat`.
- Accent chair and bar stool: `textures/single seat chairs/single seat chairs.mat`.
- Coffee table and sideboard: `textures/side tables/side tables.mat`.
- Dining chair/table: `textures/l furniture/l furntirue.mat` (source spelling).
- Rug: `textures/rug/rugs.mat`; floor lamp: `textures/lights/lights.mat`.

## Branding Review

Visually inspected five files from the supplied LOGOS directory:
`IMG_0217.JPG` (square monogram), `pattern-WMCYN LOGO.png` (bordered monogram),
`IMG_3893_Original.jpg` (wide chrome wordmark), `WMCYN-HQ-BANNER.JPEG`
(black-on-white mark/QR), and `SIX_C5E9EAF8-3C17-474F-9FA2-68033FCCAB69.PNG`
(light-on-black mark/QR). Two HEIC files have not been reviewed.
QR destinations/readability have not been validated. Do not claim they work in-game.

## Remaining Audit Boundaries

No new full-gallery visual review was performed in this pass; reuse the twelve-image
review in `Docs/CRIB_SOURCE_RESTORATION_AUDIT.md`. FBX thumbnail renders, resolved
scene placement differences, current Unreal duplicate inventory, measured zone
layout, texture-resolution review and two HEIC previews remain pending. This report
is not an import/apply approval or a finished visual/performance audit.
