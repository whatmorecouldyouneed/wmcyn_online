"""Repair verified WMCYN floor duplicates, preserving geometry and source assets."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == '/Game/Levels/L_WMCYNOnline.L_WMCYNOnline'
root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
rows = json.loads((root/'crib_walkable_surfaces.json').read_text())
crib = next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
            if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
report = []
for name, suffix in [('Plane_004','059'),('Plane_005','060'),('Plane_084','090')]:
    comp = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()==name)
    mesh = comp.static_mesh
    assert mesh.get_path_name().startswith('/Game/WMCYN/Environment/Crib/Meshes/SM_Crib_Lightmap_Scene_'+suffix+'.')
    body = mesh.get_editor_property('body_setup')
    old_flag = body.get_editor_property('collision_trace_flag')
    old_double = body.get_editor_property('double_sided_geometry')
    samples = [unreal.Vector(*r['point']) for r in rows if r['component']==name]
    assert samples
    def hits():
        return sum(bool(comp.line_trace_component(p+unreal.Vector(0,0,2),p-unreal.Vector(0,0,2),False,False,False)) for p in samples)
    before = hits()
    try:
        body.set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        body.set_editor_property('double_sided_geometry', True)
        editor.set_lod_build_settings(mesh,0,editor.get_lod_build_settings(mesh,0))
        comp.set_static_mesh(None)
        comp.set_static_mesh(mesh)
        after = hits()
        assert after == len(samples), (name,after,len(samples))
        assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        report.append(dict(component=name,mesh=mesh.get_path_name(),before=before,after=after,
                           samples=len(samples),old_flag=str(old_flag),old_double_sided=old_double))
    except Exception:
        body.set_editor_property('collision_trace_flag', old_flag)
        body.set_editor_property('double_sided_geometry', old_double)
        editor.set_lod_build_settings(mesh,0,editor.get_lod_build_settings(mesh,0))
        comp.set_static_mesh(None)
        comp.set_static_mesh(mesh)
        raise
(root/'walkable_collision_repair.json').write_text(json.dumps(report,indent=2))
print('WMCYN_WALKABLE_REPAIR',report)
