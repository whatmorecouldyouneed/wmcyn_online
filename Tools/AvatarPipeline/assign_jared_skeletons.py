"""Give the Jared preview meshes their skeletons.

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/AvatarPipeline/assign_jared_skeletons.py

or paste into the editor's Python console.

`SKM_Jared_Body_Preview` and `SKM_Jared_Head_Preview` were duplicated out of the
MetaHuman import without their skeleton links, so both carry `Skeleton = None`.
Unreal then refuses to drive them:

    SkeletalMesh ... has no skeleton. This needs to be fixed before an animation
    can be set

which stops the retarget AnimBlueprint dead, whatever the avatar manifest says.

An earlier attempt added `[Core.Redirects]` entries in `Config/DefaultEngine.ini`
pointing the previews' byte-clone skeleton assets at the real MetaHuman ones.
That cannot work: a redirect rewrites a reference to an object that moved, and
these properties are not stale references, they are null. There is nothing to
redirect. Those two entries are dead weight and should be removed.

The skeletons assigned here are the same ones the working `WMCYNAvatar_52b64e54`
family already uses, and `Saved/Diagnostics/Avatars/jared_skeleton_check.json`
recorded the previews as bone-identical to that family - 342/342 body,
875/875 head, zero mismatches - so the merge is a formality rather than a
retarget.

Note both skeletons live in the MetaHumanCharacter *engine plugin*, not project
content. The 52b64e54 meshes already depend on them, so this adds no new
packaging requirement, but that plugin's content must be cooked into any
packaged build.
"""
import json
from pathlib import Path

import unreal

PREVIEW = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
BODY_SKELETON = '/MetaHumanCharacter/Female/Medium/NormalWeight/Body/metahuman_base_skel'

ASSIGNMENTS = [
    (f'{PREVIEW}/SKM_Jared_Body_Preview', BODY_SKELETON),
    (f'{PREVIEW}/SKM_Jared_Head_Preview',
     '/MetaHumanCharacter/Face/Face_Archetype_Skeleton'),
    # The garments carry the same 342-bone body tree, so they take the body
    # skeleton and can then follow it by leader pose.
    (f'{PREVIEW}/SKM_Jared_Outfit_Preview', BODY_SKELETON),
    (f'{PREVIEW}/SKM_Jared_Shoes_Preview', BODY_SKELETON),
]


def bone_count(skeleton):
    """Bone total, used to prove the shared skeleton was not altered."""
    try:
        return len(skeleton.get_editor_property('bone_tree'))
    except Exception:
        return None


def describe(skeleton):
    return skeleton.get_path_name() if skeleton else None


def main():
    report = []
    for mesh_path, skeleton_path in ASSIGNMENTS:
        entry = {'mesh': mesh_path, 'skeleton': skeleton_path}
        report.append(entry)

        mesh = unreal.load_asset(mesh_path)
        skeleton = unreal.load_asset(skeleton_path)
        if mesh is None or skeleton is None:
            entry['result'] = 'missing_asset'
            entry['mesh_loaded'] = mesh is not None
            entry['skeleton_loaded'] = skeleton is not None
            continue

        entry['before'] = describe(mesh.get_editor_property('skeleton'))
        if entry['before']:
            entry['result'] = 'already_assigned'
            continue

        # USkeletalMesh::Skeleton is read-only to Python and to the editor
        # property bridge, so this goes through the WMCYN editor library, which
        # merges the bone tree and calls SetSkeleton in C++. Same reason that
        # module exists for the retarget AnimGraph.
        bridge = getattr(unreal, 'WMCYNRetargetEditorLibrary', None)
        if bridge is None:
            entry['result'] = 'bridge_unavailable'
            continue

        # These skeletons live in the MetaHumanCharacter engine plugin, under
        # Program Files, and are shared by everything MetaHuman. The bridge
        # merges the mesh's bone tree in, which *adds* any bones the skeleton is
        # missing - so a garment that is not actually body-bound would silently
        # mutate a shared engine asset. Record the count either side; a change
        # means stop and look, not carry on.
        entry['skeleton_bones_before'] = bone_count(skeleton)
        ok = bridge.assign_skeletal_mesh_skeleton(mesh, skeleton)
        entry['skeleton_bones_after'] = bone_count(skeleton)
        if entry['skeleton_bones_before'] != entry['skeleton_bones_after']:
            entry['result'] = 'aborted_skeleton_would_change'
            entry['warning'] = (
                'the shared engine skeleton gained bones; this mesh is not '
                'body-bound and must not be forced onto it')
            continue
        entry['assigned_via_bridge'] = bool(ok)
        entry['after'] = describe(mesh.get_editor_property('skeleton'))
        entry['result'] = 'assigned' if entry['after'] else 'failed'

        if entry['after']:
            saved = unreal.EditorAssetLibrary.save_loaded_asset(mesh)
            entry['saved'] = bool(saved)
            # The skeleton gains the bone tree, so it is dirty too.
            unreal.EditorAssetLibrary.save_loaded_asset(skeleton)

    out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Avatars')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'jared_skeleton_assign.json').write_text(json.dumps(report, indent=2))
    print('WMCYN_SKELETON_ASSIGN ' + json.dumps(report, indent=2))


main()
