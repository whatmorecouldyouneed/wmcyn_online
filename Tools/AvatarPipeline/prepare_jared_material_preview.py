"""Build reversible WMCYN preview copies using Jared's available baked maps.

No source MetaHuman/framework edits, account publication, skeleton assignment,
or pawn replacement. These default-lit materials are a diagnostic fallback for
the migrated skin shader's missing function references, not final skin shading.
"""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
ROOT = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'
SRC = '/Game/MetaHumans/Jared'
assets = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
report = {'materials': [], 'meshes': [], 'limitations': [
    'Diagnostic default-lit baked materials, not full MetaHuman skin shading.',
    'No skeleton has been assigned; runtime/account binding is not enabled.']}


def material(name, base, normal=None, roughness=0.65):
    path = ROOT + '/Materials/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    tex = unreal.load_asset(base)
    assert isinstance(tex, unreal.Texture2D), base
    norm = unreal.load_asset(normal) if normal else None
    assert not normal or isinstance(norm, unreal.Texture2D), normal
    mat = assets.create_asset(name, ROOT + '/Materials', unreal.Material, unreal.MaterialFactoryNew())
    for texture, prop, sampler in [(tex, unreal.MaterialProperty.MP_BASE_COLOR, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR),
                                    (norm, unreal.MaterialProperty.MP_NORMAL, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)]:
        if not texture:
            continue
        node = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample)
        node.set_editor_property('texture', texture)
        node.set_editor_property('sampler_type', sampler)
        assert mel.connect_material_property(node, 'RGB', prop)
    for value, prop in [(roughness, unreal.MaterialProperty.MP_ROUGHNESS), (0.3, unreal.MaterialProperty.MP_SPECULAR)]:
        node = mel.create_material_expression(mat, unreal.MaterialExpressionConstant)
        node.set_editor_property('r', value)
        assert mel.connect_material_property(node, '', prop)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    report['materials'].append({'path': path, 'basecolor': base, 'normal': normal})
    return mat


bodymat = material('M_Jared_Body_BakedPreview', SRC+'/Body/Baked/T_Body_BC', SRC+'/Body/Baked/T_Body_N')
headmats = {}
for suffix in ('', '_LOD2', '_LOD5to7'):
    headmats[suffix] = material('M_Jared_Head'+suffix+'_BakedPreview',
                               SRC+'/Face/Baked/T_Head'+suffix+'_BC', SRC+'/Face/Baked/T_Head'+suffix+'_N')
teeth = material('M_Jared_Teeth_BakedPreview', SRC+'/Face/Baked/T_Teeth_BC', SRC+'/Face/Baked/T_Teeth_N', 0.35)
for part, source in [('Body', SRC+'/Body/SKM_Jared_BodyMesh'), ('Head', SRC+'/Face/SKM_Jared_FaceMesh')]:
    path = ROOT+'/SKM_Jared_'+part+'_Preview'
    mesh = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.EditorAssetLibrary.duplicate_asset(source, path)
    assert isinstance(mesh, unreal.SkeletalMesh)
    slots = list(mesh.get_editor_property('materials'))
    for i, slot in enumerate(slots):
        name = str(slot.material_slot_name)
        if part == 'Body':
            slot.material_interface = bodymat
        elif name.startswith('head') or name.startswith('cartilage'):
            suffix = '_LOD5to7' if 'LOD57' in name else '_LOD2' if any(x in name for x in ('LOD2', 'LOD3', 'LOD4')) else ''
            slot.material_interface = headmats[suffix]
        elif name.startswith('teeth'):
            slot.material_interface = teeth
    mesh.set_editor_property('materials', slots)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    report['meshes'].append({'path': path, 'skeleton': str(mesh.get_editor_property('skeleton'))})
out = Path(unreal.Paths.project_saved_dir())/'Diagnostics'/'Avatars'
out.mkdir(parents=True, exist_ok=True)
(out/'jared_material_preview.json').write_text(json.dumps(report, indent=2))
print('WMCYN_JARED_MATERIAL_PREVIEW_READY')
