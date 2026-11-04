"""Read-only grid and capsule tests; does not replace headset acceptance."""
import json
from collections import Counter
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib=next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
floor=next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='Plane_004')
samples=[]
for x in range(-1800,-1099,100):
    for y in range(350,901,50):
        hit=floor.line_trace_component(unreal.Vector(x,y,490),unreal.Vector(x,y,430),False,False,False)
        samples.append(dict(x=x,y=y,z=hit[0].z if hit else None))
for x in range(-1750,-1449,100):
    for y in range(50,301,50):
        hit=floor.line_trace_component(unreal.Vector(x,y,490),unreal.Vector(x,y,430),False,False,False)
        samples.append(dict(x=x,y=y,z=hit[0].z if hit else None))
paths=[('WindowAlcove',(-1600,500,570),(-1600,60,570)),
       ('Door03',(-900,385,570),(-1250,385,570)),
       ('Door04',(-990,791,570),(-1300,791,570)),
       ('InterviewInterior',(-1600,420,570),(-1600,850,570))]
capsules=[]
for name,start,end in paths:
    hit=unreal.SystemLibrary.capsule_trace_single_for_objects(world,unreal.Vector(*start),unreal.Vector(*end),
        26.0,90.0,[unreal.ObjectTypeQuery.ECC_WORLD_STATIC],False,[],unreal.DrawDebugTrace.NONE,True)
    capsules.append(dict(path=name,clear=hit is None,hit=str(hit) if hit else None))
out=dict(floor_samples=len(samples),floor_hits=sum(s['z'] is not None for s in samples),
         missing=[s for s in samples if s['z'] is None],capsule_paths=capsules,
         note='Sweeps at Z570 allow the existing raised thresholds; not a stair-walking test.')
def horizontal_faces(path):
    mesh=unreal.load_asset(path)
    verts,indices,_,_,_=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh,0,0)
    result=[]
    for i in range(0,len(indices),3):
        points=[verts[j] for j in indices[i:i+3]]
        n=(points[1]-points[0]).cross(points[2]-points[0])
        if n.length() and abs(n.z)/n.length()>.5:
            result.append(tuple(sorted(tuple(round(v,3) for v in p.to_tuple()) for p in points)))
    return Counter(result)
before=horizontal_faces('/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Collision_Doorways')
after=horizontal_faces('/Game/WMCYN/Environment/Crib/InterviewRoom/SM_InterviewRoom_Collision_01')
out['shell_horizontal_faces_preserved']=before==after
out['shell_horizontal_faces']=sum(before.values())
root=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
(root/'combined_room_tests.json').write_text(json.dumps(out,indent=2))
print('WMCYN_COMBINED_ROOM_TESTS',out)
assert out['floor_hits']==out['floor_samples']
assert all(p['clear'] for p in capsules)
assert out['shell_horizontal_faces_preserved']
