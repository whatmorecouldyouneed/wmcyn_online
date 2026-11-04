"""Run in background Blender; measure source furniture without saving source files."""
import json
from pathlib import Path
import sys
import bpy
from mathutils import Vector

source, output = map(Path, sys.argv[sys.argv.index('--') + 1:])
names = ['sectional_sofa01', 'accent_chair01', 'coffee_table', 'dining_chair01',
         'floor_lamp01', 'kitchen_table', 'rug01', 'sideboard01', 'bar_stool01',
         'side_table', 'side_table01']
results = []
for name in names:
    path = source / (name + '.fbx')
    if not path.exists():
        continue
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.fbx(filepath=str(path), use_image_search=False)
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    corners = [o.matrix_world @ Vector(p) for o in meshes for p in o.bound_box]
    low = [min(p[i] for p in corners) for i in range(3)]
    high = [max(p[i] for p in corners) for i in range(3)]
    for obj in meshes:
        obj.data.calc_loop_triangles()
    results.append(dict(source=path.name, bounds_m=[low, high],
        dimensions_m=[round(high[i]-low[i], 4) for i in range(3)],
        triangles=sum(len(o.data.loop_triangles) for o in meshes),
        materials=sorted({m.name for o in meshes for m in o.data.materials if m}),
        objects=[o.name for o in meshes]))
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(results, indent=2))
print('WMCYN_FURNITURE_BOUNDS', json.dumps(results))
