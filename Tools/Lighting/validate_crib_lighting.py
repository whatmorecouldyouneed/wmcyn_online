"""Read-only checks after saving/reloading the Crib lighting pass."""
import hashlib
import json
from pathlib import Path
import unreal

out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
out.mkdir(parents=True, exist_ok=True)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == "/Game/Levels/L_WMCYNOnline.L_WMCYNOnline"
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
ss = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
lights = [a for a in actors if a.get_actor_label().startswith("LGT_Crib_")]
assert len(lights) == 83
assert len({a.get_actor_label() for a in lights}) == 83
for actor in lights:
    light = actor.get_component_by_class(unreal.SpotLightComponent)
    assert light and light.get_editor_property("mobility") == unreal.ComponentMobility.STATIC
    assert light.get_forward_vector().z < -0.999
    assert light.get_editor_property("intensity") > 0

def geometry_signature(mesh):
    """Compare oriented triangles + material UV0, independent of vertex reindexing."""
    sections = []
    for section in range(mesh.get_num_sections(0)):
        vertices, indices, normals, uvs, tangents = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, section)
        triangles = []
        for start in range(0, len(indices), 3):
            corners = [tuple(round(v, 5) for v in (*vertices[i].to_tuple(), *uvs[i].to_tuple()))
                       for i in indices[start:start+3]]
            triangles.append(min(tuple(corners[i:]+corners[:i]) for i in range(3)))
        sections.append(sorted(triangles))
    return hashlib.sha256(json.dumps(sections).encode()).hexdigest()

crib = next(a for a in actors if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
repairs = [{"component": comp.get_name(),
            "source": "/Game/Environments/WMCYN_Crib/" + comp.static_mesh.get_name().removeprefix("SM_Crib_Lightmap_"),
            "replacement": comp.static_mesh.get_path_name().split(".")[0]}
           for comp in crib.get_components_by_class(unreal.StaticMeshComponent)
           if comp.static_mesh and comp.static_mesh.get_name().startswith("SM_Crib_Lightmap_")]
assert len(repairs) == 6, "Missing or unexpected repaired mesh overrides"
for repair in repairs:
    component = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent)
                     if c.get_name() == repair["component"])
    source = unreal.load_asset(repair["source"])
    mesh = component.static_mesh
    assert mesh.get_path_name().split(".")[0] == repair["replacement"], "Override did not survive reload"
    assert geometry_signature(source) == geometry_signature(mesh), "Geometry/material UV0 changed"
    for method in ("get_collision_complexity", "get_convex_collision_count", "get_simple_collision_count"):
        assert getattr(ss, method)(source) == getattr(ss, method)(mesh), "Collision changed"
    assert [source.get_material(i) for i in range(source.get_num_sections(0))] == [mesh.get_material(i) for i in range(mesh.get_num_sections(0))]
    repair["geometry_uv0_collision_verified"] = True
report = {"lights": len(lights), "all_static_and_downward": True, "repairs": repairs}
(out / "validation.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
unreal.log("WMCYN lighting validation passed: 83 static lights; six geometry/UV0/collision-preserving mesh overrides")
