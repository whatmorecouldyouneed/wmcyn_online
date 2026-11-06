"""Install a private, repeatable two-lane pilot over the measured Crib bowling strip."""
from pathlib import Path
import unreal as u

root = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))
dst = '/Game/WMCYN/BowlingPrivate'
tools = u.AssetToolsHelpers.get_asset_tools()
world = u.EditorLoadingAndSavingUtils.load_map('/Game/Levels/L_WMCYNOnline')
assert world and not u.EditorLevelLibrary.get_pie_worlds(False)
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
sub = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)

def import_asset(file, name, options=None):
    path = dst+'/'+name
    if not u.EditorAssetLibrary.does_asset_exist(path):
        t = u.AssetImportTask()
        t.filename, t.destination_path, t.destination_name = str(file), dst, name
        t.automated, t.save, t.options = True, True, options
        tools.import_asset_tasks([t])
    a = u.load_asset(path)
    assert a, path
    return a

def surface(name, texture):
    path = dst+'/M_'+name
    if u.EditorAssetLibrary.does_asset_exist(path):
        return u.load_asset(path)
    tex = import_asset(next((Path.home()/'WMCYN v2.0/Assets/Bowling Alley Prefab').rglob(texture)), 'T_'+name)
    tex.set_editor_property('max_texture_size',1024)
    u.EditorAssetLibrary.save_loaded_asset(tex)
    mat = tools.create_asset('M_'+name,dst,u.Material,u.MaterialFactoryNew())
    mel = u.MaterialEditingLibrary
    node = mel.create_material_expression(mat,u.MaterialExpressionTextureSample)
    node.texture = tex
    mel.connect_material_property(node,'RGB',u.MaterialProperty.MP_BASE_COLOR)
    mel.recompile_material(mat)
    u.EditorAssetLibrary.save_loaded_asset(mat)
    return mat

assets = {}
for key in ['Lane','Pin','Ball']:
    opts = u.FbxImportUI()
    opts.import_materials = opts.import_textures = opts.import_as_skeletal = False
    opts.automated_import_should_detect_type = False
    opts.mesh_type_to_import = u.FBXImportType.FBXIT_STATIC_MESH
    opts.static_mesh_import_data.combine_meshes = True
    mesh = import_asset(root/'Saved/BowlingSource'/(key+'.fbx'),'SM_'+key,opts)
    sub.remove_collisions(mesh)
    if key == 'Lane':
        mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    elif key == 'Ball':
        sub.add_simple_collisions(mesh,u.ScriptingCollisionShapeType.SPHERE)
    else:
        assert sub.set_convex_decomposition_collisions(mesh,1,24,100000)
    texture = 'bowling balls_Base_color.png' if key == 'Ball' else 'bowling alley_Base_color.png'
    mesh.set_material(0,surface(key,texture))
    u.EditorAssetLibrary.save_loaded_asset(mesh)
    assets[key] = mesh

def place(label, cls, loc, yaw):
    a = existing.get(label) or actors.spawn_actor_from_class(cls,loc)
    assert a
    a.set_actor_label(label)
    a.set_folder_path('WMCYN_Bowling')
    a.set_actor_location_and_rotation(loc,u.Rotator(0,0,yaw),False,True)
    return a

ext = assets['Lane'].get_bounds().box_extent
along_x = ext.x > ext.y
deck = place('BOWL_Crib_Deck',u.StaticMeshActor,u.Vector(-911,645,-459),0 if along_x else 90)
deck.static_mesh_component.set_static_mesh(assets['Lane'])
deck.set_actor_scale3d(u.Vector(495/ext.x if along_x else 180/ext.x,
                                180/ext.y if along_x else 495/ext.y,1))
deck.static_mesh_component.set_collision_profile_name('BlockAll')
for i,y in enumerate([554,736]):
    ball = place('BOWL_Crib_Ball_'+str(i+1),u.WMCYNBowlingBall,u.Vector(-447,y,-379),180)
    ball.get_component_by_class(u.StaticMeshComponent).set_static_mesh(assets['Ball'])
    ball.set_editor_property('pin_mesh',assets['Pin'])
    ball.set_editor_property('head_pin',u.Vector(-1300,y,-459+ext.z+assets['Pin'].get_bounds().box_extent.z+1))
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/Levels/L_WMCYNOnline')
u.log('WMCYN_BOWLING installed two racks and pickups; deck half-extents '+str(ext))
