"""Read-only live Crib material graph and import-source inventory."""
import json
from pathlib import Path
import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
components = []
materials = {}
for actor in actors:
    if not actor.get_actor_label().startswith("FbxScene_wmcyn"):
        continue
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = comp.static_mesh
        if not mesh:
            continue
        slots = []
        for i in range(comp.get_num_materials()):
            mat = comp.get_material(i)
            if not mat:
                slots.append(None)
                continue
            path = mat.get_path_name()
            slots.append(path)
            if path in materials or not path.startswith("/Game/Environments/WMCYN_Crib/"):
                continue
            entry = {"class": mat.get_class().get_name(), "inputs": {}}
            if isinstance(mat, unreal.Material):
                entry["blend_mode"] = str(mat.get_editor_property("blend_mode"))
                entry["two_sided"] = mat.get_editor_property("two_sided")
                for prop in ("BASE_COLOR", "NORMAL", "ROUGHNESS", "METALLIC", "OPACITY", "EMISSIVE_COLOR", "WORLD_POSITION_OFFSET"):
                    node = unreal.MaterialEditingLibrary.get_material_property_input_node(mat, getattr(unreal.MaterialProperty, "MP_" + prop))
                    value = {"node": node.get_class().get_name()} if node else None
                    if node:
                        for field in ("texture", "default_value", "r", "constant"):
                            try:
                                field_value = node.get_editor_property(field)
                                value[field] = field_value.get_path_name() if isinstance(field_value, unreal.Object) else str(field_value)
                            except Exception:
                                pass
                    entry["inputs"][prop] = value
            materials[path] = entry
        try:
            sources = list(mesh.get_editor_property("asset_import_data").extract_filenames())
        except Exception:
            sources = []
        components.append({"name": comp.get_name(), "mesh": mesh.get_path_name(), "transform": str(comp.get_world_transform()), "materials": slots, "source": sources})
output = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting/crib_material_audit.json"
output.write_text(json.dumps({"components": components, "materials": materials}, indent=2), encoding="utf-8")
unreal.log("WMCYN material audit: " + str(output))
