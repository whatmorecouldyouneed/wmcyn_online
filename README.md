# WMCYN Online

WMCYN Online is an Unreal Engine 5.8 persistent-world VR project. The first target is The WMCYN Crib: three standalone Quest users and one PCVR recording user enter the same internet-hosted world, see basic presence and nameplates, move with VR controls, use voice, and support OBS capture.

## Current baseline

- Map: `/Game/Levels/L_WMCYNOnline`
- World: one persistent Crib world; no host/guest or session-code flow
- Login: username or email plus password, then Enter World
- Native player source of truth: `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal`
- Body baseline: Mimic Pro `BP_VRBodyCharacer`, preserving its tracking, seated calibration, locomotion, footsteps, and floor behavior
- Framework baseline: HumanCodeable AFCore / Advanced VR Framework; AFCore assets are not edited by default
- Backend project: Firebase `wmcyn-online-mobile`
- Backend health endpoint: `https://us-central1-wmcyn-online-mobile.cloudfunctions.net/api/health`

## Avatar bridge

The WMCYN MetaHuman presentation layer is now connected to the live Mimic pose through WMCYN-owned IK assets:

- Source rig: `IKR_WMCYN_Mimic_Source`
- Target rig: `IKR_WMCYN_MetaHuman_Target`
- Retargeter: `RTG_WMCYN_Mimic_To_MetaHuman`
- Presentation AnimBlueprint: `ABP_WMCYN_MetaHuman_Presentation`

The Mimic pawn remains authoritative. The MetaHuman body is a visual wrapper driven by `Retarget Pose From Mesh`; it does not replace the playable pawn or modify AFCore.

See [Docs/MIMIC_METAHUMAN_RETARGET_BRIDGE.md](Docs/MIMIC_METAHUMAN_RETARGET_BRIDGE.md) for the bridge details and test gate.

## Build and test

Open `wmcyn_online.uproject` in UE 5.8 and test `L_WMCYNOnline`. For a source build, use the UE 5.8 toolchain documented in the project notes. A packaged build must complete a successful platform cook before staging; do not stage from a partial `Saved/Cooked` directory because missing global shader maps can crash at startup with a `WorldGridMaterial` error.

Useful checks:

```powershell
git lfs pull
git status --short
```

The project must be cooked for the target platform before packaging. After packaging, verify the archive contains the cooked global shader cache and the WMCYN avatar assets before headset testing.

## Documentation order

1. [AGENTS.md](AGENTS.md)
2. [Docs/CURRENT_STATE.md](Docs/CURRENT_STATE.md)
3. [Docs/FIRST_SIGNAL_TASKS.md](Docs/FIRST_SIGNAL_TASKS.md)
4. [Docs/MIMIC_METAHUMAN_RETARGET_BRIDGE.md](Docs/MIMIC_METAHUMAN_RETARGET_BRIDGE.md)
5. [Docs/WMCYN_ONLINE_PRD.md](Docs/WMCYN_ONLINE_PRD.md)

Do not commit `Saved`, `Intermediate`, `DerivedDataCache`, temporary exports, credentials, or local billing/payment data.
