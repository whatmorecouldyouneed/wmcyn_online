"""Bake Joint Roller meshes to item space in metres and prepare capped textures.

Run: blender -b --factory-startup -P Tools/TraySession/prepare_tray_source.py
Item origins and scales come from the Unity prefabs, so Unreal needs no extra scale
except the joint, which keeps prefab scale (3,3,2) on its component.
"""
import json
from pathlib import Path

import bpy
import numpy as np
from mathutils import Matrix

root = Path(__file__).resolve().parents[2]
out = root / 'SourceArt/TraySession'
source = 'C:/Users/jvred/WMCYN v2.0/Assets/Joints By Matic 954/Joint Roller/'
data = source + 'DataStore/'
logo_path = 'C:/Users/jvred/Documents/WMCYN/wmcyn-online-front-end/out/wmcyn_logo_white.png'
TEXTURE_CAP = 1024


def load(path):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if path.lower().endswith('.fbx'):
        bpy.ops.import_scene.fbx(filepath=path, use_image_search=False)
    elif path.endswith('.dae'):
        bpy.ops.wm.collada_import(filepath=path)
    else:
        bpy.ops.wm.obj_import(filepath=path)


def bake(name, keep=None, transform=Matrix.Identity(4), recenter=None):
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH' and (keep is None or o.name in keep)]
    assert meshes, name
    matrices = {o.name: o.matrix_world.copy() for o in meshes}
    for o in list(bpy.context.scene.objects):
        if o not in meshes:
            bpy.data.objects.remove(o)
    bpy.ops.object.select_all(action='DESELECT')
    for o in meshes:
        o.parent = None
        o.matrix_world = transform @ matrices[o.name]
        o.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    verts = [o.matrix_world @ v.co for o in meshes for v in o.data.vertices]
    low = [min(v[i] for v in verts) for i in range(3)]
    high = [max(v[i] for v in verts) for i in range(3)]
    if recenter:
        offset = [-(low[i] + high[i]) / 2 if recenter[i] == 'c' else -low[i] if recenter[i] == 'b' else 0 for i in range(3)]
        for o in meshes:
            o.location = offset
        bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)
        low = [low[i] + offset[i] for i in range(3)]
        high = [high[i] + offset[i] for i in range(3)]
    bpy.ops.export_scene.fbx(filepath=str(out / (name + '.fbx')), use_selection=True, object_types={'MESH'},
                             axis_forward='-Y', axis_up='Z', bake_anim=False, add_leaf_bones=False,
                             mesh_smooth_type='FACE')
    return dict(mesh=name, triangles=sum(len(p.vertices) - 2 for o in meshes for p in o.data.polygons),
                size_cm=[round((high[i] - low[i]) * 100, 2) for i in range(3)],
                materials=sorted({m.name for o in meshes for m in o.data.materials if m}))


def pixels(path, size):
    image = bpy.data.images.load(path)
    image.scale(size, size)
    array = np.array(image.pixels[:], dtype=np.float32).reshape(size, size, 4)
    bpy.data.images.remove(image)
    return array


def save(array, name):
    height, width = array.shape[:2]
    image = bpy.data.images.new(name, width, height, alpha=True)
    image.pixels.foreach_set(array.ravel())
    image.filepath_raw = str(out / (name + '.png'))
    image.file_format = 'PNG'
    image.save()
    bpy.data.images.remove(image)


def convert(path, name, size=TEXTURE_CAP):
    save(pixels(path, size), name)


def panel(array, left, top, right, bottom, colour=(0.012, 0.012, 0.012)):
    """Fill a rectangle given as fractions from the image's top-left corner."""
    height, width = array.shape[:2]
    array[int((1 - bottom) * height):int((1 - top) * height), int(left * width):int(right * width), :3] = colour


def stamp(array, logo, left, top, right, bottom):
    """Composite the rotated logo, fitted inside a rectangle of the pack atlas."""
    height, width = array.shape[:2]
    x0, x1 = int(left * width), int(right * width)
    y0, y1 = int((1 - bottom) * height), int((1 - top) * height)
    box_w, box_h = x1 - x0, y1 - y0
    rotated = np.rot90(logo, k=-1)
    scale = min(box_w / rotated.shape[1], box_h / rotated.shape[0])
    w, h = max(int(rotated.shape[1] * scale), 1), max(int(rotated.shape[0] * scale), 1)
    rows = (np.linspace(0, rotated.shape[0] - 1, h)).astype(int)
    cols = (np.linspace(0, rotated.shape[1] - 1, w)).astype(int)
    fitted = rotated[rows][:, cols]
    ox, oy = x0 + (box_w - w) // 2, y0 + (box_h - h) // 2
    region = array[oy:oy + h, ox:ox + w, :3]
    alpha = fitted[..., 3:4]
    array[oy:oy + h, ox:ox + w, :3] = fitted[..., :3] * alpha + region * (1 - alpha)


def brand_pack():
    """Replace OCB marks with WMCYN placeholder art. The creator credit panel stays."""
    size = TEXTURE_CAP
    pack = pixels(data + 'OCB_Slim_model/Paquet_Diffuse 1.png', size)
    logo_image = bpy.data.images.load(logo_path)
    logo_w, logo_h = logo_image.size
    logo = np.array(logo_image.pixels[:], dtype=np.float32).reshape(logo_h, logo_w, 4)
    for left, right in ((8 / 480, 60 / 480), (134 / 480, 186 / 480)):
        panel(pack, left, 0, right, 208 / 480)
        stamp(pack, logo, left + .012, .04, right - .012, 200 / 480)
    panel(pack, 342 / 480, 0, 402 / 480, 232 / 480)
    # Manufacturer address lines.
    panel(pack, 985 / 1024, 360 / 1024, 1006 / 1024, 505 / 1024)
    panel(pack, 600 / 1024, 790 / 1024, 620 / 1024, 1, colour=(.95, .95, .95))
    save(pack, 'T_Tray_Pack_WMCYN')


def rainbow_lines():
    """Procedural stand-in for the stock rainbow image, tiled by the high overlay."""
    size = 512
    v, u = np.mgrid[0:size, 0:size] / size
    wave = v + .08 * np.sin(u * 6.283 * 2)
    bands = .5 + .5 * np.cos(wave * 6.283 * 6)
    hue = (wave * 3) % 1
    rgb = np.stack([.5 + .5 * np.cos(6.283 * (hue + phase)) for phase in (0, .33, .67)], axis=-1)
    image = np.dstack([rgb * bands[..., None], np.ones((size, size))]).astype(np.float32)
    save(image, 'T_Tray_HighLines')


def burn_noise():
    """Tiled fractal noise replacing the source shader's per-pixel fBm on Quest."""
    size = 256
    rng = np.random.default_rng(954)
    total = np.zeros((size, size))
    amplitude = .5
    for cells in (8, 16, 32, 64):
        grid = rng.random((cells, cells))
        scaled = np.kron(grid, np.ones((size // cells, size // cells)))
        for _ in range(3):
            scaled = (scaled + np.roll(scaled, 1, 0) + np.roll(scaled, 1, 1) + np.roll(scaled, (1, 1), (0, 1))) / 4
        total += amplitude * scaled
        amplitude /= 2
    total = (total - total.min()) / (total.max() - total.min())
    save(np.dstack([total, total, total, np.ones_like(total)]).astype(np.float32), 'T_Tray_BurnNoise')


report = []
# Unity Joint mesh: position y .04126, scale .33. Blender's import already lands on that span.
load(data + 'Models/Cigarettes.fbx')
report.append(bake('SM_Tray_Joint', keep={'Cigarettes1:pCylinder1'}, recenter='ccb'))
# Prefab chain resolves to a uniform 1.385 on the imported tray.
load(data + 'Models/metal-tray/source/model.fbx')
report.append(bake('SM_Tray_Tray', transform=Matrix.Scale(1.385, 4), recenter='ccb'))
# Root scale .4,.4,.6527 with Unity's -90 degree X; Blender's Z already carries the 1.63 ratio.
load(data + 'Models/Ashtray/Ashtray.FBX')
report.append(bake('SM_Tray_Ashtray', transform=Matrix.Scale(.4, 4), recenter='ccb'))
# Root 1.5 times child .04 gives a 12 cm lighter, standing on its base.
load(data + 'basic-lighter (1)/source/model/model/model.dae')
report.append(bake('SM_Tray_Lighter', transform=Matrix.Scale(.06, 4), recenter='ccb'))
# Root .05 uniform once parent and child scales combine.
load(data + 'Models/weed-grinder-rasta-color/source/model/model/model.dae')
report.append(bake('SM_Tray_Grinder', transform=Matrix.Scale(.05, 4), recenter='ccb'))
# Parent and child scales combine to .000455 uniform.
load(data + 'OCB_Slim_model/OCB_Slim_model.obj')
report.append(bake('SM_Tray_Papers', transform=Matrix.Scale(.000455, 4), recenter='ccb'))

bpy.ops.wm.read_factory_settings(use_empty=True)
textures = {
    'T_Tray_Joint_Albedo': 'TexMat/MainTex-3.png', 'T_Tray_Joint_Normal': 'TexMat/Normal.png',
    'T_Tray_Joint_BurnMask': 'TexMat/BurningMask.png', 'T_Tray_Joint_Cheek': 'TexMat/Cheek.png',
    'T_Tray_Metal_Albedo': 'Models/metal-tray/textures/Tray_MAT_albedo.jpeg',
    'T_Tray_Metal_Normal': 'Models/metal-tray/textures/Tray_MAT_normal.png',
    'T_Tray_Ashtray_Albedo': 'Models/Ashtray/Textures/Astr_Dif.png',
    'T_Tray_Ashtray_Normal': 'Models/Ashtray/Textures/Astr_Nor.jpg',
    'T_Tray_Lighter_Body': 'basic-lighter (1)/textures/lambert2SG_albedo.jpeg',
    'T_Tray_Lighter_Cap': 'basic-lighter (1)/textures/lambert4SG_albedo.jpeg',
    'T_Tray_Grinder_Albedo': 'Models/weed-grinder-rasta-color/textures/initialShadingGroup_albedo.jpeg',
    'T_Tray_Paper_Leaf': 'OCB_Slim_model/Feuille_Diffuse.jpg',
}
for name, relative in textures.items():
    convert(data + relative, name)
for name, relative, size in (('T_Tray_Flame', 'TexMat/fire.png', 512), ('T_Tray_Spark', 'TexMat/lighterSpark.png', 256),
                             ('T_Tray_Exhale', 'SteamParticle/TexMat/Joint Smoke.png', 256)):
    convert(data + relative, name, size)
brand_pack()
rainbow_lines()
burn_noise()
(out / 'tray_source_provenance.json').write_text(json.dumps(dict(source=source, meshes=report), indent=2))
print('WMCYN_TRAY_SOURCE', json.dumps(report))
