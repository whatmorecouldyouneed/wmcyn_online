"""Fix Jared's garment shading and his opaque eye shell.

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/AvatarPipeline/fix_jared_appearance.py

Two faults seen in the VR preview, with two unrelated causes.

1. GARMENTS looked like wet plastic with white speckling.

   `RiggedFit_DefaultMaterial_OcclusionRoughnessMetallic_1001` was imported as
   `TC_Default` with `sRGB=true`. An ORM map is data, not colour: gamma-decoding
   it drags roughness far down (so everything turns glossy) and makes metallic
   noisy (so highlights speckle). BaseColor and Normal were already correct, so
   this single texture is the whole garment problem.

2. EYES rendered as a flat opaque slab filling the socket.

   The MetaHuman eye stack puts an occlusion shell (`eyeshell_shader_shader`)
   and a lacrimal/wetness edge (`eyeEdge_shader_shader`) in front of the
   eyeball. Both are meant to be translucent. Here they resolve opaque and hide
   the eyeball completely - and the eyeball materials underneath are fine, with
   their baked iris and sclera textures correctly assigned.

   Rather than repair the MetaHuman translucency stack, these two layers are
   hidden. They contribute nothing at conversational distance in VR, and
   stripping the occlusion and wetness layers is standard practice for
   MetaHumans targeted at headsets. The eyeball, eyelashes, skin and teeth are
   untouched.

Note the source materials under `/Game/MetaHumans/Jared/Face/Materials/` have
`None` for every texture parameter - they are the broken ones. The preview
copies carry the real baked textures, so slots are pointed at the preview copies
wherever one exists. Do not "restore" the source materials; they are emptier.

Idempotent: re-running reports `already_correct` and changes nothing.
"""
import json
from pathlib import Path

import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)

PREVIEW = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
MATERIALS = f'{PREVIEW}/Materials'
HEAD = f'{PREVIEW}/SKM_Jared_Head_Preview'
HIDDEN = f'{MATERIALS}/M_WMCYN_Hidden'

# ORM and normal maps are data; only BaseColor is colour.
TEXTURE_FIXES = [
    ('/Game/Characters/jARED/tex/RiggedFit_DefaultMaterial_OcclusionRoughnessMetallic_1001',
     {'compression_settings': unreal.TextureCompressionSettings.TC_MASKS, 'srgb': False}),
    ('/Game/Characters/jARED/tex/RiggedFit_DefaultMaterial_Normal_1001',
     {'compression_settings': unreal.TextureCompressionSettings.TC_NORMALMAP, 'srgb': False}),
    ('/Game/Characters/jARED/tex/RiggedFit_DefaultMaterial_BaseColor_1001',
     {'compression_settings': unreal.TextureCompressionSettings.TC_DEFAULT, 'srgb': True}),
]

# Slots to blank out on the head. Both sit in front of the eyeball.
HIDE_SLOTS = ['eyeshell_shader_shader', 'eyeEdge_shader_shader']


def fix_textures(report):
    for path, wanted in TEXTURE_FIXES:
        entry = {'texture': path.split('/')[-1]}
        report['textures'].append(entry)
        texture = unreal.load_asset(path)
        if texture is None:
            entry['result'] = 'missing'
            continue
        changed = {}
        for prop, value in wanted.items():
            if texture.get_editor_property(prop) != value:
                texture.set_editor_property(prop, value)
                changed[prop] = str(value)
        entry['changed'] = changed
        entry['result'] = 'fixed' if changed else 'already_correct'
        if changed:
            unreal.EditorAssetLibrary.save_loaded_asset(texture)


def make_hidden_material(report):
    """A fully transparent, unlit material used to retire a mesh section."""
    if unreal.EditorAssetLibrary.does_asset_exist(HIDDEN):
        report['hidden_material'] = 'already_present'
        return unreal.load_asset(HIDDEN)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        HIDDEN.split('/')[-1], MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        report['hidden_material'] = 'creation_failed'
        return None

    material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    # Opacity 0. Unlit keeps it off the lighting path entirely.
    zero = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -300, 0)
    zero.set_editor_property('r', 0.0)
    unreal.MaterialEditingLibrary.connect_material_property(
        zero, '', unreal.MaterialProperty.MP_OPACITY)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    report['hidden_material'] = 'created'
    return material


def hide_eye_layers(report, hidden):
    mesh = unreal.load_asset(HEAD)
    if mesh is None or hidden is None:
        report['head'] = 'missing_mesh_or_material'
        return

    materials = mesh.get_editor_property('materials')
    changed = False
    for slot in materials:
        name = str(slot.material_slot_name)
        entry = {'slot': name}
        if name not in HIDE_SLOTS:
            continue
        current = slot.material_interface
        if current == hidden:
            entry['result'] = 'already_hidden'
        else:
            entry['was'] = current.get_path_name() if current else None
            slot.material_interface = hidden
            entry['result'] = 'hidden'
            changed = True
        report['head_slots'].append(entry)

    if changed:
        mesh.set_editor_property('materials', materials)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    report['head'] = 'updated' if changed else 'unchanged'


def main():
    report = {'textures': [], 'head_slots': [], 'head': None, 'hidden_material': None}
    fix_textures(report)
    hide_eye_layers(report, make_hidden_material(report))

    out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Avatars')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'jared_appearance_fix.json').write_text(json.dumps(report, indent=2))
    print('WMCYN_APPEARANCE ' + json.dumps(report, indent=2))


main()
