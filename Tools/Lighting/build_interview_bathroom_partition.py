"""Enclose Door 03's bathroom; preserve Door 04 and the interview window."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == '/Game/Levels/L_WMCYNOnline.L_WMCYNOnline'
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube = unreal.load_asset('/Engine/BasicShapes/Cube')
plaster = unreal.load_asset('/Game/WMCYN/Environment/Crib/InterviewRoom/M_InterviewRoom_Plaster')
assert cube and plaster

# Three sides join the existing hallway facade. Ends meet inside wall thickness,
# with the tops buried 1 cm into the current ceiling and bottoms below the floor.
specs = [
    ('ARCH_Bathroom_Door03_WestPartition', (-1460, 333.75, 645.5), (12, 591.5, 371)),
    ('ARCH_Bathroom_Door03_NorthPartition', (-1217, 623.5, 645.5), (498, 12, 371)),
    ('ARCH_Bathroom_Door03_SouthPartition', (-1329, 44, 645.5), (274, 12, 371)),
]
with unreal.ScopedEditorTransaction('WMCYN Door03 bathroom partition'):
    for label, center, size in specs:
        matches = [a for a in sub.get_all_level_actors() if a.get_actor_label() == label]
        assert len(matches) <= 1
        actor = matches[0] if matches else sub.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*center))
        actor.modify()
        actor.set_actor_label(label)
        actor.set_folder_path('WMCYN_Environment/InterviewRoom_Blockout/Bathroom_Door03')
        actor.set_actor_location(unreal.Vector(*center), False, False)
        actor.set_actor_rotation(unreal.Rotator(pitch=0, yaw=0, roll=0), False)
        actor.set_actor_scale3d(unreal.Vector(*(v / 100 for v in size)))
        component = actor.static_mesh_component
        component.set_static_mesh(cube)
        component.set_material(0, plaster)
        component.set_mobility(unreal.ComponentMobility.STATIC)
        component.set_collision_profile_name('BlockAll')

# The extended south partition replaces this shorter coplanar return.
obsolete = next(a for a in sub.get_all_level_actors()
                if a.get_actor_label() == 'ARCH_InterviewRoom_WindowEastReturn')
obsolete.modify()
obsolete.set_actor_hidden_in_game(True)
obsolete.set_is_temporarily_hidden_in_editor(True)
obsolete.set_actor_enable_collision(False)
obsolete.static_mesh_component.set_visibility(False)

light_name = 'DEV_Bathroom_Door03_WorkLight'
infill = next(a for a in sub.get_all_level_actors()
              if a.get_actor_label() == 'ARCH_InterviewRoom_WindowEastFloor')
infill.modify()
infill.set_actor_location(unreal.Vector(-1329,184,456.7), False, False)
infill.set_actor_scale3d(unreal.Vector(2.74,2.80,0.10))

if not any(a.get_actor_label() == light_name for a in sub.get_all_level_actors()):
    light = sub.spawn_actor_from_class(unreal.PointLight, unreal.Vector(-1230,470,790))
    light.set_actor_label(light_name)
    light.set_folder_path('WMCYN_Environment/InterviewRoom_Blockout/Bathroom_Door03')
    component = light.point_light_component
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property('intensity_units', unreal.LightUnits.LUMENS)
    component.set_editor_property('intensity', 200.0)
    component.set_editor_property('attenuation_radius', 550.0)
    component.set_editor_property('source_radius', 15.0)

checks = []
for name, start, end, expected_clear in [
    ('Door03BathroomEntry', (-900,385,570), (-1250,385,570), True),
    ('Door04InterviewEntry', (-990,791,570), (-1300,791,570), True),
    ('InterviewToWindow', (-1600,800,570), (-1600,60,570), True),
    ('BathroomWestClosed', (-1300,470,570), (-1650,470,570), False),
    ('BathroomNorthClosed', (-1300,470,570), (-1300,790,570), False),
    ('BathroomCornerAccessible', (-1300,470,570), (-1300,100,570), True),
    ('BathroomSouthClosed', (-1300,200,570), (-1300,0,570), False),
    ('BathroomExtendedWestClosed', (-1300,200,570), (-1650,200,570), False),
]:
    hit = unreal.SystemLibrary.capsule_trace_single_for_objects(
        world, unreal.Vector(*start), unreal.Vector(*end), 26.0, 90.0,
        [unreal.ObjectTypeQuery.ECC_WORLD_STATIC], False, [], unreal.DrawDebugTrace.NONE, True)
    checks.append(dict(name=name, clear=hit is None, expected_clear=expected_clear,
                       passed=(hit is None) == expected_clear, hit=str(hit) if hit else None))
report = dict(walls=[s[0] for s in specs], checks=checks,
              note='Shell only; no fixtures. Headset traversal and lighting bake pending.')
root = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting'
root.mkdir(parents=True, exist_ok=True)
(root / 'bathroom_partition_tests.json').write_text(json.dumps(report, indent=2))
print('WMCYN_BATHROOM_PARTITION', report)
assert all(c['passed'] for c in checks), 'Review report before saving'
unreal.EditorLevelLibrary.editor_invalidate_viewports()
