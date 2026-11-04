"""Read-only architectural triangle inventory near the Door 03/04 candidate."""
import json
from pathlib import Path
import unreal

actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib=next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
rows=[]
for c in crib.get_components_by_class(unreal.StaticMeshComponent):
    if not c.static_mesh or (not c.is_visible() and c.get_name()!='collider'): continue
    tf=c.get_world_transform()
    tris=[]
    for section in range(c.static_mesh.get_num_sections(0)):
        verts,indices,normals,uvs,tangents=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(c.static_mesh,0,section)
        for i in range(0,len(indices),3):
            points=[tf.transform_location(verts[j]) for j in indices[i:i+3]]
            coords=[list(v.to_tuple()) for v in points]
            if any(max(v[j] for v in coords)<lo or min(v[j] for v in coords)>hi
                   for j,lo,hi in [(0,-1950,-950),(1,100,1150),(2,450,1050)]): continue
            n=(points[1]-points[0]).cross(points[2]-points[0])
            if n.length()<100: continue
            tris.append(dict(section=section,triangle=i//3,vertices=coords,normal=list((n/n.length()).to_tuple())))
    if tris: rows.append(dict(component=c.get_name(),mesh=c.static_mesh.get_path_name(),triangles=tris))
out=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/combined_room_geometry.json'
out.write_text(json.dumps(rows))
print('WMCYN_COMBINED_ROOM_COMPONENTS',[(r['component'],len(r['triangles'])) for r in rows])
