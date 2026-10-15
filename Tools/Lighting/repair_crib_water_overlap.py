"""Suppress only the malformed duplicate upper water section, preserving collision."""
import json
from pathlib import Path
import unreal

level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor()
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == '/Game/Levels/L_WMCYNOnline.L_WMCYNOnline'
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_name() == 'FbxScene_wmcyn_crib_default_unity_transfer_C_1')
comp = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name() == 'Plane_099')
assert comp.static_mesh.get_path_name() == '/Game/Environments/WMCYN_Crib/Scene_094.Scene_094'
assert comp.static_mesh.get_num_sections(0) == 2 and comp.get_num_materials() == 2
dest = '/Game/WMCYN/Environment/Crib/Materials/SourceRestoration'
name = 'M_Crib_Water_DuplicateSection_Hidden'
path = dest + '/' + name
out = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting'
rollback = out / 'water_overlap_rollback.json'
if not rollback.exists():
    rollback.write_text(json.dumps({'component': comp.get_name(), 'slot': 1,
                                   'material': comp.get_material(1).get_path_name()}, indent=2))
mat = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, dest, unreal.Material, unreal.MaterialFactoryNew())
assert mat
mel = unreal.MaterialEditingLibrary
mel.delete_all_material_expressions(mat)
mat.set_editor_property('blend_mode', unreal.BlendMode.BLEND_MASKED)
mat.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
zero = mel.create_material_expression(mat, unreal.MaterialExpressionConstant)
zero.set_editor_property('r', 0.0)
assert mel.connect_material_property(zero, '', unreal.MaterialProperty.MP_OPACITY_MASK)
mel.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
lower = comp.get_material(0)
collision = comp.get_collision_enabled()
with unreal.ScopedEditorTransaction('Suppress duplicate Crib upper water surface'):
    comp.set_material(1, mat)
assert comp.get_material(0) == lower
assert comp.get_collision_enabled() == collision
assert level.save_current_level()
(out / 'water_overlap_result.json').write_text(json.dumps({
    'component': comp.get_name(), 'hidden_material_slot': 1,
    'lower_pool_slot_preserved': True, 'collision_preserved': True,
    'retained_upper_surface': 'Plane_037', 'retained_waterfall': 'Plane_101',
    'method': 'WMCYN masked section override; original geometry unchanged'
}, indent=2))
unreal.log('WMCYN water: duplicate upper section suppressed; lower pool and waterfall preserved')
