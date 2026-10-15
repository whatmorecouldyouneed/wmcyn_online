"""Compare traces at every horizontal source-shell triangle centroid."""
import json
import math
from pathlib import Path
import unreal

assert not unreal.EditorLevelLibrary.get_pie_worlds(False)
root = Path(unreal.Paths.project_saved_dir())/'Diagnostics/Lighting'
data = json.loads((root/'door_shell_geometry.json').read_text())
shell = next(c for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() for c in a.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='collider')
original = unreal.load_asset('/Game/Environments/WMCYN_Crib/Scene_002')
replacement = unreal.load_asset('/Game/WMCYN/Environment/Crib/Doors/SM_Crib_Collision_Doorways')
points = []
for i in range(0,len(data['triangles']),3):
    tri = [data['vertices'][j] for j in data['triangles'][i:i+3]]
    a = unreal.Vector(*tri[1])-unreal.Vector(*tri[0])
    b = unreal.Vector(*tri[2])-unreal.Vector(*tri[0])
    n = a.cross(b)
    if n.length()>0 and abs(n.z)/n.length()>0.5:
        points.append(unreal.Vector(*[sum(p[j] for p in tri)/3 for j in range(3)]))
samples = []
passed = False
try:
    for mesh in (original,replacement):
        shell.set_static_mesh(mesh)
        samples.append([shell.line_trace_component(p+unreal.Vector(0,0,5),p-unreal.Vector(0,0,5),True,False,False) for p in points])
    mismatches = []
    for i,(before,after) in enumerate(zip(*samples)):
        if bool(before)!=bool(after) or (before and (before[0]-after[0]).length()>0.1):
            mismatches.append(i)
    passed = not mismatches
    result = dict(samples=len(points),original_hits=sum(bool(x) for x in samples[0]),replacement_hits=sum(bool(x) for x in samples[1]),mismatches=mismatches,passed=passed)
    (root/'door_floor_preservation.json').write_text(json.dumps(result,indent=2))
    print('WMCYN_DOOR_FLOOR_PRESERVATION',result)
finally:
    shell.set_static_mesh(replacement if passed else original)
assert passed
