"""Idempotent fixture-aligned lights. Run in editor with --phase pilot|all."""
import argparse
import json
from pathlib import Path
import unreal

args = argparse.ArgumentParser()
args.add_argument("--phase", choices=("pilot", "all"), default="pilot")
phase = args.parse_args().phase
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor(), "Stop PIE before editing lighting"
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world.get_path_name() == "/Game/Levels/L_WMCYNOnline.L_WMCYNOnline", world.get_path_name()
out = Path(unreal.Paths.project_saved_dir()) / "Diagnostics/Lighting"
surfaces = json.loads((out / "fixture_islands.json").read_text(encoding="utf-8"))
existing = {a.get_actor_label(): a for a in sub.get_all_level_actors()}
fixture_components = {"house parts low poly_001", "house parts low poly_003",
                      "house parts low poly_010", "house parts low poly_011",
                      "house parts low poly_017"}
fixtures = [s for s in surfaces if s["component"] in fixture_components
            and 20 < s["size"][0] < 22 and 20 < s["size"][1] < 22
            and 6 < s["size"][2] < 7]
fixtures.sort(key=lambda s: (s["component"], *s["center"]))
assert len(fixtures) == 73, "Fixture layout changed; inspect survey before applying"

def place(cls, label, position, rotation, folder):
    actor = existing.get(label)
    if actor is None:
        actor = sub.spawn_actor_from_class(cls, unreal.Vector(*position), rotation)
        assert actor
        actor.set_actor_label(label)
        existing[label] = actor
    actor.set_actor_location_and_rotation(unreal.Vector(*position), rotation, False, True)
    actor.set_folder_path("WMCYN_02_Crib_Lighting/" + folder)
    return actor

placements = []
with unreal.ScopedEditorTransaction("WMCYN fixture lighting " + phase):
    crib = next(a for a in sub.get_all_level_actors()
                if a.get_name() == "FbxScene_wmcyn_crib_default_unity_transfer_C_1")
    for comp in crib.get_components_by_class(unreal.StaticMeshComponent):
        if comp.get_name() in ("Plane_003", "Plane_004"):
            comp.set_editor_property("override_light_map_res", True)
            comp.set_editor_property("overridden_light_map_res", 512)
    for i, fixture in enumerate(fixtures):
        if phase == "pilot" and fixture["component"] != "house parts low poly_001":
            continue
        x, y, z = fixture["center"]
        zone = "Upper" if z > 600 else "Main" if z > 100 else "Lower"
        label = "LGT_Crib_%s_Ceiling_%02d" % (zone, i + 1)
        position = [x, y, fixture["min"][2] - 6]
        actor = place(unreal.SpotLight, label, position, unreal.Rotator(pitch=-90, yaw=0, roll=0), "Ceiling")
        light = actor.get_component_by_class(unreal.SpotLightComponent)
        light.set_mobility(unreal.ComponentMobility.STATIC)
        light.set_intensity_units(unreal.LightUnits.LUMENS)
        light.set_intensity(2200)
        light.set_temperature(4000)
        light.set_use_temperature(True)
        light.set_attenuation_radius(850)
        light.set_inner_cone_angle(35)
        light.set_outer_cone_angle(75)
        light.set_source_radius(7)
        light.set_cast_shadows(True)
        assert light.get_forward_vector().z < -0.999, "Ceiling light must point down"
        placements.append({"label": label, "position": position, "source": fixture})
    if phase == "all":
        pendants = [s for s in surfaces if s["component"] in ("Overhead bar lights", "overhead bar lights 2")
                    and 8 < s["size"][0] < 10 and 8 < s["size"][1] < 10 and s["size"][2] < 0.1]
        pendants.sort(key=lambda s: (s["component"], *s["center"]))
        assert len(pendants) == 10
        for i, fixture in enumerate(pendants):
            x, y, z = fixture["center"]
            label = "LGT_Crib_Pendant_%02d" % (i + 1)
            position = [x, y, z - 6]
            actor = place(unreal.SpotLight, label, position,
                          unreal.Rotator(pitch=-90, yaw=0, roll=0), "Ceiling")
            light = actor.get_component_by_class(unreal.SpotLightComponent)
            light.set_mobility(unreal.ComponentMobility.STATIC)
            light.set_intensity_units(unreal.LightUnits.LUMENS)
            light.set_intensity(650)
            light.set_temperature(3500)
            light.set_use_temperature(True)
            light.set_attenuation_radius(450)
            light.set_inner_cone_angle(30)
            light.set_outer_cone_angle(65)
            light.set_source_radius(4)
            light.set_cast_shadows(True)
            assert light.get_forward_vector().z < -0.999
            placements.append({"label": label, "position": position, "source": fixture})
        for zone, position in [("Upper", [-700, -400, 650]), ("Main", [-700, 400, 180]),
                               ("Lower", [-1500, 300, -300])]:
            capture = place(unreal.SphereReflectionCapture, "RC_Crib_" + zone,
                            position, unreal.Rotator(), "BakeSupport")
            capture.get_component_by_class(unreal.SphereReflectionCaptureComponent).set_editor_property("influence_radius", 1600)
    volume = place(unreal.LightmassImportanceVolume, "LMIV_Crib_Occupied",
                   [-700, -500, 300], unreal.Rotator(), "BakeSupport")
    # Editor actor factory creates a 200 cm brush; verify before scaling it.
    center, extent = volume.get_actor_bounds(False)
    if min(extent.to_tuple()) > 0:
        volume.set_actor_scale3d(unreal.Vector(30, 35, 10))
    else:
        unreal.log_warning("WMCYN importance volume has no brush; requires editor volume setup")
    sub.clear_actor_selection_set()
assert level.save_current_level(), "Level save failed"
(out / ("placements_" + phase + ".json")).write_text(json.dumps(placements, indent=2), encoding="utf-8")
unreal.log("WMCYN lighting applied: %s, %d fixtures" % (phase, len(placements)))
