"""Read-only collision geometry and source component inventory."""
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting'
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
crib = next(a for a in sub.get_all_level_actors() if a.get_name() == 'FbxScene_wmcyn_crib_default_unity_transfer_C_1')
rows = []
for c in crib.get_components_by_class(unreal.StaticMeshComponent):
    if not c.static_mesh:
        continue
    lo, hi = c.get_local_bounds()
    origin = c.get_world_transform().transform_location((lo + hi) * 0.5)
    extent = (hi - lo) * c.get_world_scale() * 0.5
    rows.append(dict(name=c.get_name(), mesh=c.static_mesh.get_path_name(), origin=list(origin.to_tuple()), extent=list(extent.to_tuple()), collision=str(c.get_collision_enabled())))
    if c.static_mesh.get_name() in ['Scene_029', 'Scene_030', 'Scene_031', 'Scene_032', 'Scene_033', 'Scene_045']:
        sections = []
        for section in range(c.static_mesh.get_num_sections(0)):
            vs, ts, ns, uv, tangents = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(c.static_mesh, 0, section)
            tf = c.get_world_transform()
            sections.append(dict(vertices=[list(tf.transform_location(v).to_tuple()) for v in vs], triangles=list(ts), normals=[list(tf.transform_direction(n).to_tuple()) for n in ns], uv=[list(x.to_tuple()) for x in uv], material=c.get_material(section).get_path_name()))
        (root / (c.static_mesh.get_name() + '_doors.json')).write_text(json.dumps(sections))
    if c.get_name() == 'collider':
        original_shell = unreal.load_asset('/Game/Environments/WMCYN_Crib/Scene_002')
        vertices, triangles, normals, uvs, tangents = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(original_shell, 0, 0)
        tf = c.get_world_transform()
        data = dict(vertices=[list(tf.transform_location(v).to_tuple()) for v in vertices], triangles=list(triangles))
        (root / 'door_shell_geometry.json').write_text(json.dumps(data))
(root / 'door_rollout_inventory.json').write_text(json.dumps(rows, indent=2))
print('WMCYN_DOOR_INVENTORY', len(rows))
