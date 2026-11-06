"""Repair the existing local pilot in the active editor. Do not load or replace the map."""
from pathlib import Path
import unreal as u

w = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert w.get_name() == 'L_WMCYNOnline' and not u.EditorLevelLibrary.get_pie_worlds(False)
sub = u.get_editor_subsystem(u.EditorActorSubsystem)
actors = {a.get_actor_label():a for a in sub.get_all_level_actors()}
deck = actors['BOWL_Crib_Deck']
dst = '/Game/WMCYN/BowlingPrivate'
floor = deck.get_actor_location().z + 6.9
material = u.load_asset(dst+'/M_Lane')
pin = u.load_asset(dst+'/SM_Pin')
assert material and pin
meshes = {}
for name in ['Return','Backstop','Canopy','OverheadTVs']:
    path = dst+'/SM_'+name
    if not u.EditorAssetLibrary.does_asset_exist(path):
        task = u.AssetImportTask()
        task.filename = str(Path(__file__).resolve().parents[2]/'Saved/BowlingSource'/(name+'.fbx'))
        task.destination_path,task.destination_name = dst,'SM_'+name
        task.automated,task.save = True,True
        opt = u.FbxImportUI()
        opt.import_materials = opt.import_textures = opt.import_as_skeletal = False
        opt.automated_import_should_detect_type = False
        opt.mesh_type_to_import = u.FBXImportType.FBXIT_STATIC_MESH
        opt.static_mesh_import_data.combine_meshes = True
        opt.static_mesh_import_data.auto_generate_collision = False
        task.options = opt
        u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    meshes[name] = u.load_asset(path)
    assert meshes[name]

def visual(label,mesh,pos,yaw,scale,preview=False):
    a = actors.get(label) or sub.spawn_actor_from_class(u.StaticMeshActor,pos)
    a.modify()
    a.set_actor_label(label)
    a.set_folder_path('WMCYN_Bowling/EditorPreview' if preview else 'WMCYN_Bowling')
    a.set_actor_location_and_rotation(pos,u.Rotator(0,0,yaw),False,True)
    a.set_actor_scale3d(scale)
    a.static_mesh_component.set_static_mesh(mesh)
    if not preview:
        for i in range(a.static_mesh_component.get_num_materials()):
            a.static_mesh_component.set_material(i,material)
    a.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    a.set_editor_property('is_editor_only_actor',preview)
    a.set_actor_hidden_in_game(preview)
    return a

with u.ScopedEditorTransaction('Restore Crib bowling fixtures and rack preview'):
    deck.modify()
    deck.set_actor_rotation(u.Rotator(0,0,-90),False)
    # Three architecture rays across the former couch bay hit collision at
    # X=-3178.1 and the rendered wall at -3186.1. Keep the approach at -416.
    far, near = -3177., -416.
    deck.set_actor_location(u.Vector((far+near)/2,645,-459),False,True)
    bounds = deck.static_mesh_component.static_mesh.get_bounds().box_extent
    deck.set_actor_scale3d(u.Vector(180/bounds.x,(near-far)/(2*bounds.y),1))
    rack = visual('BOWL_Return',meshes['Return'],u.Vector(-511,645,floor+40),-90,u.Vector(1,.52,.75))
    visual('BOWL_Backstop',meshes['Backstop'],u.Vector(far+40,645,floor+65),-90,u.Vector(1,.34,.82))
    visual('BOWL_Canopy',meshes['Canopy'],u.Vector(far+183,645,floor+142),-90,u.Vector(.98,.65,.6))
    visual('BOWL_OverheadTVs',meshes['OverheadTVs'],u.Vector(near-885,645,floor+224),-90,u.Vector(.98,1,.8))
    for lane,y in enumerate([554,736]):
        ball=actors['BOWL_Crib_Ball_'+str(lane+1)]
        ball.modify()
        # The two rack troughs flank its centre rails. The balls cannot both
        # balance on Y=645; seat each toward the lane it serves.
        x = -483-lane*6
        rack_y = 645 + (-10.7 if lane == 0 else 10.7)
        # Intersect a vertical slice with the actual rails; rest the sphere on
        # their highest contact, not on the overall bounds of the taller housing.
        hits = []
        mesh = meshes['Return']
        tr = rack.static_mesh_component.get_world_transform()
        radius = ball.get_component_by_class(u.StaticMeshComponent).static_mesh.get_bounds().box_extent.z
        for section in range(mesh.get_num_sections(0)):
            vs,idx,*_ = u.ProceduralMeshLibrary.get_section_from_static_mesh(mesh,0,section)
            pts = [tr.transform_location(v) for v in vs]
            for j in range(0,len(idx),3):
                a,b,c = (pts[k] for k in idx[j:j+3])
                for p,q in [(a,b),(b,c),(c,a)]:
                    if (p.x-x)*(q.x-x)>0 or abs(q.x-p.x)<.001:
                        continue
                    t=(x-p.x)/(q.x-p.x)
                    dy=p.y+t*(q.y-p.y)-rack_y
                    if abs(dy)<radius:
                        hits.append(p.z+t*(q.z-p.z)+(radius**2-dy**2)**.5)
        assert hits, 'Ball return rails not found'
        ball.set_actor_location(u.Vector(x,rack_y,max(hits)),False,True)
        u.log('BOWLSEAT '+ball.get_actor_label()+' '+str(ball.get_actor_location()))
        head=u.Vector(far+175,y,floor+pin.get_bounds().box_extent.z+1)
        ball.set_editor_property('head_pin',head)
        for row in range(4):
            for col in range(row+1):
                visual('BOWL_EditorPin_%d_%d_%d'%(lane,row,col),pin,
                       head+u.Vector(-row*26.3,-(col-row*.5)*30.4,0),180,u.Vector(1,1,1),True)
assert u.get_editor_subsystem(u.LevelEditorSubsystem).save_current_level()
u.log('BOWLFIT saved return, backstop, canopy, and 20 editor-only preview pins')
