"""Build tray session materials, sounds and mesh slots. Safe to rerun: each asset is rebuilt.

Run headless with the editor closed, for example:
UnrealEditor-Cmd wmcyn_online.uproject -run=pythonscript -script=Tools/TraySession/build_tray_materials.py
"""
import json
from pathlib import Path

import unreal

ROOT = '/Game/WMCYN/Interactables/TraySession'
project = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
sounds = Path('C:/Users/jvred/WMCYN v2.0/Assets/Joints By Matic 954/Joint Roller/DataStore/Sounds')
lib = unreal.MaterialEditingLibrary
assets = unreal.EditorAssetLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def texture(name):
    asset = unreal.load_asset(f'{ROOT}/Textures/{name}')
    assert asset, name
    return asset


class Graph:
    def __init__(self, name, **properties):
        path = f'{ROOT}/Materials/{name}'
        if assets.does_asset_exist(path):
            assets.delete_asset(path)
        self.material = tools.create_asset(name, f'{ROOT}/Materials', unreal.Material, unreal.MaterialFactoryNew())
        for key, value in properties.items():
            self.material.set_editor_property(key, value)

    def node(self, cls, **properties):
        expression = lib.create_material_expression(self.material, getattr(unreal, 'MaterialExpression' + cls))
        for key, value in properties.items():
            expression.set_editor_property(key, value)
        return expression

    def link(self, source, output, target, pin):
        assert lib.connect_material_expressions(source, output, target, pin), (source, output, pin)
        return target

    def binary(self, cls, a, b, a_out='', b_out=''):
        expression = self.node(cls)
        self.link(a, a_out, expression, 'A')
        self.link(b, b_out, expression, 'B')
        return expression

    def unary(self, cls, source, source_out='', **properties):
        return self.link(source, source_out, self.node(cls, **properties), '')

    def lerp(self, a, b, alpha, a_out='', b_out='', alpha_out=''):
        expression = self.node('LinearInterpolate')
        self.link(a, a_out, expression, 'A')
        self.link(b, b_out, expression, 'B')
        self.link(alpha, alpha_out, expression, 'Alpha')
        return expression

    def scalar(self, value):
        return self.node('Constant', r=value)

    def colour(self, r, g, b):
        return self.node('Constant3Vector', constant=unreal.LinearColor(r, g, b, 1))

    def sample(self, name, uv=None):
        expression = self.node('TextureSample', texture=texture(name))
        if uv:
            self.link(uv, '', expression, 'UVs')
        return expression

    def output(self, expression, prop, pin=''):
        assert lib.connect_material_property(expression, pin, prop), prop

    def finish(self):
        lib.layout_material_expressions(self.material)
        lib.recompile_material(self.material)
        assets.save_loaded_asset(self.material)
        return self.material


def prop_material():
    graph = Graph('M_Tray_Prop')
    albedo = graph.node('TextureSampleParameter2D', parameter_name='Albedo', texture=texture('T_Tray_Metal_Albedo'))
    graph.output(albedo, unreal.MaterialProperty.MP_BASE_COLOR, 'RGB')
    graph.output(graph.node('ScalarParameter', parameter_name='Roughness', default_value=.6), unreal.MaterialProperty.MP_ROUGHNESS)
    graph.output(graph.node('ScalarParameter', parameter_name='Metallic', default_value=0), unreal.MaterialProperty.MP_METALLIC)
    parent = graph.finish()
    made = {}
    for name, albedo_name, roughness, metallic in (
            ('Metal', 'T_Tray_Metal_Albedo', .45, .15), ('Ashtray', 'T_Tray_Ashtray_Albedo', .4, .2),
            ('LighterBody', 'T_Tray_Lighter_Body', .45, .1), ('LighterCap', 'T_Tray_Lighter_Cap', .4, .3),
            ('Grinder', 'T_Tray_Grinder_Albedo', .5, .1), ('Pack', 'T_Tray_Pack_WMCYN', .7, 0),
            ('Leaf', 'T_Tray_Paper_Leaf', .8, 0)):
        asset = f'MI_Tray_{name}'
        path = f'{ROOT}/Materials/{asset}'
        if assets.does_asset_exist(path):
            assets.delete_asset(path)
        instance = tools.create_asset(asset, f'{ROOT}/Materials', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        lib.set_material_instance_parent(instance, parent)
        lib.set_material_instance_texture_parameter_value(instance, 'Albedo', texture(albedo_name))
        lib.set_material_instance_scalar_parameter_value(instance, 'Roughness', roughness)
        lib.set_material_instance_scalar_parameter_value(instance, 'Metallic', metallic)
        assets.save_loaded_asset(instance)
        made[name] = instance
    return made


def joint_material():
    """Port of Joint.shader. The source's side burn band sits above the mesh's V range, so the
    glow lives on the masked tip. Per-pixel fBm becomes a tiled texture for Quest."""
    graph = Graph('M_Tray_Joint', two_sided=True, fully_rough=True)
    uv = graph.node('TextureCoordinate')
    albedo = graph.sample('T_Tray_Joint_Albedo', uv)
    mask = graph.sample('T_Tray_Joint_BurnMask', uv)
    cheek = graph.sample('T_Tray_Joint_Cheek', uv)
    noise = graph.sample('T_Tray_BurnNoise', graph.node('TextureCoordinate', u_tiling=12.5, v_tiling=12.5))
    # Matches the source fBm's narrower range around 0.5.
    fbm = graph.binary('Add', graph.binary('Multiply', noise, graph.scalar(.5), 'R'), graph.scalar(.25))
    value = graph.node('ScalarParameter', parameter_name='BurnValue', default_value=0,
                       use_custom_primitive_data=True, primitive_data_index=0)
    threshold = graph.node('ScalarParameter', parameter_name='BurnThreshold', default_value=1,
                           use_custom_primitive_data=True, primitive_data_index=1)
    flicker = graph.binary('Multiply', graph.unary('Sine', graph.node('Time'), period=.7), graph.scalar(.01))
    hot = graph.unary('Saturate', graph.binary('Multiply', graph.binary('Subtract', fbm, graph.binary('Add', threshold, flicker)), graph.scalar(200)))
    tip = graph.unary('Saturate', graph.binary('Multiply', graph.binary('Subtract', mask, graph.scalar(.5), 'R'), graph.scalar(100)))
    burn = graph.lerp(graph.colour(.934, .1806, .2117), graph.colour(.2627, .0688, 0), hot)
    base = graph.lerp(albedo, burn, tip, a_out='RGB')
    tint = graph.binary('Multiply', cheek, graph.colour(.6235, .2521, .0078), 'RGB')
    charred = graph.lerp(base, tint, value)
    char_alpha = graph.lerp(graph.scalar(1), cheek, value, b_out='A')
    graph.output(graph.lerp(base, charred, char_alpha), unreal.MaterialProperty.MP_BASE_COLOR)
    # BurnColor0 at the source's emission multiplier of 14.1.
    glow = graph.binary('Multiply', graph.binary('Multiply', hot, tip),
                        graph.binary('Multiply', graph.unary('Saturate', graph.binary('Multiply', fbm, graph.scalar(4))), graph.colour(3.705, .97, 0)))
    graph.output(glow, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    graph.output(graph.scalar(1), unreal.MaterialProperty.MP_ROUGHNESS)
    graph.output(graph.scalar(0), unreal.MaterialProperty.MP_METALLIC)
    height = graph.unary('ComponentMask', graph.node('PreSkinnedPosition'), r=False, g=False, b=True, a=False)
    above = graph.unary('Saturate', graph.binary('Multiply', graph.binary('Subtract', height, graph.scalar(6.11)), graph.scalar(100)))
    shorten = graph.binary('Multiply', graph.binary('Multiply', graph.colour(0, 0, -4.785), value), above)
    world = graph.unary('Transform', shorten, transform_source_type=unreal.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_LOCAL,
                        transform_type=unreal.MaterialVectorCoordTransform.TRANSFORM_WORLD)
    graph.output(world, unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    return graph.finish()


def flame_material():
    graph = Graph('M_Tray_Flame', blend_mode=unreal.BlendMode.BLEND_ADDITIVE, shading_model=unreal.MaterialShadingModel.MSM_UNLIT, two_sided=True)
    uv = graph.node('TextureCoordinate')
    panner = graph.link(uv, '', graph.node('Panner', speed_x=0, speed_y=-.6), 'Coordinate')
    noise = graph.sample('T_Tray_BurnNoise', panner)
    bent = graph.binary('Add', uv, graph.binary('Multiply', graph.binary('Subtract', noise, graph.scalar(.5), 'R'), graph.scalar(.06)))
    flame = graph.sample('T_Tray_Flame', bent)
    graph.output(graph.binary('Multiply', flame, graph.colour(2.6, 1.62, .82), 'RGB'), unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    return graph.finish()


def smoke_material(name, texture_name, near_fade):
    graph = Graph(name, blend_mode=unreal.BlendMode.BLEND_TRANSLUCENT, shading_model=unreal.MaterialShadingModel.MSM_UNLIT,
                  two_sided=True, used_with_niagara_sprites=True, used_with_niagara_ribbons=True)
    particle = graph.node('ParticleColor')
    smoke = graph.sample(texture_name)
    graph.output(graph.binary('Multiply', particle, smoke, 'RGB', 'RGB'), unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    opacity = graph.binary('Multiply', particle, smoke, 'A', 'A')
    if near_fade:
        # Keeps a breath from filling the holder's own view.
        near = graph.unary('Saturate', graph.binary('Multiply', graph.binary('Subtract', graph.node('PixelDepth'), graph.scalar(20)), graph.scalar(.025)))
        opacity = graph.binary('Multiply', opacity, near)
    graph.output(opacity, unreal.MaterialProperty.MP_OPACITY)
    return graph.finish()


def overlay_material():
    graph = Graph('M_Tray_HighOverlay', blend_mode=unreal.BlendMode.BLEND_TRANSLUCENT, shading_model=unreal.MaterialShadingModel.MSM_UNLIT,
                  two_sided=True, disable_depth_test=True)
    uv = graph.node('TextureCoordinate', u_tiling=2, v_tiling=2)
    panner = graph.link(uv, '', graph.node('Panner', speed_x=.01, speed_y=.02), 'Coordinate')
    graph.output(graph.sample('T_Tray_HighLines', panner), unreal.MaterialProperty.MP_EMISSIVE_COLOR, 'RGB')
    graph.output(graph.node('ScalarParameter', parameter_name='Opacity', default_value=0), unreal.MaterialProperty.MP_OPACITY)
    return graph.finish()


def assign_slots(instances, joint):
    slots = {
        'SM_Tray_Joint': {'Cigarette': joint}, 'SM_Tray_Tray': {'Tray_MAT': instances['Metal']},
        'SM_Tray_Ashtray': {'Material #2': instances['Ashtray']},
        'SM_Tray_Lighter': {'lambert2SG': instances['LighterBody'], 'lambert4SG': instances['LighterCap']},
        'SM_Tray_Grinder': {'initialShadingGroup': instances['Grinder']},
        'SM_Tray_Papers': {'Paquet': instances['Pack'], 'Feuille': instances['Leaf']},
    }
    for mesh_name, mapping in slots.items():
        mesh = unreal.load_asset(f'{ROOT}/Meshes/{mesh_name}')
        for index, slot in enumerate(mesh.get_editor_property('static_materials')):
            mesh.set_material(index, mapping[str(slot.material_slot_name)])
        assets.save_loaded_asset(mesh)


def import_sounds():
    made = []
    tasks = []
    for source in sorted(sounds.glob('*.mp3')):
        task = unreal.AssetImportTask()
        for key, value in dict(filename=str(source), destination_path=f'{ROOT}/Sounds', destination_name='S_Tray_' + source.stem,
                               automated=True, save=True, replace_existing=True).items():
            task.set_editor_property(key, value)
        tasks.append(task)
    tools.import_asset_tasks(tasks)
    for task in tasks:
        made.extend(str(path) for path in task.get_editor_property('imported_object_paths'))
    return made


instances = prop_material()
joint = joint_material()
flame_material()
smoke_material('M_Tray_Exhale', 'T_Tray_Exhale', True)
overlay_material()
assign_slots(instances, joint)
report = dict(sounds=import_sounds(), materials=[str(p) for p in assets.list_assets(f'{ROOT}/Materials', recursive=False)])
out = project / 'Saved/Diagnostics/TraySession'
out.mkdir(parents=True, exist_ok=True)
(out / 'materials.json').write_text(json.dumps(report, indent=2))
print('WMCYN_TRAY_MATERIALS', json.dumps(report))
