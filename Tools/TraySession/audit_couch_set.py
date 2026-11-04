"""Report actors around the couch set so the tray has a measured surface."""
import json
from pathlib import Path

import unreal

world = unreal.EditorLoadingAndSavingUtils.load_map('/Game/Levels/L_WMCYNOnline')
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
words = ('couch', 'sofa', 'rug', 'banner', 'table', 'desk', 'workstation', 'ottoman', 'interview', 'flower', 'seat')
rows = []
for actor in actors:
    label = actor.get_actor_label()
    if any(word in label.lower() for word in words):
        origin, extent = actor.get_actor_bounds(False)
        rows.append(dict(label=label, cls=actor.get_class().get_name(), folder=str(actor.get_folder_path()),
                         origin=[round(origin.x), round(origin.y), round(origin.z)],
                         extent=[round(extent.x), round(extent.y), round(extent.z)]))
out = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())) / 'Saved/Diagnostics/TraySession'
out.mkdir(parents=True, exist_ok=True)
(out / 'couch_set.json').write_text(json.dumps(rows, indent=1))
print('WMCYN_COUCH_SET', len(rows))
