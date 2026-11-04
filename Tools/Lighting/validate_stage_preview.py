"""Verify local stage support and record layout; save only the active level."""
import json
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
assert not unreal.EditorAssetLibrary.does_asset_exist('/Game/WMCYN/Environment/Crib/StagePilot/Meshes/SM_StageComplete'), 'Superseded by complete stage layout'
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors={a.get_actor_label():a for a in sub.get_all_level_actors()}
deck=actors['STAGE_Pilot_Deck']
step=actors['STAGE_Pilot_FrontStep']
booth=actors['DJ_Pilot_AlephStandard']
booth.set_actor_location(unreal.Vector(-1890,-2050,32),False,False)
booth.set_actor_rotation(unreal.Rotator(pitch=0,yaw=270,roll=0),False)
light=actors.get('LGT_StagePilot_Preview')
if not light:
    light=sub.spawn_actor_from_class(unreal.PointLight,unreal.Vector(-1770,-2050,320))
    light.set_actor_label('LGT_StagePilot_Preview')
    light.set_folder_path('WMCYN_Production/StagePilot')
    light.tags=[unreal.Name('WMCYN_StagePilot')]
    light.point_light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.point_light_component.set_editor_property('intensity_units',unreal.LightUnits.LUMENS)
    light.point_light_component.set_intensity(350)
    light.point_light_component.set_attenuation_radius(650)
    light.point_light_component.set_cast_shadows(False)
light.point_light_component.set_intensity(350)

checks=[]
for actor,xs,ys,z in [(deck,[-1980,-1900,-1800,-1720],[-2230,-2150,-2050,-1950,-1870],32),(step,[-1680,-1640],[-2100,-2050,-2000],20)]:
    comp=actor.static_mesh_component
    for x in xs:
        for y in ys:
            hit=comp.line_trace_component(unreal.Vector(x,y,100),unreal.Vector(x,y,-50),False,False,False)
            passed=bool(hit and abs(hit[0].z-z)<.5 and hit[1].z>.95)
            checks.append(dict(actor=actor.get_actor_label(),x=x,y=y,expected_z=z,hit_z=hit[0].z if hit else None,passed=passed))
assert all(c['passed'] for c in checks),checks
crib=next(a for a in actors.values() if a.get_actor_label().startswith('FbxScene_wmcyn_cri'))
stairs=next(c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='stairs')
so,se,_=unreal.SystemLibrary.get_component_bounds(stairs)
do,de,_=unreal.SystemLibrary.get_component_bounds(deck.static_mesh_component)
gap_y=(so.y-se.y)-(do.y+de.y)
assert gap_y>100,gap_y
report=dict(simple_support_checks=checks,stairs_bounds_separation_y_cm=gap_y,deck_top_cm=32,step_top_cm=20,headset_walkability='pending',remote='not tested',light='temporary local layout preview: one non-shadowing point light',actors=[dict(label=a.get_actor_label(),location=list(a.get_actor_location().to_tuple()),rotation=str(a.get_actor_rotation())) for a in [deck,step,booth,light]])
path=Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting/stage_preview_validation.json'
path.write_text(json.dumps(report,indent=2))
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('WMCYN_STAGE_VALIDATED',len(checks),'support checks, stair Y separation',gap_y)
