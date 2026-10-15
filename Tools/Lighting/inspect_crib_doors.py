"""Read-only door-sized mesh, geometry and build-settings inspection."""
import json
from pathlib import Path
import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
ss = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
report = []
for comp in crib.get_components_by_class(unreal.StaticMeshComponent):
    mesh = comp.static_mesh
    if not mesh or not mesh.get_path_name().startswith("/Game/Environments/WMCYN_Crib/"):
        continue
    low, high = comp.get_local_bounds()
    size = (high - low) * comp.get_world_scale()
    sizes = sorted(abs(v) for v in size.to_tuple())
    if sizes[0] > 80 or not (200 < sizes[2] < 500):
        continue
    center = comp.get_world_transform().transform_location((low + high) * 0.5)
    sections = []
    for index in range(mesh.get_num_sections(0)):
        v, tri, n, uv, tangents = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, index)
        sections.append({"vertices": [list(p.to_tuple()) for p in v], "triangles": list(tri), "normals": [list(p.to_tuple()) for p in n]})
    report.append({"component": comp.get_name(), "mesh": mesh.get_path_name(), "world_center": list(center.to_tuple()), "size": list(size.to_tuple()), "build": str(ss.get_lod_build_settings(mesh, 0)), "sections": sections})
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting/door_audit.json"
out.write_text(json.dumps(report, indent=2))
unreal.log("WMCYN door candidates: " + str(len(report)))
