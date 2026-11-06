# Basement bowling pilot

The two lanes fill the 27.61 m bay from approach to former couch area in
`L_WMCYNOnline`. Each has a full-size ball, ten physics pins and an independent
return/reset. The normal world-item grip is reused, not a new controller binding.
This is per-roll pinfall, not a ten-frame scorecard or the animated Unity pinsetter.

## Reproduce

- The purchased source remains local. Blender's `Tools/Bowling/prepare.py` reads the
  original FBXs and exports centered geometry under Saved. Run `place.py` using
  the full Unreal Editor's ExecutePythonScript mode, not a Python commandlet.
- After base placement, run Blender's `prepare_trim.py` and the editor's
  `fit_trim.py` with the Crib open. These orient the approach toward the pickups,
  fit the return, backstop and canopy, and place editor-only pin previews.
- Three wall probes agreed at X=-3178.1 cm. The deck ends at -3177 cm,
  leaving 1.1 cm to wall collision. The prefab's twin overhead TVs are visual
  fixtures only. The revised level references thirteen private packages under
  `Content/WMCYN/BowlingPrivate`.
  Do not commit the private packages or original prefab. Release cooks run
  `Tools/Bowling/Sync-PrivateAssets.ps1` against a hash-pinned runner cache.
  Seed a replacement runner under its service account using `-Seed` with the
  exact matched packages present. Regenerating asset packages can change hashes.
- Local smoke: start UnrealEditor-Cmd with the project and `/Game/Levels/L_WMCYNOnline
  -game -nullrhi -nohmd -nosound -unattended -WMCYNBowlingSmoke -benchmark -fps=60
  -seconds=18`. Expect ten upright pins, successful take/drop, a rolling body and
  nonzero pinfall. The scripted launch tests lane physics, not controller velocity.
- November 5: uncooked standalone smoke recorded ten upright pins and eight down.
- Windows Editor, Android ARM64 Development, and Linux dedicated server compiled.
  Android ASTC Crib cook exited successfully (0 errors, 286 warnings); all nine
  private packages exist in cooked content. No signed bowling release was
  promoted to ALPHA, and dedicated multiplayer physics has not been verified.
  That cook and smoke predate the visual fit; rerun both before promotion.

## Alpha acceptance (still manual)

1. Use matching server and clients. On Quest take a ball with side grip, swing
   toward the pins and release. Confirm light and strong throws differ.
2. From another device verify the held ball, release, pin impacts and reset.
   Repeat on the adjacent lane; its rack must be independent.
3. Check pickup after a gutter ball and after the thrower disconnects. Verify
   walking past the lanes and frame time with both racks active.
4. Record the tested Quest version code, server revision and results before
   describing this as working on the live Alpha channel.
