"""Repair Plane_049 collision on a WMCYN copy without changing its geometry."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == '/Game/Levels/L_WMCYNOnline.L_WMCYNOnline'
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
floor = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name() == 'Plane_049')
source = floor.static_mesh
destination = '/Game/WMCYN/Environment/Crib/Meshes/SM_Crib_Walkable_Scene_086'
assert source.get_path_name() in ['/Game/Environments/WMCYN_Crib/Scene_086.Scene_086', destination+'.SM_Crib_Walkable_Scene_086']
mesh = unreal.load_asset(destination) if unreal.EditorAssetLibrary.does_asset_exist(destination) else unreal.EditorAssetLibrary.duplicate_asset(source.get_path_name(), destination)
body = mesh.get_editor_property('body_setup')
editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
body.set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
body.set_editor_property('double_sided_geometry', True)
editor.set_lod_build_settings(mesh, 0, editor.get_lod_build_settings(mesh, 0))
materials = [floor.get_material(i) for i in range(floor.get_num_materials())]
floor.set_static_mesh(mesh)
for i, material in enumerate(materials):
    floor.set_material(i, material)
# Validate triangle centroids against the rendered surface, not its bounding box.
samples = []
tf = floor.get_world_transform()
for section in range(mesh.get_num_sections(0)):
    vs, ids, _, _, _ = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, section)
    for i in range(0, len(ids), 3):
        a, b, c = [tf.transform_location(vs[j]) for j in ids[i:i+3]]
        n = (b-a).cross(c-a)
        if n.length() > 100 and abs(n.z)/n.length() > .98:
            p = (a+b+c)/3
            hit = floor.line_trace_component(p+unreal.Vector(0,0,2),p-unreal.Vector(0,0,2),False,False,False)
            samples.append(dict(point=list(p.to_tuple()), hit=bool(hit)))
if not samples or not all(s['hit'] for s in samples):
    floor.set_static_mesh(source)
    raise RuntimeError('Floor validation failed; original component mesh restored')
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Lighting/lounge_floor_repair.json').write_text(json.dumps(samples, indent=2))
print('WMCYN_LOUNGE_FLOOR_PASS', len(samples), mesh.get_path_name())
