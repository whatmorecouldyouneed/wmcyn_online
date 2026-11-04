# Seated Posture Check

Goal: physically seated user, naturally standing avatar while navigating.

## Bounded Crouch Implemented

Hardware follow-up: user confirms bounded crouch works, but reports neck stretch,
resting bent knees and downstairs floor penetration. At mirror (20:31 UTC),
Height Offset equaled calibrated baseline 73.02 cm (no synthetic crouch), camera
relative height was 113.11 cm, placing camera 186.13 cm above VR Origin versus
the 168.06 cm calibration marker. Screenshot shows elongated neck. This is
consistent with posture changing after calibration, but not a complete IK diagnosis.
At this location Plane_003 simple/complex hits both equal Z461.695 cm, mesh root
Z461.695 and capsule bottom Z463.845. Mirror-location floor matches; do not
globally raise the pawn to fix the separately reported downstairs location.
Read-only Tools/Lighting/audit_runtime_body_floor.py records bones and per-component
surface hits. Awaiting downstairs capture. Nameplate repair remains requested;
do not compile/reinstance the active player during this hardware inspection.

WMCYN pawn now overrides the inherited Custom Crouch event and delegates to
WMCYNBoundedCrouch. Seated-mode Z is clamped between calibrated offset minus
45 cm and calibrated offset. X/Y are preserved. Input is clamped to [-1,1],
delta time to [0,0.05], with the existing 100 cm/s adjustment rate. Standing
mode remains a no-op as in the native function. Parent BeginPlay/Tick, native
calibration and movement graphs are unchanged; no Mimic or AFCore assets edited.

Strict Blueprint compile passed. Temporary-editor-pawn tests use native calibration
at four offsets, verify lower bound, upward recovery, preserved X/Y and calibration,
hitch limiting, and inherited Custom Crouch dispatch staying within bounds.
Script: Tools/Lighting/test_bounded_seated_crouch.py. Headset acceptance pending.

Test: start seated, click left stick to calibrate, fully crouch with right stick,
then raise with right stick. Check feet, knees, hands and floor contact while
walking. This does not add automatic startup calibration or fix the separately
identified nameplate identity-source conflict. No packaged builds deployed.

## Live Follow-up

Full right-stick-down capture (20:21:51 UTC): stored seated offset remained
59.46 cm, but current Height Offset became -66.58 cm (126.04 cm reduction).
Camera-relative height stayed 112.87 cm; camera world Z was 507.99 cm and
VR Origin Z 461.70 cm (46.29 cm head height above origin). Capsule half-height
was 28.30 cm; its bottom remained approximately Z463.85 cm. Walking mode and
zero velocity persisted. Compared with calibrated standing, floor/origin moved
only about 0.23 cm, not a floor fall-through. The excessive synthetic crouch
range is verified; individual foot/IK penetration has not been measured.
Recommended next correction: WMCYN-owned override of native Custom Crouch with
a bounded seated crouch depth relative to the calibrated offset. Preserve the
native calibration, floor-following capsule and tracking hierarchy. Apply outside
PIE and validate shallow crouch, full input, recovery, hand alignment and walking.

User reports left-stick click restores floor alignment, with slightly bent knees;
right-stick down lowers the avatar into the floor. A calibrated stationary PIE
snapshot at 20:19 UTC confirmed the production pawn, Seated mode, stored/current
Height Offset 59.46 cm, camera-relative Z 113.68 cm, VR Origin world Z 461.94 cm,
capsule half-height 91.73 cm, Walking mode and zero velocity. IsCrouched=false.
This proves calibration executes despite the incomplete static input inspection
below. Do not add a duplicate calibration binding based on that inspection.
The crouched snapshot is pending; no tracking changes applied during headset use.

Concurrent nameplate regression: live PlayerState engine name was `jared`, while
WMCYN Username and DisplayName were both `AP-1743`. The WMCYN PlayerState
Updated_PlayerName_Event is bound to Comp_PlayerInfo_Basic and overwrites both
identity fields on authority. ResolvePreferredDisplayName then prefers DisplayName
over engine PlayerName. This is an identity-source conflict, not a font defect.
Next repair should keep verified account identity authoritative and stop generic
AFCore name notifications from overwriting it; do not hardcode jvredmoore or treat
arbitrary client-provided names as authenticated identity. Confirm verified account
username versus profile display name before selecting the visible label.

## Verified in Editor

- WMCYN native pawn default PlayMode is Seated.
- Child BeginPlay/Tick call the native parent; no replacement calibration graph.
- Native Calibrate Player Height sets Height Offset to FullBody_Component marker
  height minus camera-relative height in Seated mode, and stores the result.
- Native Custom Crouch also changes Height Offset. IsCrouched=false alone cannot
  exclude an intentionally lowered pose.
- Dynamic Adjustments derives capsule height and movement speed from tracked
  height plus that offset; default Blueprint cache values of zero are not runtime evidence.
- Native IMC default mappings include left-stick calibration for Valve Index,
  but no Quest calibration mapping was observed. The WMCYN IMC has no matching
  Mimic calibration/crouch/move actions. Runtime-added bindings remain unverified.
- No PIE world was active. No runtime pose or headset acceptance is claimed.

## Next Hardware Check

1. Start VR Preview seated, log in, and remain stationary facing the mirror.
2. Run Tools/Lighting/audit_seated_runtime.py through the editor Python console.
3. Click the left thumbstick once, then capture another snapshot. Determine
   whether Height Offset changed and whether the standing posture recovered.
4. Walk on a flat floor, stop, and capture again. Check whether offset or floor
   alignment drifts. Test right-stick input separately for custom crouch effects.
5. If click does nothing, inspect active Enhanced Input bindings before adding
   a WMCYN-owned binding to the existing native calibration function.

Do not change source Mimic/AFCore assets, scale the camera, or add per-tick body
height compensation. No gameplay assets or locomotion settings changed in this check.
