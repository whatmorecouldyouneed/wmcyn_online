"""Inspect disconnected source pieces; export one approved pillow by island index."""
import bpy, json, sys
from pathlib import Path
args = sys.argv[sys.argv.index('--')+1:]
out = Path(args[0])
out.mkdir(parents=True, exist_ok=True)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath='C:/Users/jvred/WMCYN v2.0/Assets/Hidden Heights SGB/fbx/sectional_sofa01.fbx')
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
bpy.ops.object.select_all(action='DESELECT')
for o in meshes: o.select_set(True)
bpy.context.view_layer.objects.active = meshes[0]
bpy.ops.object.join()
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.mesh.separate(type='LOOSE')
bpy.ops.object.mode_set(mode='OBJECT')
pieces = sorted([o for o in bpy.context.scene.objects if o.type=='MESH'], key=lambda o:o.name)
rows=[]
for i,o in enumerate(pieces):
    from mathutils import Vector
    pts=[o.matrix_world @ Vector(p) for p in o.bound_box]
    lo=[min(p[j] for p in pts) for j in range(3)]
    hi=[max(p[j] for p in pts) for j in range(3)]
    rows.append(dict(index=i,name=o.name,vertices=len(o.data.vertices),min=lo,max=hi,size=[hi[j]-lo[j] for j in range(3)]))
(out/'sectional_islands.json').write_text(json.dumps(rows,indent=2))
if len(args)>1:
    o=pieces[int(args[1])]
    bpy.ops.object.select_all(action='DESELECT')
    o.select_set(True)
    bpy.context.view_layer.objects.active=o
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY',center='BOUNDS')
    o.location=(0,0,0)
    bpy.ops.export_scene.fbx(filepath=str(out/'SM_Crib_SectionalThrowPillow.fbx'),use_selection=True,object_types={'MESH'},add_leaf_bones=False)
