"""Create the first WMCYN-owned AnimBlueprint for the exported MetaHuman body.

This is intentionally a compatibility pass. It creates an AnimBlueprint on the
exported body's skeleton and leaves the graph at reference pose until a
MetaHuman-compatible locomotion graph is authored and tested.
"""

import json
import os
from pathlib import Path

import unreal


BODY_PATH = os.environ.get(
    "WMCYN_AVATAR_BODY_PATH",
    "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/"
    "WMCYNAvatar_52b64e54_Body.WMCYNAvatar_52b64e54_Body",
)
ANIM_PACKAGE = os.environ.get(
    "WMCYN_AVATAR_ANIM_PACKAGE",
    "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54",
)
ANIM_NAME = os.environ.get("WMCYN_AVATAR_ANIM_NAME", "ABP_WMCYN_MetaHuman_Presentation")
RESULT_PATH = Path(
    os.environ.get(
        "WMCYN_AVATAR_ANIM_RESULT",
        "Saved/AvatarPipeline/Results/WMCYNAvatar_52b64e54_Identity_animation.json",
    )
)


def asset_path(asset):
    return asset.get_path_name() if asset else ""


def main():
    body = unreal.load_asset(BODY_PATH)
    if not body:
        raise RuntimeError(f"Could not load exported body mesh: {BODY_PATH}")

    skeleton = body.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError(f"Exported body has no skeleton: {BODY_PATH}")

    package_path = f"{ANIM_PACKAGE}/{ANIM_NAME}"
    anim_blueprint = unreal.load_asset(package_path)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    if not anim_blueprint:
        factory = unreal.AnimBlueprintFactory()
        factory.set_editor_property("parent_class", unreal.AnimInstance)
        factory.set_editor_property("target_skeleton", skeleton)
        anim_blueprint = asset_tools.create_asset(
            ANIM_NAME,
            ANIM_PACKAGE,
            unreal.AnimBlueprint,
            factory,
        )
    else:
        existing_skeleton = anim_blueprint.get_editor_property("target_skeleton")
        if asset_path(existing_skeleton) != asset_path(skeleton):
            raise RuntimeError(
                "Existing WMCYN animation targets a different skeleton: "
                f"{asset_path(existing_skeleton)} != {asset_path(skeleton)}"
            )

    if not anim_blueprint:
        raise RuntimeError(f"Could not create animation blueprint: {package_path}")

    unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)
    generated_class = anim_blueprint.generated_class()
    generated_class_path = asset_path(generated_class)
    if not generated_class_path:
        raise RuntimeError("Animation blueprint has no generated class after save")

    result = {
        "phase": "animation",
        "success": True,
        "bodySkeletalMeshPath": BODY_PATH,
        "targetSkeletonPath": asset_path(skeleton),
        "animBlueprintPath": asset_path(anim_blueprint),
        "animClassPath": generated_class_path,
        "mode": "reference_pose_compatibility_wrapper",
        "note": (
            "This class proves MetaHuman skeleton compatibility. It intentionally has no locomotion graph yet; "
            "native Mimic movement and tracking remain the active pawn source of truth."
        ),
    }

    result_file = Path(unreal.Paths.project_dir()) / RESULT_PATH
    result_file.parent.mkdir(parents=True, exist_ok=True)
    result_file.write_text(json.dumps(result, indent=2), encoding="utf-8")
    unreal.log(f"WMCYN avatar animation wrapper created: {generated_class_path}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        unreal.log_error(f"WMCYN avatar animation wrapper failed: {exc}")
        raise
