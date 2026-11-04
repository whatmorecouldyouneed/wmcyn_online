"""Compile WMCYN baked materials for skeletal mesh rendering."""
import unreal
root = "/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview/Materials"
for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
    material = unreal.load_asset(path)
    if isinstance(material, unreal.Material):
        unreal.MaterialEditingLibrary.set_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
        unreal.MaterialEditingLibrary.recompile_material(material)
        assert unreal.EditorAssetLibrary.save_loaded_asset(material)
print("WMCYN_JARED_SKELETAL_MATERIALS_READY")
