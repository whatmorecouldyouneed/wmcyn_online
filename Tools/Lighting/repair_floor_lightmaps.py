"""Repair baking UVs on WMCYN duplicates; retain source geometry and collision."""
import json
from pathlib import Path
import unreal

sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ss = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor(), "Stop PIE before repairing lightmaps"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == "/Game/Levels/L_WMCYNOnline.L_WMCYNOnline"
crib = next(a for a in sub.get_all_level_actors()
            if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
repairs = [("Plane_004", "Scene_059"), ("Plane_005", "Scene_060"),
           ("Plane_048", "Scene_085"), ("Plane_084", "Scene_090"),
           ("Plane_086", "Scene_091"), ("Plane_124", "Scene_097")]
report = []
for component_name, source_name in repairs:
    comp = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent)
                if c.get_name() == component_name)
    destination = "/Game/WMCYN/Environment/Crib/Meshes/SM_Crib_Lightmap_" + source_name
    mesh = unreal.load_asset(destination) if unreal.EditorAssetLibrary.does_asset_exist(destination) else unreal.EditorAssetLibrary.duplicate_asset(
        "/Game/Environments/WMCYN_Crib/" + source_name, destination)
    assert mesh
    source = unreal.load_asset("/Game/Environments/WMCYN_Crib/" + source_name)
    bounds = mesh.get_bounds()
    source_collision = source.get_editor_property("body_setup").get_editor_property("collision_trace_flag")
    assert ss.generate_box_uv_channel(mesh, 0, 1, bounds.origin,
        unreal.Rotator(), bounds.box_extent * 2)
    settings = ss.get_lod_build_settings(mesh, 0)
    settings.generate_lightmap_u_vs = True
    settings.src_lightmap_index = 1
    settings.dst_lightmap_index = 2
    settings.min_lightmap_resolution = 256
    ss.set_lod_build_settings(mesh, 0, settings)
    mesh.set_editor_property("light_map_coordinate_index", 2)
    mesh.set_editor_property("light_map_resolution", 512)
    comp.set_static_mesh(mesh)
    new_bounds, old_bounds = mesh.get_bounds(), source.get_bounds()
    for attribute in ("origin", "box_extent"):
        assert all(abs(a-b) < 0.0001 for a, b in zip(getattr(new_bounds, attribute).to_tuple(),
                   getattr(old_bounds, attribute).to_tuple())), "Geometry bounds changed"
    assert mesh.get_editor_property("body_setup").get_editor_property("collision_trace_flag") == source_collision
    report.append({"component": component_name, "source": source.get_path_name(),
                   "replacement": destination, "collision_trace_flag": str(source_collision)})
    assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
assert level.save_current_level()
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting/lightmap_repairs.json"
out.write_text(json.dumps(report, indent=2), encoding="utf-8")
