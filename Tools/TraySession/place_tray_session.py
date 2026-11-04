"""First-time placement of the tray session near the interview couch. Refuses to rerun.

Offsets come from WEED SESSION WITH EFFECTS.prefab. Unity (x, y, z) in metres maps to
Unreal (z, x, y) in centimetres, which keeps the layout's handedness.
"""
import json
from pathlib import Path

import unreal

ROOT = '/Game/WMCYN/Interactables/TraySession'
TAG = 'WMCYN_TraySession'
FOLDER = 'WMCYN_Production/TraySession'
# On the interview rug, facing the couch at X -1127.
TABLE_CENTRE = unreal.Vector(-1000, -481, 470)
# Tray collider centre in session space: the tray's own pivot sits .054 m toward the ashtray side.
TRAY_UNITY = (.079, .0084, .3056)
TRAY_FLOOR_THICKNESS = 1.0

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Levels/L_WMCYNOnline')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
# The founder hand-placed the session on the living room coffee table. Never overwrite that.
if any(unreal.Name(TAG) in actor.tags for actor in actors.get_all_level_actors()):
    raise SystemExit('Tray session already placed; delete it by hand to place it again.')
source_table = next(a for a in actors.get_all_level_actors() if a.get_actor_label() == 'FURN_Lounge_CoffeeTable_01')


def asset(path):
    loaded = unreal.load_asset(f'{ROOT}/{path}')
    assert loaded, path
    return loaded


def spawn(cls, label, location, rotation=unreal.Rotator(0, 0, 0)):
    actor = actors.spawn_actor_from_class(cls, location, rotation)
    actor.set_actor_label(label)
    actor.set_folder_path(FOLDER)
    actor.tags = [unreal.Name(TAG)]
    return actor


def mesh_component(actor):
    return actor.get_component_by_class(unreal.StaticMeshComponent)


table = spawn(unreal.StaticMeshActor, 'FURN_Interview_CoffeeTable', TABLE_CENTRE)
table_component = table.static_mesh_component
source_component = source_table.static_mesh_component
table_component.set_static_mesh(source_component.static_mesh)
for index in range(source_component.get_num_materials()):
    table_component.set_material(index, source_component.get_material(index))
table.set_actor_rotation(source_table.get_actor_rotation(), False)
table.set_actor_scale3d(source_table.get_actor_scale3d())
origin, extent = table.get_actor_bounds(False)
table.add_actor_world_offset(unreal.Vector(TABLE_CENTRE.x - origin.x, TABLE_CENTRE.y - origin.y, TABLE_CENTRE.z + extent.z - origin.z), False, False)
top = TABLE_CENTRE.z + extent.z * 2


def session(unity, z_offset=0.0):
    """Convert a session position to world, measured from the tray's centre on the table."""
    return unreal.Vector(TABLE_CENTRE.x + (unity[2] - TRAY_UNITY[2]) * 100,
                         TABLE_CENTRE.y + (unity[0] - TRAY_UNITY[0]) * 100, top + z_offset)


def fixture(label, mesh, location, rotation, kind, grabbable=False):
    actor = spawn(unreal.WMCYNSessionFixture, label, location, rotation)
    mesh_component(actor).set_static_mesh(asset('Meshes/' + mesh))
    actor.set_editor_property('kind', kind)
    actor.set_editor_property('grabbable', grabbable)
    return actor


kinds = unreal.WMCYNFixtureKind
tray = fixture('ITEM_Tray', 'SM_Tray_Tray', session(TRAY_UNITY), unreal.Rotator(0, 0, 90), kinds.DECOR, grabbable=True)
tray.set_editor_property('carries_items', True)
fixture('ITEM_Ashtray', 'SM_Tray_Ashtray', session((.169, 0, -.330)), unreal.Rotator(0, 0, 0), kinds.ASHTRAY, grabbable=True)
fixture('ITEM_Papers', 'SM_Tray_Papers', session((.166, 0, .2884), TRAY_FLOOR_THICKNESS), unreal.Rotator(0, 0, 90), kinds.PAPERS)
# Grinder is a child of the tray in the prefab, near its centre.
fixture('ITEM_Grinder', 'SM_Tray_Grinder', session((.0, 0, .2518), TRAY_FLOOR_THICKNESS), unreal.Rotator(0, 0, 0), kinds.DECOR)

joint_rotation = unreal.MathLibrary.make_rot_from_z(unreal.Vector(0, -1, 0))
overlay = asset('Materials/M_Tray_HighOverlay')
for index, depth in enumerate((.139, .227, .3687, .4682)):
    joint = spawn(unreal.WMCYNJointItem, f'ITEM_Joint_{index + 1}', session((.098, 0, depth), TRAY_FLOOR_THICKNESS + 1), joint_rotation)
    mesh_component(joint).set_static_mesh(asset('Meshes/SM_Tray_Joint'))
    joint.set_editor_property('high_overlay_material', overlay)

lighter = spawn(unreal.WMCYNLighterItem, 'ITEM_Lighter', session((-.054, 0, -.097)))
parts = lighter.get_components_by_class(unreal.StaticMeshComponent)
for part in parts:
    if part.get_name() == 'Flame':
        part.set_material(0, asset('Materials/M_Tray_Flame'))
    else:
        part.set_static_mesh(asset('Meshes/SM_Tray_Lighter'))
sounds = {path.split('.')[0].rsplit('/', 1)[-1]: path for path in unreal.EditorAssetLibrary.list_assets(f'{ROOT}/Sounds', recursive=False)}
lighter.set_editor_property('light_sounds', [unreal.load_asset(sounds[f'S_Tray_LighterFire{i}']) for i in range(4) if f'S_Tray_LighterFire{i}' in sounds])
lighter.set_editor_property('fail_sounds', [unreal.load_asset(sounds[f'S_Tray_LighterNotFire{i}']) for i in range(4) if f'S_Tray_LighterNotFire{i}' in sounds])

assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Levels/L_WMCYNOnline')
placed = [dict(label=a.get_actor_label(), location=[round(v, 1) for v in (a.get_actor_location().x, a.get_actor_location().y, a.get_actor_location().z)])
          for a in actors.get_all_level_actors() if unreal.Name(TAG) in a.tags]
report = dict(table_top=top, placed=placed, sounds=sorted(sounds))
out = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())) / 'Saved/Diagnostics/TraySession'
(out / 'placement.json').write_text(json.dumps(report, indent=1))
print('WMCYN_TRAY_PLACED', json.dumps(report))
