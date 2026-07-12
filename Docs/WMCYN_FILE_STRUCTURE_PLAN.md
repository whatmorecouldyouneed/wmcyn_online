# WMCYN File Structure Plan

## 1. Current Observed Structure

At the time of this setup, the project root already contained:

- `AGENTS.md`
- `Docs/`
- `Content/AFCore/`
- `Content/Blueprints/`
- `Content/Collections/`
- `Content/Developers/`
- `Content/Environments/`
- `Content/JaredMetaHuman/`
- `Content/Levels/`
- `Content/Skills/`

Important existing Unreal assets observed but intentionally not touched:

- `Content/Levels/L_WMCYNOnline.umap`
- `Content/Levels/L_crib.umap`
- all assets under `Content/AFCore/`
- any existing Crib/static FBX assets

## 2. Created Structure

Documentation/spec folders:

- `Docs/`
- `Specs/`
- `Specs/000-constitution/`
- `Specs/001-avf-class-selection/`
- `Specs/002-crib-environment-integration/`

WMCYN Unreal content scaffold:

- `Content/WMCYN/`
- `Content/WMCYN/Core/`
- `Content/WMCYN/Pawns/`
- `Content/WMCYN/Roles/` (legacy scaffold name; use it for presence/capability data only if keeping the folder is easier than renaming)
- `Content/WMCYN/UI/`
- `Content/WMCYN/Production/`
- `Content/WMCYN/Verbatim/`
- `Content/WMCYN/Environment/`
- `Content/WMCYN/Environment/Crib/`
- `Content/WMCYN/Environment/Crib/Meshes/`
- `Content/WMCYN/Environment/Crib/Materials/`
- `Content/WMCYN/Environment/Crib/Lighting/`
- `Content/WMCYN/Environment/Crib/Collision/`
- `Content/WMCYN/Environment/Crib/Props/`
- `Content/WMCYN/Data/`
- `Content/WMCYN/Dev/`

`.gitkeep` files were added only to newly created empty leaf folders so Git can track the scaffolding.

## 3. What Remains Manual Inside Unreal Editor

- Refresh or reopen the Content Browser so `Content/WMCYN` appears.
- Create any real Unreal assets from inside the Editor, not by hand-writing `.uasset` files.
- Use `L_WMCYNOnline` and the visible Crib as the target integration context.
- Keep AFCore as the working baseline and create WMCYN wrapper/child assets only when needed.
- Place Crib stabilization helpers manually using `Docs/CRIB_STABILIZATION_PASS_001.md`.

## 4. What Must Remain Untouched

- Do not modify, move, or rename anything under `Content/AFCore/`.
- Do not modify `Content/Levels/L_WMCYNOnline.umap` unless explicitly asked.
- Do not modify `Content/Levels/L_crib.umap` unless explicitly asked.
- Do not move the Crib FBX or any existing asset.
- Do not edit config files unless explicitly asked.
- Do not rebuild locomotion, grab, hand, menu, or base multiplayer systems.
- Do not delete existing folders or assets.

## 5. Naming Conventions for WMCYN Files and Assets

Use clear prefixes and keep WMCYN-specific work under `Content/WMCYN/`.

- Blueprints: `BP_WMCYN_<Purpose>`
- Child/wrapper pawns: `BP_WMCYN_Pawn_<PresenceModeOrCapability>`
- Presence/capability assets/data: `DA_WMCYN_Presence_<Mode>` or `DA_WMCYN_Capability_<Name>`
- UI widgets: `WBP_WMCYN_<Purpose>`
- Production/camera assets: `BP_WMCYN_Production_<Purpose>` or `BP_WMCYN_Camera_<Purpose>`
- Verbatim assets: `BP_WMCYN_Verbatim_<Purpose>` or `DA_WMCYN_Verbatim_<Purpose>`
- Data assets: `DA_WMCYN_<Purpose>`
- Crib meshes/material instances: `SM_WMCYN_Crib_<Name>`, `MI_WMCYN_Crib_<Name>`
- Lighting helpers: `LIGHT_WMCYN_Crib_<Purpose>`
- Collision helpers: `COL_WMCYN_Crib_<Purpose>`

Recommended presence/capability names:

- `Quest`
- `PCVR`
- `Recording`
- `CanTriggerVerbatimMarker`

## 6. Next Manual Unreal Steps

1. Open `L_WMCYNOnline` in Unreal Engine 5.8.
2. Confirm `Content/WMCYN` appears in the Content Browser.
3. Do the Crib stabilization pass from `Docs/CRIB_STABILIZATION_PASS_001.md`.
4. Do not touch multiplayer, presence/capability logic, Verbatim, or custom Blueprints during the stabilization pass.
5. After one readable Crib conversation area is stable, choose AFCore presence baselines for:
   - standalone VR users
   - PCVR recording user
6. Only then create the smallest WMCYN wrapper/child assets needed under `Content/WMCYN`.
