"""Compare visible horizontal Crib geometry with blocking component traces.

Read-only: writes diagnostics under Saved; never changes assets or collision.
Run in the editor outside PIE. Samples large upward-facing triangles, not a
navigation guarantee: furniture, ceilings and intentional gaps need review.
"""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False), 'Stop PIE before auditing'
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
crib = next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
components = [c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.static_mesh]
blockers = [c for c in components
            if c.get_collision_enabled() != unreal.CollisionEnabled.NO_COLLISION
            and c.get_collision_response_to_channel(unreal.CollisionChannel.ECC_PAWN)
            == unreal.CollisionResponseType.ECR_BLOCK]
rows = []
for comp in components:
    if not comp.get_editor_property('visible') or comp.get_editor_property('hidden_in_game'):
        continue
    transform = comp.get_world_transform()
    for section in range(comp.static_mesh.get_num_sections(0)):
        verts, indices, _, _, _ = unreal.ProceduralMeshLibrary.get_section_from_static_mesh(comp.static_mesh, 0, section)
        for i in range(0, len(indices), 3):
            a, b, c = [transform.transform_location(verts[j]) for j in indices[i:i+3]]
            n = (b-a).cross(c-a)
            if n.length() < 2000 or n.z/n.length() < 0.98:
                continue
            p = (a+b+c)/3.0
            hits = []
            for blocker in blockers:
                # CharacterMovement uses simple queries; complex queries are a comparison.
                for complex_trace in (False, True):
                    hit = blocker.line_trace_component(p+unreal.Vector(0,0,30), p-unreal.Vector(0,0,200), complex_trace, False, False)
                    if hit and hit[1].z > 0.7:
                        hits.append(dict(component=blocker.get_name(), complex=complex_trace,
                                         z=hit[0].z, delta_cm=hit[0].z-p.z))
            simple = [h for h in hits if not h['complex']]
            closest = min(simple, key=lambda h: abs(h['delta_cm'])) if simple else None
            rows.append(dict(component=comp.get_name(), mesh=comp.static_mesh.get_path_name(),
                             point=list(p.to_tuple()), closest_simple=closest, hits=hits))
out = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/crib_walkable_surfaces.json'
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(rows, indent=2))
print('WMCYN_WALKABLE_AUDIT samples',len(rows),'missing',sum(r['closest_simple'] is None for r in rows),str(out))
