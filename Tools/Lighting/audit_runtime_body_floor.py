"""Read-only local VR bone and imported-floor collision evidence."""
import json
from datetime import datetime, timezone
from pathlib import Path
import unreal

worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
assert worlds, 'Start VR Preview first'
world = worlds[0]
pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
assert pawn
pos = pawn.get_actor_location()
result = dict(pawn_class=pawn.get_class().get_path_name(), position=list(pos.to_tuple()),
              capsule_bottom=pos.z-pawn.capsule_component.get_scaled_capsule_half_height(),
              bones={}, surface_hits=[])
mesh = pawn.mesh
for bone in ['root', 'pelvis', 'spine_01', 'spine_05', 'neck_01', 'neck_02', 'head',
             'thigh_l', 'calf_l', 'foot_l', 'ball_l', 'thigh_r', 'calf_r', 'foot_r', 'ball_r']:
    if mesh.does_socket_exist(bone):
        result['bones'][bone] = list(mesh.get_socket_location(bone).to_tuple())
for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
    if actor == pawn:
        continue
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        if not comp.static_mesh:
            continue
        for complex_trace in [False, True]:
            hit = comp.line_trace_component(pos + unreal.Vector(0, 0, 200),
                                            pos - unreal.Vector(0, 0, 600),
                                            complex_trace, False, False)
            if hit:
                result['surface_hits'].append(dict(actor=actor.get_name(), component=comp.get_name(),
                    mesh=comp.static_mesh.get_path_name(), visible=comp.is_visible(),
                    complex=complex_trace, point=list(hit[0].to_tuple())))
root = Path(unreal.Paths.project_saved_dir()) / 'Diagnostics/Lighting'
stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
(root / ('runtime_body_floor_' + stamp + '.json')).write_text(json.dumps(result, indent=2))
print('WMCYN_BODY_FLOOR', json.dumps(result))
