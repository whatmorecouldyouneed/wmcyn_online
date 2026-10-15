"""PIE-only synthetic hand trajectory test; no editor-level actors are changed."""
import json
from pathlib import Path
import unreal

worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
assert worlds, 'Start PIE/simulation first'
world = worlds[0]
cls = unreal.load_class(None, '/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor_GripPilot.BP_WMCYN_CribDoor_GripPilot_C')
actor = next(a for a in unreal.GameplayStatics.get_all_actors_of_class(world, cls)
             if a.actor_has_tag('WMCYN_DoorSyntheticTest'))
result = {}
try:
    drag = next(c for c in actor.get_components_by_class(unreal.ActorComponent) if c.get_class().get_name() == 'Comp_Drag_C')
    leaf = next(c for c in actor.get_components_by_class(unreal.StaticMeshComponent) if c.component_has_tag('Mesh_Lever'))
    hinge = leaf.get_world_location()
    initial = hinge + unreal.Vector(0, -80, 100)
    drag.call_method('InitiateDrag', args=(initial,))
    results = []
    for point in (initial, hinge + unreal.Vector(80, 0, 100), initial):
        for _ in range(60):
            drag.call_method('AdjustPositionToReferenceLocation', args=(None, actor, point, unreal.Rotator(), 1.0/60.0, True))
        results.append(leaf.get_relative_transform().rotation.rotator().yaw)
    drag.call_method('ReleaseDrag')
    result = {'yaw_samples': results, 'synthetic_only': True,
              'passed': abs(results[0]) < 2 and results[1] > 70 and abs(results[2]) < 2}
    assert result['passed'], result
finally:
    actor.destroy_actor()
    (Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting/door_grip_smoke.json').write_text(json.dumps(result, indent=2))
unreal.log('WMCYN door synthetic grip test: ' + str(result))
