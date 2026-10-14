"""One editor command: survey, repair lightmaps, place lights, bake, validate."""
import argparse
from pathlib import Path
import runpy
import sys
import unreal

parser = argparse.ArgumentParser()
parser.add_argument("--quality", choices=("preview", "medium", "production"), default="production")
args = parser.parse_args()
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert not level.is_in_play_in_editor(), "Stop PIE first"
assert world.get_path_name() == "/Game/Levels/L_WMCYNOnline.L_WMCYNOnline", "Open L_WMCYNOnline first"
directory = Path(__file__).resolve().parent
original_argv = sys.argv[:]
try:
    for filename, options in [
        ("survey_crib_fixtures.py", []),
        ("repair_floor_lightmaps.py", []),
        ("apply_crib_lighting.py", ["--phase", "all"]),
        ("build_crib_lighting.py", ["--quality", args.quality]),
        ("validate_crib_lighting.py", []),
    ]:
        path = directory / filename
        sys.argv = [str(path), *options]
        runpy.run_path(str(path), run_name="__main__")
finally:
    sys.argv = original_argv
unreal.log("WMCYN Crib lighting batch complete; device acceptance remains a headset test")
