# Crib Door Rollout

## Implemented

- Converted the adjacent right-hand door and the remaining identified wooden hinged doors: **15 leaves total**, numbered `DOOR_Crib_01_Grip` through `DOOR_Crib_15_Grip` in `L_WMCYNOnline`.
- Reused the working WMCYN right-controller grip adapter and AFCore `Comp_Drag` hinge behavior. No intentional changes to AFCore/Mimic logic, player tracking, login, or locomotion. Seven framework source packages appeared modified after editor saves; these are excluded from this commit and preserved locally pending review.
- Retained original UVs, restored atlas materials, handles, and fixed frames. Six source groups were partitioned exactly, with no missing or duplicated triangles in the replacement geometry.
- Source groups: Scene_036 (five upstairs), Scene_029 (two), Scene_031 (three), Scene_032 (two), Scene_033 (two open double-door leaves), Scene_045 (one).
- Glass/window panels and non-door props were deliberately not converted into hinged doors.
- Imported source components remain hidden for rollback. The earlier remainder actor is hidden in `InteractiveDoors/Rollback`; new frames and leaves have separate Outliner subfolders.

## Collision

The original shared `Scene_002` shell blocked 12 closed doorways independently of the visible door meshes. A WMCYN-owned `SM_Crib_Collision_Doorways` now replaces that component's mesh, retaining its transform, collision profile, two-sided collision, and invisible rendering.

Only vertical triangles within bounded doorway openings were clipped. All 811 horizontal floor/ceiling triangles were preserved. The original Scene_002 asset is unchanged.

Moving leaves intentionally remain nonblocking, matching the approved pilot. They do not push the pawn, prevent passage while closed, or implement physical hand contact. Grip-and-pull drives the hinge using tracked hand position.

## Verification

- All 15 WMCYN door Blueprints compiled with warnings treated as errors.
- PIE synthetic hinge tests passed for all 15: original pose -> configured swing limit -> original pose, within two degrees. This tests the AFCore hinge path, not hardware grip input.
- All 15 doorway center traces are clear of the imported static geometry after the cut.
- Floor/ceiling regression: **811/811 original and replacement trace hits, zero position mismatches** (0.1 cm tolerance).
- The generator initially hit UE's missing-UV static-mesh assertion. Editor recovered; UV0 and original two-sided collision are now explicit. Changes were reapplied and saved after recovery.
- Production lighting rebake succeeded. Right-hand door screenshots verify closed and open positions, retained frame/materials, and an unobstructed opening. The saved starting pose is restored to closed.
- Screenshots: `Saved/Diagnostics/Lighting/Crib_DoorBatch_Right_Closed.png` and `Crib_DoorBatch_Right_Open.png` (local diagnostics, not committed).

## Remaining Gates

- Headset: approach the right door, hold right side grip near its handle, swing it, release, and walk through. Repeat representative doors on each floor and the downstairs double doors.
- Confirm each swing direction clears nearby decoration and feels natural; hinge anchors for baked-open double leaves are approximate and need headset review.
- **Standalone VR Preview only:** the grip adapter still uses `IsStandalone`. Replication flags do not make this a working multiplayer door system. Server-authoritative interaction routing and multi-user conflict/release tests remain pending before packaged online rollout.
- No client/server build or deployment was performed.

## Reproduction / Rollback

Scripts are in `Tools/Lighting`. Run Unreal exports `export_crib_door_geometry.py` and `inspect_door_rollout.py`; run local `analyze_rollout_geometry.py` and `plan_crib_door_rollout.py`; run Unreal `convert_crib_doors_batch.py` then `wire_crib_doors_batch.py`. Preserve the existing door01 baseline.

For collision, run Unreal `check_crib_door_collision.py`, local `cut_crib_doorway_collision.py`, then Unreal `apply_crib_doorway_collision.py`. Run `test_crib_shell_floor_preservation.py` outside PIE and `test_crib_door_rollout.py` inside PIE. Production-rebake with `build_crib_lighting.py --quality production` after geometry changes.

Generators reuse existing named assets; they do not silently rebuild an existing mesh when changing the partition recipe. Use a new versioned asset name for revised geometry.

Reports and component rollback snapshots live in `Saved/Diagnostics/Lighting/door_*`. To revert the shell, assign original `/Game/Environments/WMCYN_Crib/Scene_002` to the imported `collider` component while keeping its rendering hidden. Restore source group visibility and hide replacement leaves/frames together when reverting door geometry; do not render both.
