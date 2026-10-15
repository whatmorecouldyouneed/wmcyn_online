"""Read-only world-space water geometry and normals for surface repair."""
import json
from pathlib import Path
import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
rows = []
for comp in crib.get_components_by_class(unreal.StaticMeshComponent):
    if comp.get_name() not in ("Plane_037", "Plane_099", "Plane_101"):
        continue
    mesh = comp.static_mesh
    transform = comp.get_world_transform()
    sections = []
    for section in range(mesh.get_num_sections(0)):
        v, tri, n, uv, tang = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, section)
        sections.append({"vertices": [list(p.to_tuple()) for p in v], "world_vertices": [list(transform.transform_location(p).to_tuple()) for p in v], "triangles": list(tri), "normals": [list(p.to_tuple()) for p in n], "uvs": [list(p.to_tuple()) for p in uv]})
    rows.append({"component": comp.get_name(), "mesh": mesh.get_path_name(), "transform": str(transform), "sections": sections})
(Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting/water_audit.json").write_text(json.dumps(rows, indent=2))
