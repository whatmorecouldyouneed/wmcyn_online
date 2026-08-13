import json
import os
from pathlib import Path

import unreal


def path_of(value):
    try:
        return value.get_path_name() if value else ""
    except Exception:
        return str(value)


def bone_names(skeleton):
    if not skeleton:
        return []
    for method_name in ("get_bone_tree", "get_bone_names"):
        method = getattr(skeleton, method_name, None)
        if method:
            try:
                value = method()
                if value:
                    return [str(item) for item in value]
            except Exception:
                pass
    reference_skeleton = None
    try:
        reference_skeleton = skeleton.get_editor_property("reference_skeleton")
    except Exception:
        pass
    if reference_skeleton:
        try:
            count = reference_skeleton.get_raw_bone_num()
            return [str(reference_skeleton.get_bone_name(index)) for index in range(count)]
        except Exception:
            pass
    return []


def main():
    result = {
        "unreal_version": unreal.SystemLibrary.get_engine_version(),
        "classes": {},
        "assets": {},
        "unreal_names": sorted(
            name for name in dir(unreal)
            if any(token in name.lower() for token in ("ik", "rig", "retarget", "factory"))
        ),
    }
    for class_name in (
        "IKRigController",
        "IKRetargeterController",
        "IKRigDefinition",
        "IKRetargeter",
        "AnimBlueprintFactory",
        "AnimGraphNode_RetargetPoseFromMesh",
        "AnimNode_RetargetPoseFromMesh",
        "Skeleton",
        "ReferenceSkeleton",
        "AnimGraphNode_Base",
        "AnimGraphNode_Root",
        "AnimationGraph",
        "AnimationGraphSchema",
        "AnimBlueprint",
        "Blueprint",
        "BlueprintEditorLibrary",
        "BlueprintFunctionLibrary",
        "EdGraphNode",
        "EdGraphPin",
        "EdGraph",
    ):
        cls = getattr(unreal, class_name, None)
        result["classes"][class_name] = sorted(name for name in dir(cls) if not name.startswith("_")) if cls else None

    asset_paths = [
        "/Game/FullBodyVRTemplate/BluePrints/BP_VRBodyCharacer",
        "/Game/FullBodyVRTemplate/Animation/ABP_VRBody",
        "/Game/FullBodyVRTemplate/Demo/EngineContent/Mannequins/Meshes/SKM_Manny",
        "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/WMCYNAvatar_52b64e54_Body",
        "/Game/MetaHumans/WMCYN/WMCYNAvatar_52b64e54",
        "/MetaHumanCharacter/Female/Medium/NormalWeight/Body/metahuman_base_skel",
        "/MetaHumanCharacter/Female/Medium/NormalWeight/Body/metahuman_base_body",
    ]
    for asset_path in asset_paths:
        asset = unreal.load_asset(asset_path)
        item = {"loaded": bool(asset), "class": asset.get_class().get_name() if asset else ""}
        if asset and isinstance(asset, unreal.SkeletalMesh):
            skeleton = asset.get_editor_property("skeleton")
            item["skeleton"] = path_of(skeleton)
            item["bones"] = bone_names(skeleton)
            item["mesh"] = path_of(asset)
        if asset and isinstance(asset, unreal.AnimBlueprint):
            item["target_skeleton"] = path_of(asset.get_editor_property("target_skeleton"))
        result["assets"][asset_path] = item

    for prefix in (
        "/Game/FullBodyVRTemplate",
        "/Game/WMCYN/AvatarPipeline",
        "/Game/MetaHumans/WMCYN",
    ):
        result["assets"][prefix] = unreal.EditorAssetLibrary.list_assets(prefix, recursive=True, include_folder=False)

    anim_blueprint = unreal.load_asset(
        "/Game/WMCYN/AvatarPipeline/Runtime/WMCYNAvatar_52b64e54/ABP_WMCYN_MetaHuman_Presentation"
    )
    if anim_blueprint:
        graphs = anim_blueprint.get_animation_graphs()
        result["anim_blueprint_graphs"] = []
        for graph in graphs:
            graph_info = {"path": path_of(graph), "class": graph.get_class().get_name()}
            for property_name in ("nodes", "graph_nodes"):
                try:
                    graph_info[property_name] = [path_of(node) for node in graph.get_editor_property(property_name)]
                except Exception as exc:
                    graph_info[property_name] = f"<unavailable: {exc}>"
            result["anim_blueprint_graphs"].append(graph_info)

    output = Path(
        os.environ.get(
            "WMCYN_RETARGET_INSPECT_OUT",
            str(Path(unreal.Paths.project_dir()) / "Saved/AvatarPipeline/Results/wmcyn_retarget_api.json"),
        )
    )
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2, default=str), encoding="utf-8")
    unreal.log(f"WMCYN retarget API inspection written to {output}")


if __name__ == "__main__":
    main()
