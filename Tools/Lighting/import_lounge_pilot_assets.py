"""Import only the two approved lounge candidates; never place actors or save a level."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
source=Path('C:/Users/jvred/WMCYN v2.0/Assets/Hidden Heights SGB')
dest='/Game/WMCYN/Environment/Crib/Furniture/LoungePilot'
tools=unreal.AssetToolsHelpers.get_asset_tools()
mel=unreal.MaterialEditingLibrary
report=[]
specs=[('Sectional','sectional_sofa01.fbx','large sectional sofa','large sectional sofa'),
       ('CoffeeTable','coffee_table.fbx','side tables','side tables')]

def import_task(filename,folder,name,options=None):
    path=folder+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path),False
    task=unreal.AssetImportTask()
    task.set_editor_property('filename',str(filename))
    task.set_editor_property('destination_path',folder)
    task.set_editor_property('destination_name',name)
    task.set_editor_property('automated',True)
    task.set_editor_property('save',False)
    task.set_editor_property('replace_existing',False)
    if options: task.set_editor_property('options',options)
    tools.import_asset_tasks([task])
    asset=unreal.load_asset(path)
    assert asset, (path,list(task.imported_object_paths))
    return asset,True

for key,fbx,texdir,prefix in specs:
    opts=unreal.FbxImportUI()
    opts.set_editor_property('import_mesh',True)
    opts.set_editor_property('import_as_skeletal',False)
    opts.set_editor_property('import_materials',False)
    opts.set_editor_property('import_textures',False)
    opts.set_editor_property('automated_import_should_detect_type',False)
    opts.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_STATIC_MESH)
    opts.static_mesh_import_data.set_editor_property('combine_meshes',True)
    opts.static_mesh_import_data.set_editor_property('generate_lightmap_u_vs',True)
    opts.static_mesh_import_data.set_editor_property('auto_generate_collision',False)
    mesh,new_mesh=import_task(source/'fbx'/fbx,dest+'/Meshes','SM_Crib_'+key,opts)
    assert isinstance(mesh,unreal.StaticMesh)
    mat_path=dest+'/Materials/M_Crib_'+key
    mat=unreal.load_asset(mat_path) if unreal.EditorAssetLibrary.does_asset_exist(mat_path) else None
    new_mat=mat is None
    if new_mat:
        mat=tools.create_asset('M_Crib_'+key,dest+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
        for suffix,prop,kind in [('Base_color','BASE_COLOR','color'),('Normal_OpenGL','NORMAL','normal'),('Roughness','ROUGHNESS','linear'),('Metallic','METALLIC','linear')]:
            candidates=list((source/'textures'/texdir).glob(prefix+'_'+suffix+'.*'))
            candidates=[p for p in candidates if p.suffix.lower() in ('.jpg','.png','.tif')]
            assert len(candidates)==1,candidates
            tex,new_tex=import_task(candidates[0],dest+'/Textures','T_Crib_'+key+'_'+suffix)
            if new_tex:
                tex.set_editor_property('srgb',kind=='color')
                tex.set_editor_property('max_texture_size',2048)
                if kind=='normal':
                    tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
                    tex.set_editor_property('flip_green_channel',True)
                unreal.EditorAssetLibrary.save_loaded_asset(tex)
            sample=mel.create_material_expression(mat,unreal.MaterialExpressionTextureSample)
            sample.set_editor_property('texture',tex)
            sampler=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if kind=='normal' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if kind=='color' else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR
            sample.set_editor_property('sampler_type',sampler)
            assert mel.connect_material_property(sample,'R' if kind=='linear' else 'RGB',getattr(unreal.MaterialProperty,'MP_'+prop))
        mel.layout_material_expressions(mat)
        mel.recompile_material(mat)
        unreal.EditorAssetLibrary.save_loaded_asset(mat)
    if new_mesh:
        for index in range(len(mesh.get_editor_property('static_materials'))):
            mesh.set_material(index,mat)
        mesh.set_editor_property('light_map_resolution',256)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    box=mesh.get_bounding_box()
    report.append(dict(mesh=mesh.get_path_name(),material=mat.get_path_name(),created_mesh=new_mesh,
                       bounds=str(box),collision='Not configured; placement blocked by floor-support gate'))
root=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
(root/'lounge_pilot_import.json').write_text(json.dumps(report,indent=2))
print('WMCYN_LOUNGE_ASSETS_READY',report)
