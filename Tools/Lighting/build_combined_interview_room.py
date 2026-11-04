"""Reversible Door 03/04 room blockout; source Crib assets remain untouched."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name()=='/Game/Levels/L_WMCYNOnline.L_WMCYNOnline'
actors=sub.get_all_level_actors()
crib=next(a for a in actors if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
comps={c.get_name():c for c in crib.get_components_by_class(unreal.StaticMeshComponent)}
dest='/Game/WMCYN/Environment/Crib/InterviewRoom'
folder='WMCYN_Environment/InterviewRoom_Blockout'
root=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
unreal.EditorAssetLibrary.make_directory(dest)

def in_box(points,lo,hi):
    return all(lo[j]<=p[j]<=hi[j] for p in points for j in range(3))

def make_filtered(component,source_path,name,remove,collision):
    source=unreal.load_asset(source_path)
    assert source.get_num_sections(0)==1
    verts,indices,normals,uvs,_=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(source,0,0)
    tf=component.get_world_transform()
    keep=[]
    removed=[]
    for i in range(0,len(indices),3):
        ids=indices[i:i+3]
        points=[list(tf.transform_location(verts[j]).to_tuple()) for j in ids]
        if remove(points): removed.append(i//3)
        else: keep.append(unreal.IntVector(*ids))
    assert removed, name
    path=dest+'/'+name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mesh=unreal.load_asset(path)
    else:
        buffers=unreal.GeometryScriptSimpleMeshBuffers()
        buffers.vertices=verts
        buffers.normals=normals
        buffers.uv0=uvs
        buffers.triangles=keep
        dynamic=unreal.DynamicMesh()
        unreal.GeometryScript_MeshEdits.append_buffers_to_mesh(dynamic,buffers)
        options=unreal.GeometryScriptCreateNewStaticMeshAssetOptions()
        options.enable_nanite=False
        options.enable_collision=collision
        options.collision_mode=unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
        options.enable_recompute_normals=False
        options.enable_recompute_tangents=True
        mesh,outcome=unreal.GeometryScript_NewAssetUtils.create_new_static_mesh_asset_from_mesh(dynamic,path,options)
        assert mesh,str(outcome)
        mesh.set_material(0,component.get_material(0))
        if collision:
            mesh.get_editor_property('body_setup').set_editor_property('double_sided_geometry',True)
        editor=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
        build=editor.get_lod_build_settings(mesh,0)
        build.generate_lightmap_u_vs=True
        build.src_lightmap_index=0
        build.dst_lightmap_index=1
        editor.set_lod_build_settings(mesh,0,build)
        mesh.set_editor_property('light_map_coordinate_index',1)
        mesh.set_editor_property('light_map_resolution',512)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    return mesh,dict(component=component.get_name(),source=source_path,replacement=path,
                     removed_triangles=removed,remaining_triangles=len(keep))

def remove_partition(p):
    return (in_box(p,[-1470,615,440],[-920,633,885]) or
            in_box(p,[-1470,310,440],[-1450,633,885]) or
            in_box(p,[-1400,-8,440],[-1389,318,885]) or
            in_box(p,[-1470,313,440],[-1190,324,885]))

# Measure the second cap's exact X from the current shell, rather than rounding it.
shell=comps['collider']
source_shell='/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Collision_Doorways'
sv,si,_,_,_=unreal.ProceduralMeshLibrary.get_section_from_static_mesh(unreal.load_asset(source_shell),0,0)
tf=shell.get_world_transform()
caps=[]
for i in range(0,len(si),3):
    p=[list(tf.transform_location(sv[j]).to_tuple()) for j in si[i:i+3]]
    if max(v[0] for v in p)-min(v[0] for v in p)<.01 and (
            in_box(p,[-1032,310,450],[-1030,450,755]) or
            in_box(p,[-1135,720,450],[-1133,860,755])):
        caps.append(i//3)
assert len(caps)==4, caps
cap_centers=[sum(tf.transform_location(sv[si[i*3+j]]).x for j in range(3))/3 for i in caps]
def remove_caps(p):
    return max(v[0] for v in p)-min(v[0] for v in p)<.01 and any(
        in_box(p,[x-.02,310 if x>-1100 else 720,450],[x+.02,450 if x>-1100 else 860,755]) for x in cap_centers)

walls,wall_report=make_filtered(comps['Plane_020'],'/Game/Environments/WMCYN_Crib/Scene_074',
                               'SM_InterviewRoom_Architecture_02',remove_partition,False)
new_shell,shell_report=make_filtered(shell,source_shell,'SM_InterviewRoom_Collision_01',remove_caps,True)
assert len(shell_report['removed_triangles'])==4

rollback=root/'combined_room_rollback.json'
if not rollback.exists():
    rollback.write_text(json.dumps(dict(walls=comps['Plane_020'].static_mesh.get_path_name(),
                                       shell=shell.static_mesh.get_path_name()),indent=2))
comps['Plane_020'].set_static_mesh(walls)
shell.set_static_mesh(new_shell)

cube=unreal.load_asset('/Engine/BasicShapes/Cube')
plaster_path=dest+'/M_InterviewRoom_Plaster'
material=unreal.load_asset(plaster_path)
if not material:
    material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_InterviewRoom_Plaster',dest,unreal.Material,unreal.MaterialFactoryNew())
    color=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant3Vector)
    color.set_editor_property('constant',unreal.LinearColor(0.68,0.68,0.66,1))
    rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant)
    rough.set_editor_property('r',0.85)
    unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    assert unreal.EditorAssetLibrary.save_loaded_asset(material)
def wall(name,center,size):
    actor=next((a for a in sub.get_all_level_actors() if a.get_actor_label()==name),None)
    if not actor: actor=sub.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*center))
    actor.set_actor_label(name)
    actor.set_folder_path(folder)
    actor.set_actor_location(unreal.Vector(*center),False,False)
    actor.set_actor_scale3d(unreal.Vector(*(v/100 for v in size)))
    comp=actor.static_mesh_component
    comp.set_static_mesh(cube)
    comp.set_material(0,material)
    comp.set_mobility(unreal.ComponentMobility.STATIC)
    comp.set_collision_profile_name('BlockAll')
    return actor

# Finish the formerly inaccessible back/side boundary. Retain both hallway doors.
wall('ARCH_InterviewRoom_HallwayReturn',[-998,623.5,667],[152,7,426])
wall('ARCH_InterviewRoom_RearWall',[-1850,474.5,678],[12,933,434])
# The original south facade/window at Y=8 encloses this alcove already.
# Retain the obsolete return for rollback, but never block the window alcove.
south=wall('ARCH_InterviewRoom_SouthReturn',[-1652,318,678],[408,12,434])
south.set_actor_hidden_in_game(True)
south.set_is_temporarily_hidden_in_editor(True)
south.set_actor_enable_collision(False)
south.static_mesh_component.set_visibility(False)
wall('ARCH_InterviewRoom_WindowAlcoveCeiling',[-1627,166,835],[458,316,10])
wall('ARCH_InterviewRoom_WindowWestLiner',[-1755,166,647],[12,316,372])
wall('ARCH_InterviewRoom_WindowWestReturn',[-1802,318,647],[108,12,372])
wall('ARCH_InterviewRoom_WindowEastLiner',[-1198,184,647],[12,280,372])
wall('ARCH_InterviewRoom_WindowEastReturn',[-1299,44,647],[214,12,372])
wall('ARCH_InterviewRoom_WindowEastCeiling',[-1299,184,835],[214,280,10])
infill=wall('ARCH_InterviewRoom_WindowEastFloor',[-1299,184,456.7],[214,280,10])
infill.static_mesh_component.set_material(0,comps['Plane_004'].get_material(0))
wall('ARCH_InterviewRoom_NorthReturn',[-1488,939,678],[736,12,434])
wall('ARCH_InterviewRoom_Ceiling',[-1467.5,628,835],[775,626,10])
wall('ARCH_InterviewRoom_EntryCeiling',[-1025,470,835],[110,310,10])
wall('ARCH_InterviewRoom_Door04Ceiling',[-1102,782,835],[76,316,10])
for index,position in enumerate([(-1550,470,820),(-1450,790,820)],1):
    name='DEV_InterviewRoom_WorkLight_%02d'%index
    light=next((a for a in sub.get_all_level_actors() if a.get_actor_label()==name),None)
    if not light: light=sub.spawn_actor_from_class(unreal.PointLight,unreal.Vector(*position))
    light.set_actor_label(name)
    light.set_folder_path(folder+'/TemporaryLighting')
    component=light.point_light_component
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property('intensity_units',unreal.LightUnits.LUMENS)
    component.set_editor_property('intensity',400.0)
    component.set_editor_property('attenuation_radius',700.0)
    component.set_editor_property('source_radius',15.0)
report=dict(walls=wall_report,shell=shell_report,folder=folder,
            new_actors=['ARCH_InterviewRoom_HallwayReturn','ARCH_InterviewRoom_RearWall','ARCH_InterviewRoom_SouthReturn','ARCH_InterviewRoom_NorthReturn',
                        'ARCH_InterviewRoom_Ceiling','ARCH_InterviewRoom_EntryCeiling','ARCH_InterviewRoom_WindowAlcoveCeiling',
                        'ARCH_InterviewRoom_WindowWestLiner','ARCH_InterviewRoom_WindowWestReturn','ARCH_InterviewRoom_Door04Ceiling',
                        'DEV_InterviewRoom_WorkLight_01','DEV_InterviewRoom_WorkLight_02'],
            note='Blockout only. Lighting bake and headset traversal pending.')
(root/'combined_room_build.json').write_text(json.dumps(report,indent=2))
unreal.EditorLevelLibrary.editor_invalidate_viewports()
print('WMCYN_COMBINED_ROOM_BUILT',report)
