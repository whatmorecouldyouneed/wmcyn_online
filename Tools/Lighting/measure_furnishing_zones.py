"""Read-only live Unreal component bounds and floor-triangle export."""
import json
from pathlib import Path
import unreal

sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = sub.get_all_level_actors()
crib = next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
rows = []
for c in crib.get_components_by_class(unreal.StaticMeshComponent):
    if not c.static_mesh or not c.is_visible():
        continue
    tf = c.get_world_transform()
    points, floors = [], []
    for section in range(c.static_mesh.get_num_sections(0)):
        vs, ids, _, _, _ = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(c.static_mesh,0,section)
        ws = [tf.transform_location(v) for v in vs]
        points.extend([list(v.to_tuple()) for v in ws])
        for i in range(0,len(ids),3):
            p = [ws[j] for j in ids[i:i+3]]
            n = (p[1]-p[0]).cross(p[2]-p[0])
            if n.length() > 100 and abs(n.z)/n.length() > .98:
                floors.append([list(v.to_tuple()) for v in p])
    if points:
        rows.append(dict(component=c.get_name(), mesh=c.static_mesh.get_path_name(),
            materials=[c.get_material(i).get_path_name() if c.get_material(i) else None for i in range(c.get_num_materials())],
            bounds=[[min(p[i] for p in points) for i in range(3)], [max(p[i] for p in points) for i in range(3)]],
            horizontal_triangles=floors))
out = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/furnishing_live_geometry.json'
out.write_text(json.dumps(rows))
print('WMCYN_FURNISHING_READONLY_COMPONENTS',len(rows))
