"""Read-only component traces across each doorway and below its threshold."""
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
plan = json.loads((root/'door_rollout_plan.json').read_text())
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
crib = next(a for a in sub.get_all_level_actors() if a.get_name()=='FbxScene_wmcyn_crib_default_unity_transfer_C_1')
components = [c for c in crib.get_components_by_class(unreal.StaticMeshComponent) if c.static_mesh and c.get_collision_enabled()!=unreal.CollisionEnabled.NO_COLLISION]
rows = []
for group in plan:
    for item in group['leaves']:
        lo, hi = item['source_bounds']
        center = [(a+b)/2 for a,b in zip(lo,hi)]
        if item['number']==1:
            center = [-798,90,600]
            axis = 1
        elif item['number'] in [13,14]:
            center = [-939 if item['number']==13 else -1049,-121,-330]
            axis = 1
        else:
            axis = min((0,1),key=lambda a:hi[a]-lo[a])
        start, end = list(center), list(center)
        start[axis]-=40
        end[axis]+=40
        hits = []
        for c in components:
            hit = c.line_trace_component(unreal.Vector(*start),unreal.Vector(*end),True,False,False)
            if hit:
                hits.append(dict(component=c.get_name(),location=list(hit[0].to_tuple())))
        floor = []
        for c in components:
            hit = c.line_trace_component(unreal.Vector(center[0],center[1],lo[2]+60),unreal.Vector(center[0],center[1],lo[2]-100),True,False,False)
            if hit:
                floor.append(dict(component=c.get_name(),location=list(hit[0].to_tuple())))
        rows.append(dict(number=item['number'],across=hits,floor=floor,center=center,axis=axis))
(root/'door_collision_checks.json').write_text(json.dumps(rows,indent=2))
print('WMCYN_DOOR_COLLISION_CHECKS',rows)
