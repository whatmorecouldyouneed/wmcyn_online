"""Find disconnected fixture surfaces without changing imported mesh assets."""
import json
from pathlib import Path
import unreal

out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
out.mkdir(parents=True, exist_ok=True)
actor = next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
             if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
sub = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
islands = []
for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
    mesh = comp.static_mesh
    if not mesh:
        continue
    for section in range(mesh.get_num_sections(0)):
        material = comp.get_material(sub.get_lod_material_slot(mesh, 0, section))
        if not material or mesh.get_num_triangles(0) > 100000:
            continue
        vertices, triangles, normals, uvs, tangents = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, section)
        transform = comp.get_world_transform()
        world = [transform.transform_location(v).to_tuple() for v in vertices]
        parent = list(range(len(world)))
        def root(i):
            while parent[i] != i:
                parent[i] = parent[parent[i]]
                i = parent[i]
            return i
        def union(a, b):
            parent[root(a)] = root(b)
        welded = {}
        for i, p in enumerate(world):
            key = tuple(round(x, 2) for x in p)
            if key in welded:
                union(i, welded[key])
            welded[key] = i
        for i in range(0, len(triangles), 3):
            union(triangles[i], triangles[i + 1])
            union(triangles[i], triangles[i + 2])
        groups = {}
        for i in set(triangles):
            groups.setdefault(root(i), []).append(world[i])
        for points in groups.values():
            lo = [min(p[j] for p in points) for j in range(3)]
            hi = [max(p[j] for p in points) for j in range(3)]
            size = [hi[j] - lo[j] for j in range(3)]
            is_fixture_material = material.get_name() in ("lights", "emissive")
            is_small_horizontal = 5 < size[0] < 65 and 5 < size[1] < 65 and size[2] < 12
            if not (is_fixture_material or is_small_horizontal):
                continue
            islands.append({"component": comp.get_name(), "mesh": mesh.get_path_name(),
                            "material": material.get_name(), "section": section,
                            "min": lo, "max": hi,
                            "center": [(lo[j] + hi[j]) / 2 for j in range(3)],
                            "size": [hi[j] - lo[j] for j in range(3)], "vertices": len(points)})
(out / "fixture_islands.json").write_text(json.dumps(islands, indent=2), encoding="utf-8")
unreal.log("WMCYN fixture survey: %d surfaces" % len(islands))
