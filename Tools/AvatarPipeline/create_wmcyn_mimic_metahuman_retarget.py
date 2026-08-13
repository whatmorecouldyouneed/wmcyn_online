"""Create WMCYN-owned IK Rig and IK Retargeter assets for the live avatar bridge.

The Mimic pawn remains the authoritative tracked body. These assets only convert
that live Manny pose to the exported MetaHuman presentation skeleton.
"""

import json
import os
from pathlib import Path

import unreal


SOURCE_MESH_PATH = os.environ.get(
    "WMCYN_RETARGET_SOURCE_MESH_PATH",
    "/Game/FullBodyVRTemplate/Demo/EngineContent/Mannequins/Meshes/SKM_Manny.SKM_Manny",
)
TARGET_MESH_PATH = os.environ.get(
    "WMCYN_RETARGET_TARGET_MESH_PATH",
    "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/"
    "WMCYNAvatar_52b64e54_Body.WMCYNAvatar_52b64e54_Body",
)
PACKAGE_PATH = os.environ.get(
    "WMCYN_RETARGET_PACKAGE_PATH",
    "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/Retargeting",
)
SOURCE_RIG_NAME = "IKR_WMCYN_Mimic_Source"
TARGET_RIG_NAME = "IKR_WMCYN_MetaHuman_Target"
RETARGETER_NAME = "RTG_WMCYN_Mimic_To_MetaHuman"
RESULT_PATH = Path(
    os.environ.get(
        "WMCYN_RETARGET_RESULT",
        "Saved/AvatarPipeline/Results/WMCYNAvatar_52b64e54_Identity_retarget.json",
    )
)


CHAINS = (
    ("Root", "root", "root"),
    ("Spine", "spine_01", "spine_03"),
    ("Neck", "neck_01", "neck_01"),
    ("Head", "head", "head"),
    ("Arm_L", "upperarm_l", "hand_l"),
    ("Arm_R", "upperarm_r", "hand_r"),
    ("Leg_L", "thigh_l", "foot_l"),
    ("Leg_R", "thigh_r", "foot_r"),
)


def path_of(asset):
    return asset.get_path_name() if asset else ""


def ensure_folder(package_path):
    unreal.EditorAssetLibrary.make_directory(package_path)


def ensure_ik_rig(asset_tools, name, mesh):
    asset_path = f"{PACKAGE_PATH}/{name}"
    rig = unreal.load_asset(asset_path)
    if not rig:
        rig = asset_tools.create_asset(
            name,
            PACKAGE_PATH,
            unreal.IKRigDefinition,
            unreal.IKRigDefinitionFactory(),
        )
    if not rig:
        raise RuntimeError(f"Could not create IK Rig: {asset_path}")

    controller = unreal.IKRigController.get_controller(rig)
    if not controller:
        raise RuntimeError(f"Could not get IK Rig controller: {asset_path}")

    if path_of(controller.get_skeletal_mesh()) != path_of(mesh):
        if not controller.set_skeletal_mesh(mesh):
            raise RuntimeError(f"Could not set skeletal mesh on IK Rig: {asset_path}")

    controller.set_retarget_root("pelvis")
    chains = []
    existing = {str(chain.get_editor_property("chain_name")) for chain in controller.get_retarget_chains()}
    for chain_name, start_bone, end_bone in CHAINS:
        if chain_name not in existing:
            if not controller.add_retarget_chain(chain_name, start_bone, end_bone, ""):
                raise RuntimeError(
                    f"Could not add chain {chain_name} to {asset_path}: "
                    f"{start_bone} -> {end_bone}"
                )
        chains.append(chain_name)

    unreal.EditorAssetLibrary.save_loaded_asset(rig)
    return rig, chains


def ensure_retargeter(asset_tools, source_rig, target_rig, source_mesh, target_mesh):
    asset_path = f"{PACKAGE_PATH}/{RETARGETER_NAME}"
    retargeter = unreal.load_asset(asset_path)
    if not retargeter:
        retargeter = asset_tools.create_asset(
            RETARGETER_NAME,
            PACKAGE_PATH,
            unreal.IKRetargeter,
            unreal.IKRetargetFactory(),
        )
    if not retargeter:
        raise RuntimeError(f"Could not create IK Retargeter: {asset_path}")

    controller = unreal.IKRetargeterController.get_controller(retargeter)
    if not controller:
        raise RuntimeError(f"Could not get IK Retargeter controller: {asset_path}")

    source_target = unreal.RetargetSourceOrTarget
    controller.set_ik_rig(source_target.SOURCE, source_rig)
    controller.set_ik_rig(source_target.TARGET, target_rig)
    controller.set_preview_mesh(source_target.SOURCE, source_mesh)
    controller.set_preview_mesh(source_target.TARGET, target_mesh)
    controller.add_default_ops()
    auto_map_enum = getattr(unreal, "AutoMapChainType", None)
    if auto_map_enum is None:
        auto_map_enum = getattr(unreal, "EAutoMapChainType", None)
    if auto_map_enum is None:
        raise RuntimeError("UE Python did not expose AutoMapChainType")
    auto_map_type = getattr(auto_map_enum, "EXACT", None)
    if auto_map_type is None:
        auto_map_type = getattr(auto_map_enum, "FUZZY", None)
    if auto_map_type is None:
        raise RuntimeError(f"UE Python did not expose an exact/fuzzy auto-map enum: {dir(auto_map_enum)}")
    controller.auto_map_chains(auto_map_type, True)
    unreal.EditorAssetLibrary.save_loaded_asset(retargeter)
    return retargeter, controller


def main():
    source_mesh = unreal.load_asset(SOURCE_MESH_PATH)
    target_mesh = unreal.load_asset(TARGET_MESH_PATH)
    if not source_mesh:
        raise RuntimeError(f"Could not load Mimic source mesh: {SOURCE_MESH_PATH}")
    if not target_mesh:
        raise RuntimeError(f"Could not load MetaHuman target mesh: {TARGET_MESH_PATH}")

    ensure_folder(PACKAGE_PATH)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    source_rig, source_chains = ensure_ik_rig(asset_tools, SOURCE_RIG_NAME, source_mesh)
    target_rig, target_chains = ensure_ik_rig(asset_tools, TARGET_RIG_NAME, target_mesh)
    retargeter, controller = ensure_retargeter(
        asset_tools, source_rig, target_rig, source_mesh, target_mesh
    )

    anim_blueprint = unreal.load_asset(
        "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/"
        "ABP_WMCYN_MetaHuman_Presentation"
    )
    if not anim_blueprint:
        raise RuntimeError("Could not load WMCYN MetaHuman presentation AnimBlueprint")
    editor_bridge = getattr(unreal, "WMCYNRetargetEditorLibrary", None)
    if editor_bridge is None:
        raise RuntimeError("WMCYNRuntimeEditor module did not expose the retarget editor bridge")
    if not editor_bridge.build_meta_human_retarget_anim_graph(anim_blueprint, retargeter):
        raise RuntimeError("WMCYN editor bridge could not build the MetaHuman retarget graph")
    unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)

    output = Path(unreal.Paths.project_dir()) / RESULT_PATH
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(
            {
                "phase": "mimic_to_metahuman_retarget",
                "success": True,
                "sourceMeshPath": SOURCE_MESH_PATH,
                "targetMeshPath": TARGET_MESH_PATH,
                "sourceIKRigPath": path_of(source_rig),
                "targetIKRigPath": path_of(target_rig),
                "retargeterPath": path_of(retargeter),
                "sourceChains": source_chains,
                "targetChains": target_chains,
                "retargetOpCount": controller.get_num_retarget_ops(),
                "animBlueprintPath": anim_blueprint.get_path_name(),
                "animGraphBridge": "Retarget Pose From Mesh -> AnimGraph Root",
                "runtimeMode": "ParentSkeletalMeshComponent",
                "note": (
                    "The native Mimic body remains authoritative. The WMCYN MetaHuman visual "
                    "uses Retarget Pose From Mesh with the native Mimic BodyMesh as its parent source."
                ),
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    unreal.log(f"WMCYN Mimic-to-MetaHuman retarget assets written to {output}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        unreal.log_error(f"WMCYN Mimic-to-MetaHuman retarget failed: {exc}")
        raise
