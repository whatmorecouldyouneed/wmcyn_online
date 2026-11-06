"""Fit stage controls to both Aleph laptop LCDs in the current Crib editor world."""
import unreal as u

sub = u.get_editor_subsystem(u.EditorActorSubsystem)
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert world.get_name() == "L_WMCYNOnline"
actors = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
stage = actors["VIDEO_Panel_Stage"]
channel = stage.get_editor_property("channel")
assert str(channel.get_editor_property("channel_id")) == "Stage"
mesh_sub = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
placements = []
for label in ("DJ_Pilot_AlephStandard", "SM_AlephStandard"):
    booth = actors[label]
    comp = booth.get_component_by_class(u.StaticMeshComponent)
    mesh = comp.static_mesh
    section = next(i for i in range(mesh.get_num_sections(0))
                   if mesh_sub.get_lod_material_slot(mesh, 0, i) == comp.get_material_index("DISP"))
    vs, indices, *_ = u.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, section)
    points = [comp.get_world_transform().transform_location(v) for v in vs]
    faces = [tuple(points[k] for k in indices[j:j+3]) for j in range(0,len(indices),3)]
    faces.sort(key=lambda f: (f[1]-f[0]).cross(f[2]-f[0]).length(), reverse=True)
    corners = {tuple(round(q,3) for q in p.to_tuple()) for f in faces[:2] for p in f}
    assert len(corners) == 4
    bl, br, tl, tr = sorted([u.Vector(*p) for p in corners], key=lambda p:(round(p.z,2),p.x))
    right, up = tr-tl, tl-bl
    normal = right.cross(up)
    normal = normal / normal.length()
    if normal.z < 0: normal *= -1
    center = (tl+tr+bl+br)*.25
    placements.append((center+up*.075+normal*.35, u.MathLibrary.make_rot_from_x(normal), right.length(), up.length()))

with u.ScopedEditorTransaction("Place stage link controls on laptop displays"):
    other = actors.get("VIDEO_Panel_Stage_Balcony") or sub.duplicate_actor(stage, world)
    assert other
    other.set_actor_label("VIDEO_Panel_Stage_Balcony")
    for panel, (pos, rot, width, height) in zip((stage, other), placements):
        panel.modify()
        panel.set_editor_property("channel", channel)
        panel.set_editor_property("tags", list(set(panel.tags) | {u.Name("DJLaptop")}))
        panel.set_actor_location_and_rotation(pos, rot, False, True)
        base = panel.get_component_by_class(u.WidgetComponent).get_relative_transform().scale3d
        panel.set_actor_scale3d(u.Vector(1,width/(1400*base.y),height/(1060*base.z)))
        u.log("DJFIT " + panel.get_actor_label() + " " + str(pos))
assert u.get_editor_subsystem(u.LevelEditorSubsystem).save_current_level()
