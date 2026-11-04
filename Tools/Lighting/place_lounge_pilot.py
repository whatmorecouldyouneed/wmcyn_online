"""Idempotent two-piece static lounge pilot. No player or source-framework edits."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == '/Game/Levels/L_WMCYNOnline.L_WMCYNOnline'
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
crib = next(a for a in sub.get_all_level_actors() if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
floor = next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name() == 'Plane_049')
assets = '/Game/WMCYN/Environment/Crib/Furniture/LoungePilot/Meshes/'
batch = 'WMCYNBatch_LoungePilot'
report = []
with unreal.ScopedEditorTransaction('WMCYN two-piece lounge pilot'):
    for label, asset, x, y, hulls in [
        ('FURN_Lounge_Sectional_01', 'SM_Crib_Sectional', -810, -600, 8),
        ('FURN_Lounge_CoffeeTable_01', 'SM_Crib_CoffeeTable', -720, -480, 4),
    ]:
        mesh = unreal.load_asset(assets+asset)
        assert mesh
        if editor.get_simple_collision_count(mesh) == 0:
            assert editor.set_convex_decomposition_collisions(mesh, hulls, 16, 100000)
            assert editor.get_simple_collision_count(mesh) > 0
            assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        # Rest each piece on the floor actually measured under it. The approval
        # board's "Z ~6" was a projected render band, not a collision surface.
        hit = floor.line_trace_component(unreal.Vector(x,y,200), unreal.Vector(x,y,-50), False, False, False)
        assert hit, (label, 'no floor support at placement point; do not guess an elevation')
        z = hit[0].z - mesh.get_bounding_box().min.z
        actor = next((a for a in sub.get_all_level_actors() if a.get_actor_label() == label), None)
        conflict = None
        if actor is None:
            actor = sub.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x,y,z))
            actor.set_actor_label(label)
            actor.set_folder_path('WMCYN_Environment/Furniture/LoungePilot')
            actor.set_editor_property('tags', [batch, 'WMCYNItem_'+label])
            comp = actor.static_mesh_component
            comp.set_static_mesh(mesh)
            comp.set_collision_profile_name('BlockAll')
            comp.set_mobility(unreal.ComponentMobility.STATIC)
        else:
            # Repeated apply is idempotent; a moved actor is reported, never overwritten.
            at = actor.get_actor_location()
            if max(abs(at.x-x), abs(at.y-y), abs(at.z-z)) > 1.0:
                conflict = 'manual transform change since last apply: %s' % list(at.to_tuple())
        bounds = actor.get_actor_bounds(False)
        report.append(dict(label=label, location=list(actor.get_actor_location().to_tuple()),
                           mesh=mesh.get_path_name(), collision_shapes=editor.get_simple_collision_count(mesh),
                           measured_floor_z=hit[0].z, floor_contact_delta=(bounds[0].z-bounds[1].z)-hit[0].z,
                           tags=[str(t) for t in actor.get_editor_property('tags')], conflict=conflict))
Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Lighting/lounge_pilot_placement.json').write_text(json.dumps(report,indent=2))
print('WMCYN_LOUNGE_PLACED', report)
