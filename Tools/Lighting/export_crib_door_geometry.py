"""Export door triangles/UVs in world centimeters for a scoped leaf separation."""
import json
from pathlib import Path
import unreal
crib=next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
comp=next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='house parts low poly_009')
tf=comp.get_world_transform()
v,t,n,uv,tan=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(comp.static_mesh,0,0)
data=dict(component=comp.get_path_name(),source=comp.static_mesh.get_path_name(),material=comp.get_material(0).get_path_name(),vertices=[list(tf.transform_location(p).to_tuple()) for p in v],triangles=list(t),uv=[list(p.to_tuple()) for p in uv],normals=[list(tf.transform_direction(p).to_tuple()) for p in n])
(Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/door_world_geometry.json').write_text(json.dumps(data))
print('DOOR_GEOMETRY_EXPORTED',len(v),len(t)//3)
