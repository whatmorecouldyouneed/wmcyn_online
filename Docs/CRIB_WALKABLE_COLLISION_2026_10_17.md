# Crib Walking Collision Repair

## Symptom and Evidence

User reports progressively sinking outside First Signal, throughout the Crib.
The earlier 81-sample entry audit covered only Plane_003; its 1.4 mm height
variation does not establish collision coverage elsewhere.

The wider visible-surface audit found three WMCYN floor duplicates configured
with UseSimpleAsComplex but no usable simple collision on their sampled surfaces.
The separate shell does not provide matching floor support at these locations.
This is a verified collision defect; headset camera/IK behavior still needs testing.

## Applied

UseComplexAsSimple and double-sided collision, on existing WMCYN-owned assets only:

| Component | Mesh suffix | Simple-query samples before / after |
| --- | --- | --- |
| Plane_004 | SM_Crib_Lightmap_Scene_059 | 0/124 -> 124/124 |
| Plane_005 | SM_Crib_Lightmap_Scene_060 | 0/174 -> 174/174 |
| Plane_084 | SM_Crib_Lightmap_Scene_090 | 0/57 -> 57/57 |

Mesh geometry, UVs, materials, and transforms were not changed. No invisible
floor, floor guard, pawn height offset, or AFCore/Mimic edit was introduced.
The lighting repair script now preserves an existing WMCYN mesh's collision mode.
Only the three repaired mesh assets were saved for the collision pass; no Save All.

RespawnButton remains bound to the existing authoritative respawn request. It
was moved to the top of Settings in the Designer tree. The camera container's
fixed height was cleared so a collapsed preview no longer reserves 360 pixels.

## Verification and Limits

`Tools/Lighting/repair_crib_walkable_collision.py` tests each repaired mesh's
simple line traces within 2 cm of 355 horizontal triangle centroids and asserts
all hits before saving. Results: `Saved/Diagnostics/Lighting/walkable_collision_repair.json`.
The wider audit also includes roofs, furniture and water; its aggregate missing
count must not be interpreted as a count of broken walking surfaces.

Not yet verified: capsule sweeps, headset comfort, stairs/threshold traversal,
camera/feet drift during movement, packaged Quest performance, or remote play.
Complex static collision matches the working spawn-floor approach but must be
profiled on Quest. Do not apply this to dynamic objects indiscriminately.

## Next Headset Check

1. Test the current editor build locally; the deployed server/client checksum
   mismatch remains separate and remote builds do not yet contain these assets.
2. Login, open Settings and use Respawn to Entry.
3. Walk from spawn into adjacent rooms, cross door thresholds, then test stairs
   and the lower floor. Stop if sinking/disorientation continues.
4. Check whether the camera gets lower, the avatar feet sink, or both. A remaining
   progressive offset needs capsule/camera/body telemetry, not blanket height fixes.

Rollback: on these three assets only, restore UseSimpleAsComplex and disable
double-sided geometry. This restores the previous broken coverage for comparison.
