"""Build WMCYN's pose-copy head graph; no source/framework edits."""
import unreal

root = "/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview"
name = "ABP_Jared_Head_CopyPose"
head = unreal.load_asset(root + "/SKM_Jared_Head_Preview")
assert head and head.get_editor_property("skeleton")
bp = unreal.load_asset(root + "/" + name)
if not bp:
    factory = unreal.AnimBlueprintFactory()
    factory.set_editor_property("target_skeleton", head.get_editor_property("skeleton"))
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, root, unreal.AnimBlueprint, factory)
assert unreal.WMCYNRetargetEditorLibrary.build_attached_copy_pose_anim_graph(bp)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp)
print("WMCYN_JARED_HEAD_COPYPOSE_READY")
