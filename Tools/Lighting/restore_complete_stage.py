"""Add all 89 source mesh pieces as one WMCYN visual, retaining simple support."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
root=Path(unreal.Paths.project_dir())
dest='/Game/WMCYN/Environment/Crib/StagePilot'
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors={a.get_actor_label():a for a in sub.get_all_level_actors()}
deck=actors['STAGE_Pilot_Deck']
tools=unreal.AssetToolsHelpers.get_asset_tools()
path=dest+'/Meshes/SM_StageComplete'
mesh=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
if not mesh:
    options=unreal.FbxImportUI()
    for k,v in dict(import_mesh=True,import_as_skeletal=False,import_materials=False,import_textures=False,automated_import_should_detect_type=False,mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH).items(): options.set_editor_property(k,v)
    for k,v in dict(combine_meshes=True,generate_lightmap_u_vs=True,auto_generate_collision=False).items(): options.static_mesh_import_data.set_editor_property(k,v)
    task=unreal.AssetImportTask()
    for k,v in dict(filename=str(root/'SourceArt/StagePilot/StageComplete.fbx'),destination_path=dest+'/Meshes',destination_name='SM_StageComplete',automated=True,save=False,replace_existing=False,options=options).items(): task.set_editor_property(k,v)
    tools.import_asset_tasks([task])
    mesh=unreal.load_asset(path)
assert mesh
mapping={'Chrome':'Metal','floor':'Deck','footings':'Metal','light_handle':'Metal','light_inside_bar':'White','light_pot':'Deck'}
for i,slot in enumerate(mesh.get_editor_property('static_materials')):
    name=str(slot.material_slot_name)
    assert name in mapping,name
    material=unreal.load_asset(dest+'/Materials/M_StagePilot_'+mapping[name])
    assert material
    mesh.set_material(i,material)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
scale=5/19.388954162597656
depth=13.336772918701172*scale*100
pos=deck.get_actor_location()
top=pos.z+24
actor=actors.get('STAGE_Pilot_CompleteAssembly')
if not actor:
    actor=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(pos.x,pos.y,top),unreal.Rotator(pitch=0,yaw=90,roll=0))
    actor.set_actor_label('STAGE_Pilot_CompleteAssembly')
    actor.set_folder_path('WMCYN_Production/StagePilot')
    actor.tags=[unreal.Name('WMCYN_StagePilot')]
actor.static_mesh_component.set_static_mesh(mesh)
actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
actor.set_actor_scale3d(unreal.Vector(scale,scale,scale))
deck.set_actor_scale3d(unreal.Vector(1.25,depth/300,1))
deck.static_mesh_component.set_visibility(False)
deck.static_mesh_component.set_hidden_in_game(True)
step=actors['STAGE_Pilot_FrontStep']
step.set_actor_location(unreal.Vector(pos.x+depth/2+40,pos.y,pos.z),False,False)
checks=[]
for x in [-depth/2+10,0,depth/2-10]:
    for y in [-240,0,240]:
        hit=deck.static_mesh_component.line_trace_component(unreal.Vector(pos.x+x,pos.y+y,top+50),unreal.Vector(pos.x+x,pos.y+y,top-50),False,False,False)
        assert hit and abs(hit[0].z-top)<.5
        checks.append([pos.x+x,pos.y+y,hit[0].z])
report=dict(source_mesh_count=89,placement_scale=scale,stage_width_cm=500,stage_depth_cm=depth,support_samples=checks,collision='Existing deck hidden visually; complete assembly visual only',materials='WMCYN preview approximations, not Unity shader parity')
(root/'Saved/Diagnostics/Lighting/stage_complete.json').write_text(json.dumps(report,indent=2))
sub.set_selected_level_actors([actor])
print('WMCYN_COMPLETE_STAGE',report)
