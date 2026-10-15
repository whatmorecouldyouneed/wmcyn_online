# Crib Interactive VR Doors Spike

Date: 2026-10-14. Discovery followed by an isolated mechanism prototype.

## Approved Scope and Prototype Status

User approved physical grip-and-pull in the FIRST door pass, not a later toggle-only
upgrade. The working implementation is not complete yet.

Created `/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor_GripPilot`,
a WMCYN duplicate of AFCore's `BP_Lever` using its `Comp_Latch_Drag` and `Comp_Drag`,
with AFCore's demo door/frame meshes. Corrected the lever parent's 90-degree roll
and configured AngleYaw limits 0-100 degrees, free movement, no auto-attach or
simultaneous latching. Original AFCore assets remain unchanged.

Synthetic PIE test uses the actual `InitiateDrag`,
`AdjustPositionToReferenceLocation` and `ReleaseDrag` functions on authority.
Measured yaw: 0 -> 89.997 -> 0.0055 degrees. This establishes that the AFCore
mechanism can operate with no AFCore pawn supplied on the authority path.
It does NOT establish client RPC compatibility, controller grip routing, hand
alignment, obstruction handling, or hardware acceptance. Temporary test actors
were removed and PIE stopped. No original doorway has been replaced.

Scripts: `Tools/Lighting/create_crib_grip_door_pilot.py` and
`Tools/Lighting/test_crib_grip_door_pilot.py`. The latter requires a temporary PIE
prototype tagged `WMCYN_DoorSyntheticTest`; result is in
`Saved/Diagnostics/Lighting/door_grip_smoke.json`.

### Local Headset Pilot Ready for Testing

`DEV_CribDoor_GripPilot_RightHand` is saved in `L_WMCYNOnline`, folder
`WMCYN_99_Dev_DoorGripPilot`, at approximately (-200, -400, 462.7) cm.
It is a freestanding orange demo door near the initial spawn, not a replacement
of an imported door. Placement traces the verified `Plane_003` walking surface;
a generic world trace incorrectly hit railing collision and was rejected.

The pilot EventTick reads the native Mimic RightController and Oculus right grip
axis/click without consuming or changing existing input mappings. Holding grip
within 35 cm of the knob calls AFCore InitiateDrag; holding updates Comp_Drag;
release, lost tracking, or distance over 120 cm releases it. No trigger/B changes.
This adapter is explicitly standalone-only. It is not the production owned-player
RPC adapter, and does not establish remote replication. It does not modify login
or provide a new login bypass; enter the world normally before approaching it.

Pilot meshes ignore Pawn collision to prevent pushing/trapping a user during this
mechanism-only pass. Closed-door blocking and obstruction safety remain untested.
Blueprint compile and an in-viewport startup/tick smoke test passed without door
errors. That run had no connected OpenXR headset/voice device; hardware grip input
still needs user validation. Editor placement was screenshot-checked:
`Saved/Diagnostics/Lighting/Door_GripPilot_Ready.png`. PIE was stopped afterward.

Reproducible input graph: `Tools/Lighting/crib_grip_door_pilot.dsl` (apply with
BlueprintTools.write_graph_dsl). Idempotent placement:
`Tools/Lighting/place_crib_grip_door_pilot.py`. Save the current level after reviewing.
Rollback is removal of this tagged pilot actor only; original doors are unchanged.

Test: local VR Preview -> login -> find orange door near spawn -> right hand at
knob -> squeeze side grip -> pull/push -> release -> re-grip and close. Logs:
`WMCYN Door: right grip attached` / `WMCYN Door: released`.

Hardware follow-up: the initial grip test failed. Inspection found FKey pins had
serialized the attempted `(KeyName=...)` struct literal as the invalid key `(`.
Corrected to Unreal's plain FKey export strings `OculusTouch_Right_Grip_Axis` and
`OculusTouch_Right_Grip_Click`; both pass InputLibrary.key_is_valid, and the saved
Blueprint compiles with warnings treated as errors. Removed the fresh-press-only
restriction so holding grip before reaching the knob works. A once-per-squeeze
`WMCYN Door: right grip input detected` log distinguishes input from proximity.
This repair still requires a fresh headset test; nonblocking collision is intentional.

Remaining: production Mimic hand/grip adapter on the owned player, server-validated latch
ownership and hand updates, release/timeout cleanup, one measured doorway fit,
collision/obstruction tests, and Quest/PCVR plus remote-user verification.

## Recommendation

Pilot one WMCYN-owned hinged door, reusing AFCore's Comp_Drag mechanism.
Preserve the native Mimic player and adapt its input to the door. Do not replace
all imported doors or assume an AFCore example is a drop-in multiplayer VR door.
Use the pilot to open and survey a possible interview room before choosing it.

## Verified in the Editor

| Asset | Observed behavior | Reuse |
| --- | --- | --- |
| `/Game/AFCore/Examples/Blueprints/Samples/BP_Door` | Actor parent; frame and door mesh templates; event graph only calls parent events; registry dependencies are the two meshes | Geometry/hierarchy reference, not a complete interactive door |
| `/Game/AFCore/Examples/Blueprints/Samples/BP_SlidingDoor` | Actor parent; depends on `Comp_Open`, sliding-door meshes and open/close sounds | Working setup reference; not a hinged-door replacement as-is |
| `/Game/AFCore/Blueprints/Components/State/Comp_Open` | Tagged scene components receive relative translation, rotation, or scale; configurable closed/open values and curves; tick disabled when progress is empty | Preferred animation/state reuse |
| `Comp_Open.open` | RepNotify variable; graph includes authority branch and `Generic_RPC_Command` for setting state | Replication exists, but its route must be tested with our pawn/controller |
| `/Game/AFCore/Blueprints/Components/Interaction/Comp_Latch*` | Assets present; full behavior not inspected during this spike | Candidate for grip-and-pull, not yet verified |

Important compatibility detail: AFCore's interaction source structure includes
an `interactingPawn` typed as `BP_Pawn_Base`. Our production pawn is a Mimic child,
not that class. Never reparent the working player just to enable a door.
The current native UI pointer is not proof that AFCore world grabbing is wired.
The existing AFCore documentation index points to latch components on page 17
and multiplayer components on pages 30-31; local graphs above are the direct
evidence for this recommendation.

## Crib Replacement Constraints

The prior geometry audit found nine door-sized mesh components within the single
imported Crib actor (`Scene_027`, `035`, `042`, `045`-`050`). These are candidates,
not nine independently verified working doorways. Source comparison found their
vertex-distance shapes match the original FBX; do not assume corrupt geometry.

Before replacing the selected door:

1. Identify leaf versus frame, handle, wall and glazing sections visually.
2. Measure opening width/height, threshold, hinge location and swing clearance in
   world centimeters. Imported component scaling is not a reliable human scale.
3. Confirm what collision actually blocks the opening, including shared wall
   collision. Hiding an old leaf alone does not remove its blocking collision.
4. If the leaf is fused to the frame, create a WMCYN-owned separated duplicate
   from source. Keep original assets unchanged and record component overrides.
5. Retain the frame; replace only the movable leaf and its interaction/collision.
   Avoid nonuniformly scaling a complete demo door to force it into an opening.

## Proposed Pilot

Proposed assets, not created:

- `/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor`
- `/Game/WMCYN/Environment/Crib/Doors/SM_WMCYN_CribDoorLeaf_01`, only if separation is needed
- Outliner folder `WMCYN_Environment/InteractiveDoors`
- Actor label `DOOR_Crib_InterviewCandidate_01`

Hierarchy: fixed frame/root -> hinge SceneComponent -> movable leaf and handle.
Configure `Comp_Drag` to rotate the tagged leaf between measured closed/open
angles. Use simple leaf collision, not simulated complex-as-simple collision.

First interaction gate: grip the nearby handle, physically pull/push, then release.
Use existing input conventions without stealing trigger input from login/menu
widgets or B from the settings menu. Reuse the verified `Comp_Latch_Drag` /
`Comp_Drag` mechanism with a WMCYN hand-input adapter. Prefer bounded kinematic movement over unconstrained network
physics for the first pilot. Do not introduce a custom general grabbing system.

Server owns door state. Requests must go through the user's owned controller or
pawn, with server distance/state checks, then drive `Comp_Drag` on authority.
Do not rely on clients sending RPCs directly on an unowned world door. Verify
actor/component replication, late join state, concurrent requests and animation
convergence; the existence of RepNotify alone is not a multiplayer pass.
Closed-on-server-restart is sufficient initially; no Firebase persistence needed.

## Interview Room Survey

No room behind a door has been approved yet. Inspect the driveway entrance/right
route and candidate rooms with the user before placing the real-world set.
Reference dimensions: couch 210.8 cm wide, usable seat width 182.9 cm, ceiling
274.3 cm; flag 182.9 cm wide by 109.2 cm tall. Treat the earlier unlabelled 4 ft
3 in measurement as unresolved, not a room dimension.

Check couch/seat anchors, camera coverage, operator clearance, door swing and
accessible entry. Preserve the physical-to-virtual 1:1 seating zone rather than
resizing the couch or player to fit a room. A closing door does not imply voice
privacy, sound isolation or room instancing.

## Acceptance Before Rollout

- One door opens/closes from Quest and PCVR without breaking login or menus.
- Door clears the frame and floor; a user can walk through when open.
- Closing cannot trap or push the player through the floor; define obstruction handling.
- Remote users see the same state, including a late joiner.
- No changes to AFCore assets, Mimic tracking, locomotion, floor collision, or spawn.
- Capture closed/open doorway and room-wide screenshots; run hardware interaction test.
- Save a rollback manifest before hiding/replacing any imported leaf.

Next implementation step: validate the standalone pilot's right-hand grip in VR,
then implement owned-player networking and select/measure one actual doorway.
Do not bulk-swap doors until that pilot passes.
