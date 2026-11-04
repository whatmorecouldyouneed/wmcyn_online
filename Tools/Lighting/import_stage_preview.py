"""Idempotent WMCYN-only stage assets and reversible local layout pilot."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'Stop PIE'
assert not unreal.EditorAssetLibrary.does_asset_exist('/Game/WMCYN/Environment/Crib/StagePilot/Meshes/SM_StageComplete'), 'Superseded: use restore_complete_stage.py'
ROOT=Path(unreal.Paths.project_dir())
DEST='/Game/WMCYN/Environment/Crib/StagePilot'
SOURCE=Path('C:/Users/jvred/WMCYN v2.0/Assets/aleph 9/VDJ_R2100/Texture')
tools=unreal.AssetToolsHelpers.get_asset_tools()
mel=unreal.MaterialEditingLibrary
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
meshes=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_name()=='L_WMCYNOnline',world.get_name()

def load_import(file,folder,name,options=None):
    path=folder+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    task=unreal.AssetImportTask()
    for key,value in dict(filename=str(file),destination_path=folder,destination_name=name,automated=True,save=False,replace_existing=False).items():
        task.set_editor_property(key,value)
    if options: task.set_editor_property('options',options)
    tools.import_asset_tasks([task])
    asset=unreal.load_asset(path)
    assert asset,path
    return asset

def material(name,color,roughness=.6,metallic=0,texture=None,normal=None,ao=None):
    path=DEST+'/Materials/M_StagePilot_'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path): return unreal.load_asset(path)
    mat=tools.create_asset('M_StagePilot_'+name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    for prop,value in [('ROUGHNESS',roughness),('METALLIC',metallic)]:
        node=mel.create_material_expression(mat,unreal.MaterialExpressionConstant)
        node.set_editor_property('r',value)
        mel.connect_material_property(node,'',getattr(unreal.MaterialProperty,'MP_'+prop))
    if not texture:
        node=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector)
        node.set_editor_property('constant',unreal.LinearColor(*color,1))
        mel.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
    for filename,prop,kind in [(texture,'BASE_COLOR','color'),(normal,'NORMAL','normal'),(ao,'AMBIENT_OCCLUSION','linear')]:
        if not filename: continue
        tex=load_import(SOURCE/filename,DEST+'/Textures','T_StagePilot_'+Path(filename).stem)
        tex.set_editor_property('srgb',kind=='color')
        tex.set_editor_property('max_texture_size',2048)
        if kind=='normal':
            tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
            tex.set_editor_property('flip_green_channel',True)
        unreal.EditorAssetLibrary.save_loaded_asset(tex)
        node=mel.create_material_expression(mat,unreal.MaterialExpressionTextureSample)
        node.set_editor_property('texture',tex)
        node.set_editor_property('sampler_type',getattr(unreal.MaterialSamplerType,{'color':'SAMPLERTYPE_COLOR','normal':'SAMPLERTYPE_NORMAL','linear':'SAMPLERTYPE_LINEAR_COLOR'}[kind]))
        mel.connect_material_property(node,'R' if kind=='linear' else 'RGB',getattr(unreal.MaterialProperty,'MP_'+prop))
    mel.layout_material_expressions(mat)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    return mat

mats={
    'floor':material('Deck',(.035,.04,.045),.85),
    'booth':material('Booth',(1,1,1),.7,.5,'booth_tex.png','booth_Normal.png'),
    'DJset':material('Controller',(1,1,1),.3,.5,'VDJ_R2100_tex.png','NormalMap.png','VDJ_R2100_AO.png'),
    'metal':material('Metal',(.35,.38,.4),.32,.8),
    'DISP':material('Display',(.005,.015,.02),.4),
    'glass':material('GlassOpaquePreview',(.01,.025,.03),.18),
    'light':material('Teal',(.02,.45,.4),.4),
    'light2':material('White',(.7,.7,.7),.4),
}
assets={}
report=[]
for key in ['StageDeck','AlephStandard']:
    opts=unreal.FbxImportUI()
    for name,value in dict(import_mesh=True,import_as_skeletal=False,import_materials=False,import_textures=False,automated_import_should_detect_type=False,mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH).items():
        opts.set_editor_property(name,value)
    for name,value in dict(combine_meshes=True,generate_lightmap_u_vs=True,auto_generate_collision=False).items():
        opts.static_mesh_import_data.set_editor_property(name,value)
    mesh=load_import(ROOT/'SourceArt/StagePilot'/(key+'.fbx'),DEST+'/Meshes','SM_'+key,opts)
    slots=[]
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        slotname=str(slot.material_slot_name)
        assert slotname in mats,(key,slotname)
        mesh.set_material(i,mats[slotname])
        slots.append(slotname)
    meshes.remove_collisions(mesh)
    meshes.add_simple_collisions(mesh,unreal.ScriptingCollisionShapeType.BOX)
    mesh.set_editor_property('light_map_resolution',128)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    box=mesh.get_bounding_box()
    size=box.max-box.min
    assert size.z<150 and size.x<450 and size.y<450,(key,str(box))
    assets[key]=mesh
    report.append(dict(mesh=mesh.get_path_name(),bounds=str(box),slots=slots))

existing={a.get_actor_label():a for a in actors.get_all_level_actors()}
def place(label,mesh,pos,scale=(1,1,1),yaw=90):
    actor=existing.get(label)
    if actor:
        actor.set_actor_rotation(unreal.Rotator(pitch=0,yaw=yaw,roll=0),False)
        return actor
    actor=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(pitch=0,yaw=yaw,roll=0))
    actor.set_actor_label(label)
    actor.set_folder_path('WMCYN_Production/StagePilot')
    actor.static_mesh_component.set_static_mesh(mesh)
    actor.static_mesh_component.set_collision_profile_name('BlockAll')
    actor.set_actor_scale3d(unreal.Vector(*scale))
    actor.tags=[unreal.Name('WMCYN_StagePilot')]
    return actor

with unreal.ScopedEditorTransaction('WMCYN stage and Aleph layout preview'):
    deck=place('STAGE_Pilot_Deck',assets['StageDeck'],(-1850,-2050,8))
    step=place('STAGE_Pilot_FrontStep',assets['StageDeck'],(-1660,-2050,8),(.35,.2666667,.5))
    booth=place('DJ_Pilot_AlephStandard',assets['AlephStandard'],(-1890,-2050,32),yaw=270)
    actors.set_selected_level_actors([deck,booth])

out=ROOT/'Saved/Diagnostics/Lighting/stage_preview_import.json'
out.write_text(json.dumps(report,indent=2))
print('WMCYN_STAGE_PREVIEW_READY',report)
# Leave the map unsaved until rendered placement review.
