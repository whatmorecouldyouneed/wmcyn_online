"""Exercise the real separated leaf through AFCore in PIE, not controller input."""
import json
from pathlib import Path
import unreal
world=unreal.EditorLevelLibrary.get_pie_worlds(False)[0]
cls=unreal.load_class(None,'/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor_01.BP_WMCYN_CribDoor_01_C')
actor=unreal.GameplayStatics.get_all_actors_of_class(world,cls)[0]
drag=next(c for c in actor.get_components_by_class(unreal.ActorComponent) if c.get_class().get_name()=='Comp_Drag_C')
leaf=next(c for c in actor.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='SM_Lever')
pivot=leaf.get_world_location()
offset=unreal.Vector(23.5,-87,110.4)
initial=pivot+offset
closed=pivot+unreal.MathLibrary.rotate_angle_axis(offset,-100,unreal.Vector(0,0,1))
results=[]
try:
    drag.call_method('InitiateDrag',args=(initial,))
    for point in (initial,closed,initial):
        for _ in range(90):
            drag.call_method('AdjustPositionToReferenceLocation',args=(None,actor,point,unreal.Rotator(),1/60,True))
        results.append(leaf.get_relative_transform().rotation.rotator().yaw)
finally:
    drag.call_method('ReleaseDrag')
result=dict(yaw_samples=results,synthetic_only=True,passed=len(results)==3 and abs(results[0])<2 and results[1]<-95 and abs(results[2])<2)
(Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/door_01_smoke.json').write_text(json.dumps(result,indent=2))
print('WMCYN_REAL_DOOR_SMOKE',result)
assert result['passed'],result
