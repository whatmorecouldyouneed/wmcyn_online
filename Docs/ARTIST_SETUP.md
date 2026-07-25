# Artist / Level Collaborator Setup

For working on the Crib environment (lights, furniture, posters, cleanup) in the
real `L_WMCYNOnline` level. No C++ toolchain and no VR headset required.

## One-time setup

1. Install **Unreal Engine 5.8** from the Epic Games Launcher.
2. Install Git LFS once per machine: `git lfs install`
3. Clone, then pull the real asset files:

```bash
git clone git@github.com:whatmorecouldyouneed/wmcyn_online.git
cd wmcyn_online
git lfs pull
```

`git lfs pull` is the step people miss. Without it every `.uasset` and `.umap`
is a small text pointer, so the Content Browser looks empty or broken.

4. Double-click `wmcyn_online.uproject`. It should open straight to
   `/Game/Levels/L_WMCYNOnline` with no prompts.

You do **not** need Visual Studio. The project's one C++ plugin
(`WMCYNRuntime`) ships prebuilt in the repo.

## Every session

```bash
git pull
git lfs pull
```

## If you see "The following modules are missing or built with a different engine version"

Stop — do not click Yes, and do not open or save the level.

It means your engine build differs from the one the committed plugin was built
against. Compare these two files; the `BuildId` values must match:

- `Plugins/WMCYNRuntime/Binaries/Win64/UnrealEditor.modules` (currently `55116800`)
- `<your engine>/Engine/Binaries/Win64/UnrealEditor.modules`

Send Jared both values. He rebuilds the plugin against the matching engine and
commits it, then `git pull` fixes you. (Clicking Yes only works if you have
Visual Studio with the C++ game development workload.)

**Why it matters:** if the plugin fails to load, actors in the level that
reference WMCYN classes come in as missing/null. Saving in that state can strip
them permanently. If the plugin didn't load, close the editor without saving.

## Level editing etiquette

`.umap` and `.uasset` are binary. **Git cannot merge them.** If two people edit
`L_WMCYNOnline` at the same time, one person's work is lost — there is no
merge, only "whose version wins."

So:

- Say in chat that you're taking the level before you start.
- `git pull` immediately before you begin editing.
- Commit and push as soon as you're done; don't sit on level changes overnight.
- New standalone assets (meshes, materials, textures) are safe to work on any
  time — the conflict risk is the level file itself.

## What's yours vs. hands-off

**Yours:** lights, furniture, posters, decorative meshes and their materials,
overall look and cleanup of the Crib. Put new WMCYN-authored content under
`/Game/WMCYN/Environment/`.

**Hands-off:**

- `/Game/WMCYN/Core`, `/Game/WMCYN/Pawns`, `/Game/WMCYN/UI` — gameplay, login,
  presence, and networking assets.
- The four `SPAWN_FirstSignal_*` markers and `PLAYERSTART_FirstSignal_Default`
  in the level — these are where users enter the world.
- `/Game/AFCore` and `/Game/FullBodyVRTemplate` — licensed framework content,
  not to be edited (make a WMCYN copy if something needs changing).
- `Plugins/`, `Config/`, `.github/` — build and runtime configuration.

## Working without a headset

The editor and desktop Play-In-Editor work normally without VR. Don't use
"VR Preview" — that needs a headset. The full-body avatar solver stays idle
without an HMD, which is expected, not a bug.

## Quest performance guardrails

The Crib ships to standalone Quest, which is mobile-class hardware. When
adding set dressing:

- Prefer **Static** lights and baked lighting; avoid piles of movable lights.
- Keep textures at 2K or smaller; reuse materials rather than making one per
  object.
- Watch triangle counts and total object count — many small props are more
  expensive than a few larger ones.
- Skip expensive post-processing, translucency stacks, and dynamic shadows.

When in doubt, ask before adding something heavy — we measure Quest frame
timing before the next multi-user test.

## Note

The older `codex/noah-ue56-crib-set-dressing` branch (a UE 5.6 workaround kit)
is superseded now that 5.8 is available. Work on `main`.
