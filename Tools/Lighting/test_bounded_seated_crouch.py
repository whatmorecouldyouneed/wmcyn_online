"""Exercise WMCYN's crouch helper on a temporary editor pawn, never the VR user."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cls = unreal.load_class(None, '/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal.BP_WMCYN_UserPawn_FirstSignal_C')
pawn = sub.spawn_actor_from_class(cls, unreal.Vector(0, 0, -100000), transient=True)
assert pawn
results = []
try:
    offset = pawn.get_editor_property('Height Offset')
    assert offset
    for baseline in [0.0, 59.462421, 85.0, -10.0]:
        camera = pawn.get_editor_property('Camera')
        marker = pawn.get_editor_property('FullBody_Component')
        camera.set_relative_location(unreal.Vector(0, 0, marker.get_relative_transform().translation.z - baseline), False, False)
        pawn.call_method('Calibrate Player Height')
        offset.set_relative_location(unreal.Vector(3, 4, baseline), False, False)
        for _ in range(180):
            pawn.call_method('WMCYNBoundedCrouch', args=(-1.0, 1.0 / 60.0))
        low = offset.get_relative_transform().translation
        assert abs(low.z - (baseline - 45)) < .01, str(low)
        assert abs(low.x - 3) < .01 and abs(low.y - 4) < .01
        for _ in range(180):
            pawn.call_method('WMCYNBoundedCrouch', args=(1.0, 1.0 / 60.0))
        high = offset.get_relative_transform().translation.z
        assert abs(high - baseline) < .01
        pawn.call_method('WMCYNBoundedCrouch', args=(-100.0, 5.0))
        hitch = offset.get_relative_transform().translation.z
        assert abs(hitch - (baseline - 5)) < .01
        assert abs(pawn.get_editor_property('Seated Mode Height Offset') - baseline) < .01
        for _ in range(180):
            pawn.call_method('Custom Crouch', args=(-1.0,))
        dispatched = offset.get_relative_transform().translation.z
        assert dispatched >= baseline - 45.01, dispatched
        results.append(dict(baseline=baseline, minimum=low.z, recovered=high, hitch=hitch))
finally:
    sub.destroy_actor(pawn)
root = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting'
(root / 'bounded_crouch_tests.json').write_text(json.dumps(results, indent=2))
print('WMCYN_BOUNDED_CROUCH_PASS', results)
