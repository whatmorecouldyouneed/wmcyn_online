"""Make the placed tray a carrying pickup without touching its hand-set position."""
import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Levels/L_WMCYNOnline')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
tray = next(a for a in actors if a.get_actor_label() == 'ITEM_Tray')
tray.set_editor_property('grabbable', True)
tray.set_editor_property('carries_items', True)
assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Levels/L_WMCYNOnline')
print('WMCYN_TRAY_CARRY', tray.get_editor_property('grabbable'), tray.get_editor_property('carries_items'), tray.get_actor_location())
