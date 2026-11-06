import unreal as u

actors = u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors()
sub = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
for a in actors:
    if 'VideoPanel' in a.get_class().get_name():
        u.log('DJSCAN panel ' + a.get_actor_label() + ' ' + str(a.get_actor_location()))
    for c in a.get_components_by_class(u.StaticMeshComponent):
        mesh = c.static_mesh
        if not mesh or 'aleph' not in mesh.get_name().lower():
            continue
        for i in range(mesh.get_num_sections(0)):
            if sub.get_lod_material_slot(mesh, 0, i) != c.get_material_index('DISP'):
                continue
            vs, idx, ns, uv, _ = u.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, i)
            points = [c.get_world_transform().transform_location(v) for v in vs]
            faces = [tuple(points[k] for k in idx[j:j+3]) for j in range(0,len(idx),3)]
            faces.sort(key=lambda t: (t[1]-t[0]).cross(t[2]-t[0]).length(), reverse=True)
            u.log('DJSCAN ' + a.get_actor_label() + ' ' + str([[p.to_tuple() for p in t] for t in faces[:2]]))
