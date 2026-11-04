"""Read-only floor support / one-metre corridor occupancy grid for lounge pilot."""
import json
from pathlib import Path
import unreal
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib=next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
floor=next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='Plane_049')
rows=[]
for y in range(-900,0,100):
    for x in range(-1200,-200,100):
        hit=floor.line_trace_component(unreal.Vector(x,y,60),unreal.Vector(x,y,-30),False,False,False)
        z=hit[0].z if hit else None
        obstacle=unreal.SystemLibrary.capsule_trace_single_for_objects(world,unreal.Vector(x,y,105),unreal.Vector(x,y,106),50.,90.,[unreal.ObjectTypeQuery.ECC_WORLD_STATIC],False,[],unreal.DrawDebugTrace.NONE,True)
        rows.append(dict(x=x,y=y,floor_z=z,clear=obstacle is None))
out=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/lounge_pilot_clearance.json'
out.write_text(json.dumps(rows,indent=2))
print('WMCYN_LOUNGE_GRID',len(rows))
