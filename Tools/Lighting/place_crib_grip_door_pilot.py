"""Place one reversible, nonblocking standalone grip pilot beside the spawn area."""
import unreal

PATH = '/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor_GripPilot'
TAG = 'WMCYN_StandaloneGripPilot'
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_name() == 'L_WMCYNOnline', 'Wrong map'
assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'Stop PIE before placement'
bp = unreal.load_asset(PATH)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
cls = unreal.load_class(None, PATH + '.BP_WMCYN_CribDoor_GripPilot_C')
unreal.get_default_object(cls).set_editor_property('replicates', True)
unreal.EditorAssetLibrary.save_loaded_asset(bp)
existing = [a for a in sub.get_all_level_actors() if TAG in [str(t) for t in a.tags]]
assert len(existing) <= 1, 'Duplicate pilots; inspect before proceeding'
crib = next(a for a in sub.get_all_level_actors() if a.get_class().get_name().startswith('FbxScene_wmcyn'))
floor = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name() == 'Plane_003')
hit = floor.line_trace_component(unreal.Vector(-200, -400, 650), unreal.Vector(-200, -400, 300), True, False, False)
assert hit and hit[1].z > 0.9, 'No flat walking floor found'
location = unreal.Vector(-200, -400, hit[0].z + 1)
door = existing[0] if existing else sub.spawn_actor_from_class(cls, location)
door.set_actor_location(location, False, True)
door.set_actor_label('DEV_CribDoor_GripPilot_RightHand')
door.set_folder_path('WMCYN_99_Dev_DoorGripPilot')
door.set_editor_property('tags', [TAG])
door.set_replicates(True)
# This first hardware mechanism test must not push/trap the player or obstruct spawn.
for component in door.get_components_by_class(unreal.StaticMeshComponent):
    component.set_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN, unreal.CollisionResponse.IGNORE)
print('WMCYN_DOOR_PILOT_PLACED', door.get_path_name(), location)
unreal.EditorAssetLibrary.save_loaded_asset(bp)
