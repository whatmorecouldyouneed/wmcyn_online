"""Import the remaining approved Layout A furniture as WMCYN-owned assets.

Creates meshes, shared per-texture-set materials and textures only. It never
spawns an actor and never saves a level: placement stays manual.
"""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
SOURCE = Path('C:/Users/jvred/WMCYN v2.0/Assets/Hidden Heights SGB')
DEST = '/Game/WMCYN/Environment/Crib/Furniture/Batch01'
PILOT = '/Game/WMCYN/Environment/Crib/Furniture/LoungePilot/Materials/'
tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
# Only the full editor provides this subsystem; a headless commandlet returns
# None, in which case collision is reported as pending instead of silently skipped.
editor = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)


def collision_count(mesh):
    return None if editor is None else editor.get_simple_collision_count(mesh)

# key, source fbx, convex hull count. Zero hulls means no simple collision:
# rugs and flat decor must never block the pawn.
ITEMS = [('AccentChair01', 'accent_chair01.fbx', 6),
         ('DiningChair01', 'dining_chair01.fbx', 5),
         ('FloorLamp01', 'floor_lamp01.fbx', 6),
         ('KitchenTable', 'kitchen_table.fbx', 5),
         ('Rug01', 'rug01.fbx', 0),
         ('Sideboard01', 'sideboard01.fbx', 4),
         ('BarStool01', 'bar_stool01.fbx', 6),
         ('SideTable01', 'side_table01.fbx', 4),
         # Decor stays walk-through per the batch collision policy. house_plant04
         # measures 41 x 48 x 27 cm, so it is a tabletop plant, not the
         # floor-standing corner plant the schedule implies.
         ('HousePlant04', 'house_plant04.fbx', 0),
         ('Vase01', 'vase01.fbx', 0),
         ('CoffeeCup', 'coffee_cup.fbx', 0)]

# Source texture sets keyed by the normalised material-slot name Unreal creates.
# Folder, file prefix and per-map filenames all differ between sets; none of this
# is inferred from the misleading FBX material names.
SETS = {
    'large sectional sofa': dict(folder='large sectional sofa', prefix='large sectional sofa',
                                 asset='M_Crib_Sectional', existing=PILOT + 'M_Crib_Sectional'),
    'side tables': dict(folder='side tables', prefix='side tables',
                        asset='M_Crib_CoffeeTable', existing=PILOT + 'M_Crib_CoffeeTable'),
    'l furniture': dict(folder='l furniture', prefix='L furniture', asset='M_Crib_LFurniture'),
    'single seat chairs': dict(folder='single seat chairs', prefix='single_seat_chairs',
                               asset='M_Crib_SingleSeatChairs'),
    'rug': dict(folder='rug', prefix='rugs', asset='M_Crib_Rug'),
    'lights': dict(folder='lights', prefix='lights', asset='M_Crib_Lights'),
    'bean bag seating': dict(folder='bean bag seats', prefix='bean bag seating',
                             asset='M_Crib_BeanBagSeating'),
    'house atlas 2': dict(folder='house atlas 2', prefix='house atlas 2', asset='M_Crib_HouseAtlas2'),
    # Small decor sets stay at 1K per the batch texture budget.
    'plants': dict(folder='plants', prefix='plants', asset='M_Crib_Plants', max_size=1024),
    'props atlas 1': dict(folder='props atlas 1', prefix='props atlas 1',
                          asset='M_Crib_PropsAtlas1', max_size=1024),
    'banana plant': dict(folder='banana plant', prefix='banana plant',
                         asset='M_Crib_BananaPlant', max_size=1024),
    'stone wall': dict(folder='stone wall', prefix='TexturesCom_Brick_StoneCladding3_2K',
                       asset='M_Crib_StoneWall',
                       maps=dict(Base_color='albedo', Normal_OpenGL='normal', Roughness='roughness')),
}
# Emissive maps are deliberately ignored. Lamps stay decorative until the
# separate lighting check approves illumination.
CHANNELS = [('Base_color', 'BASE_COLOR', 'color'), ('Normal_OpenGL', 'NORMAL', 'normal'),
            ('Roughness', 'ROUGHNESS', 'linear'), ('Metallic', 'METALLIC', 'linear')]


def normalise(name):
    """Unreal slot name to source texture-set key, keeping trailing atlas digits."""
    text = str(name)
    if len(text) > 4 and text[-4] in '._' and text[-3:].isdigit():
        text = text[:-4]          # Blender duplicate suffix such as .002
    return ' '.join(text.lower().replace('_', ' ').split())


def import_task(filename, folder, name, options=None):
    path = folder + '/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path), False
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', str(filename))
    task.set_editor_property('destination_path', folder)
    task.set_editor_property('destination_name', name)
    task.set_editor_property('automated', True)
    task.set_editor_property('save', False)
    task.set_editor_property('replace_existing', False)
    if options:
        task.set_editor_property('options', options)
    tools.import_asset_tasks([task])
    asset = unreal.load_asset(path)
    assert asset, (path, list(task.imported_object_paths))
    return asset, True


def build_material(key, spec, notes):
    """One shared material per texture set, reusing the pilot material if present."""
    if spec.get('existing') and unreal.EditorAssetLibrary.does_asset_exist(spec['existing']):
        return unreal.load_asset(spec['existing']), 'reused pilot material'
    path = DEST + '/Materials/' + spec['asset']
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path), 'already imported'
    material = tools.create_asset(spec['asset'], DEST + '/Materials', unreal.Material,
                                  unreal.MaterialFactoryNew())
    for suffix, prop, kind in CHANNELS:
        stem = spec.get('maps', {}).get(suffix, suffix)
        found = [p for p in (SOURCE / 'textures' / spec['folder']).glob(spec['prefix'] + '_' + stem + '.*')
                 if p.suffix.lower() in ('.jpg', '.png', '.tif')]
        if not found:
            notes.append(key + ': no ' + suffix + ' map in source; channel left at material default')
            continue
        if len(found) > 1:
            notes.append(key + ': ambiguous ' + suffix + ' candidates; skipped')
            continue
        texture, created = import_task(found[0], DEST + '/Textures',
                                       'T_Crib_' + spec['asset'][7:] + '_' + suffix)
        if created:
            texture.set_editor_property('srgb', kind == 'color')
            texture.set_editor_property('max_texture_size', spec.get('max_size', 2048))
            if kind == 'normal':
                texture.set_editor_property('compression_settings',
                                            unreal.TextureCompressionSettings.TC_NORMALMAP)
                texture.set_editor_property('flip_green_channel', True)
            unreal.EditorAssetLibrary.save_loaded_asset(texture)
        sample = mel.create_material_expression(material, unreal.MaterialExpressionTextureSample)
        sample.set_editor_property('texture', texture)
        sample.set_editor_property('sampler_type',
                                   unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if kind == 'normal'
                                   else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if kind == 'color'
                                   else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        assert mel.connect_material_property(sample, 'R' if kind == 'linear' else 'RGB',
                                             getattr(unreal.MaterialProperty, 'MP_' + prop))
    mel.layout_material_expressions(material)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material, 'created'


report = []
for key, fbx, hulls in ITEMS:
    notes = []
    opts = unreal.FbxImportUI()
    opts.set_editor_property('import_mesh', True)
    opts.set_editor_property('import_as_skeletal', False)
    opts.set_editor_property('import_materials', False)
    opts.set_editor_property('import_textures', False)
    opts.set_editor_property('automated_import_should_detect_type', False)
    opts.set_editor_property('mesh_type_to_import', unreal.FBXImportType.FBXIT_STATIC_MESH)
    opts.static_mesh_import_data.set_editor_property('combine_meshes', True)
    opts.static_mesh_import_data.set_editor_property('generate_lightmap_u_vs', True)
    opts.static_mesh_import_data.set_editor_property('auto_generate_collision', False)
    mesh, created = import_task(SOURCE / 'fbx' / fbx, DEST + '/Meshes', 'SM_Crib_' + key, opts)
    assert isinstance(mesh, unreal.StaticMesh)

    slots = []
    for index, slot in enumerate(mesh.get_editor_property('static_materials')):
        name = str(slot.get_editor_property('material_slot_name'))
        spec = SETS.get(normalise(name))
        if spec is None:
            notes.append(key + ': slot "' + name + '" matches no known source texture set; left unassigned')
            slots.append(dict(slot=name, material=None))
            continue
        if created:
            material, how = build_material(key, spec, notes)
            mesh.set_material(index, material)
        else:
            # Already imported: report what is there, never rewrite it. Re-saving
            # an asset the running editor holds open also fails outright.
            material, how = mesh.get_material(index), 'already imported; left untouched'
        slots.append(dict(slot=name, material=material.get_path_name() if material else None,
                          source_set=normalise(name), material_status=how))

    box = mesh.get_bounding_box()
    size = [round(box.max.x - box.min.x, 2), round(box.max.y - box.min.y, 2),
            round(box.max.z - box.min.z, 2)]
    if created:
        mesh.set_editor_property('light_map_resolution',
                                 256 if max(size) > 400 else 128 if max(size) > 150 else 64)
        if hulls and editor is None:
            notes.append(key + ': convex collision still pending; this host has no '
                                'StaticMeshEditorSubsystem, so run from the editor console')
        elif hulls and collision_count(mesh) == 0:
            assert editor.set_convex_decomposition_collisions(mesh, hulls, 16, 100000)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    try:
        triangles = mesh.get_num_triangles(0)
    except Exception:
        triangles = None
    report.append(dict(item=key, mesh=mesh.get_path_name(), imported=created, size_cm=size,
                       bounds_min_z=box.min.z, triangles=triangles,
                       collision_shapes=collision_count(mesh),
                       blocks_pawn=bool(hulls), slots=slots, notes=notes))

out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Lighting/furnishing_batch_import.json')
out.write_text(json.dumps(report, indent=2))
print('WMCYN_FURNISHING_BATCH', json.dumps(report, indent=2))
