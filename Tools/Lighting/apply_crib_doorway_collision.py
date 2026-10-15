"""Apply a WMCYN shell duplicate with door openings; preserve the source mesh."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
data = json.loads((root/'door_shell_cut.json').read_text())
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
crib = next(a for a in sub.get_all_level_actors() if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
shell = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='collider')
path = '/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Collision_Doorways'
previous = shell.static_mesh
if unreal.EditorAssetLibrary.does_asset_exist(path):
    mesh = unreal.load_asset(path)
else:
    tf = shell.get_world_transform()
    buffers = unreal.GeometryScriptSimpleMeshBuffers()
    buffers.vertices = [tf.inverse_transform_location(unreal.Vector(*v)) for v in data['vertices']]
    # UE's static-mesh UV density builder requires at least one UV channel.
    buffers.uv0 = [unreal.Vector2D(v[0]/1000.0,v[1]/1000.0) for v in data['vertices']]
    buffers.triangles = [unreal.IntVector(*data['triangles'][i:i+3]) for i in range(0,len(data['triangles']),3)]
    dynamic = unreal.DynamicMesh()
    unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dynamic,buffers)
    options = unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
    options.enable_nanite = False
    options.enable_collision = True
    options.collision_mode = unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
    mesh,outcome = unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(dynamic,path,options)
    assert mesh, str(outcome)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
# Imported shell triangles face outward; retain its two-sided interior collision.
mesh.get_editor_property('body_setup').set_editor_property('double_sided_geometry', True)
editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
editor.set_lod_build_settings(mesh,0,editor.get_lod_build_settings(mesh,0))
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
rollback = root/'door_shell_mesh_rollback.json'
if not rollback.exists():
    rollback.write_text(json.dumps(dict(component=shell.get_path_name(),mesh=previous.get_path_name()),indent=2))
shell.set_static_mesh(mesh)
shell.set_visibility(False)
shell.set_hidden_in_game(True)
shell.set_cast_shadow(False)
print('WMCYN_DOOR_COLLISION_APPLIED',mesh.get_path_name(),shell.get_collision_enabled())
