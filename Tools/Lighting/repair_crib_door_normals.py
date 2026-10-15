"""Preserve source normals on a reversible WMCYN duplicate of the combined doors."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
crib=next(a for a in sub.get_all_level_actors() if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
comp=next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='house parts low poly_009')
dest='/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Doors_SourceNormals'
rollback=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/door_normals_rollback.json'
if not rollback.exists():
    rollback.write_text(json.dumps(dict(component=comp.get_path_name(),mesh=comp.static_mesh.get_path_name(),materials=[m.get_path_name() if m else None for m in comp.get_materials()]),indent=2))
mesh=unreal.load_asset(dest) if unreal.EditorAssetLibrary.does_asset_exist(dest) else unreal.EditorAssetLibrary.duplicate_asset('/Game/Environments/WMCYN_Crib/Scene_036',dest)
assert mesh
ss=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
settings=ss.get_lod_build_settings(mesh,0)
settings.set_editor_property('recompute_normals',False)
settings.set_editor_property('recompute_tangents',True)
ss.set_lod_build_settings(mesh,0,settings)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
comp.set_static_mesh(mesh)
print('WMCYN_DOOR_NORMALS_APPLIED',dest)
