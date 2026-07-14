# First Signal Player Presence Audit

## Conclusion

The active Quest pawn is correctly scaled at the actor, capsule, camera, and AFCore component levels. The earlier tiny-player appearance came from AFCore's tracked head-and-torso placeholder, not a world-scale problem.

The production lane now uses the full-height native Mimic Pro pawn through the WMCYN-owned `BP_WMCYN_UserPawn_FirstSignal`. Its native body hierarchy, Stage tracking, seated calibration, locomotion, footsteps, and floor behavior are the source of truth. AFCore remains a reusable framework/reference source, not the active body pawn.

## Current Player Path

- Production pawn: `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal`
- Native parent: `/Game/FullBodyVRTemplate/BluePrints/BP_VRBodyCharacer`
- Stable rollback pawn: `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn`
- Runtime selection: `/Game/WMCYN/Core/BP_WMCYN_GameMode_FirstSignal.DefaultPawnClass`
- Capsule: 26 cm radius, 90 cm half-height, scale `1.0`
- Body: the native package's Manny mesh, `ABP_VRBody`, `CR_VRBody`, `FullBody_Component`, `VR Origin`, `Height Offset`, camera, and motion controllers
- WMCYN adapter: `Plugins/WMCYNRuntime` component `WMCYN_FirstSignalPresence`
- Network path: Character movement for root locomotion plus 20 Hz owner-skipped, interpolated camera/left-controller/right-controller transforms
- Menu interaction: the native left/right trigger graphs remain intact; the WMCYN adapter configures both inherited widget rays for `Visibility`, 750 cm reach, and hit testing
- Identity path: reliable client-to-server username/display-name submission into replicated `BP_WMCYN_PlayerState_FirstSignal`
- Presentation path: owner-hidden, remote-visible `WMCYN_Nameplate_Runtime` plus dynamically registered `WMCYN_VOIPTalker_Runtime`
- Seated calibration: preserve the native package's left-thumbstick calibration and dedicated `Height Offset`; do not add AFCore camera-handle rewrites
- Head presentation: Manny's attached head remains the source of truth
- Walking presentation: Mimic `ABP_VRBody` calls `CalculateFeetTargets` every update and uses smoothed HMD/world velocity for procedural alternating steps
- Indexed marker presentation: `BAD SIZE` is suppressed per instance because these AFCore PlayerStart-derived actors are post-possession anchors; marker transforms and collision remain unchanged

Measured AFCore placeholder bounds:

- Head: approximately 22 x 18 x 25 cm
- Torso: approximately 36 x 41 x 64 cm
- Combined visible bust: roughly 90 cm tall, with no pelvis or legs

These dimensions explain the earlier 1/4-to-1/6-size impression. Enlarging the existing pieces would produce an oversized head and torso and would not create a valid full body.

## AFCore Mannequin Check

- Mesh: `/Game/AFCore/Meshes/Pawn/Manequin/SK_Framework_Movement_Mannequin`
- Animation Blueprint: `/Game/AFCore/AnimBP/Mannequin/AnimBP_Mannequin`
- Height: approximately 183 cm
- Existing use: AFCore controlled-character, ghost-character, and locomotion example paths
- VR IK result: no FABRIK, Two Bone IK, CCDIK, Control Rig, or equivalent headset/controller tracking graph was found

Decision: do not use this mannequin or Animation Blueprint as a complete VR body by itself. The previous WMCYN arm-only rig remains available on the stable base pawn only as rollback while Mimic is validated.

## Development Mirror

- WMCYN asset: `/Game/WMCYN/Dev/AvatarMirror/BP_WMCYN_AvatarMirror`
- Source pattern: duplicated from AFCore's render-target mirror; no AFCore asset was edited
- Level actor: `DEV_AvatarMirror_Tracking`
- Outliner folder: `WMCYN_99_Dev_AvatarMirror`
- Transform: location `(-250, -700, 558)`, rotation `(0, -90, 0)`, scale `1.0`
- Tags: `WMCYN_DevOnly`, `WMCYN_AvatarMirror`
- Capture: 500 x 250 render target at 24 FPS

The mirror is map-loaded on clients but does not replicate. Each device renders its own local mirror view. It is a development diagnostic and should be disabled or removed before a performance-sensitive release build.

## Headset Check

1. Start VR Preview in `L_WMCYNOnline` and complete login.
2. Confirm the spawned class is `BP_WMCYN_UserPawn_FirstSignal` and the Output Log reports native pawn sync to indexed slot `0`.
3. At `StandaloneVR_A`, turn approximately 180 degrees to face the mirror.
4. Look down and confirm chest and arms are visible without Manny head geometry entering the HMD view.
5. Start the run while seated, use the native left-thumbstick calibration, and confirm WMCYN is not displacing the native camera or controller hierarchy.
6. Confirm the mirror shows one attached Manny head and no separate AFCore bust head. Check head height, neck, torso, hips, knees, feet, and floor contact; record the remaining seated mismatch instead of accepting a camera offset.
7. Confirm the 12 cm sole clearance puts the feet on the floor rather than through it or visibly above it.
8. Hold both hands low, forward, wide, crossed, and overhead. Check that elbows stay on the correct side and shoulders remain attached.
9. Use the left stick to walk a short arc and confirm alternating procedural foot movement appears.
10. Crouch, lean left/right, and turn in place.
11. Confirm there is one body, one head, and one pair of hands with no scale multiplication, inversion, shoulder separation, foot sliding, or offset.
12. Watch the Output Log for recurring divide-by-zero, missing rig, missing AnimBP, or Blueprint runtime errors.
13. Record one screenshot or short OBS clip from the PCVR view for comparison.

## Pass Criteria

- World, capsule, eye height, and controller reach read as human-scale.
- The owning user sees chest and arms in first person without seeing inside Manny's head or duplicate hands.
- Hips, knees, and feet remain plausible during standing, crouching, turning, and short locomotion.
- Head and controller/hand tracking remain aligned in the mirror; shoulders stay attached and elbows bend naturally across the tested reach envelope.
- VR Preview produces no recurring Mimic runtime warning and Quest frame timing remains usable.
- No AFCore asset is modified.

If this check fails, tune or replace only the WMCYN-owned tracked-body layer. Do not edit AFCore or block multiplayer/nameplate validation on final avatar fidelity.
