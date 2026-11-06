"""Find existing Unity-transfer bowling surfaces before placing physics."""
import unreal as u

world = u.EditorLoadingAndSavingUtils.load_map('/Game/Levels/L_WMCYNOnline')
for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
    for c in a.get_components_by_class(u.StaticMeshComponent):
        mesh = c.static_mesh
        if not mesh:
            continue
        if not any(m and 'bowling_alley' in m.get_name().lower() for m in c.get_materials()):
            continue
        b = mesh.get_bounds()
        t = c.get_world_transform()
        u.log('BOWLSURFACE %s %s %s %s %s' % (c.get_name(), mesh.get_name(),t.transform_location(b.origin),b.box_extent,t))
