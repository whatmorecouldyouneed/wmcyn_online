"""Give Jared's grooms their materials, and his eyes a working stand-in.

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/AvatarPipeline/fix_jared_groom_materials.py

GROOMS RENDERED WHITE
---------------------
Every groom material slot is `None` - in the *source* assets, not as a result of
duplicating them. A groom with no material falls back to the engine default hair
shader, which is white. That is why hair and goatee both came out silver.

The source `Grooms/` folder carries saved material instances whose parent names
map exactly onto the slot names:

    slot MI_Hair         <- an instance parented to Common/Materials/MI_Hair
    slot MI_Hair_Cards   <- ... MI_Hair_Cards
    slot MI_Hair_Helmet  <- ... MI_Hair_Helmet
    slot MI_Facial_Hair  <- ... MI_Facial_Hair

So the assignment is derived, not hardcoded: for each slot, pick the instance
whose parent material name equals the slot name. The script asserts the match
and records every choice, because several instances share the `MI_Hair` parent
(205/206/208/211/213) and two share `MI_Facial_Hair` (207/212). Where more than
one candidate fits, the lowest-numbered is taken deterministically and the
alternatives are listed in the report, so a wrong pick shows up as a recorded
decision rather than a silent one.

Materials are duplicated into the preview folder first, so nothing in the
preview family reaches back into the source import at runtime - the same rule
already applied to the meshes and the bindings.

EYES RENDERED AS A DARK SLAB
----------------------------
Hiding the occlusion shell and lacrimal edge did not change them, which rules
out anything covering the eyeball: the eyeball material itself is at fault. The
MetaHuman unified eye shader is present and so is its subsurface profile, but it
is not resolving here, and its baked-texture path is driven by a static switch
that is not reachable from script.

So the eyes get the same treatment that already works for the skin and the
teeth: a plain baked stand-in. `T_EyeScleraL/R_BC` are *baked* full-eye
textures, so sampling one as base colour should read correctly on its own.

This is the one step here that is a judgement call rather than a derivation. If
the result looks wrong, the next suspects are that the sclera bake does not
include the iris (in which case the iris texture needs compositing over it) or
that the eyeball UVs are not what this assumes - not more material swapping.

Idempotent: re-running reports `already_*` and changes nothing.
"""
import json
import re
from pathlib import Path

import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)

PREVIEW = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
GROOMS = f'{PREVIEW}/Grooms'
GROOM_MATS = f'{GROOMS}/Materials'
PREVIEW_MATS = f'{PREVIEW}/Materials'
SOURCE_GROOMS = '/Game/MetaHumans/Jared/Grooms'
HEAD = f'{PREVIEW}/SKM_Jared_Head_Preview'
BAKED = '/Game/MetaHumans/Jared/Face/Baked'

GROOM_ASSETS = ['Hair_S_360Waves', 'Eyebrows_M_Wide', 'Goatee_L_Wavy']

EYES = [
    ('eyeLeft_shader_shader', 'MI_Jared_EyeL_Preview', f'{BAKED}/T_EyeScleraL_BC'),
    ('eyeRight_shader_shader', 'MI_Jared_EyeR_Preview', f'{BAKED}/T_EyeScleraR_BC'),
]

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def parent_name(material):
    """Name of the material instance's parent, which is what a slot matches on."""
    try:
        parent = material.get_editor_property('parent')
    except Exception:
        return None
    return parent.get_name() if parent else None


def collect_candidates(report):
    """Map parent-material name -> duplicated instances, lowest number first."""
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    candidates = {}
    for data in registry.get_assets_by_path(SOURCE_GROOMS, recursive=False):
        name = str(data.asset_name)
        if not name.startswith('MID_'):
            continue
        source = f'{SOURCE_GROOMS}/{name}'
        dest = f'{GROOM_MATS}/{name}'
        if unreal.EditorAssetLibrary.does_asset_exist(dest):
            material = unreal.load_asset(dest)
        else:
            material = unreal.EditorAssetLibrary.duplicate_asset(source, dest)
            if material:
                unreal.EditorAssetLibrary.save_loaded_asset(material)
        if material is None:
            continue
        key = parent_name(material)
        if key:
            candidates.setdefault(key, []).append((name, material))

    for key in candidates:
        # Deterministic: lowest trailing number wins.
        candidates[key].sort(key=lambda pair: int(
            re.search(r'(\d+)$', pair[0]).group(1) if re.search(r'(\d+)$', pair[0]) else 0))
    report['candidates'] = {k: [n for n, _ in v] for k, v in candidates.items()}
    return candidates


def assign_groom_materials(report, candidates):
    for name in GROOM_ASSETS:
        entry = {'groom': name, 'slots': []}
        report['grooms'].append(entry)

        groom = unreal.load_asset(f'{GROOMS}/{name}')
        if groom is None:
            entry['result'] = 'missing'
            continue

        slots = groom.get_editor_property('hair_groups_materials')
        changed = False
        for slot in slots:
            slot_name = str(slot.get_editor_property('slot_name'))
            record = {'slot': slot_name}
            entry['slots'].append(record)

            if slot.get_editor_property('material') is not None:
                record['result'] = 'already_assigned'
                continue

            options = candidates.get(slot_name, [])
            if not options:
                # The slot names a parent material we have no instance for.
                # Report it rather than guessing at a near match.
                record['result'] = 'no_candidate'
                continue

            chosen_name, chosen = options[0]
            slot.set_editor_property('material', chosen)
            record['result'] = 'assigned'
            record['material'] = chosen_name
            if len(options) > 1:
                record['other_candidates'] = [n for n, _ in options[1:]]
            changed = True

        if changed:
            groom.set_editor_property('hair_groups_materials', slots)
            unreal.EditorAssetLibrary.save_loaded_asset(groom)
        entry['result'] = 'updated' if changed else 'unchanged'


def make_eye_material(asset_name, texture_path, record):
    """Flat DefaultLit stand-in driven by the baked sclera texture."""
    dest = f'{PREVIEW_MATS}/{asset_name}'
    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        record['material'] = 'already_present'
        return unreal.load_asset(dest)

    texture = unreal.load_asset(texture_path)
    if texture is None:
        record['material'] = 'texture_missing'
        return None

    material = ASSET_TOOLS.create_asset(
        asset_name, PREVIEW_MATS, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        record['material'] = 'creation_failed'
        return None

    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -400, 0)
    sample.set_editor_property('texture', texture)
    unreal.MaterialEditingLibrary.connect_material_property(
        sample, '', unreal.MaterialProperty.MP_BASE_COLOR)

    # An eye is wet: low roughness reads right even without the full shader.
    rough = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 260)
    rough.set_editor_property('r', 0.15)
    unreal.MaterialEditingLibrary.connect_material_property(
        rough, '', unreal.MaterialProperty.MP_ROUGHNESS)

    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    record['material'] = 'created'
    record['texture'] = texture_path.split('/')[-1]
    return material


def assign_eyes(report):
    mesh = unreal.load_asset(HEAD)
    if mesh is None:
        report['eyes'] = 'head_missing'
        return

    materials = mesh.get_editor_property('materials')
    changed = False
    for slot_name, asset_name, texture_path in EYES:
        record = {'slot': slot_name}
        report['eye_slots'].append(record)

        material = make_eye_material(asset_name, texture_path, record)
        if material is None:
            continue

        for slot in materials:
            if str(slot.material_slot_name) != slot_name:
                continue
            if slot.material_interface == material:
                record['result'] = 'already_assigned'
            else:
                record['was'] = (slot.material_interface.get_path_name()
                                 if slot.material_interface else None)
                slot.material_interface = material
                record['result'] = 'assigned'
                changed = True

    if changed:
        mesh.set_editor_property('materials', materials)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    report['eyes'] = 'updated' if changed else 'unchanged'


def main():
    report = {'grooms': [], 'candidates': {}, 'eye_slots': [], 'eyes': None}
    assign_groom_materials(report, collect_candidates(report))
    assign_eyes(report)

    out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Avatars')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'jared_groom_materials.json').write_text(json.dumps(report, indent=2))
    print('WMCYN_GROOM_MATS ' + json.dumps(report, indent=2))


main()
