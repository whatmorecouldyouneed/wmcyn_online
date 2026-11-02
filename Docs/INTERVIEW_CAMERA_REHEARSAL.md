# Interview Camera Rehearsal

## Checkpoint: November 2, 2026

Distribution is closed per founder; Gio and Cole are green. Today's goal is a
usable recording, not another avatar/distribution pass. Headset microphones only.

The camera keeps the existing WMCYN actor, stable controller grip and one shared
render target. No AFCore/Mimic assets, tracking, furniture or world configuration
changed. Save/select slots already existed in native code but their optional
UMG controls were missing. This pass adds `CameraShotSelector`,
`CameraShotSaveButton` and `CameraShotSelectButton` to the Designer tree in
`/Game/WMCYN/UI/Settings/WBP_WMCYN_Settings_FirstSignal`. They use existing
button styling and appear in the camera section while the camera is enabled.

- Master, Interviewer and Guest store world pose plus FOV for this pawn lifetime.
  Respawn/world travel loses presets; save again after respawning.
- Menu closure must not disable capture or move a released camera.
- FOV UI follows saved shots rather than resetting the lens on camera toggle.
- Nameplates remain in output; other world widgets and camera prop are excluded.
- Losing possession can shut down capture even after local ownership is gone.
- Console `wmcyn.Camera.CaptureHeight 360`, `720`, or `1080` changes the same
  target to 640x360, 1280x720 or 1920x1080. Default/invalid value uses 360p.
  While enabled, changes apply within one second. This is a test control, not
  automatic quality selection; no new permanent capture or video encoder.
- Online camera controls retain the verified Recording capability gate. Local
  non-Shipping standalone preview is allowed without granting online authority.

## First Short Take

1. PCVR: log in, open the existing menu, enable camera and recall within reach.
2. Grip near the camera with right side grip, aim at couch, release. Walk away;
   framing must stay fixed. Verify no nearby door also moves. Test seated/standing.
3. Save Master, then aim/save Interviewer and Guest. Select each; confirm framing
   and FOV return. Open/close menu and check shot remains still.
4. Enable PC spectator output. Headset view must stay unchanged. Check desktop,
   menu preview and camera display agree and no feedback image is captured.
5. Start at 360p for interaction. Measure `stat unit` / `stat gpu` with camera off,
   then 360/720/1080 on the same shot. Record GPU/frame times and dropped frames.
   Reject a quality level that compromises headset comfort; 360p is not HD proof.
6. OBS: capture the PC game/spectator window. Confirm actual source dimensions,
   not just a 1080p canvas stretching 360p. Target 1080p30 only after GPU check.
7. Record 60-90 seconds, switching among the three shots. Capture received game
   voices and operator microphone once each; listen for doubled voice/echo.
8. Stop and play the actual file. Check video, all voices, sync and camera cuts.
   Turn output off and confirm previous spectator mode returns; camera off must
   stop capture. Leave remote-avatar acceptance open until peers join the take.

## Evidence

- Compile: Launcher UE 5.8, BuildId 55116800, Windows; native build passed Nov 2.
- Automation: `WMCYN.UI.HandheldCamera` passed, 0 warnings/errors; NullRHI.
  `Saved/Diagnostics/Camera20261102/index.json` and `Saved/Logs/Camera20261102.log`.
- Covers grab/release/recall, tracking-loss release, synthetic door priority,
  three pose/FOV slots, shared target resizing, capture exclusions and shutdown.
- Editor: restarted with rebuilt DLL; Settings Blueprint compiled/saved, all
  three shot widgets confirmed as real variable-bound Designer controls. Editor
  screenshot inspected; this is not runtime headset layout/click acceptance.
- Pending: rendered orientation, real grip/door arbitration, headset comfort,
  GPU measurements, packaged PCVR, remote avatar/nameplate capture and OBS file.
- No camera acceptance inferred from Gio/Cole distribution success.

Record each result with date, build/commit, device, shot, capture resolution,
GPU/frame time, OBS missed frames, output path and pass/fail. Then proceed to
two-person seating; do not alter calibration to fake a seated body.
