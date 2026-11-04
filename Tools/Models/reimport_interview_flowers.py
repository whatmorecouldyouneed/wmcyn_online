"""Reimport only the two WMCYN flower meshes, preserving the shared material."""
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
source = Path(unreal.Paths.project_dir()) / 'SourceArt' / 'InterviewFlowers'
dest = '/Game/WMCYN/Environment/Crib/Furniture/InterviewFlowers'
material = unreal.load_asset(dest + '/M_InterviewFlower_Fabric')
assert material
for variant in ('Large', 'Small'):
    name = 'SM_InterviewFlower_' + variant
    options = unreal.FbxImportUI()
    options.set_editor_property('import_mesh', True)
    options.set_editor_property('import_as_skeletal', False)
    options.set_editor_property('import_materials', False)
    options.set_editor_property('import_textures', False)
    options.set_editor_property('automated_import_should_detect_type', False)
    options.set_editor_property('mesh_type_to_import', unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.static_mesh_import_data.set_editor_property('combine_meshes', True)
    options.static_mesh_import_data.set_editor_property('generate_lightmap_u_vs', True)
    options.static_mesh_import_data.set_editor_property('auto_generate_collision', False)
    task = unreal.AssetImportTask()
    for key, value in dict(filename=str(source / (name + '.fbx')), destination_path=dest,
                           destination_name=name, automated=True, save=False,
                           replace_existing=True, replace_existing_settings=True,
                           options=options).items():
        task.set_editor_property(key, value)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(dest + '/' + name)
    assert isinstance(mesh, unreal.StaticMesh)
    mesh.set_material(0, material)
    unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem).remove_collisions(mesh)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
print('WMCYN_FLOWERS_REIMPORT_OK')
