"""Repair missing eye/lash maps on WMCYN copies only."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
root = "/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview"
source = "/Game/MetaHumans/Jared"
mel = unreal.MaterialEditingLibrary
report = []
copies = {}
for side in ("L", "R"):
    name = "MI_Eye" + side + "_Baked"
    path = root + "/Materials/" + name
    mat = unreal.load_asset(path) or unreal.EditorAssetLibrary.duplicate_asset(
        source + "/Face/Materials/" + name, path)
    for parameter, texture in {
        "Iris Normal Baked": "T_EyeIris" + side + "_N",
        "Iris Basecolor Baked": "T_EyeIris" + side + "_BC",
        "Sclera Normal Baked": "T_EyeSclera" + side + "_N",
        "Sclera Basecolor Baked": "T_EyeSclera" + side + "_BC",
    }.items():
        tex = unreal.load_asset(source + "/Face/Baked/" + texture)
        assert tex, texture
        values = list(mat.get_editor_property("texture_parameter_values"))
        matched = False
        for value in values:
            if str(value.parameter_info.name) == parameter:
                value.parameter_value = tex
                matched = True
        assert matched, parameter
        mat.set_editor_property("texture_parameter_values", values)
    mel.update_material_instance(mat)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    copies[name] = mat
    report.append(path)

for name in ("MI_Face_Eyelashes", "MI_Face_EyelashesHiLODs"):
    path = root + "/Materials/" + name
    mat = unreal.load_asset(path) or unreal.EditorAssetLibrary.duplicate_asset(
        source + "/Face/Materials/" + name, path)
    tex = unreal.load_asset("/Game/MetaHumans/Common/Textures/Eyelashes/T_Eyelashes_S_Fine_Coverage")
    assert tex
    values = list(mat.get_editor_property("texture_parameter_values"))
    assert values
    for value in values:
        if str(value.parameter_info.name) == "Texture":
            value.parameter_value = tex
    mat.set_editor_property("texture_parameter_values", values)
    mel.update_material_instance(mat)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    copies[name] = mat
    report.append(path)

head = unreal.load_asset(root + "/SKM_Jared_Head_Preview")
slots = list(head.get_editor_property("materials"))
for slot in slots:
    original = slot.material_interface
    if original and original.get_name() in copies:
        slot.material_interface = copies[original.get_name()]
head.set_editor_property("materials", slots)
assert unreal.EditorAssetLibrary.save_loaded_asset(head)
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Avatars/jared_face_repair.json"
out.write_text(json.dumps({"repaired_materials": report}, indent=2))
print("WMCYN_JARED_FACE_MATERIALS_REPAIRED")
