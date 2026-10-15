"""Create a WMCYN-only AFCore drag-door prototype; do not replace a level door."""
import unreal

source = '/Game/AFCore/Examples/Blueprints/Samples/BP_Lever'
target = '/Game/WMCYN/Environment/Crib/Doors/BP_WMCYN_CribDoor_GripPilot'
assert not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor()
assert not unreal.EditorAssetLibrary.does_asset_exist(target), 'Pilot exists; inspect before changing it'
bp = unreal.EditorAssetLibrary.duplicate_asset(source, target)
assert bp
ss = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = ss.k2_gather_subobject_data_for_blueprint(bp)
objects = {o.get_name(): o for h in handles
           for o in [unreal.SubobjectDataBlueprintFunctionLibrary.get_object(
               unreal.SubobjectDataBlueprintFunctionLibrary.get_data(h))] if o}
frame = objects['SM_Lever_Base_GEN_VARIABLE']
leaf = objects['SM_Lever_GEN_VARIABLE']
objects['Scene_GEN_VARIABLE'].set_editor_property('relative_rotation', unreal.Rotator(pitch=0, yaw=0, roll=0))
frame.set_editor_property('static_mesh', unreal.load_asset('/Game/AFCore/Examples/Meshes/Door/SM_DoorFrame_Demo'))
leaf.set_editor_property('static_mesh', unreal.load_asset('/Game/AFCore/Examples/Meshes/Door/SM_Door_Demo'))
frame.set_editor_property('override_materials', [])
leaf.set_editor_property('override_materials', [])
frame.set_editor_property('relative_location', unreal.Vector(0, 0, 0))
leaf.set_editor_property('relative_location', unreal.Vector(0, 45, 0))
leaf.set_editor_property('relative_rotation', unreal.Rotator(pitch=0, yaw=0, roll=0))
leaf.set_editor_property('relative_scale3d', unreal.Vector(1, 1, 1))
leaf.set_mobility(unreal.ComponentMobility.MOVABLE)
latch = objects['Comp_Latch_Drag_GEN_VARIABLE']
latch.set_editor_property('autoAttach', False)
latch.set_editor_property('allowSimultaniousLatching', False)
drag = objects['Comp_Drag_GEN_VARIABLE']
settings = drag.get_editor_property('dragType')
assert len(settings) == 1
key = next(iter(settings.keys()))
entry = settings[key]
# AFCore user-defined structs expose their serialized field name to Python.
range_field = 'ValueRange_37_A0D2863A4F0DE164EEE38C93FC835CD5'
limits = entry.get_editor_property(range_field)
limits.set_editor_property('min', 0.0)
limits.set_editor_property('max', 100.0)
entry.set_editor_property(range_field, limits)
settings[key] = entry
drag.set_editor_property('dragType', settings)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert unreal.EditorAssetLibrary.save_asset(target, only_if_is_dirty=False)
unreal.log('WMCYN grip door prototype created; Mimic input adapter requires validation')
