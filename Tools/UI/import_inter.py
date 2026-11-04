"""Import licensed static Inter faces into WMCYN-owned content."""
import unreal

tasks = []
for weight in ('Regular', 'Medium', 'SemiBold'):
    if unreal.EditorAssetLibrary.does_asset_exist('/Game/WMCYN/UI/Fonts/Inter_' + weight):
        continue
    task = unreal.AssetImportTask()
    task.filename = unreal.Paths.convert_relative_path_to_full(
        unreal.Paths.project_dir() + 'SourceArt/UI/Fonts/Inter/Inter-' + weight + '.ttf')
    task.destination_path = '/Game/WMCYN/UI/Fonts'
    task.destination_name = 'Inter_' + weight
    task.automated = True
    task.save = True
    task.factory = unreal.FontFileImportFactory()
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    print('INTER_IMPORTED', task.imported_object_paths)

# UMG expects a composite UFont, not a direct FontFace reference.
path = '/Game/WMCYN/UI/Fonts/F_Inter'
font = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.EditorAssetLibrary.duplicate_asset('/Engine/EngineFonts/Roboto', path)
entries = []
for weight in ('Regular', 'Medium', 'SemiBold'):
    entries.append('(Name="' + weight + '",Font=(FontFaceAsset="/Script/Engine.FontFace\'/Game/WMCYN/UI/Fonts/Inter_' + weight + '.Inter_' + weight + '\'"))')
composite = unreal.CompositeFont()
composite.import_text('(DefaultTypeface=(Fonts=(' + ','.join(entries) + ')))')
font.set_editor_property('composite_font', composite)
unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
