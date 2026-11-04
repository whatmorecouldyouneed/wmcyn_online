"""Read-only active-pawn collision settings and inherited baseline comparison."""
import json
from pathlib import Path
import unreal

rows = []
for path in ['/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal', '/Game/FullBodyVRTemplate/BluePrints/BP_VRBodyCharacer']:
    name = path.rsplit('/',1)[1]
    cls = unreal.load_class(None,path+'.'+name+'_C')
    obj = unreal.get_default_object(cls)
    components = []
    for c in obj.get_components_by_class(unreal.ActorComponent):
        props = {}
        for key in ['capsule_radius','capsule_half_height','relative_scale3d','max_step_height','walkable_floor_angle','perch_radius_threshold','perch_additional_height','use_flat_base_for_floor_checks','max_depenetration_with_geometry','max_depenetration_with_pawn']:
            try:
                props[key] = str(c.get_editor_property(key))
            except Exception:
                pass
        if isinstance(c,unreal.PrimitiveComponent):
            props.update(profile=str(c.get_collision_profile_name()),enabled=str(c.get_collision_enabled()),pawn_response=str(c.get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN)))
        if props:
            components.append(dict(name=c.get_name(),type=c.get_class().get_name(),properties=props))
    rows.append(dict(pawn=path,components=components))
root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
(root/'player_collision_polish_audit.json').write_text(json.dumps(rows,indent=2))
print('WMCYN_PLAYER_COLLISION_AUDIT',rows)
