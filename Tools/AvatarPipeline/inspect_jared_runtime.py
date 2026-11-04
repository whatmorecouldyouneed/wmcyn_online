"""Read-only inspection of the current Jared presentation assets."""
import json
from pathlib import Path
import unreal

root = "/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview"
paths = [
    root + "/SKM_Jared_Body_Preview",
    root + "/SKM_Jared_Head_Preview",
    "/Game/MetaHumans/Jared/RiggedFit",
    "/Game/MetaHumans/Jared/Shoes",
]
report = []
for path in paths:
    mesh = unreal.load_asset(path)
    if not mesh:
        report.append({"path": path, "missing": True})
        continue
    component = unreal.SkeletalMeshComponent()
    component.set_skeletal_mesh_asset(mesh)
    materials = []
    for slot in mesh.get_editor_property("materials"):
        mat = slot.material_interface
        entry = {"slot": str(slot.material_slot_name), "material": mat.get_path_name() if mat else None}
        if isinstance(mat, unreal.MaterialInstanceConstant):
            entry["textures"] = [
                {"name": str(p.parameter_info.name),
                 "value": p.parameter_value.get_path_name() if p.parameter_value else None}
                for p in mat.get_editor_property("texture_parameter_values")]
        materials.append(entry)
    report.append({
        "path": path,
        "skeleton": str(mesh.get_editor_property("skeleton")),
        "bounds": str(mesh.get_bounds()),
        "head_reference": str(component.get_socket_transform("head", unreal.RelativeTransformSpace.RTS_COMPONENT)),
        "bones": [str(component.get_bone_name(i)) for i in range(component.get_num_bones())],
        "materials": materials,
    })
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Avatars/jared_runtime_inspection.json"
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, indent=2))
print("WMCYN_JARED_RUNTIME_INSPECTED")
