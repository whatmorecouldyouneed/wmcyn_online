"""Read-only doorway/interior collision probes for the user's left-door candidate."""
import json
from pathlib import Path
import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
components = [c for c in crib.get_components_by_class(unreal.StaticMeshComponent)
              if c.static_mesh and c.get_collision_enabled()!=unreal.CollisionEnabled.NO_COLLISION
              and c.get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN)==unreal.CollisionResponseType.ECR_BLOCK]
def trace(start,end):
    hits=[]
    for comp in components:
        for complex_trace in (False,True):
            h=comp.line_trace_component(unreal.Vector(*start),unreal.Vector(*end),complex_trace,False,False)
            if h:
                hits.append(dict(component=comp.get_name(),mesh=comp.static_mesh.get_path_name(),
                                 hidden=bool(comp.get_editor_property('hidden_in_game')),
                                 visible=bool(comp.get_editor_property('visible')),
                                 complex=complex_trace,point=list(h[0].to_tuple())))
    return hits
across=[]
for y in (365,385,405):
    for z in (490,550,620,680):
        across.append(dict(y=y,z=z,hits=trace((-900,y,z),(-1400,y,z))))
floor=[]
for x in (-1050,-1150,-1250,-1350,-1450,-1550,-1650):
    for y in (250,385,520):
        floor.append(dict(x=x,y=y,hits=trace((x,y,510),(x,y,250))))
out=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/interview_room_door03.json'
out.write_text(json.dumps(dict(across=across,floor=floor),indent=2))
print('WMCYN_ROOM03_AUDIT',str(out))
