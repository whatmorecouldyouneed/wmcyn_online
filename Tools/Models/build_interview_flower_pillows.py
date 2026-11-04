"""Recreate measured decorative flower pillows from Jared's interview-set photo.

Run with Blender --background --python this_file. Geometry is authored in metres;
FBX carries units. One shared atlas/material, no fur, rigging, or physics.
"""
import json
import math
from pathlib import Path

import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'SourceArt' / 'InterviewFlowers'
OUT.mkdir(parents=True, exist_ok=True)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

# Clockwise from twelve o'clock, matching the supplied physical pillows.
COLORS = [(219, 65, 132), (255, 151, 43), (232, 211, 57), (239, 235, 93),
          (217, 222, 210), (143, 197, 51), (111, 199, 174), (64, 163, 129),
          (37, 160, 199), (26, 102, 185), (19, 40, 49), (137, 64, 170),
          (14, 19, 23), (78, 191, 192), (243, 220, 88), (220, 29, 46),
          (233, 231, 210), (39, 32, 28), (249, 249, 239), (191, 97, 50)]
SIZE = 1024
GRID = 8
image = bpy.data.images.new('T_InterviewFlower_Fabric', SIZE, SIZE)
pixels = [0.0] * (SIZE * SIZE * 4)
for y in range(SIZE):
    for x in range(SIZE):
        color = COLORS[min((y // 128) * GRID + x // 128, len(COLORS) - 1)]
        noise = (((x * 73856093) ^ (y * 19349663)) % 997) / 997.0
        grain = 0.92 + noise * 0.16
        i = (y * SIZE + x) * 4
        pixels[i:i+4] = [min(1.0, c / 255.0 * grain) for c in color] + [1.0]
image.pixels.foreach_set(pixels)
image.filepath_raw = str(OUT / 'T_InterviewFlower_Fabric.png')
image.file_format = 'PNG'
image.save()
material = bpy.data.materials.new('M_InterviewFlower_Fabric')
material.use_nodes = True
shader = material.node_tree.nodes.get('Principled BSDF')
shader.inputs['Roughness'].default_value = 0.95
tex = material.node_tree.nodes.new('ShaderNodeTexImage')
tex.image = image
material.node_tree.links.new(tex.outputs['Color'], shader.inputs['Base Color'])


def finish(obj, color):
    obj.data.materials.clear()
    obj.data.materials.append(material)
    for face in obj.data.polygons:
        face.use_smooth = True
    for uv in obj.data.uv_layers.active.data:
        uv.uv.x = (color % GRID + 0.06 + uv.uv.x * 0.88) / GRID
        uv.uv.y = (color // GRID + 0.06 + uv.uv.y * 0.88) / GRID
    return obj


def oval(name, pos, scale, color, angle=0, segments=24, rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=pos)
    obj = bpy.context.object
    obj.name = name
    if name.startswith('Petal_'):
        for vertex in obj.data.vertices:
            for axis in (1, 2):
                v = vertex.co[axis]
                vertex.co[axis] = math.copysign(abs(v)**0.82, v)
    obj.scale = scale
    obj.rotation_euler.x = -angle
    finish(obj, color)
    return obj


def smile(radius, color):
    # Curved applique conforming to the front of the flattened face cushion.
    verts = [(0.068, 0, -0.013)]
    rings, steps = 6, 32
    for ring in range(1, rings + 1):
        r = ring / rings
        for j in range(steps + 1):
            t = math.pi * j / steps
            y = radius * math.cos(t) * r
            z = -0.013 - radius * 0.82 * math.sin(t) * r
            x = 0.066 * math.sqrt(max(0.04, 1 - (y / 0.133)**2 - (z / 0.133)**2)) + 0.002
            verts.append((x, y, z))
    faces = [(0, 1+j, 2+j) for j in range(steps)]
    for ring in range(rings-1):
        a, b = 1+ring*(steps+1), 1+(ring+1)*(steps+1)
        faces.extend((a+j, b+j, b+j+1, a+j+1) for j in range(steps))
    mesh = bpy.data.meshes.new('SmileApplique')
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new('Smile', mesh)
    bpy.context.collection.objects.link(obj)
    uv = mesh.uv_layers.new()
    for poly in mesh.polygons:
        for li in poly.loop_indices:
            co = mesh.vertices[mesh.loops[li].vertex_index].co
            uv.data[li].uv = (co.y / (radius*2)+0.5, co.z / (radius*2)+0.5)
    finish(obj, color)
    # The generated arc runs clockwise when seen from +X.
    for p in mesh.polygons:
        if p.normal.x < 0:
            p.flip()
    return obj


reports = []
for variant, diameter, facecolor, mouthcolor in [('Large', 0.5588, 12, 13), ('Small', 0.3048, 14, 15)]:
    bpy.ops.object.select_all(action='DESELECT')
    objects = []
    for p in range(12):
        a = p * math.tau / 12
        objects.append(oval('Petal_%02d' % p, (0, math.sin(a)*0.193, math.cos(a)*0.193),
                            (0.047, 0.057, 0.0864), p, a))
    objects.append(oval('Face', (0, 0, 0), (0.066, 0.133, 0.133), facecolor, segments=40, rings=20))
    objects.append(smile(0.105, mouthcolor))
    for y in [-0.042, 0.042]:
        objects.append(oval('Eye_Stitch', (0.063, y, 0.035), (0.005, 0.008, 0.014), 16, segments=16, rings=8))
        objects.append(oval('Eye_Pupil', (0.067, y, 0.034), (0.003, 0.0037, 0.008), 17, segments=12, rings=8))
        objects.append(oval('Eye_Glint', (0.070, y-0.001, 0.038), (0.001, 0.0016, 0.0025), 18, segments=8, rings=6))
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = 'SM_InterviewFlower_' + variant
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    obj.scale = (diameter / 0.5588,) * 3
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    # Collapse identical material slots after joining to retain one draw section.
    obj.data.materials.clear()
    obj.data.materials.append(material)
    for p in obj.data.polygons:
        p.material_index = 0
    bpy.ops.export_scene.fbx(filepath=str(OUT / (obj.name + '.fbx')), use_selection=True,
                             object_types={'MESH'}, add_leaf_bones=False, mesh_smooth_type='FACE',
                             path_mode='COPY', embed_textures=False)
    reports.append({'mesh': obj.name, 'diameter_cm': diameter*100,
                    'dimensions_cm': [v*100 for v in obj.dimensions],
                    'triangles': sum(len(p.vertices)-2 for p in obj.data.polygons),
                    'material_slots': len(obj.data.materials)})
    obj.hide_set(True)
    obj.select_set(False)
(OUT / 'build_report.json').write_text(json.dumps(reports, indent=2))
print(json.dumps(reports, indent=2))
