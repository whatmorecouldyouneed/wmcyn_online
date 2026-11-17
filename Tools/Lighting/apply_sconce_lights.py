"""Batch static up/down washes on paired Crib sconce shells; SCONCE_PHASE=pilot or all."""
import json
import math
from pathlib import Path
import unreal

sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert not level.is_in_play_in_editor() and world.get_name() == "L_WMCYNOnline"
root = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
rows = json.loads((root / "fixture_islands.json").read_text())
shells = [r for r in rows if r["material"] == "lights" and
          all(abs(a-b) < 1 for a,b in zip(r["size"], (11,11,17)))]
actors = sub.get_all_level_actors()
by_label = {a.get_actor_label(): a for a in actors}
count = 0
for top in shells:
    x,y,z = top["center"]
    lower = [r for r in shells if r["component"] == top["component"]
             and math.dist((x,y), r["center"][:2]) < 1 and 34 < z-r["center"][2] < 41]
    if not lower:
        continue
    assert len(lower) == 1
    if globals().get("SCONCE_PHASE", "pilot") != "all" and not (
            abs(y+22)<2 and -1762<x<-1556 and abs(z+269)<2):
        continue
    for side, height, pitch in [('Up',top['max'][2]+2,90),('Down',lower[0]['min'][2]-2,-90)]:
        label = 'LGT_Crib_Sconce_%d_%d_%d_%s' % (round(x),round(y),round(z),side)
        pos = unreal.Vector(x,y,height)
        actor = by_label.get(label)
        if not actor:
            if any(isinstance(a,unreal.Light) and math.dist(a.get_actor_location().to_tuple(),pos.to_tuple())<8
                   for a in actors):
                unreal.log_warning('Existing light at '+label)
                continue
            actor = sub.spawn_actor_from_class(unreal.SpotLight,pos,unreal.Rotator(pitch=pitch))
            actor.set_actor_label(label)
        actor.set_folder_path('WMCYN_02_Crib_Lighting/Sconces')
        light = actor.get_component_by_class(unreal.SpotLightComponent)
        light.set_mobility(unreal.ComponentMobility.STATIC)
        light.set_intensity_units(unreal.LightUnits.LUMENS)
        light.set_use_temperature(True)
        light.set_temperature(3500)
        for key,val in [('intensity',12.),('attenuation_radius',320.),('inner_cone_angle',22.),
                        ('outer_cone_angle',62.),('source_radius',4.)]:
            light.set_editor_property(key,val)
        count += 1
unreal.log('WMCYN sconce pass: %d light placements' % count)
