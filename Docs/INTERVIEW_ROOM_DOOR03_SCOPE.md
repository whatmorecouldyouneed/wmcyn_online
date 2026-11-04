# Interview Room Candidate: Door 03

## Window Return Follow-up

Current viewport inspection identified the remaining original Plane_020 return
at X approximately -1398 as the projection beside the window. Architecture
revision 02 removes that return and its adjoining partition strip, retaining
the original source asset. WMCYN floor/ceiling infill and east lining enclose
the exposed recess. This moves the return farther back, not a complete rectangular
redesign of the room. Screenshot shows remaining finish mismatches in the new
recess: floor UV scale/light bake and wall junctions need another finish pass.
Do not call the room final-quality yet. Existing window and door assets unchanged.

## Window Alcove Correction (2026-10-17)

Follow-up: restored the short hallway return at Y623.5, X[-1074,-922]
that the partition removal inadvertently opened. Screenshot confirms the gap
is closed. Door 04's placed SM_Lever is left at 100 degrees open for editor
remodeling; runtime hinge initialization may reset it. No door Blueprint edited.
The 120 floor samples and four passage sweeps still pass.

User reports persistent crouched-looking posture. Read-only pawn defaults show
IsCrouched=false, capsule half-height 90 cm and unit scale, but no PIE world was
running to inspect actual HMD height, IK or runtime crouch state. Seated/standing
and camera-height clarification is pending. Do not offset the body or change
Mimic calibration based on these defaults alone. Living-room reference images
are expected next pass; no furniture layout is committed yet.

The first blockout excluded the original south window behind its new return wall.
The return is now hidden with collision disabled, and the rear wall and ceiling
extend around the original window alcove. Original window/frame/glass assets are
unchanged. Editor screenshot verification shows the window and exterior staircase
from inside the joined room. Existing rock intrusion beside the window remains
an art-polish item. Updated tests pass 120/120 floor samples and four capsule
passage sweeps, preserving all 811 horizontal shell faces. Map saved; headset
traversal and lighting rebake remain pending.

## Combined Room Blockout Applied

User approved joining the spaces behind Doors 03 and 04 (2026-10-17).
`Tools/Lighting/build_combined_interview_room.py` creates reversible WMCYN assets
and a bounded room shell, leaving imported source assets and framework assets intact.

- Plane_020 now references `SM_InterviewRoom_Architecture_01` in
  `/Game/WMCYN/Environment/Crib/InterviewRoom`. Removes 24 verified partition/back
  wall triangles; retains the other 396 triangles with their positions and UV0.
- Hidden collider now references `SM_InterviewRoom_Collision_01`, removing only
  the four triangles that sealed the backs of Doors 03/04. All 811 horizontal
  collision faces match the previous WMCYN shell.
- Added rear wall, north/south returns, ceiling and entry ceiling, plus two
  temporary 400-lumen movable work lights in
  `WMCYN_Environment/InterviewRoom_Blockout`.
- Both existing interactive hallway doors remain in place. The broad interior
  is approximately 7.6 by 6.1 metres, with an irregular entrance-side outline.
  This is not the final 1:1 living-room calibration or set dressing.
- Verified 96/96 floor samples and three 26 cm radius / 90 cm half-height capsule
  sweeps: Door03, Door04 and the joined interior. Sweeps are at Z570, above raised
  thresholds; they do not prove grounded stair/threshold traversal.
- Screenshots inspected from the original interior angle and from Door03 looking
  into the new rear corner. The ceiling opening exposed by demolition was capped.

Remaining: headset walkthrough, entrance finishing, materials/UV scale polish,
and lighting rebake. Temporary work lights are not a Quest performance solution.
Replacing the architectural mesh invalidates its prior lighting; do not call this
final lighting acceptance. No pawn, tracking, authentication or network changes.

Test: `Tools/Lighting/test_combined_interview_room.py`.
Reports: `Saved/Diagnostics/Lighting/combined_room_build.json` and
`combined_room_tests.json`. The build script saves new mesh assets; the map was
saved explicitly after inspection. Re-running the script does not overwrite its
generated mesh assets; geometry revisions require a new revision name.

Rollback: restore Plane_020 to `/Game/Environments/WMCYN_Crib/Scene_074` and
collider to `/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Collision_Doorways`, then
hide/disable the seven new blockout actors listed in the build report. Original
component mesh paths are recorded in `combined_room_rollback.json`. Preserve the
separate floor-collision repairs.

## Original Inspection

2026-10-17: user identified the room behind the left door in the current viewport
as the first-interview candidate. Screenshot shows two dark wooden doors separated
by a large white column. Viewpoint: (-567.56, 650.41, 709.00), yaw -175.60,
pitch -12.40. The left leaf is DOOR_Crib_03_Grip, the right is Door 04.

## Verified Blocking Boundary

- Door 03 plane: X -978.12, doorway center Y 385.42.
- All 12 inward traces (three widths, four heights) hit X -1031.18.
- Blocking component: imported actor's `collider`, hidden and not visible.
- Assigned WMCYN mesh: `/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Collision_Doorways`.
- Blocker is approximately 53 cm behind the door plane. Prior bounded doorway
  clipping extends only 45 cm each side; earlier +/-40 cm tests missed it.
- Both simple and complex queries hit the blocker. Opening the leaf alone cannot
  remove it. Do not disable the whole shell.
- Interior floor probes: 19/21 hit repaired Plane_004 at approximately Z 461.8.
  Missing probes: (-1250,250) and (-1350,250), from Z 510 to 250. These may be
  intentional geometry gaps; inspect visually before filling anything.

## Next Scoped Implementation

1. Inspect the room geometry/bounds and those two floor gaps from inside.
2. Extend only Door 03's bounded shell opening through the identified wall,
   retaining jambs, surrounding walls, floor and ceiling. Work on a reversible
   WMCYN shell revision, not the original imported asset.
3. Test the complete doorway depth with a pawn-sized capsule, not only a line
   through the door plane. Repeat floor/ceiling regression and headset traversal.
4. Only then establish interview set bounds and place the measured couch/flag.
5. Audit other doors for the same beyond-threshold blocker; do not assume every
   door has a finished, usable room behind it.

Read-only audit: `Tools/Lighting/audit_interview_room_door03.py`.
Results: `Saved/Diagnostics/Lighting/interview_room_door03.json`.
No Unreal assets were changed during this inspection; original viewport restored.
