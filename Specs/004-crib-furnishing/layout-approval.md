# Layout A: Furniture and Zone Approval

2026-10-17. Layout A furniture shapes and zone allocation approved by Jared.
No furniture imported and no level assets changed as part of the approval.

## Review Images

![Actual source furniture in neutral clay](Previews/furniture-candidates.png)

![Candidate zones over measured Unreal floor surfaces](Previews/layout-a-measured-floors.png)

The furniture images are offline Blender geometry renders, not final materials or
in-world screenshots. Measurements are source FBX bounds, before scene scaling.
The plan uses live Unreal floor triangles, but proposed envelopes are not measured
clear room dimensions. Walls, pillars and door swing volumes are not all represented.

## Proposed Allocation

| Zone | Candidate world bounds in cm | Floor band | Proposal |
| --- | --- | --- | --- |
| A: Main lounge | X -1200 to -300, Y -900 to -100 | Z ~6 | Sectional, table, two chairs; rug resized to group |
| B: Counter-side review | X -500 to -100, Y 600 to 1000 | Z ~6 | Four stools, aligned to surveyed counter before placement |
| C: Upper dining | X -1000 to -400, Y 1200 to 1800 | Z ~462 | Round table plus four chairs and sideboard |

The interview/bathroom envelope on the upper-floor plan is protected. Nothing from
this batch enters it. Main-world floor bands were independently exported near
Z -470, +6 and +462 cm; do not assume a shared ground plane.

## Sizing Decisions for Review

- Keep the mansion sectional's full 4.616 x 5.091 m source footprint as the initial
  lounge fit candidate. Its internal empty area can contain the coffee table;
  overlapping bounding rectangles alone do not mean mesh intersection.
- Chair footprint is 1.449 x 1.186 m: these are broad accent seats, not dining chairs.
- Round table source height is 96.8 cm. Confirm actual prefab instance scaling and
  chair seat height before choosing a dining scale; no silent nonuniform squash.
- Rug source is 6.848 x 7.893 m. Its final size remains a layout-fit choice, not 1:1.
- Existing source material families remain the finish starting point. Clay colors
  in the preview are diagnostic only, not proposed upholstery colors.

## Approval Boundary

Approval covers the furniture shapes and zone allocation, not import-ready transforms.
Before applying, finish the source/world calibration, counter-height and seat-height
survey, asset duplicate check, wall/door clearance checks and final orientation.
Keep a 100 cm primary route, actual capsule sweeps, and unchanged door behavior.
No measured-clearance pass, VR acceptance or Quest optimization is claimed here.

## Reproduction

- `Tools/Lighting/measure_furnishing_zones.py`: read-only Unreal geometry export.
- `Tools/Lighting/render_furniture_previews.py`: background Blender source renders.
- `Tools/Lighting/build_furnishing_approval_board.py`: offline approval boards.
- `Tools/Lighting/plot_furnishing_floorplan.py`: three-level measurement overview.

Scripts write reports/images only. They do not create Unreal actors or assets.
