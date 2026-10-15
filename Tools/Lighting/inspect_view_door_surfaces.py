"""Read-only rendered-triangle picking; imported meshes often have no trace collision."""
import json
import math
from pathlib import Path
import unreal

def sub(a, b): return tuple(x-y for x,y in zip(a,b))
def dot(a, b): return sum(x*y for x,y in zip(a,b))
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
origin=(-1043.706938,-277.565580,664.614580)
rays={yaw:unreal.MathLibrary.get_forward_vector(unreal.Rotator(pitch=-6,yaw=yaw,roll=0)).to_tuple() for yaw in (40,50,61,75,85)}
hits={yaw:[] for yaw in rays}
crib=next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
for comp in crib.get_components_by_class(unreal.StaticMeshComponent):
    mesh=comp.static_mesh
    if not mesh or comp.get_name()=='collider': continue
    tf=comp.get_world_transform()
    for section in range(mesh.get_num_sections(0)):
        vertices,indices,normals,uv,tangents=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh,0,section)
        vertices=[tf.transform_location(v).to_tuple() for v in vertices]
        for i in range(0,len(indices),3):
            a,b,c=(vertices[indices[i+j]] for j in range(3))
            e1,e2=sub(b,a),sub(c,a)
            for yaw,d in rays.items():
                p=cross(d,e2);det=dot(e1,p)
                if abs(det)<1e-8: continue
                tvec=sub(origin,a);u=dot(tvec,p)/det
                if u<0 or u>1: continue
                q=cross(tvec,e1);v=dot(d,q)/det
                if v<0 or u+v>1: continue
                t=dot(e2,q)/det
                if 0<t<1500:
                    mat=comp.get_material(section)
                    hits[yaw].append(dict(component=comp.get_name(),mesh=mesh.get_path_name(),section=section,triangle=i//3,distance=t,point=[origin[j]+d[j]*t for j in range(3)],material=mat.get_path_name() if mat else None))
report={yaw:sorted(rows,key=lambda r:r['distance'])[:6] for yaw,rows in hits.items()}
out=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/door_surface_hits.json'
out.write_text(json.dumps(report,indent=2))
print('DOOR_SURFACES',json.dumps(report))
