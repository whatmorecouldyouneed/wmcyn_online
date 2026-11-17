"""Soft baked washes for the Crib's tilted round inset lamps."""
import json
import math
from pathlib import Path
import unreal

level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not level.is_in_play_in_editor()
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name() == "L_WMCYNOnline"
rows = json.loads((Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting/fixture_islands.json").read_text())
existing = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
plan = []
for component, expected in (("Cylinder", 8), ("Cylinder_002", 10)):
    parts = [r for r in rows if r["component"] == component and r["material"] == "lights"]
    shells = [r for r in parts if r["vertices"] == 50]
    faces = [r for r in parts if r["vertices"] != 50]
    assert len(shells) == len(faces) == expected
    for shell in shells:
        matches = [r for r in faces if 1 < math.dist(shell["center"], r["center"]) < 3]
        assert len(matches) == 1
        face = matches[0]
        faces.remove(face)
        delta = unreal.Vector(*(a-b for a,b in zip(face["center"],shell["center"])))
        normal = delta / math.dist(face["center"],shell["center"])
        pos = unreal.Vector(*face["center"]) + normal * 3
        label = "LGT_Crib_Round_%s_%d_%d_%d" % (component,*map(round,face["center"]))
        plan.append((label,pos,unreal.MathLibrary.make_rot_from_x(normal)))
with unreal.ScopedEditorTransaction("Light round inset fixtures"):
    for label,pos,rot in plan:
        actor = existing.get(label)
        if not actor and any(isinstance(a,unreal.Light) and math.dist(pos.to_tuple(),a.get_actor_location().to_tuple()) < 8 for a in existing.values()):
            unreal.log_warning("Preserving nearby light: "+label)
            continue
        actor = actor or sub.spawn_actor_from_class(unreal.SpotLight,pos,rot)
        actor.set_actor_label(label)
        actor.set_actor_location_and_rotation(pos,rot,False,True)
        actor.set_folder_path("WMCYN_02_Crib_Lighting/RoundInsets")
        light = actor.light_component
        light.set_mobility(unreal.ComponentMobility.STATIC)
        light.set_intensity_units(unreal.LightUnits.LUMENS)
        light.set_use_temperature(True)
        light.set_temperature(3500)
        for key,value in (("intensity",18.),("attenuation_radius",380.),("source_radius",3.),
                          ("inner_cone_angle",25.),("outer_cone_angle",72.)):
            light.set_editor_property(key,value)
assert level.save_current_level()
unreal.log("WMCYN round inset pass complete")
