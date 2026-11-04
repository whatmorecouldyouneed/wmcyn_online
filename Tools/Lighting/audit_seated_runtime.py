"""Read-only PIE posture snapshot; run before/after native height calibration."""
import json
from pathlib import Path
from datetime import datetime, timezone
import unreal

worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
rows = []
for world in worlds:
    pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
    if not pawn:
        continue
    row = dict(world=world.get_name(), pawn_class=pawn.get_class().get_path_name(),
               location=str(pawn.get_actor_location()), properties={}, components=[])
    for key in ['PlayMode', 'Seated Mode Height Offset', 'Capsule Half Height', 'is_crouched']:
        try:
            row['properties'][key] = str(pawn.get_editor_property(key))
        except Exception:
            row['properties'][key] = 'unavailable'
    for comp in pawn.get_components_by_class(unreal.SceneComponent):
        if any(term in comp.get_name().lower() for term in ['camera', 'height', 'origin', 'fullbody', 'collision', 'controller']):
            row['components'].append(dict(name=comp.get_name(),
                world=str(comp.get_world_location()), relative=str(comp.get_relative_transform())))
    if isinstance(pawn, unreal.Character):
        row['capsule_half_height'] = pawn.capsule_component.get_scaled_capsule_half_height()
        row['movement_mode'] = str(pawn.character_movement.movement_mode)
        row['velocity'] = str(pawn.get_velocity())
    rows.append(row)
root = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting'
root.mkdir(parents=True, exist_ok=True)
stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
result = dict(timestamp_utc=stamp, pie_world_count=len(worlds), pawns=rows)
(root / ('seated_runtime_' + stamp + '.json')).write_text(json.dumps(result, indent=2))
print('WMCYN_SEATED_AUDIT', json.dumps(result))
