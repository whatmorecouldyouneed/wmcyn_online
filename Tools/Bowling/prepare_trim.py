"""Extract the authored ball return and backboard with a shared, centred object pivot."""
from pathlib import Path
import bpy
from mathutils import Matrix, Vector

src = Path.home()/'WMCYN v2.0/Assets/Bowling Alley Prefab'
out = Path(__file__).resolve().parents[2]/'Saved/BowlingSource'
out.mkdir(exist_ok=True)
for source, name in [('ball_return','Return'),('lane_end','Backstop'),('Alley_Cover','Canopy'),('TVs','OverheadTVs')]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(next(src.rglob(source+'.fbx'))),use_image_search=False)
    parts = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    pts = [o.matrix_world @ Vector(p) for o in parts for p in o.bound_box]
    pivot = Vector([(min(p[i] for p in pts)+max(p[i] for p in pts))/2 for i in range(3)])
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:
        m = o.matrix_world.copy()
        o.data = o.data.copy()
        o.parent = None
        o.data.transform(Matrix.Translation(-pivot) @ m)
        o.matrix_world = Matrix.Identity(4)
        o.select_set(True)
    bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,object_types={'MESH'},
                             axis_forward='-Y',axis_up='Z',bake_anim=False)
