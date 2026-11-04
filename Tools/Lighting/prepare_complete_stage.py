"""Preserve the entire source assembly; center XY and use deck top as Z origin."""
import bpy
import json
from pathlib import Path
from mathutils import Vector

root=Path(__file__).resolve().parents[2]
out=root/'SourceArt/StagePilot'
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath='C:/Users/jvred/WMCYN v2.0/Assets/stage.fbx',use_image_search=False)
objects=[o for o in bpy.context.scene.objects if o.type=='MESH']
deck=next(o for o in objects if o.name=='Cube.001')
vs=[deck.matrix_world@Vector(v) for v in deck.bound_box]
center=Vector(((min(v.x for v in vs)+max(v.x for v in vs))/2,(min(v.y for v in vs)+max(v.y for v in vs))/2,max(v.z for v in vs)))
# Capture matrices before unparenting so children cannot inherit intermediate edits.
matrices={o.name:o.matrix_world.copy() for o in objects}
bpy.ops.object.select_all(action='DESELECT')
for o in objects:
    o.parent=None
    o.matrix_world=matrices[o.name]
    o.location-=center
    o.select_set(True)
bpy.context.view_layer.objects.active=deck
bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
bpy.ops.export_scene.fbx(filepath=str(out/'StageComplete.fbx'),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',bake_anim=False,add_leaf_bones=False)
report=dict(meshes=len(objects),polygons=sum(len(o.data.polygons) for o in objects),materials=sorted({m.name for o in objects for m in o.data.materials if m}),origin='Original deck top center',placement_scale=5/19.388954162597656)
(out/'complete_stage_provenance.json').write_text(json.dumps(report,indent=2))
print(report)
