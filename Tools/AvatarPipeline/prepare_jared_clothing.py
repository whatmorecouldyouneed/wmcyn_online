"""Bring Jared's outfit and shoes into the WMCYN preview family.

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/AvatarPipeline/prepare_jared_clothing.py

The avatar renders but is naked: `prepare_jared_material_preview.py` duplicated
only the body and face meshes, so the two garments never reached the WMCYN
runtime folder. This duplicates them the same reversible way, leaving the source
import untouched.

The garments keep their own materials. Unlike the skin shader, which needed
default-lit baked stand-ins, `RiggedFit` already points at `Base1` with real
BaseColor/Normal/ORM maps under `Content/Characters/jARED/tex/`, and `Shoes`
at `g0`.

Skeletons are *not* assigned here - run `assign_jared_skeletons.py` afterwards,
which owns that step for the whole family and goes through the C++ bridge
because the property is read-only to Python.

The report records bone counts and bounds for both garments and the body,
because the garments' bounds put their long axis on Y while the body's is on Z.
If the ref poses really are unposed relative to the body, a leader-pose bind
will put the clothes on sideways, and that is far easier to catch here than in
a headset.
"""
import json
from pathlib import Path

import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)

PREVIEW = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
SOURCE = '/Game/MetaHumans/Jared'

GARMENTS = [
    ('Outfit', f'{SOURCE}/RiggedFit', f'{PREVIEW}/SKM_Jared_Outfit_Preview'),
    ('Shoes', f'{SOURCE}/Shoes', f'{PREVIEW}/SKM_Jared_Shoes_Preview'),
]
# Compared against, never copied.
BODY = f'{PREVIEW}/SKM_Jared_Body_Preview'


def measure(mesh):
    """Bone count, bounds and material slots, enough to spot a bad bind pose."""
    box = mesh.get_bounds().box_extent
    origin = mesh.get_bounds().origin
    skeleton = mesh.get_editor_property('skeleton')
    bones = []
    if skeleton:
        try:
            bones = [str(b) for b in skeleton.get_editor_property('bone_tree')]
        except Exception:
            bones = []
    return {
        'origin': [round(origin.x, 2), round(origin.y, 2), round(origin.z, 2)],
        'extent': [round(box.x, 2), round(box.y, 2), round(box.z, 2)],
        'longest_axis': 'XYZ'[max(range(3), key=lambda i: [box.x, box.y, box.z][i])],
        'skeleton': skeleton.get_path_name() if skeleton else None,
        'bone_count': len(bones) or None,
        'slots': [str(s.material_slot_name) for s in mesh.get_editor_property('materials')],
        'materials': [s.material_interface.get_path_name() if s.material_interface else None
                      for s in mesh.get_editor_property('materials')],
    }


def main():
    report = {'garments': [], 'body': None, 'warnings': []}

    body = unreal.load_asset(BODY)
    if body:
        report['body'] = measure(body)

    for label, source, dest in GARMENTS:
        entry = {'label': label, 'source': source, 'dest': dest}
        report['garments'].append(entry)

        if unreal.EditorAssetLibrary.does_asset_exist(dest):
            mesh = unreal.load_asset(dest)
            entry['action'] = 'already_present'
        else:
            if not unreal.EditorAssetLibrary.does_asset_exist(source):
                entry['action'] = 'source_missing'
                continue
            mesh = unreal.EditorAssetLibrary.duplicate_asset(source, dest)
            entry['action'] = 'duplicated'

        if not isinstance(mesh, unreal.SkeletalMesh):
            entry['action'] = 'not_a_skeletal_mesh'
            continue

        entry.update(measure(mesh))
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        entry['saved'] = True

        if report['body'] and entry.get('longest_axis') != report['body']['longest_axis']:
            report['warnings'].append(
                f"{label}: longest axis {entry['longest_axis']} does not match the body's "
                f"{report['body']['longest_axis']}; check the bind pose before trusting a "
                f"leader-pose bind")

    out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Avatars')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'jared_clothing_prepare.json').write_text(json.dumps(report, indent=2))
    print('WMCYN_JARED_CLOTHING ' + json.dumps(report, indent=2))


main()
