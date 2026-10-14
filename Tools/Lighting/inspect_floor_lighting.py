"""Read-only geometry/lightmap diagnostics for the imported floor surfaces."""
import json
from collections import Counter
from pathlib import Path
import unreal

sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ss = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
report = []
for actor in sub.get_all_level_actors():
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        if comp.get_name() not in ("Plane_003", "Plane_004", "Plane_005", "Plane_048", "Plane_084", "Plane_086"):
            continue
        mesh = comp.static_mesh
        sections = []
        for section in range(mesh.get_num_sections(0)):
            verts, tris, normals, uvs, tangents = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, section)
            world = [comp.get_world_transform().transform_location(v).to_tuple() for v in verts]
            sections.append({"index": section, "triangles": len(tris)//3,
                "normals": Counter(str(tuple(round(v, 2) for v in n.to_tuple())) for n in normals),
                "min": [min(v[i] for v in world) for i in range(3)],
                "max": [max(v[i] for v in world) for i in range(3)]})
        report.append({"component": comp.get_name(), "transform": str(comp.get_world_transform()),
                       "lightmap": mesh.get_editor_property("light_map_coordinate_index"),
                       "resolution": mesh.get_editor_property("light_map_resolution"),
                       "sections": sections})
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting/floors.json"
out.write_text(json.dumps(report, indent=2), encoding="utf-8")
