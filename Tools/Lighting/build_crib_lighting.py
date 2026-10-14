"""Bake the current Crib and save only its map and build data on success."""
import argparse
import json
from pathlib import Path
import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == "/Game/Levels/L_WMCYNOnline.L_WMCYNOnline"
parser = argparse.ArgumentParser()
parser.add_argument("--quality", choices=("preview", "medium", "production"), default="preview")
quality = parser.parse_args().quality
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor(), "Stop PIE before baking"
success = level.build_light_maps(getattr(unreal.LightingBuildQuality, "QUALITY_" + quality.upper()), True)
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
(out / "build_result.json").write_text(json.dumps({"success": success, "quality": quality}), encoding="utf-8")
if success:
    assert level.save_current_level()
    if unreal.EditorAssetLibrary.does_asset_exist("/Game/Levels/L_WMCYNOnline_BuiltData"):
        unreal.EditorAssetLibrary.save_asset("/Game/Levels/L_WMCYNOnline_BuiltData", only_if_is_dirty=True)
unreal.log("WMCYN lighting build success: " + str(success))
assert success, "Lighting build failed; inspect Output Log"
