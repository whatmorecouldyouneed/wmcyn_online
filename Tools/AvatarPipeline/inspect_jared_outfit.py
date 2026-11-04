"""Read original Jared component assembly without editing assets."""
import json
from pathlib import Path
import unreal

bp = unreal.load_asset('/Game/MetaHumans/Jared/BP_Jared')
subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
library = unreal.SubobjectDataBlueprintFunctionLibrary
rows = []
for handle in handles:
    obj = library.get_object(library.get_data(handle))
    if not obj:
        continue
    row = {'name': obj.get_name(), 'class': obj.get_class().get_name()}
    for prop in ('relative_location', 'relative_rotation', 'relative_scale3d',
                 'skeletal_mesh_asset', 'groom_asset', 'binding_asset', 'override_materials'):
        try:
            row[prop] = str(obj.get_editor_property(prop))
        except Exception:
            pass
    rows.append(row)
output = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Avatars/jared_outfit_assembly.json'
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(rows, indent=2), encoding='utf-8')
unreal.log('WMCYN_JARED_OUTFIT_INSPECTED')
