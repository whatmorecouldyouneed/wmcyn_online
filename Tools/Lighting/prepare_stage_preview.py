"""Blender batch extraction: source geometry remains unchanged on disk."""
import bpy
import json
from pathlib import Path
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[2]
SRC = Path('C:/Users/jvred/WMCYN v2.0/Assets')
OUT = ROOT/'SourceArt/StagePilot'
OUT.mkdir(parents=True,exist_ok=True)
report=[]
for key,source in [('StageDeck',SRC/'stage.fbx'),('AlephStandard',SRC/'aleph 9/VDJ_R2100/Model/Booth_StandardSet.fbx')]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(source),use_image_search=False)
    objects=[o for o in bpy.context.scene.objects if o.type=='MESH' and (key!='StageDeck' or o.name=='Cube.001')]
    assert objects,key
    vs=[o.matrix_world@Vector(v) for o in objects for v in o.bound_box]
    lo=Vector([min(v[i] for v in vs) for i in range(3)])
    hi=Vector([max(v[i] for v in vs) for i in range(3)])
    center=Vector(((lo.x+hi.x)/2,(lo.y+hi.y)/2,lo.z))
    bpy.ops.object.select_all(action='DESELECT')
    for o in objects:
        matrix=o.matrix_world.copy()
        o.parent=None
        o.matrix_world=matrix
        o.location-=center
        o.select_set(True)
    bpy.context.view_layer.objects.active=objects[0]
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    if key=='StageDeck':
        for o in objects:
            for v in o.data.vertices:
                v.co.x*=4.0/(hi.x-lo.x)
                v.co.y*=3.0/(hi.y-lo.y)
                v.co.z*=.24/(hi.z-lo.z)
    bpy.ops.export_scene.fbx(filepath=str(OUT/(key+'.fbx')),use_selection=True,object_types={'MESH'},add_leaf_bones=False,axis_forward='-Y',axis_up='Z',bake_anim=False)
    report.append(dict(name=key,source=str(source),source_dimensions_m=list(hi-lo),materials=sorted({m.name for o in objects for m in o.data.materials if m})))
(OUT/'provenance.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
