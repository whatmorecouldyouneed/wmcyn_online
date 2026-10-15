"""Hide the imported collision-only shell without changing its collision behavior."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
crib=next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
components=crib.get_components_by_class(unreal.StaticMeshComponent)
shell=next(c for c in components if c.get_name()=='collider')
path=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/collision_visual_rollback.json'
if not path.exists():
    path.write_text(json.dumps(dict(component=shell.get_path_name(),visible=shell.is_visible(),hidden_in_game=shell.get_editor_property('hidden_in_game'),cast_shadow=shell.get_editor_property('cast_shadow'),collision_profile=str(shell.get_collision_profile_name()),collision_enabled=str(shell.get_collision_enabled()),mesh=shell.static_mesh.get_path_name()),indent=2))
collision_before=(shell.get_collision_profile_name(),shell.get_collision_enabled())
shell.set_visibility(False)
shell.set_hidden_in_game(True)
shell.set_cast_shadow(False)
assert collision_before==(shell.get_collision_profile_name(),shell.get_collision_enabled())
# Revert only this pass's unsuccessful diagnostic mesh override.
door=next(c for c in components if c.get_name()=='house parts low poly_009')
if door.static_mesh.get_path_name().startswith('/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Doors_SourceNormals'):
    door.set_static_mesh(unreal.load_asset('/Game/Environments/WMCYN_Crib/Scene_036'))
print('WMCYN_COLLISION_RENDER_HIDDEN_COLLISION_PRESERVED',collision_before)
