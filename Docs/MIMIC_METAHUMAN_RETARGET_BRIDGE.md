# Mimic to MetaHuman Retarget Bridge

## Purpose

The proven Mimic `BP_VRBodyCharacer` remains the First Signal pawn and tracking authority. The WMCYN MetaHuman body is a visual presentation layer that receives the live Mimic pose through Unreal's IK Retargeter.

## Saved WMCYN assets

- Source IK Rig: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/Retargeting/IKR_WMCYN_Mimic_Source`
- Target IK Rig: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/Retargeting/IKR_WMCYN_MetaHuman_Target`
- Retargeter: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/Retargeting/RTG_WMCYN_Mimic_To_MetaHuman`
- Presentation AnimBlueprint: `/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation`

The bridge maps root, spine, neck, head, both arms, and both legs. Its AnimGraph is `Retarget Pose From Mesh -> AnimGraph Root`, using `ParentSkeletalMeshComponent` mode.

## Runtime shape

`UWMCYNFirstSignalPresenceComponent` keeps Mimic's native BodyMesh instantiated and ticking, hides that native visual when the WMCYN avatar is active, and attaches the WMCYN body visual directly to the native BodyMesh. The saved retarget node discovers that parent component and copies its live pose. No AFCore asset is edited.

## Package

The stage-only Windows archive was created at:

`D:\WMCYN_Packages\WMCYN_MimicMetaHumanBridge_20260731_StageOnly`

Pak inspection confirmed the body, head, IK rigs, retargeter, and presentation AnimBlueprint are present. The full fresh cook was started but exceeded the local 20-minute build window while processing the large MetaHuman shader/texture set; the archive uses the existing cooked output that already contained the bridge assets.

## Next test

Run the archived Windows build in the editor/PCVR path first. Confirm the MetaHuman visual follows Mimic head, hands, torso, and legs without changing locomotion, floor behavior, scale, or possession. Then repeat the same check on Quest after a platform-specific cook.
