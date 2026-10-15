"""Separate the approved open leaf; preserve every other source triangle and UV."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
root=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
data=json.loads((root/'door_world_geometry.json').read_text())
islands=json.loads((root/'door_islands.json').read_text())
leaf_ids={1299,143,147,1315,190,215,334,359,1349,1330,1336,1357,1363}
assert leaf_ids.issubset({r['id'] for r in islands})
leaf_tris={t for r in islands if r['id'] in leaf_ids for t in r['triangles']}
all_tris=set(range(len(data['triangles'])//3))
assert len(all_tris)==1010 and len(leaf_tris)==184, (len(all_tris),len(leaf_tris))
PIVOT=unreal.Vector(-747.5,85.0,460.6)
dest='/Game/WMCYN/Environment/Crib/Doors'
material=unreal.load_asset(data['material'])

def make_mesh(name,tri_ids,pivot):
    path=dest+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path): return unreal.load_asset(path)
    used=sorted({data['triangles'][t*3+j] for t in tri_ids for j in range(3)})
    remap={old:new for new,old in enumerate(used)}
    buffers=unreal.GeometryScriptSimpleMeshBuffers()
    buffers.vertices=[unreal.Vector(*data['vertices'][i])-pivot for i in used]
    buffers.normals=[unreal.Vector(*data['normals'][i]) for i in used]
    buffers.uv0=[unreal.Vector2D(*data['uv'][i]) for i in used]
    buffers.triangles=[unreal.IntVector(*[remap[data['triangles'][t*3+j]] for j in range(3)]) for t in sorted(tri_ids)]
    dynamic=unreal.DynamicMesh()
    unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dynamic,buffers)
    options=unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    options.enable_nanite=False
    options.enable_collision=False
    options.enable_recompute_normals=False
    options.enable_recompute_tangents=True
    mesh,outcome=unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(dynamic,path,options)
    assert mesh, str(outcome)
    mesh.set_material(0,material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return mesh

leaf=make_mesh('SM_Crib_OpenDoor01_Leaf',leaf_tris,PIVOT)
rest=make_mesh('SM_Crib_Doors_Without_OpenDoor01',all_tris-leaf_tris,unreal.Vector())
mesh_editor=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
build=mesh_editor.get_lod_build_settings(rest,0)
build.set_editor_property('generate_lightmap_u_vs',True)
build.set_editor_property('src_lightmap_index',0)
build.set_editor_property('dst_lightmap_index',1)
mesh_editor.set_lod_build_settings(rest,0,build)
rest.set_editor_property('light_map_coordinate_index',1)
rest.set_editor_property('light_map_resolution',256)
unreal.EditorAssetLibrary.save_loaded_asset(rest)
bp_path=dest+'/BP_WMCYN_CribDoor_01'
bp=unreal.load_asset(bp_path) if unreal.EditorAssetLibrary.does_asset_exist(bp_path) else unreal.EditorAssetLibrary.duplicate_asset(dest+'/BP_WMCYN_CribDoor_GripPilot',bp_path)
ss=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
objects={o.get_name():o for h in ss.k2_gather_subobject_data_for_blueprint(bp) for o in [unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))] if o}
objects['SM_Lever_Base_GEN_VARIABLE'].set_editor_property('static_mesh',None)
moving=objects['SM_Lever_GEN_VARIABLE']
moving.set_editor_property('static_mesh',leaf)
moving.set_editor_property('override_materials',[material])
moving.set_editor_property('relative_location',unreal.Vector())
moving.set_editor_property('relative_rotation',unreal.Rotator())
moving.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
drag=objects['Comp_Drag_GEN_VARIABLE']
settings=drag.get_editor_property('dragType');key=next(iter(settings.keys()));entry=settings[key]
field='ValueRange_37_A0D2863A4F0DE164EEE38C93FC835CD5'
limits=entry.get_editor_property(field);limits.min=-100.0;limits.max=0.0
entry.set_editor_property(field,limits);settings[key]=entry;drag.set_editor_property('dragType',settings)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.EditorAssetLibrary.save_loaded_asset(bp)
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=sub.get_all_level_actors()
crib=next(a for a in actors if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
source=next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='house parts low poly_009')
rollback=root/'door_01_conversion_rollback.json'
if not rollback.exists():
    rollback.write_text(json.dumps(dict(component=source.get_path_name(),visible=source.is_visible(),hidden_in_game=source.get_editor_property('hidden_in_game'),cast_shadow=source.get_editor_property('cast_shadow'),mesh=source.static_mesh.get_path_name(),pivot=list(PIVOT.to_tuple()),leaf_triangles=sorted(leaf_tris)),indent=2))
rest_actor=next((a for a in actors if a.get_actor_label()=='ARCH_Crib_Doors_Remainder_01'),None)
if not rest_actor: rest_actor=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
rest_actor.set_actor_label('ARCH_Crib_Doors_Remainder_01')
rest_actor.set_folder_path('WMCYN_Environment/InteractiveDoors')
rest_actor.static_mesh_component.set_static_mesh(rest)
rest_actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
cls=unreal.load_class(None,bp_path+'.BP_WMCYN_CribDoor_01_C')
door=next((a for a in actors if a.get_actor_label()=='DOOR_Crib_01_Grip'),None)
if not door: door=sub.spawn_actor_from_class(cls,PIVOT)
door.set_actor_label('DOOR_Crib_01_Grip')
door.set_folder_path('WMCYN_Environment/InteractiveDoors')
door.set_replicates(True)
source.set_visibility(False)
source.set_hidden_in_game(True)
source.set_cast_shadow(False)
print('WMCYN_DOOR_01_CONVERTED',len(leaf_tris),len(all_tris-leaf_tris))
