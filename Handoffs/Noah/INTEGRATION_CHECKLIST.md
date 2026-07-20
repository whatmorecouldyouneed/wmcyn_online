# Noah To UE 5.8 Integration Checklist

Use this after Noah pushes a stable set-dressing checkpoint.

1. Fetch the handoff branch without switching the production workspace while Unreal is open.
2. Create a temporary worktree or clean clone for `codex/noah-ue56-crib-set-dressing`.
3. Run `git lfs pull` in that checkout.
4. Make a backup branch before conversion.
5. Open `WMCYN_Crib_SetDress_UE56.uproject` with Unreal Engine 5.8 and allow the one-way project conversion.
6. Open `L_WMCYN_Crib_SetDress_Noah` and verify the reference Crib remains at identity transform.
7. Verify Noah's authored assets have no dependencies outside `/Game/WMCYN/Environment/Crib/Noah` except licensed engine/plugin content approved for production.
8. Migrate only Noah-authored assets and the Noah level into the UE 5.8 production project. Do not migrate `/Game/WMCYN/Reference/Crib`.
9. Open Noah's level and `L_WMCYNOnline` side by side, then copy only actors from Outliner folders prefixed `NOAH_` into the production map.
10. Place integrated actors under `WMCYN_20_Noah_SetDressing` in the production Outliner.
11. Validate Quest performance, lighting, collision, material compatibility, and missing references.
12. Commit the production integration on a new integration branch; do not merge Noah's 5.6 project files into `main` as runtime content.

The source FBX and reference-import assets are alignment aids, not production deliverables.
