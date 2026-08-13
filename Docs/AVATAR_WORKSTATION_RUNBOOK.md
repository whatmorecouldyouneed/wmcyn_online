# Avatar Workstation Runbook

Last updated: 2026-07-31

## What the mobile upload actually is

The current phone-side avatar upload is not a ready-made Unreal mesh.

The real payload we inspected from storage is a Live Link Face style archive:

- `.mov` video
- `depth_data.bin`
- `depth_metadata.mhaical`
- `frame_log.csv`
- `take.json`
- `thumbnail.jpg`

That means the missing lane is the Unreal workstation generation step:

1. ingest the take into Unreal
2. create MetaHuman capture data
3. create a MetaHuman identity from a neutral frame
4. conform/build a MetaHuman character
5. choose Quest and PCVR runtime outputs
6. publish the resolved manifest
7. let the game load that manifest after login

## Current WMCYN-owned tools

Added in this repo:

- `Tools/AvatarPipeline/wmcyn_metahuman_pipeline.py`
- `Tools/AvatarPipeline/Run-WMCYNAvatarPipeline.ps1`
- `Tools/AvatarPipeline/Publish-WMCYNAvatarRuntimeMesh.ps1`

These wrap Epic's official MetaHuman Python helpers instead of rebuilding the ingest/conform flow from scratch.

## Prerequisites

Before running the workstation lane:

- UE 5.8 source build is installed at `C:\UE58`
- this project opens successfully in that engine
- MetaHuman plugins are enabled
- the operator is logged into Epic/MetaHuman services inside the editor
- the raw take zip has been downloaded from Cloud Storage

Important limitation from Epic's own scripts:

- creating an identity from footage depends on the MetaHuman service login path
- that step is not a pure backend function
- the identity may be created before its DNA/service step is fully ready

## Phase 1: ingest take and create identity

Example:

```powershell
.\Tools\AvatarPipeline\Run-WMCYNAvatarPipeline.ps1 `
  -TakeZipPath "C:\path\to\take.zip" `
  -Phase identity `
  -IdentityName "TesterAvatarIdentity" `
  -NeutralFrame 120 `
  -BodyIndex 1
```

What this does:

- extracts the zip under `Saved/AvatarPipeline/Inputs`
- imports the Live Link Face archive into project content
- creates capture data
- creates a MetaHuman identity from the selected neutral frame
- writes a result file under `Saved/AvatarPipeline/Results`

If the result says `identityReadyForCharacterBuild = false`, the MetaHuman service round-trip is still pending. Rerun the character phase after the identity has DNA.

## Phase 2: build a MetaHuman character asset

Example:

```powershell
.\Tools\AvatarPipeline\Run-WMCYNAvatarPipeline.ps1 `
  -Phase character `
  -IdentityName "TesterAvatarIdentity" `
  -CharacterName "TesterAvatar" `
  -StoragePath "/Game/WMCYN/AvatarPipeline" `
  -CharacterPackagePath "/Game/MetaHumans/WMCYN" `
  -BuildPath "/Game/MetaHumans/WMCYN" `
  -CommonPath "/Game/MetaHumans/Common" `
  -PipelineQuality medium
```

What this does:

- loads the existing identity
- verifies that the identity has DNA
- creates or reuses a `MetaHumanCharacter` asset
- conforms from the identity
- requests auto-rigging
- downloads texture sources unless skipped
- runs the optimized build pipeline

Result files again land under `Saved/AvatarPipeline/Results`.

## Phase 3: extract WMCYN runtime geometry and materials

Run this after the character phase succeeds:

```powershell
.\Tools\AvatarPipeline\Run-WMCYNAvatarPipeline.ps1 `
  -Phase runtime `
  -IdentityName "WMCYNAvatar_52b64e54_Identity" `
  -CharacterName "WMCYNAvatar_52b64e54"
```

This uses UE 5.8's official `MetaHumanCharacterExportBlueprintLibrary` to:

- export the generated body and head skeletal meshes
- persist the generated face/body textures and material instances
- remap the exported mesh material slots to WMCYN-owned material assets
- write a machine-readable result to `Saved/AvatarPipeline/Results/*_runtime.json`

The current successful output is:

- Quest body: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Body.WMCYNAvatar_52b64e54_Body`
- Quest head: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Head.WMCYNAvatar_52b64e54_Head`
- PCVR body: same verified WMCYN body mesh path for now
- PCVR head: same verified WMCYN head mesh path for now
- exported materials: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/*_MaterialsExport_9`

Quest and PCVR intentionally share the exported mesh until a real Quest optimization pass exists. The result keeps
`animClassPath` empty because the MetaHuman body uses the MetaHuman base skeleton and the existing mannequin animation
blueprint is not compatible. Do not publish the old Quinn animation class against this mesh.

## Phase 4: WMCYN presentation wrapper and publish runtime mesh manifest fields

Initial runtime proof in WMCYN still uses the simpler `runtime_mesh` manifest lane.

The first WMCYN-owned presentation boundary now exists in `UWMCYNFirstSignalPresenceComponent`. It creates separate
transient body/head visuals, preserves the native Mimic pawn for movement and tracking, and accepts an optional
MetaHuman-compatible animation class. The first compatible class is:

`/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C`

It is reference-pose only for this checkpoint. Do not claim walking animation until its graph is authored and tested.

Only after that animation check, publish the mesh paths onto the avatar job:

```powershell
.\Tools\AvatarPipeline\Publish-WMCYNAvatarRuntimeMesh.ps1 `
  -JobId "52b64e54-4f1f-4b05-8ad1-84795576f543" `
  -QuestSkeletalMeshPath "/Game/Path/To/QuestMesh.QuestMesh" `
  -PCVRSkeletalMeshPath "/Game/Path/To/PCVRMesh.PCVRMesh"
```

That wrapper calls the backend tool in `wmcyn-backend-infra`:

- `functions/tools/publish-avatar-runtime-mesh.mjs`

The backend delivery object may include:

```json
{
  "skeletalMeshPath": "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Body.WMCYNAvatar_52b64e54_Body",
  "headSkeletalMeshPath": "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Head.WMCYNAvatar_52b64e54_Head",
  "animClassPath": "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation.ABP_WMCYN_MetaHuman_Presentation_C"
}
```

The explicit head field is preferred; the runtime can derive the sibling `_Head` path as a compatibility fallback.

## Current product truth

The runtime consumer now exists in source:

- login can request `GET /v1/avatar/manifest?platform=quest|pcvr`
- the pawn presence component can apply a returned `skeletalMeshPath`, optional `headSkeletalMeshPath`, and `animClassPath`
- WMCYN-owned runtime assets are isolated from the native Mimic visual and never replace its movement/tracking mesh directly

What is still not solved automatically:

- authoring and verifying locomotion behavior inside the WMCYN-owned animation class
- producing a genuinely Quest-optimized variant; both platforms currently use the verified shared mesh
- deciding whether ship quality should stay on `runtime_mesh` or move to cooked `pak` delivery

## Next concrete test

1. run Phase 1 on the real uploaded tester take
2. confirm the identity becomes DNA-ready
3. run Phase 2 to build the MetaHuman character asset
4. inspect `*_runtime.json` and confirm WMCYN-owned mesh/material paths
5. run the compiled wrapper with the generated body/head assets in editor/VR Preview
6. publish the verified mesh + compatible presentation animation manifest to the tester job
7. rebuild the Unreal runtime module if needed, log in again, and confirm the avatar apply log appears
8. author and test locomotion behavior on the MetaHuman skeleton before calling the avatar animated
