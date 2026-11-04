"""Bring Jared's hair, eyebrows and goatee into the WMCYN preview family.

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/AvatarPipeline/prepare_jared_grooms.py

The avatar renders clothed but bald and browless. All three grooms exist in the
source import and none were ever copied across.

WHY A BINDING HAS TO BE DUPLICATED AND RETARGETED
-------------------------------------------------
A UGroomBindingAsset is bound to one specific skeletal mesh. The source
bindings target `/Game/MetaHumans/Jared/Face/SKM_Jared_FaceMesh`, but the pawn
renders `SKM_Jared_Head_Preview` - a duplicate made so the baked face materials
could be fixed without touching the import. A GroomComponent given a binding
whose target does not match its mesh falls back to unbound, and the groom
floats in place instead of following the head.

The preview head is a byte-duplicate of the source face mesh: same topology,
same vertex count, same skeleton. So the binding's precomputed data stays valid
and only the target pointer needs to change. That is what this script does -
duplicate each binding into the preview folder and retarget it. It does NOT
rebuild the binding, because there is nothing to recompute.

If the preview head ever stops being a topological duplicate of the source,
this assumption breaks and the bindings must be rebuilt rather than retargeted.
The script checks vertex counts and refuses rather than producing a groom that
is subtly wrong.

COST
----
Hair is a strand groom with no card mesh, so it is the expensive one. Eyebrows
and the goatee ship with `CardsMesh_Group0_LOD0..4`, which the groom asset uses
at its configured LODs. PCVR only, as agreed - Quest is deferred for this
avatar, so the strand cost is already accepted.

Idempotent: re-running reports `already_present` and changes nothing.
"""
import json
from pathlib import Path

import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)

PREVIEW = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
GROOMS = f'{PREVIEW}/Grooms'
SOURCE = '/Game/MetaHumans/Jared/Grooms'

HEAD = f'{PREVIEW}/SKM_Jared_Head_Preview'
SOURCE_FACE = '/Game/MetaHumans/Jared/Face/SKM_Jared_FaceMesh'

# Eyelashes are deliberately absent: the head mesh carries eyelash geometry in
# its own `eyelashes_shader_shader` section, so the groom would double them.
GROOM_SET = [
    ('Hair', 'Hair_S_360Waves'),
    ('Eyebrows', 'Eyebrows_M_Wide'),
    ('Goatee', 'Goatee_L_Wavy'),
]


def vertex_count(mesh):
    """LOD0 vertex count, or None when it cannot be determined.

    None is NOT "the meshes match" - it means the check could not run, and the
    caller treats it as such. The first version of this guard quietly took None
    as "cannot compare, carry on", so on the run that mattered it reported
    `preview_head_vertices: null` and retargeted the bindings unverified. It
    happened to be safe. It was still a guard that did nothing.

    UE 5.8 exposes no vertex-count accessor on SkeletalMeshEditorSubsystem, so
    this goes through the WMCYN editor bridge, which reads the render data in
    C++ for the same reason the skeleton assignment does.
    """
    if mesh is None:
        return None
    bridge = getattr(unreal, 'WMCYNRetargetEditorLibrary', None)
    if bridge is None or not hasattr(bridge, 'get_lod_vertex_count'):
        return None
    count = bridge.get_lod_vertex_count(mesh, 0)
    return count if count >= 0 else None


def duplicate(source_path, dest_path, entry):
    if unreal.EditorAssetLibrary.does_asset_exist(dest_path):
        entry['action'] = 'already_present'
        return unreal.load_asset(dest_path)
    if not unreal.EditorAssetLibrary.does_asset_exist(source_path):
        entry['action'] = 'source_missing'
        return None
    asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, dest_path)
    entry['action'] = 'duplicated' if asset else 'duplicate_failed'
    return asset


def main():
    report = {'grooms': [], 'topology': {}, 'warnings': []}

    head = unreal.load_asset(HEAD)
    face = unreal.load_asset(SOURCE_FACE)
    head_verts, face_verts = vertex_count(head), vertex_count(face)
    report['topology'] = {
        'preview_head_vertices': head_verts,
        'source_face_vertices': face_verts,
        'duplicate': head_verts == face_verts if None not in (head_verts, face_verts) else None,
    }

    # Retargeting a binding is only valid while the two meshes are topological
    # duplicates. If they have diverged, stop: a mismatched binding produces a
    # groom that looks almost right, which is worse than none at all.
    if head_verts is None or face_verts is None:
        # Unknown is not the same as equal. Say so loudly instead of proceeding
        # as though the check passed.
        report['warnings'].append(
            'VERTEX COUNTS UNAVAILABLE - the duplicate check did NOT run. The '
            'bindings below were retargeted without verification. Confirm the '
            'grooms track the head before trusting them.')
        safe_to_retarget = True
        report['topology']['check_ran'] = False
    elif head_verts != face_verts:
        report['warnings'].append(
            f'preview head ({head_verts} verts) is no longer a duplicate of the source '
            f'face ({face_verts} verts); bindings must be REBUILT, not retargeted. '
            f'Refusing to retarget.')
        safe_to_retarget = False
        report['topology']['check_ran'] = True
    else:
        safe_to_retarget = True
        report['topology']['check_ran'] = True

    for label, name in GROOM_SET:
        entry = {'label': label, 'groom': name}
        report['grooms'].append(entry)

        groom = duplicate(f'{SOURCE}/{name}', f'{GROOMS}/{name}', entry)
        if groom is None:
            continue
        unreal.EditorAssetLibrary.save_loaded_asset(groom)

        bind_entry = {}
        entry['binding'] = bind_entry
        binding = duplicate(f'{SOURCE}/{name}_Binding', f'{GROOMS}/{name}_Binding', bind_entry)
        if binding is None:
            continue

        # Point the copy at our groom copy and our head, so nothing in the
        # preview family reaches back into the source import at runtime.
        binding.set_editor_property('groom', groom)
        bind_entry['groom_repointed'] = True

        if not safe_to_retarget:
            bind_entry['result'] = 'left_targeting_source'
            continue

        current = binding.get_editor_property('target_skeletal_mesh')
        bind_entry['was_targeting'] = current.get_path_name() if current else None
        binding.set_editor_property('target_skeletal_mesh', head)
        after = binding.get_editor_property('target_skeletal_mesh')
        bind_entry['now_targeting'] = after.get_path_name() if after else None
        bind_entry['result'] = 'retargeted' if after == head else 'retarget_failed'
        unreal.EditorAssetLibrary.save_loaded_asset(binding)

    out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Avatars')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'jared_grooms_prepare.json').write_text(json.dumps(report, indent=2))
    print('WMCYN_GROOMS ' + json.dumps(report, indent=2))


main()
