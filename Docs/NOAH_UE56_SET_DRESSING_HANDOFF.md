# Noah UE 5.6 Set-Dressing Handoff

Branch: `codex/noah-ue56-crib-set-dressing`

## Purpose

Noah decorates The WMCYN Crib in Unreal Engine 5.6 without opening or modifying the UE 5.8 gameplay project. This branch contains a small UE 5.6 artist workspace and the original Crib source FBX.

The FBX is reference geometry. Noah owns lights, furniture, props, decals, signage, and supporting materials created under the Noah namespace. Jared performs the final UE 5.8 integration into `/Game/Levels/L_WMCYNOnline`.

## First Setup

```powershell
git fetch origin
git switch codex/noah-ue56-crib-set-dressing
git pull --ff-only
git lfs install
git lfs pull
```

Open this project with Unreal Engine 5.6:

`Handoffs/Noah/UE56/WMCYN_Crib_SetDress_UE56/WMCYN_Crib_SetDress_UE56.uproject`

Do not open the root `wmcyn_online.uproject` in UE 5.6. It is the production UE 5.8 project.

## Import The Crib Reference

Source file:

`Handoffs/Noah/UE56/WMCYN_Crib_SetDress_UE56/SourceArt/Crib/wmcyn_crib_default_unity_transfer.fbx`

1. In UE 5.6, use **File > Import Into Level**.
2. Import into `/Game/WMCYN/Reference/Crib`.
3. Preserve the FBX scene hierarchy; do not combine the room into a new authored mesh.
4. Do not import materials or textures unless needed for orientation.
5. Keep the imported scene actor at location `(0, 0, 0)`, rotation `(0, 0, 0)`, scale `(1, 1, 1)`.
6. Put the reference actor in Outliner folder `NOAH_00_REFERENCE_LOCKED` and lock it.
7. Never edit or migrate the reference mesh assets back into the production project.

The reference exists only to preserve scale, walls, floors, doorways, sight lines, and placement coordinates.

## Authored Content Contract

Create all original work under:

`/Game/WMCYN/Environment/Crib/Noah/`

Use these folders:

- `Blueprints`
- `Decals`
- `Lighting`
- `Materials`
- `Meshes`
- `Props`
- `Textures`
- `Maps`

Create the working level as:

`/Game/WMCYN/Environment/Crib/Noah/Maps/L_WMCYN_Crib_SetDress_Noah`

Suggested Outliner folders:

- `NOAH_00_REFERENCE_LOCKED`
- `NOAH_10_ARCHITECTURE_ADDITIONS`
- `NOAH_20_FURNITURE`
- `NOAH_30_PROPS`
- `NOAH_40_LIGHTING`
- `NOAH_50_DECALS_SIGNAGE`
- `NOAH_90_NOTES`

Keep authored actors at their final world transforms. Do not move the Crib reference to make placements convenient.

## Naming

- Static meshes: `SM_NOAH_*`
- Blueprints: `BP_NOAH_*`
- Materials: `M_NOAH_*`
- Material instances: `MI_NOAH_*`
- Textures: `T_NOAH_*`
- Levels: `L_WMCYN_Crib_SetDress_Noah`
- Placed actor labels: `NOAH_*`

## Do Not Touch

- Root UE 5.8 project files
- `/Game/AFCore/*`
- `/Game/FullBodyVRTemplate/*`
- `L_WMCYNOnline`
- login, voice, avatars, networking, collision, spawn markers, or runtime Blueprints
- the Crib reference transform or source FBX

## Daily Git Flow

Before work:

```powershell
git switch codex/noah-ue56-crib-set-dressing
git pull --ff-only
git lfs pull
```

After a focused change:

```powershell
git status --short
git lfs status
git add Handoffs/Noah/UE56/WMCYN_Crib_SetDress_UE56/Content/WMCYN/Environment/Crib/Noah
git commit -m "Noah: describe the set-dressing change"
git push origin codex/noah-ue56-crib-set-dressing
```

Never commit `Saved`, `Intermediate`, `DerivedDataCache`, or `Binaries`.

## Delivery Gate

- Working level opens in UE 5.6 without missing authored assets.
- Reference Crib remains at identity transform.
- All deliverable assets are under the Noah namespace.
- Lights and materials are suitable for the Quest-first performance target.
- No gameplay or framework assets are included.
- Branch is pushed and `git lfs status` has no unpushed objects.

Jared's 5.8 integration checklist is in `Handoffs/Noah/INTEGRATION_CHECKLIST.md`.
