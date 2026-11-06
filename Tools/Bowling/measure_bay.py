"""Read wall intersections from rendered architecture, excluding props and lane fixtures."""
import unreal as u
sub=u.get_editor_subsystem(u.EditorActorSubsystem)
crib=next(a for a in sub.get_all_level_actors() if 'FbxScene_wmcyn_crib' in a.get_name())
for y in (480,645,810):
    z=-335
    hits=[]
    for c in crib.get_components_by_class(u.StaticMeshComponent):
        mesh=c.static_mesh
        if not mesh: continue
        origin,ext,_=u.SystemLibrary.get_component_bounds(c)
        if abs(y-origin.y)>ext.y+1 or abs(z-origin.z)>ext.z+1 or origin.x-ext.x>-1400: continue
        tr=c.get_world_transform()
        for sec in range(mesh.get_num_sections(0)):
            vs,idx,*_=u.ProceduralMeshLibrary.get_section_from_static_mesh(mesh,0,sec)
            pts=[tr.transform_location(v) for v in vs]
            for j in range(0,len(idx),3):
                a,b,d=(pts[k] for k in idx[j:j+3])
                v=b-a; q=d-a
                den=v.y*q.z-v.z*q.y
                if abs(den)<.001: continue
                s=((y-a.y)*q.z-(z-a.z)*q.y)/den
                t=(v.y*(z-a.z)-v.z*(y-a.y))/den
                if s>=0 and t>=0 and s+t<=1:
                    x=a.x+s*v.x+t*q.x
                    if x < -1400: hits.append((round(x,1),c.get_name()))
    u.log('BAYCAST '+str(y)+' '+str(sorted(set(hits),reverse=True)[:8]))
