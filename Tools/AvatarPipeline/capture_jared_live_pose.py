"""Capture local PIE presentation bones without changing the pawn or its pose."""
import json
from datetime import datetime, timezone
from pathlib import Path

import unreal


def capture():
    worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
    if not worlds:
        raise RuntimeError("Start VR Preview and log in before capturing")
    pawn = unreal.GameplayStatics.get_player_pawn(worlds[0], 0)
    if not pawn:
        raise RuntimeError("No local player pawn")
    components = []
    for component in pawn.get_components_by_class(unreal.SkeletalMeshComponent):
        mesh = component.get_editor_property("skeletal_mesh_asset")
        components.append({
            "component": component.get_name(),
            "mesh": mesh.get_path_name() if mesh else None,
            "visible": component.is_visible(),
            "tick_policy": str(component.get_editor_property("visibility_based_anim_tick_option")),
            "world_transform": str(component.get_world_transform()),
            "bones_world_cm": {
                bone: list(component.get_socket_location(bone).to_tuple())
                for bone in ("root", "pelvis", "head", "hand_l", "hand_r", "foot_l", "foot_r")
                if component.does_socket_exist(bone)
            },
        })
    cameras = pawn.get_components_by_class(unreal.CameraComponent)
    result = {
        "pawn_class": pawn.get_class().get_path_name(),
        "camera_world_cm": list(cameras[0].get_world_location().to_tuple()) if cameras else None,
        "components": components,
    }
    directory = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Avatars"
    directory.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    output = directory / f"jared_live_pose_{stamp}.json"
    output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    unreal.log(f"WMCYN_AVATAR_POSE_CAPTURE {output}")


capture()
