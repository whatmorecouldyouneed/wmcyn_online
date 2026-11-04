"""Read-only per-component floor traces around the approved entry marker."""
import json
from pathlib import Path
import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
marker = next(a for a in actors if a.get_actor_label() == 'SPAWN_FirstSignal_StandaloneVR_A')
p = marker.get_actor_location()
components = [c for a in actors for c in a.get_components_by_class(unreal.StaticMeshComponent)
              if c.get_collision_enabled() != unreal.CollisionEnabled.NO_COLLISION
              and c.get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN) == unreal.CollisionResponseType.ECR_BLOCK]
rows = []
for dx in range(-200, 201, 50):
    for dy in range(-200, 201, 50):
        start = unreal.Vector(p.x + dx, p.y + dy, p.z + 40)
        end = start - unreal.Vector(0, 0, 300)
        hits = []
        for c in components:
            hit = c.line_trace_component(start, end, True, False, False)
            if hit:
                hits.append(dict(component=c.get_name(), actor=c.get_owner().get_actor_label(),
                                 z=hit[0].z, normal_z=hit[1].z,
                                 hidden=bool(c.get_editor_property('hidden_in_game')),
                                 visible=bool(c.get_editor_property('visible'))))
        rows.append(dict(dx=dx, dy=dy, hits=sorted(hits, key=lambda h: -h['z'])))
result = dict(marker=[p.x,p.y,p.z], samples=rows)
out = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/entry_floor_audit.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(result, indent=2))
print('WMCYN_ENTRY_FLOOR_AUDIT', str(out), 'samples', len(rows))
