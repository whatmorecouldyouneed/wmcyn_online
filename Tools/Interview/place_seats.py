"""Place two reusable cushion anchors on the approved interview couch, not on decorative meshes."""
import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Levels/L_WMCYNOnline")
assert world and world.get_name() == "L_WMCYNOnline"
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
lookup = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
couch = lookup["SM_Crib_CouchAbbotsford"]
center, half = couch.get_actor_bounds(False)
# This authored layout faces world +X; fail closed if it is rotated or replaced.
assert 45 < half.x < 65 and 145 < half.y < 180 and 36 < half.z < 55
for suffix, side in (("A", -1), ("B", 1)):
    label = "SEAT_InterviewCouch_" + suffix
    position = center + unreal.Vector(8, side * half.y * 0.40, 0)
    seat = lookup.get(label) or actors.spawn_actor_from_class(unreal.WMCYNSeat, position)
    assert isinstance(seat, unreal.WMCYNSeat)
    seat.set_actor_location_and_rotation(position, unreal.Rotator(), False, True)
    seat.set_actor_label(label)
    seat.set_folder_path("WMCYN_Production/InterviewSeats")
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log("WMCYN_SEATS placed two interview cushion anchors")
