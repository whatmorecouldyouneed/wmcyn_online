# Crib Lighting Automation

Run only in UE 5.8 with `L_WMCYNOnline` open and PIE stopped. Save unrelated
work first. The batch saves the map, its lighting data, and six WMCYN mesh
duplicates. It does not change renderer config, AFCore, Mimic, or source meshes.

From the Unreal **Cmd** console (not the Python-only console):

```text
py "C:/Users/jvred/OneDrive/Documents/Unreal Projects/wmcyn_online/Tools/Lighting/run_crib_lighting.py"
```

Change the checkout path on other machines. Add `--quality preview` for a quick
iteration; the default is `production`. No per-fixture editor work is needed.

The runner surveys connected geometry islands, identifies 73 recessed discs
and ten pendant faces by source component and measured dimensions, and places
downward Static spotlights just below them. It updates actors by stable label.
If the fixture count changes, it stops for review instead of guessing. It does
not remove retired fixtures automatically. Positions are world-space, including
the imported components' actual transforms.

Six floor/interior meshes with demonstrated overlapping lightmaps are duplicated
under `/Game/WMCYN/Environment/Crib/Meshes/SM_Crib_Lightmap_Scene_*`. Box projection
on channel 1 feeds regenerated channel 2 lightmap UVs. Texture UV0, triangle
positions, material slots, and collision settings are preserved and validated.
The original imported mesh assets are untouched. Lightmap-only channel 1 is
replaced on these duplicates; do not use this recipe blindly on other assets
whose materials rely on UV1.

The final checks require 83 unique Static lights facing down and six mesh
overrides with matching oriented triangles, texture UV0, material slots, and
collision complexity/simple/convex counts. Run `validate_crib_lighting.py`
separately after reopening the map to check persistence without editing assets.

Diagnostics and bake status are generated under `Saved/Diagnostics/Lighting`.
Source backups from the initial pass are in its `Before` directory on Jared's
machine; Git retains the original committed map as well. The scripts never load
or overwrite that backup. A successful bake does not imply zero UV warnings or
Quest performance acceptance. See `Docs/CRIB_BASIC_LIGHTING_PLAN.md` for results
and the remaining checks.
