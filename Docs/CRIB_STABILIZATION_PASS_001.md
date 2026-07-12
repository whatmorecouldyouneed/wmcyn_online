# Crib Stabilization Pass 001

Goal: make one conversation area in `L_WMCYNOnline` readable and frameable for First Signal. This pass is manual Unreal Editor work only. Do not touch multiplayer, presence/capability logic, Verbatim, custom Blueprints, or AFCore framework assets.

## 1. Manual First Steps

1. Save a backup/checkpoint of the current level state before moving actors.
2. In `L_WMCYNOnline`, pick one small conversation zone in the visible Crib FBX.
3. Ignore the rest of the Crib unless it affects the chosen camera frame.
4. Place temporary markers for two standalone VR users and one PCVR recording user.
5. Add only lighting, blocking, spawn-marker, and camera-framing helpers needed for this one shot.

## 2. Do Not Touch Yet

- AFCore framework assets
- multiplayer entry setup
- presence/capability logic
- Verbatim logic
- custom Blueprints
- locomotion/grab/menu systems
- public matchmaking or backend auth
- whole-Crib lighting polish
- final art dressing outside the selected shot

## 3. Suggested Outliner Folders

Create these exact folders in the level Outliner:

- `WMCYN_00_Crib_Static`
- `WMCYN_01_FirstSignal_Area`
- `WMCYN_02_FirstSignal_Lighting`
- `WMCYN_03_FirstSignal_Collision`
- `WMCYN_04_FirstSignal_SpawnMarkers`
- `WMCYN_05_FirstSignal_Cameras`
- `WMCYN_99_Temp_DoNotShip`

## 4. Suggested Actor Names

Use these exact names for new helper actors:

- `SM_FirstSignal_ConversationZone_Blockout`
- `LIGHT_FirstSignal_Key`
- `LIGHT_FirstSignal_Fill`
- `LIGHT_FirstSignal_Back`
- `LIGHT_FirstSignal_Practical_01`
- `PPV_FirstSignal_CameraLook`
- `COL_FirstSignal_Floor`
- `COL_FirstSignal_BackWall`
- `COL_FirstSignal_NoWalk_Left`
- `COL_FirstSignal_NoWalk_Right`
- `MARKER_FirstSignal_Quest_A`
- `MARKER_FirstSignal_Quest_B`
- `MARKER_FirstSignal_PCVR_Recording`
- `CAM_FirstSignal_Wide`
- `CAM_FirstSignal_VRPair_TwoShot`
- `CAM_FirstSignal_PCVR_Capture_Check`

## 5. Lighting Checklist

- [ ] Both standalone VR users' faces/hands are readable from `CAM_FirstSignal_VRPair_TwoShot`.
- [ ] Key light gives shape without blowing out the FBX materials.
- [ ] Fill light lifts shadows enough for standalone headset readability.
- [ ] Back/rim light separates avatars from the background.
- [ ] No large black patches dominate the chosen frame.
- [ ] No bright practical light pulls attention away from the two VR users.
- [ ] Exposure is stable from the camera view.
- [ ] Lighting is acceptable in viewport and Play mode.

## 6. Collision Checklist

- [ ] Add simple blocking only where needed for the chosen area.
- [ ] Floor collision feels stable around both standalone VR positions.
- [ ] PCVR recording area is not blocked by accidental collision.
- [ ] No invisible collision blocks the intended camera shot.
- [ ] Do not collision-polish the entire Crib yet.

## 7. Spawn Marker Checklist

- [ ] `MARKER_FirstSignal_Quest_A` faces `MARKER_FirstSignal_Quest_B`.
- [ ] `MARKER_FirstSignal_Quest_B` faces `MARKER_FirstSignal_Quest_A`.
- [ ] `MARKER_FirstSignal_PCVR_Recording` has a clear recording angle on both.
- [ ] Markers are far enough apart for avatar/head/hand presence.
- [ ] Markers are not inside furniture, walls, or bad floor collision.
- [ ] Marker locations are documented by actor name, not by memory.

## 8. Camera Framing Checklist

- [ ] `CAM_FirstSignal_Wide` shows the selected Crib conversation area.
- [ ] `CAM_FirstSignal_VRPair_TwoShot` clearly frames both seats/standing spots.
- [ ] `CAM_FirstSignal_PCVR_Capture_Check` approximates the PCVR recording viewpoint.
- [ ] The two-shot has usable headroom and does not crop hands badly.
- [ ] Background reads as The WMCYN Crib, not a generic dark room.
- [ ] OBS could capture the shot without needing a full camera system yet.

## 9. Done Means

This pass is done when one Crib conversation area is readable, both standalone VR positions and the PCVR recording position are obvious, the two-shot is frameable, and lighting/collision are stable enough for the next implementation checkpoint.

Do not continue polishing after that. Stop when the first usable shot exists.

## 10. Next Checkpoint

After this pass, pick the AFCore presence baselines for:

- standalone VR users
- PCVR recording user

Then create the smallest WMCYN wrapper/child plan for presence/capability-based spawning into the marked Crib positions.
