"""Apply reviewed source materials as reversible WMCYN-owned component overrides."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import unreal

parser = argparse.ArgumentParser()
parser.add_argument("--phase", choices=("architecture", "water", "glass"), default="architecture")
phase = parser.parse_args().phase
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert not level.is_in_play_in_editor()
assert world.get_path_name() == "/Game/Levels/L_WMCYNOnline.L_WMCYNOnline"
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
manifest = json.loads((out / "restoration_manifest.json").read_text())
dest = "/Game/WMCYN/Environment/Crib/Materials/SourceRestoration"
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
saved = set()

def texture(filename, kind):
    name = "T_Crib_" + re.sub(r"[^A-Za-z0-9_]", "_", Path(filename).stem) + "_" + hashlib.sha1(filename.encode()).hexdigest()[:6]
    path = dest + "/Textures/" + name
    tex = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if not tex:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", filename)
        task.set_editor_property("destination_path", dest + "/Textures")
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", False)
        tools.import_asset_tasks([task])
        tex = unreal.load_asset(path)
    assert isinstance(tex, unreal.Texture2D), filename
    normal = kind == "NORMAL"
    tex.set_editor_property("srgb", kind in ("BASE_COLOR", "EMISSIVE_COLOR"))
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP if normal else unreal.TextureCompressionSettings.TC_DEFAULT)
    tex.set_editor_property("max_texture_size", 2048)
    if normal:
        tex.set_editor_property("flip_green_channel", True)
    saved.add(path)
    return tex

def node(mat, cls, **props):
    obj = mel.create_material_expression(mat, cls)
    for key, value in props.items():
        obj.set_editor_property(key, value)
    return obj

def connect(a, output, b, pin):
    pins = list(mel.get_material_expression_input_names(b))
    if pin in ("Coordinates", "Coordinate") and "UVs" in pins:
        pin = "UVs"
    assert mel.connect_material_expressions(a, output, b, pin), (b.get_class().get_name(), pin, pins)

def property_input(expr, prop, channel=""):
    assert mel.connect_material_property(expr, channel, getattr(unreal.MaterialProperty, "MP_" + prop))

def material(name):
    path = dest + "/" + name
    mat = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else tools.create_asset(name, dest, unreal.Material, unreal.MaterialFactoryNew())
    assert mat
    mel.delete_all_material_expressions(mat)
    saved.add(path)
    return mat

replacements = {}
if phase == "architecture":
    for item in manifest["materials"]:
        mat = material("M_Crib_Restored_" + item["target"])
        scale = item.get("uv_scale", {"x": 1, "y": 1})
        uv = node(mat, unreal.MaterialExpressionTextureCoordinate, u_tiling=float(scale["x"]), v_tiling=float(scale["y"]))
        for prop, filename in item["maps"].items():
            sample = node(mat, unreal.MaterialExpressionTextureSample, texture=texture(filename, prop))
            sample.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if prop == "NORMAL" else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if prop in ("BASE_COLOR", "EMISSIVE_COLOR") else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
            connect(uv, "", sample, "Coordinates")
            property_input(sample, prop, "R" if prop in ("ROUGHNESS", "METALLIC") else "RGB")
        if "ROUGHNESS" not in item["maps"]:
            property_input(node(mat, unreal.MaterialExpressionConstant, r=max(0.15, float(item["roughness"]))), "ROUGHNESS")
        mel.layout_material_expressions(mat)
        mel.recompile_material(mat)
        replacements[item["target"]] = mat
elif phase == "water":
    mat = material("M_Crib_Water_Mobile")
    # Opaque, cheap dual-wave water avoids mobile transparency sorting and refraction.
    # It is a presentation surface only: no displacement or collision changes.
    mat.set_editor_property("two_sided", True)
    normals = []
    for filename, speed in zip(manifest["water_normals"], ((0.02, 0.008), (-0.012, 0.018))):
        uv = node(mat, unreal.MaterialExpressionTextureCoordinate, u_tiling=8.0, v_tiling=8.0)
        pan = node(mat, unreal.MaterialExpressionPanner, speed_x=speed[0], speed_y=speed[1])
        connect(uv, "", pan, "Coordinate")
        sample = node(mat, unreal.MaterialExpressionTextureSample, texture=texture(filename, "NORMAL"), sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        connect(pan, "", sample, "Coordinates")
        normals.append(sample)
    blend = node(mat, unreal.MaterialExpressionLinearInterpolate, const_alpha=0.5)
    connect(normals[0], "RGB", blend, "A")
    connect(normals[1], "RGB", blend, "B")
    property_input(blend, "NORMAL")
    tint = node(mat, unreal.MaterialExpressionConstant3Vector, constant=unreal.LinearColor(0.025, 0.11, 0.16, 1))
    property_input(tint, "BASE_COLOR")
    property_input(node(mat, unreal.MaterialExpressionConstant, r=0.18), "ROUGHNESS")
    property_input(node(mat, unreal.MaterialExpressionConstant, r=0.5), "SPECULAR")
    # A small tint keeps unbaked/dark exterior areas readable without a new light.
    glow = node(mat, unreal.MaterialExpressionConstant3Vector, constant=unreal.LinearColor(0.006, 0.018, 0.025, 1))
    property_input(glow, "EMISSIVE_COLOR")
    mel.layout_material_expressions(mat)
    mel.recompile_material(mat)
    replacements = {"water": mat, "water_001": mat}
elif phase == "glass":
    mat = material("M_Crib_Glass_Restored")
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("two_sided", True)
    property_input(node(mat, unreal.MaterialExpressionConstant3Vector, constant=unreal.LinearColor(0.15, 0.18, 0.2, 1)), "BASE_COLOR")
    property_input(node(mat, unreal.MaterialExpressionConstant, r=0.08), "OPACITY")
    property_input(node(mat, unreal.MaterialExpressionConstant, r=0.12), "ROUGHNESS")
    mel.layout_material_expressions(mat)
    mel.recompile_material(mat)
    replacements = {"glass": mat}

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
rollback_path = out / ("restoration_rollback_" + phase + ".json")
baseline = json.loads(rollback_path.read_text()) if rollback_path.exists() else []
seen = {(r["component"], r["slot"]) for r in baseline}
changes = []
for comp in crib.get_components_by_class(unreal.StaticMeshComponent):
    if not comp.static_mesh:
        continue
    for slot in range(comp.get_num_materials()):
        old = comp.get_material(slot)
        if not old:
            continue
        key = old.get_name().removeprefix("M_Crib_Restored_")
        if old.get_path_name().startswith("/Game/Environments/WMCYN_Crib/") and key in replacements:
            record = {"component": comp.get_name(), "slot": slot, "old": old.get_path_name(), "new": replacements[key].get_path_name()}
            if (comp.get_name(), slot) not in seen:
                baseline.append(record)
            changes.append((comp, slot, replacements[key]))
# Persist rollback before assigning any component override.
rollback_path.write_text(json.dumps(baseline, indent=2))
for comp, slot, replacement in changes:
    comp.set_material(slot, replacement)
for path in sorted(saved):
    assert unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
assert level.save_current_level()
(out / ("restoration_result_" + phase + ".json")).write_text(json.dumps({"changed_slots": len(changes), "assets": sorted(saved)}, indent=2))
unreal.log("WMCYN restoration " + phase + ": " + str(len(changes)) + " component material slots")
