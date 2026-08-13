from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
import re
import shutil
import sys
from typing import Any, Dict, Optional

import unreal


DEFAULT_STORAGE_PATH = "/Game/WMCYN/AvatarPipeline"
DEFAULT_CHARACTER_PACKAGE = "/Game/MetaHumans/WMCYN"
DEFAULT_BUILD_PATH = "/Game/MetaHumans/WMCYN"
DEFAULT_COMMON_PATH = "/Game/MetaHumans/Common"
DEFAULT_BODY_INDEX = 1
DEFAULT_PIPELINE_QUALITY = "medium"
DEFAULT_CAPTURE_MEDIA_SUBDIR = os.path.join("AvatarPipeline", "CaptureManagerMedia")


def _log(message: str) -> None:
    unreal.log(f"WMCYN Avatar Pipeline: {message}")


def _warn(message: str) -> None:
    unreal.log_warning(f"WMCYN Avatar Pipeline: {message}")


def _error(message: str) -> None:
    unreal.log_error(f"WMCYN Avatar Pipeline: {message}")


def _sanitize_asset_name(value: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9_]+", "_", value.strip())
    cleaned = cleaned.strip("_")
    return cleaned or "WMCYNAvatar"


def _normalize_content_path(value: str) -> str:
    trimmed = value.strip()
    if not trimmed.startswith("/Game"):
        raise RuntimeError(f"Expected a /Game content path, got: {value}")
    return trimmed.rstrip("/")


def _append_plugin_python_path(relative_path: str) -> None:
    plugin_root = unreal.Paths.engine_plugins_dir()
    absolute_path = os.path.join(plugin_root, relative_path)
    if absolute_path not in sys.path:
        sys.path.append(absolute_path)


def _load_epic_python_helpers() -> None:
    _append_plugin_python_path(r"MetaHuman\MetaHumanAnimator\Content\Python")
    _append_plugin_python_path(r"MetaHuman\MetaHumanCharacter\Content\Python")


def _project_saved_results_dir() -> str:
    directory = os.path.join(unreal.Paths.project_saved_dir(), "AvatarPipeline", "Results")
    os.makedirs(directory, exist_ok=True)
    return directory


def _project_saved_media_root_dir() -> str:
    directory = os.path.join(unreal.Paths.project_saved_dir(), DEFAULT_CAPTURE_MEDIA_SUBDIR)
    os.makedirs(directory, exist_ok=True)
    return directory


def _project_saved_media_run_dir(run_label: str) -> str:
    root = _project_saved_media_root_dir()
    timestamp = datetime.utcnow().strftime("%Y%m%d_%H%M%S_%f")
    return os.path.join(root, f"{_sanitize_asset_name(run_label)}_{timestamp}")


def _write_result_file(result: Dict[str, Any]) -> str:
    result_dir = _project_saved_results_dir()
    identity_name = _sanitize_asset_name(str(result.get("identityName", "WMCYNAvatar")))
    phase = _sanitize_asset_name(str(result.get("phase", "result")))
    path = os.path.join(result_dir, f"{identity_name}_{phase}.json")
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(result, handle, indent=2)
    _log(f"Wrote result file: {path}")
    return path


def _require_unreal_attr(name: str) -> Any:
    value = getattr(unreal, name, None)
    if value is None:
        raise RuntimeError(
            f"Unreal Python API is missing '{name}'. "
            "Confirm the required MetaHuman plugins are enabled for this project."
        )
    return value


def _env_or_default(name: str, default: Optional[str] = None) -> Optional[str]:
    value = os.environ.get(name)
    if value is None:
        return default
    trimmed = value.strip()
    return trimmed if trimmed else default


def _directory_path(path: str) -> unreal.DirectoryPath:
    directory_path = unreal.DirectoryPath()
    directory_path.set_editor_property("path", path)
    return directory_path


def _try_set_editor_property(target: Any, property_names: list[str], value: Any) -> bool:
    for property_name in property_names:
        try:
            target.set_editor_property(property_name, value)
            return True
        except Exception:
            continue
    return False


def _configure_capture_manager(storage_path: str, run_label: str) -> Optional[str]:
    settings_class = getattr(unreal, "CaptureManagerEditorSettings", None)
    if settings_class is None:
        _warn("Capture Manager editor settings are unavailable; falling back to deprecated MetaHuman ingest path.")
        return None

    settings = settings_class.get_capture_manager_editor_settings()
    if settings is None:
        _warn("Failed to retrieve Capture Manager editor settings; falling back to deprecated MetaHuman ingest path.")
        return None

    ffmpeg_path = (
        _env_or_default("WMCYN_AVATAR_FFMPEG_PATH")
        or shutil.which("ffmpeg")
        or r"C:\Users\jvred\OneDrive\Documents\FIFA Editor Tool v1.1.3\FIFA Editor Tool v1.1.3\FIFA Editor Tool Data\ffmpeg\ffmpeg.exe"
    )
    if ffmpeg_path and not os.path.exists(ffmpeg_path):
        ffmpeg_path = None

    capture_media_dir = _project_saved_media_run_dir(run_label)
    settings.set_import_directory(_directory_path(storage_path))
    settings.set_media_directory(_directory_path(capture_media_dir))

    if ffmpeg_path:
        _try_set_editor_property(settings, ["enable_third_party_encoder", "b_enable_third_party_encoder"], True)

        ffmpeg_file = unreal.FilePath()
        ffmpeg_file.set_editor_property("file_path", ffmpeg_path)
        if not _try_set_editor_property(settings, ["third_party_encoder"], ffmpeg_file):
            _warn("Could not assign Capture Manager ffmpeg path through editor settings.")
        else:
            _log(f"Configured Capture Manager third-party encoder: {ffmpeg_path}")
    else:
        _warn("ffmpeg was not found; Capture Manager will use engine-native readers only.")

    if not _try_set_editor_property(settings, ["auto_save_assets", "b_auto_save_assets"], True):
        _warn("Could not enable Capture Manager auto-save through editor settings.")

    return ffmpeg_path


def _ingest_capture_data_with_capture_manager(footage_path: str, storage_path: str, run_label: str) -> Optional[str]:
    ingest_library = getattr(unreal, "CaptureManagerIngestBlueprintLibrary", None)
    conversion_params_class = getattr(unreal, "CaptureManagerConversionParams", None)
    if ingest_library is None or conversion_params_class is None:
        return None

    _configure_capture_manager(storage_path, run_label)

    params = conversion_params_class()
    capture_data_asset, out_error = ingest_library.ingest_live_link_face_sync(footage_path, params)
    if not capture_data_asset:
        if hasattr(out_error, "to_string"):
            error_message = out_error.to_string()
        else:
            error_message = str(out_error) if out_error else "unknown Capture Manager ingest failure"
        raise RuntimeError(f"Capture Manager ingest failed: {error_message}")

    capture_data_path = capture_data_asset.get_path_name()
    _log(f"Capture Manager ingested Live Link Face take to: {capture_data_path}")
    return capture_data_path


def _ingest_capture_data_legacy(footage_path: str, storage_path: str) -> str:
    from create_capture_data import import_take_data_for_specified_device

    capture_data_assets = import_take_data_for_specified_device(
        footage_path=footage_path,
        using_LLF_data=True,
        storage_path=storage_path,
    )
    if not capture_data_assets:
        raise RuntimeError(f"No capture data assets were created from footage path: {footage_path}")

    capture_data_path = capture_data_assets[0]
    _log(f"Imported take to capture data asset: {capture_data_path}")
    return capture_data_path


def _parse_commandline_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="WMCYN MetaHuman avatar workstation pipeline")
    parser.add_argument("--phase", choices=["identity", "character", "runtime", "full"], default=_env_or_default("WMCYN_AVATAR_PHASE", "full"))
    parser.add_argument("--footage-path", default=_env_or_default("WMCYN_AVATAR_FOOTAGE_PATH"))
    parser.add_argument("--storage-path", default=_env_or_default("WMCYN_AVATAR_STORAGE_PATH", DEFAULT_STORAGE_PATH))
    parser.add_argument("--identity-name", default=_env_or_default("WMCYN_AVATAR_IDENTITY_NAME", "WMCYNAvatarIdentity"))
    parser.add_argument("--identity-path", default=_env_or_default("WMCYN_AVATAR_IDENTITY_PATH"))
    parser.add_argument("--character-name", default=_env_or_default("WMCYN_AVATAR_CHARACTER_NAME"))
    parser.add_argument("--character-package-path", default=_env_or_default("WMCYN_AVATAR_CHARACTER_PACKAGE", DEFAULT_CHARACTER_PACKAGE))
    parser.add_argument("--build-path", default=_env_or_default("WMCYN_AVATAR_BUILD_PATH", DEFAULT_BUILD_PATH))
    parser.add_argument("--common-path", default=_env_or_default("WMCYN_AVATAR_COMMON_PATH", DEFAULT_COMMON_PATH))
    parser.add_argument("--runtime-path", default=_env_or_default("WMCYN_AVATAR_RUNTIME_PATH", "/Game/WMCYN/AvatarPipeline/Runtime"))
    parser.add_argument("--runtime-quest-path", default=_env_or_default("WMCYN_AVATAR_RUNTIME_QUEST_PATH"))
    parser.add_argument("--runtime-pcvr-path", default=_env_or_default("WMCYN_AVATAR_RUNTIME_PCVR_PATH"))
    parser.add_argument("--animation-class-path", default=_env_or_default("WMCYN_AVATAR_ANIMATION_CLASS_PATH", ""))
    parser.add_argument("--neutral-frame", type=int, default=int(_env_or_default("WMCYN_AVATAR_NEUTRAL_FRAME", "0")))
    parser.add_argument("--body-index", type=int, default=int(_env_or_default("WMCYN_AVATAR_BODY_INDEX", str(DEFAULT_BODY_INDEX))))
    parser.add_argument("--pipeline-quality", choices=["low", "medium", "high"], default=_env_or_default("WMCYN_AVATAR_PIPELINE_QUALITY", DEFAULT_PIPELINE_QUALITY))
    parser.add_argument("--skip-textures", action="store_true", default=_env_or_default("WMCYN_AVATAR_SKIP_TEXTURES", "false").lower() == "true")
    parser.add_argument("--skip-build", action="store_true", default=_env_or_default("WMCYN_AVATAR_SKIP_BUILD", "false").lower() == "true")
    return parser.parse_args(sys.argv[1:])


def _require(value: Optional[str], label: str) -> str:
    if value is None or not str(value).strip():
        raise RuntimeError(f"Missing required value: {label}")
    return str(value).strip()


def _save_dirty_packages() -> None:
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)


def _resolve_identity_path(storage_path: str, identity_name: str, explicit_path: Optional[str]) -> str:
    if explicit_path:
        return explicit_path.strip()
    return f"{_normalize_content_path(storage_path)}/{_sanitize_asset_name(identity_name)}"


def _resolve_character_name(identity_name: str, explicit_name: Optional[str]) -> str:
    if explicit_name and explicit_name.strip():
        return _sanitize_asset_name(explicit_name)
    base_name = _sanitize_asset_name(identity_name)
    if base_name.endswith("Identity"):
        base_name = base_name[:-8] or "WMCYNAvatar"
    return base_name


def _quality_enum(value: str) -> Any:
    quality_level = _require_unreal_attr("MetaHumanQualityLevel")
    mapping = {
        "low": quality_level.LOW,
        "medium": quality_level.MEDIUM,
        "high": quality_level.HIGH,
    }
    return mapping[value]


def _create_or_load_character_asset(character_name: str, package_path: str) -> Any:
    character_asset_path = f"{package_path}/{character_name}"
    metahuman_character_class = _require_unreal_attr("MetaHumanCharacter")
    metahuman_character_factory = _require_unreal_attr("MetaHumanCharacterFactoryNew")
    if unreal.EditorAssetLibrary.does_asset_exist(character_asset_path):
        character = unreal.load_asset(character_asset_path)
        if not character:
            raise RuntimeError(f"Failed to load existing MetaHuman character: {character_asset_path}")
        return character

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    character = asset_tools.create_asset(
        asset_name=character_name,
        package_path=package_path,
        asset_class=metahuman_character_class,
        factory=unreal.new_object(type=metahuman_character_factory),
    )
    if not character:
        raise RuntimeError(f"Failed to create MetaHuman character: {character_asset_path}")
    return character


def _run_identity_phase(args: argparse.Namespace, result: Dict[str, Any]) -> Dict[str, Any]:
    footage_path = _require(args.footage_path, "footage-path")
    storage_path = _normalize_content_path(args.storage_path)
    identity_name = _sanitize_asset_name(args.identity_name)
    identity_path = _resolve_identity_path(storage_path, identity_name, args.identity_path)

    if args.neutral_frame < 0:
        raise RuntimeError("neutral-frame must be >= 0")

    from create_identity_for_performance import create_identity_from_frame

    capture_data_path = _ingest_capture_data_with_capture_manager(footage_path, storage_path, identity_name)
    if not capture_data_path:
        _warn("Capture Manager ingest unavailable; using deprecated MetaHuman capture-source import path.")
        capture_data_path = _ingest_capture_data_legacy(footage_path, storage_path)

    create_identity_from_frame(
        neutral_frame=args.neutral_frame,
        capture_source_asset_path=capture_data_path,
        asset_storage_location=storage_path,
        identity_asset_name=identity_name,
        prepare_for_performance=True,
        body_index=args.body_index,
    )
    _save_dirty_packages()

    identity_asset = unreal.load_asset(identity_path)
    if not identity_asset:
        raise RuntimeError(f"Failed to load created identity asset: {identity_path}")

    metahuman_identity_face = _require_unreal_attr("MetaHumanIdentityFace")
    face = identity_asset.get_or_create_part_of_class(metahuman_identity_face)
    has_dna = bool(face and face.has_dna_buffer())

    result.update(
        {
            "phase": "identity",
            "success": True,
            "footagePath": footage_path,
            "captureDataPath": capture_data_path,
            "identityName": identity_name,
            "identityPath": identity_path,
            "identityReadyForCharacterBuild": has_dna,
        }
    )

    if has_dna:
        _log(f"Identity is ready for character build: {identity_path}")
    else:
        _warn(
            "Identity was created, but DNA is not ready yet. "
            "Finish MetaHuman service login or wait for the service round-trip, "
            "then rerun the character phase."
        )

    return result


def _run_character_phase(args: argparse.Namespace, result: Dict[str, Any]) -> Dict[str, Any]:
    storage_path = _normalize_content_path(args.storage_path)
    identity_name = _sanitize_asset_name(args.identity_name)
    identity_path = _resolve_identity_path(storage_path, identity_name, args.identity_path)
    character_name = _resolve_character_name(identity_name, args.character_name)
    character_package_path = _normalize_content_path(args.character_package_path)
    build_path = _normalize_content_path(args.build_path)
    common_path = _normalize_content_path(args.common_path)

    identity_asset = unreal.load_asset(identity_path)
    if not identity_asset:
        raise RuntimeError(f"Identity asset does not exist: {identity_path}")

    metahuman_identity_face = _require_unreal_attr("MetaHumanIdentityFace")
    face = identity_asset.get_or_create_part_of_class(metahuman_identity_face)
    if not face or not face.has_dna_buffer():
        raise RuntimeError(
            "Identity exists but does not have DNA yet. "
            "The MetaHuman service step is still pending; rerun this phase after it completes."
        )

    character = _create_or_load_character_asset(character_name, character_package_path)
    metahuman_character_editor_subsystem = _require_unreal_attr("MetaHumanCharacterEditorSubsystem")
    metahuman_subsystem = unreal.get_editor_subsystem(metahuman_character_editor_subsystem)
    if not metahuman_subsystem.try_add_object_to_edit(character):
        raise RuntimeError("Unable to edit MetaHuman character asset. Is it already open for editing?")

    try:
        import_from_identity_params = _require_unreal_attr("ImportFromIdentityParams")
        import_error_code = _require_unreal_attr("ImportErrorCode")
        auto_rig_params = _require_unreal_attr("MetaHumanCharacterAutoRiggingRequestParams")
        rig_type = _require_unreal_attr("MetaHumanRigType")
        texture_request_params = _require_unreal_attr("MetaHumanCharacterTextureRequestParams")
        build_parameters = _require_unreal_attr("MetaHumanCharacterEditorBuildParameters")
        default_pipeline_type = _require_unreal_attr("MetaHumanDefaultPipelineType")

        import_params = import_from_identity_params()
        import_params.use_eye_meshes = True
        import_params.use_teeth_mesh = True
        import_params.use_metric_scale = False

        import_result = metahuman_subsystem.import_from_identity(character, identity_asset, import_params)
        if import_result != import_error_code.SUCCESS:
            raise RuntimeError(f"Failed to conform MetaHuman character from identity: {import_result}")

        auto_rig_request = auto_rig_params()
        auto_rig_request.blocking = True
        auto_rig_request.report_progress = False
        auto_rig_request.rig_type = rig_type.JOINTS_ONLY
        metahuman_subsystem.request_auto_rigging(character, auto_rig_request)

        if not args.skip_textures:
            texture_request = texture_request_params()
            texture_request.blocking = True
            texture_request.report_progress = False
            metahuman_subsystem.request_texture_sources(character, texture_request)

        if not args.skip_build:
            build_params = build_parameters()
            build_params.pipeline_type = default_pipeline_type.OPTIMIZED
            build_params.pipeline_quality = _quality_enum(args.pipeline_quality)
            build_params.absolute_build_path = build_path
            build_params.common_folder_path = common_path
            build_params.enable_wardrobe_item_validation = False
            metahuman_subsystem.build_meta_human(character=character, params=build_params)

        _save_dirty_packages()
    finally:
        if metahuman_subsystem.is_object_added_for_editing(character):
            metahuman_subsystem.remove_object_to_edit(character)

    result.update(
        {
            "phase": "character",
            "success": True,
            "identityName": identity_name,
            "identityPath": identity_path,
            "characterName": character_name,
            "characterPath": f"{character_package_path}/{character_name}",
            "buildPath": build_path,
            "commonPath": common_path,
            "pipelineQuality": args.pipeline_quality,
        }
    )
    _log(f"Built MetaHuman character asset: {result['characterPath']}")
    return result


def _asset_object_path(asset: Any) -> str:
    if not asset:
        return ""
    return asset.get_path_name()


def _find_asset_path(package_path: str, asset_name: str) -> str:
    return f"{package_path.rstrip('/')}/{asset_name}.{asset_name}"


def _safe_object_path(value: Any) -> str:
    if not value:
        return ""
    try:
        return value.get_path_name()
    except Exception:
        return str(value)


def _mesh_runtime_facts(mesh: Any) -> Dict[str, Any]:
    facts: Dict[str, Any] = {
        "path": _asset_object_path(mesh),
        "class": mesh.get_class().get_name() if mesh else "",
        "lodCount": 0,
        "skeletonPath": "",
        "postProcessAnimClassPath": "",
        "materials": [],
    }
    if not mesh:
        return facts
    try:
        facts["lodCount"] = int(mesh.get_num_lods())
    except Exception:
        pass
    try:
        facts["skeletonPath"] = _safe_object_path(mesh.get_editor_property("skeleton"))
    except Exception:
        pass
    try:
        facts["postProcessAnimClassPath"] = _safe_object_path(mesh.get_editor_property("post_process_anim_blueprint"))
    except Exception:
        pass
    try:
        facts["materials"] = [
            {
                "slotName": str(getattr(material, "material_slot_name", "")),
                "path": _safe_object_path(getattr(material, "material_interface", None)),
            }
            for material in mesh.get_editor_property("materials")
        ]
    except Exception:
        pass
    return facts


def _find_exported_material_path(runtime_character_root: str, material_asset_name: str) -> str:
    candidates = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(runtime_character_root, recursive=True, include_folder=False):
        if asset_path.endswith(f"/{material_asset_name}.{material_asset_name}"):
            candidates.append(asset_path)
    return sorted(candidates)[-1] if candidates else ""


def _apply_exported_materials(mesh: Any, runtime_character_root: str, is_head: bool) -> None:
    if not mesh:
        return
    materials = mesh.get_editor_property("materials")
    remapped_materials = []
    changed = False
    for material in materials:
        slot_name = str(getattr(material, "material_slot_name", ""))
        remapped_material = unreal.SkeletalMaterial()
        try:
            remapped_material.set_editor_property("material_slot_name", material.get_editor_property("material_slot_name"))
            remapped_material.set_editor_property("imported_material_slot_name", material.get_editor_property("imported_material_slot_name"))
        except Exception:
            pass
        material_asset_name = ""
        relative_path = ""
        if not is_head and slot_name == "body_shader_shader":
            relative_path = "Body/Materials/MI_Body_Skin"
        elif is_head:
            slot_to_asset = {
                "head_shader_shader": "Face/Materials/MI_Face_Skin_LOD0",
                "head_LOD1_shader_shader": "Face/Materials/MI_Face_Skin_LOD1",
                "head_LOD2_shader_shader": "Face/Materials/MI_Face_Skin_LOD2",
                "head_LOD3_shader_shader": "Face/Materials/MI_Face_Skin_LOD3",
                "head_LOD4_shader_shader": "Face/Materials/MI_Face_Skin_LOD4",
                "head_LOD57_shader_shader": "Face/Materials/MI_Face_Skin_LOD5to7",
                "teeth_shader_shader": "Face/Materials/MI_Face_Teeth",
                "eyeLeft_shader_shader": "Face/Materials/MI_Face_Eye_Left",
                "eyeRight_shader_shader": "Face/Materials/MI_Face_Eye_Right",
                "eyeshell_shader_shader": "Face/Materials/MI_Face_EyeShell",
                "eyelashes_shader_shader": "Face/Materials/MI_Face_Eyelashes",
                "eyelashes_HiLOD_shader_shader": "Face/Materials/MI_Face_EyelashesHiLODs",
            }
            relative_path = slot_to_asset.get(slot_name, "")
        if not relative_path:
            continue
        material_asset_name = relative_path.rsplit("/", 1)[-1]
        material_asset_path = _find_exported_material_path(runtime_character_root, material_asset_name)
        material_asset = unreal.load_asset(material_asset_path) if material_asset_path else None
        if material_asset:
            remapped_material.set_editor_property("material_interface", material_asset)
            changed = True
        else:
            try:
                remapped_material.set_editor_property("material_interface", material.get_editor_property("material_interface"))
            except Exception:
                pass
            _warn(f"Runtime material was not found for slot {slot_name}: {material_asset_name}")
        remapped_materials.append(remapped_material)
    if changed:
        set_materials = getattr(mesh, "set_materials", None)
        if set_materials:
            set_materials(remapped_materials)
        else:
            mesh.set_editor_property("materials", remapped_materials)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)


def _run_runtime_phase(args: argparse.Namespace, result: Dict[str, Any]) -> Dict[str, Any]:
    """Persist runtime-ready geometry/material assets from a successful character build."""
    identity_name = _sanitize_asset_name(args.identity_name)
    character_name = _resolve_character_name(identity_name, args.character_name)
    character_package_path = _normalize_content_path(args.character_package_path)
    character_path = f"{character_package_path}/{character_name}"
    runtime_root = _normalize_content_path(args.runtime_path)
    runtime_character_root = f"{runtime_root}/{character_name}"

    character = unreal.load_asset(character_path)
    if not character:
        raise RuntimeError(f"Built MetaHuman character does not exist: {character_path}")

    export_library = _require_unreal_attr("MetaHumanCharacterExportBlueprintLibrary")
    geometry_params_class = _require_unreal_attr("MetaHumanGeometryExportParams")
    materials_params_class = _require_unreal_attr("MetaHumanMaterialsExportParams")
    subsystem_class = _require_unreal_attr("MetaHumanCharacterEditorSubsystem")
    subsystem = unreal.get_editor_subsystem(subsystem_class)
    if not subsystem.is_object_added_for_editing(character):
        if not subsystem.try_add_object_to_edit(character):
            raise RuntimeError("Unable to open the built MetaHuman character for runtime export.")
        remove_after_export = True
    else:
        remove_after_export = False

    try:
        # Persist material instances and apply them to the generated character before geometry export.
        # ExportGeometry duplicates the character's current mesh materials, so this ordering keeps
        # the exported WMCYN meshes on the generated avatar appearance instead of gray topology materials.
        materials_params = materials_params_class()
        materials_params.project_path = runtime_character_root
        materials_params.apply_as_overrides = True
        export_library.export_materials(character, materials_params)

        geometry_params = geometry_params_class()
        geometry_params.project_path = runtime_character_root
        geometry_params.head_skeletal_mesh = True
        geometry_params.body_skeletal_mesh = True
        geometry_params.full_body_skeletal_mesh = False
        geometry_params.overwrite_existing_assets = True
        export_library.export_geometry(character, geometry_params)
        _save_dirty_packages()
    finally:
        if remove_after_export and subsystem.is_object_added_for_editing(character):
            subsystem.remove_object_to_edit(character)

    head_path = _find_asset_path(runtime_character_root, f"{character_name}_Head")
    body_path = _find_asset_path(runtime_character_root, f"{character_name}_Body")
    head_mesh = unreal.load_asset(head_path)
    body_mesh = unreal.load_asset(body_path)
    if not head_mesh or not body_mesh:
        raise RuntimeError(
            f"Runtime geometry export did not produce both meshes. Expected head={head_path}, body={body_path}."
        )

    _apply_exported_materials(body_mesh, runtime_character_root, is_head=False)
    _apply_exported_materials(head_mesh, runtime_character_root, is_head=True)
    _save_dirty_packages()

    # The first runtime consumer accepts one skeletal mesh. The body mesh is the stable source path;
    # retain the separately exported head path in the manifest for the later WMCYN avatar assembly pass.
    quest_root = _normalize_content_path(args.runtime_quest_path or f"{runtime_character_root}/Quest")
    pcvr_root = _normalize_content_path(args.runtime_pcvr_path or f"{runtime_character_root}/PCVR")
    animation_class_path = (args.animation_class_path or "").strip()
    head_facts = _mesh_runtime_facts(head_mesh)
    body_facts = _mesh_runtime_facts(body_mesh)
    runtime_result = {
        "phase": "runtime",
        "success": True,
        "identityName": identity_name,
        "characterName": character_name,
        "characterPath": character_path,
        "runtimeRoot": runtime_character_root,
        "quest": {
            "assetRoot": quest_root,
            "skeletalMeshPath": body_path,
            "headSkeletalMeshPath": head_path,
            "animClassPath": animation_class_path,
            "meshFacts": body_facts,
            "headMeshFacts": head_facts,
            "usesSameMeshAsPCVR": True,
            "status": "exported_shared_mesh_pending_quest_optimization" if not animation_class_path else "exported_shared_mesh_with_compatible_animation_wrapper",
        },
        "pcvr": {
            "assetRoot": pcvr_root,
            "skeletalMeshPath": body_path,
            "headSkeletalMeshPath": head_path,
            "animClassPath": animation_class_path,
            "meshFacts": body_facts,
            "headMeshFacts": head_facts,
            "usesSameMeshAsQuest": True,
            "status": "exported_shared_mesh_pending_quest_optimization" if not animation_class_path else "exported_shared_mesh_with_compatible_animation_wrapper",
        },
        "materialsRoot": runtime_character_root,
        "note": "Geometry and materials were exported from the built MetaHumanCharacter. Animation class publication is WMCYN-owned and may be supplied after skeleton compatibility validation.",
    }
    result.update(runtime_result)
    _log(f"Exported runtime body mesh: {body_path}")
    _log(f"Exported runtime head mesh: {head_path}")
    return result


def main() -> None:
    _load_epic_python_helpers()
    args = _parse_commandline_args()

    result: Dict[str, Any] = {
        "phase": args.phase,
        "success": False,
        "identityName": _sanitize_asset_name(args.identity_name),
    }

    try:
        if args.phase in ("identity", "full"):
            result = _run_identity_phase(args, result)
            if args.phase == "full" and not result.get("identityReadyForCharacterBuild"):
                result["nextStep"] = (
                    "Rerun with --phase character after the MetaHuman identity has DNA and is ready."
                )
                _write_result_file(result)
                return

        if args.phase in ("character", "full"):
            result = _run_character_phase(args, result)

        if args.phase == "runtime":
            result = _run_runtime_phase(args, result)

        result["success"] = True
        _write_result_file(result)
    except Exception as error:
        result["success"] = False
        result["error"] = str(error)
        _write_result_file(result)
        _error(str(error))
        raise
    finally:
        # Command-line editor runs must not remain as orphaned UnrealEditor processes.
        try:
            unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        except Exception:
            pass


if __name__ == "__main__":
    main()
