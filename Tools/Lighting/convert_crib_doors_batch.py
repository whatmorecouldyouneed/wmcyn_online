"""Convert inventoried wooden doors using WMCYN duplicates; never edit AFCore."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
plan = json.loads((root/'door_rollout_plan.json').read_text())
dest = '/Game/WMCYN/Environment/Crib/Doors'
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
mesh_editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
ss = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
actors = sub.get_all_level_actors()
crib = next(a for a in actors if a.get_name() == 'FbxScene_wmcyn_crib_default_unity_transfer_C_1')
rollback = []
report = []


def mesh_asset(name, data, triangles, pivot, material):
    path = dest+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    used = sorted({data['triangles'][t*3+j] for t in triangles for j in range(3)})
    remap = {old:new for new,old in enumerate(used)}
    buffers = unreal.GeometryScriptSimpleMeshBuffers()
    buffers.vertices = [unreal.Vector(*data['vertices'][i])-pivot for i in used]
    buffers.normals = [unreal.Vector(*data['normals'][i]) for i in used]
    buffers.uv0 = [unreal.Vector2D(*data['uv'][i]) for i in used]
    buffers.triangles = [unreal.IntVector(*[remap[data['triangles'][t*3+j]] for j in range(3)]) for t in triangles]
    dynamic = unreal.DynamicMesh()
    unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dynamic,buffers)
    options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    options.enable_nanite = False
    options.enable_collision = False
    options.enable_recompute_normals = False
    options.enable_recompute_tangents = True
    mesh, outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(dynamic,path,options)
    assert mesh, str(outcome)
    mesh.set_material(0,material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return mesh


for group in plan:
    data = group['data']
    material = unreal.load_asset(data['material'])
    source = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent)
                  if c.static_mesh and c.static_mesh.get_name() == group['source'])
    rollback.append(dict(component=source.get_path_name(),visible=source.is_visible(),hidden=source.get_editor_property('hidden_in_game'),shadow=source.get_editor_property('cast_shadow'),collision=str(source.get_collision_enabled())))
    rest = mesh_asset('SM_Crib_'+group['source']+'_Frames',data,group['remainder'],unreal.Vector(),material)
    build = mesh_editor.get_lod_build_settings(rest,0)
    build.set_editor_property('generate_lightmap_u_vs',True)
    build.src_lightmap_index = 0
    build.dst_lightmap_index = 1
    mesh_editor.set_lod_build_settings(rest,0,build)
    rest.set_editor_property('light_map_coordinate_index',1)
    rest.set_editor_property('light_map_resolution',256)
    unreal.EditorAssetLibrary.save_loaded_asset(rest)
    label = 'ARCH_Crib_'+group['source']+'_Frames'
    rest_actor = next((a for a in actors if a.get_actor_label()==label),None)
    if not rest_actor:
        rest_actor = sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
    rest_actor.set_actor_label(label)
    rest_actor.set_folder_path('WMCYN_Environment/InteractiveDoors/Frames')
    rest_actor.static_mesh_component.set_static_mesh(rest)
    rest_actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    for item in group['leaves']:
        num = f"{item['number']:02d}"
        if item['number'] == 1:
            continue
        pivot = unreal.Vector(*item['pivot'])
        leaf = mesh_asset('SM_Crib_Door'+num+'_Leaf',data,item['triangles'],pivot,material)
        path = dest+'/BP_WMCYN_CribDoor_'+num
        bp = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.EditorAssetLibrary.duplicate_asset(dest+'/BP_WMCYN_CribDoor_01',path)
        objects = {o.get_name():o for h in ss.k2_gather_subobject_data_for_blueprint(bp) for o in [unreal.SubobjectDataBlueprintFunctionLibrary.get_object(unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))] if o}
        moving = objects['SM_Lever_GEN_VARIABLE']
        moving.set_editor_property('static_mesh',leaf)
        moving.set_editor_property('override_materials',[material])
        moving.set_editor_property('relative_location',unreal.Vector())
        moving.set_editor_property('relative_rotation',unreal.Rotator())
        moving.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        drag = objects['Comp_Drag_GEN_VARIABLE']
        settings = drag.get_editor_property('dragType')
        key = next(iter(settings.keys()))
        entry = settings[key]
        field = 'ValueRange_37_A0D2863A4F0DE164EEE38C93FC835CD5'
        limits = entry.get_editor_property(field)
        limits.min = min(0,item['swing'])
        limits.max = max(0,item['swing'])
        entry.set_editor_property(field,limits)
        settings[key] = entry
        drag.set_editor_property('dragType',settings)
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        unreal.EditorAssetLibrary.save_loaded_asset(bp)
        label = 'DOOR_Crib_'+num+'_Grip'
        actor = next((a for a in actors if a.get_actor_label()==label),None)
        if not actor:
            actor = sub.spawn_actor_from_class(unreal.load_class(None,path+'.BP_WMCYN_CribDoor_'+num+'_C'),pivot)
        actor.set_actor_label(label)
        actor.set_folder_path('WMCYN_Environment/InteractiveDoors/Leaves')
        actor.set_replicates(True)
        report.append(dict(number=item['number'],blueprint=path,handle=item['handle']))
    source.set_visibility(False)
    source.set_hidden_in_game(True)
    source.set_cast_shadow(False)

# Retain the previous remainder actor for rollback, but avoid duplicate visuals.
old = next(a for a in actors if a.get_actor_label()=='ARCH_Crib_Doors_Remainder_01')
old.set_actor_hidden_in_game(True)
old.static_mesh_component.set_visibility(False)
old.static_mesh_component.set_cast_shadow(False)
old.set_folder_path('WMCYN_Environment/InteractiveDoors/Rollback')
if not (root/'door_rollout_rollback.json').exists():
    (root/'door_rollout_rollback.json').write_text(json.dumps(rollback,indent=2))
(root/'door_rollout_created.json').write_text(json.dumps(report,indent=2))
print('WMCYN_DOORS_CREATED',len(report),'new leaves; original door retained')
