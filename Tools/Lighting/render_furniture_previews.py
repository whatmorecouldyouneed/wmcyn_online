"""Offline neutral geometry previews; never imports into Unreal."""
import json
import sys
from pathlib import Path
import bpy
from mathutils import Vector

source, measurements, output = [Path(p).resolve() for p in sys.argv[sys.argv.index('--')+1:]]
output.mkdir(parents=True, exist_ok=True)
scene = bpy.context.scene
scene.render.engine = 'BLENDER_WORKBENCH'
scene.render.resolution_x = 640
scene.render.resolution_y = 480
scene.render.resolution_percentage = 100
scene.display.shading.light = 'STUDIO'
scene.display.shading.color_type = 'SINGLE'
scene.display.shading.single_color = (0.55,0.62,0.65)
scene.display.shading.show_shadows = True
scene.display.shading.show_cavity = True
scene.display.shading.background_type = 'WORLD'
scene.world.color = (0.85,0.85,0.85)
for item in json.loads(measurements.read_text()):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.fbx(filepath=str(source/item['source']), use_image_search=False)
    low, high = map(Vector, item['bounds_m'])
    center = (low+high)/2
    size = max(high-low)
    bpy.ops.object.camera_add(location=center+Vector((1.3,-1.7,1.3))*size)
    camera = bpy.context.object
    camera.rotation_euler = (center-camera.location).to_track_quat('-Z','Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = size*1.65
    scene.camera = camera
    scene.render.filepath = str(output/(Path(item['source']).stem+'.png'))
    bpy.ops.render.render(write_still=True)
