"""PIE synthetic AFCore hinge regression for every converted door."""
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
world = unreal.EditorLevelLibrary.get_pie_worlds(False)[0]
plan = json.loads((root/'door_rollout_plan.json').read_text())
results = []
for group in plan:
    for item in group['leaves']:
        num = f"{item['number']:02d}"
        path = '/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor_'+num
        cls = unreal.load_class(None,path+'.BP_WMCYN_CribDoor_'+num+'_C')
        actors = unreal.GameplayStatics.get_all_actors_of_class(world,cls)
        assert len(actors)==1,(num,len(actors))
        actor = actors[0]
        drag = next(c for c in actor.get_components_by_class(unreal.ActorComponent) if c.get_class().get_name()=='Comp_Drag_C')
        leaf = next(c for c in actor.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='SM_Lever')
        pivot = leaf.get_world_location()
        offset = unreal.Vector(*item['handle'])
        initial = pivot+offset
        target = pivot+unreal.MathLibrary.rotate_angle_axis(offset,item['swing'],unreal.Vector(0,0,1))
        yaws = []
        try:
            drag.call_method('InitiateDrag',args=(initial,))
            for point in (initial,target,initial):
                for _ in range(90):
                    drag.call_method('AdjustPositionToReferenceLocation',args=(None,actor,point,unreal.Rotator(),1/60,True))
                yaws.append(leaf.get_relative_transform().rotation.rotator().yaw)
        finally:
            drag.call_method('ReleaseDrag')
        passed = abs(yaws[0])<2 and abs(yaws[1]-item['swing'])<2 and abs(yaws[2])<2
        results.append(dict(number=item['number'],yaws=yaws,passed=passed))
(root/'door_rollout_smoke.json').write_text(json.dumps(results,indent=2))
print('WMCYN_DOOR_BATCH_SMOKE',results)
assert all(r['passed'] for r in results),results
