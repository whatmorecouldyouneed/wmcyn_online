"""Read-only editor audit; writes diagnostics under Saved/Diagnostics/Lighting."""
import json
from pathlib import Path
import unreal

out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
out.mkdir(parents=True, exist_ok=True)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
report = {"actors": [], "api": {}}
for actor in actors:
    entry = {"label": actor.get_actor_label(), "path": actor.get_path_name(),
             "class": actor.get_class().get_name(), "components": []}
    for comp in actor.get_components_by_class(unreal.SceneComponent):
        item = {"name": comp.get_name(), "class": comp.get_class().get_name(),
                "location": list(comp.get_world_location().to_tuple())}
        if isinstance(comp, unreal.StaticMeshComponent) and comp.static_mesh:
            mesh = comp.static_mesh
            item.update(mesh=mesh.get_path_name(),
                        materials=[str(comp.get_material(i).get_path_name()) if comp.get_material(i) else None for i in range(comp.get_num_materials())],
                        bounds=str(comp.get_local_bounds()))
        entry["components"].append(item)
    report["actors"].append(entry)
for name in ["StaticMeshDescription", "ProceduralMeshLibrary", "LevelEditorSubsystem", "StaticMesh", "StaticMeshEditorSubsystem", "GeometryScriptLibrary_StaticMeshFunctions", "GeometryScriptLibrary_MeshQueryFunctions", "GeometryScriptLibrary_MeshMaterialFunctions", "EditorLevelLibrary", "EditorBuildUtils", "EditorLevelUtils", "EditorLoadingAndSavingUtils"]:
    cls = getattr(unreal, name, None)
    report["api"][name] = {x: str(getattr(cls, x).__doc__)[:10000] for x in dir(cls or []) if any(s in x.lower() for s in ["mesh_description", "vert", "triangle", "polygon", "section", "material", "lighting", "build", "save_current"])}
(out / "audit.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
unreal.log("WMCYN lighting audit written: " + str(out))
