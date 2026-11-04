"""Read-only stage-area geometry survey; run from the editor Python console."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
rows = []
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        if not comp.static_mesh or not comp.get_editor_property('visible'):
            continue
        origin, extent, radius = unreal.SystemLibrary.get_component_bounds(comp)
        if origin.x-extent.x > -500 or origin.x+extent.x < -2500 or origin.y-extent.y > -1000 or origin.y+extent.y < -3000:
            continue
        samples = []
        for x in range(-2200, -600, 200):
            for y in range(-2800, -1000, 200):
                hit = comp.line_trace_component(unreal.Vector(x,y,700),unreal.Vector(x,y,-200),True,False,False)
                if hit:
                    samples.append([x,y,round(hit[0].z,1),round(hit[1].z,2)])
        triangles = []
        transform = comp.get_world_transform()
        for section in range(comp.static_mesh.get_num_sections(0)):
            verts, indices, _, _, _ = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(comp.static_mesh,0,section)
            for i in range(0,len(indices),3):
                vs = [transform.transform_location(verts[j]) for j in indices[i:i+3]]
                p = sum(vs,unreal.Vector())/3
                n = (vs[1]-vs[0]).cross(vs[2]-vs[0])
                if -2500<p.x<-500 and -3000<p.y<-1000 and n.length()>1 and abs(n.z/n.length())>.8:
                    triangles.append([list(v.to_tuple()) for v in vs])
        rows.append(dict(actor=actor.get_actor_label(),component=comp.get_name(),mesh=comp.static_mesh.get_path_name(),origin=list(origin.to_tuple()),extent=list(extent.to_tuple()),materials=[str(comp.get_material(i).get_name()) if comp.get_material(i) else None for i in range(comp.get_num_materials())],triangles=triangles,samples=samples))
out = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/stage_alcove.json'
out.parent.mkdir(parents=True,exist_ok=True)
out.write_text(json.dumps(rows,indent=2))
print('WMCYN_STAGE_SURVEY',len(rows))
