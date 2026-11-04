"""Compare embedded mesh bone parents before repairing preview skeleton links."""
import json
from pathlib import Path
import unreal

root = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
baseline = '/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_'
report = []


def hierarchy(mesh):
    component = unreal.SkeletalMeshComponent()
    component.set_skeletal_mesh_asset(mesh)
    return {str(component.get_bone_name(i)): str(component.get_parent_bone(component.get_bone_name(i)))
            for i in range(component.get_num_bones())}


for part in ('Body', 'Head'):
    target = unreal.load_asset(root+'/SKM_Jared_'+part+'_Preview')
    reference = unreal.load_asset(baseline+part)
    assert target and reference
    target_bones = hierarchy(target)
    reference_bones = hierarchy(reference)
    mismatches = [{'bone': n, 'target_parent': p, 'reference_parent': reference_bones.get(n)}
                  for n, p in target_bones.items() if reference_bones.get(n) != p]
    compatible = bool(target_bones) and not mismatches
    entry = {'part': part, 'target_bone_count': len(target_bones),
             'reference_bone_count': len(reference_bones), 'mismatches': mismatches,
             'compatible_hierarchy': compatible, 'assigned': False}
    if compatible:
        skeleton = reference.get_editor_property('skeleton')
        # This is Epic's shared skeleton, not the prior account's geometry/DNA.
        assert skeleton and skeleton.get_path_name().startswith('/MetaHumanCharacter/')
        entry['assigned'] = target.get_editor_property('skeleton') == skeleton
        entry['skeleton'] = skeleton.get_path_name()
    report.append(entry)
out = Path(unreal.Paths.project_saved_dir())/'Diagnostics'/'Avatars'/'jared_skeleton_check.json'
out.write_text(json.dumps(report, indent=2))
print('WMCYN_JARED_SKELETON_CHECK', report)
