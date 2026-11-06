"""Blender background script. Keep derived vendor geometry outside Git."""
from pathlib import Path
import bpy
from mathutils import Vector, Matrix

root = Path(__file__).resolve().parents[2]
source = Path.home() / 'WMCYN v2.0/Assets/Bowling Alley Prefab'
dest = root / 'Saved/BowlingSource'
dest.mkdir(exist_ok=True)
for src, name in [('bowling_lanes','Lane'), ('bowling_ball_1','Ball'), ('pins','Pin')]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(next(source.rglob(src+'.fbx'))), use_image_search=False)
    obj = next(o for o in bpy.context.scene.objects if o.type == 'MESH')
    corners = [obj.matrix_world @ Vector(v) for v in obj.bound_box]
    center = sum(corners,Vector()) / 8
    obj.data = obj.data.copy()
    obj.data.transform(Matrix.Translation(-center) @ obj.matrix_world)
    obj.parent = None
    obj.matrix_world = Matrix.Identity(4)
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    obj.name = name
    bpy.ops.export_scene.fbx(filepath=str(dest/(name+'.fbx')), use_selection=True,
                             object_types={'MESH'}, axis_forward='-Y', axis_up='Z', bake_anim=False)
