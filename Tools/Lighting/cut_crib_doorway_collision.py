"""Offline triangle/box clipping: preserve horizontal shell faces exactly."""
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]/'Saved/Diagnostics/Lighting'
data = json.loads((ROOT/'door_shell_geometry.json').read_text())
plan = json.loads((ROOT/'door_rollout_plan.json').read_text())
checks = {r['number']:r for r in json.loads((ROOT/'door_collision_checks.json').read_text())}
boxes = []
for group in plan:
    for leaf in group['leaves']:
        check = checks[leaf['number']]
        # Original-shell traces confirmed blockers at these closed leaves.
        # Do not depend on a later post-repair trace report to reproduce the cut.
        if leaf['number'] in (1,13,14):
            continue
        lo, hi = [list(p) for p in leaf['source_bounds']]
        axis = check['axis']
        lo[axis] = check['center'][axis]-45
        hi[axis] = check['center'][axis]+45
        lo[1-axis] += 3
        hi[1-axis] -= 3
        lo[2] += 5
        hi[2] -= 2
        boxes.append((leaf['number'],lo,hi))


def split(poly, axis, bound, sign):
    inside, outside = [], []
    for a,b in zip(poly,poly[1:]+poly[:1]):
        da, db = sign*(a[axis]-bound), sign*(b[axis]-bound)
        (inside if da<=0 else outside).append(a)
        if (da<0 and db>0) or (da>0 and db<0):
            t = da/(da-db)
            p = [a[j]+t*(b[j]-a[j]) for j in range(3)]
            inside.append(p)
            outside.append(p)
    return inside,outside


def subtract(poly, lo, hi):
    if any(max(p[a] for p in poly)<lo[a] or min(p[a] for p in poly)>hi[a] for a in range(3)):
        return [poly]
    outside = []
    for axis in range(3):
        for bound,sign in [(lo[axis],-1),(hi[axis],1)]:
            if len(poly)<3:
                return outside
            poly,part = split(poly,axis,bound,sign)
            if len(part)>=3:
                outside.append(part)
    return outside


out = []
horizontal = 0
changed = 0
for i in range(0,len(data['triangles']),3):
    tri = [data['vertices'][j] for j in data['triangles'][i:i+3]]
    a = [tri[1][j]-tri[0][j] for j in range(3)]
    b = [tri[2][j]-tri[0][j] for j in range(3)]
    n = [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
    length = math.sqrt(sum(x*x for x in n))
    polys = [tri]
    if length and abs(n[2])/length>0.5:
        horizontal += 1
    else:
        for number,lo,hi in boxes:
            polys = [p for poly in polys for p in subtract(poly,lo,hi)]
        if polys != [tri]:
            changed += 1
    for poly in polys:
        for j in range(1,len(poly)-1):
            out.append([poly[0],poly[j],poly[j+1]])
assert changed>0
vertices = [p for tri in out for p in tri]
result = dict(vertices=vertices,triangles=list(range(len(vertices))),horizontal_triangles_preserved=horizontal,
              changed_original_triangles=changed,original_triangles=len(data['triangles'])//3,boxes=boxes)
(ROOT/'door_shell_cut.json').write_text(json.dumps(result))
print({k:v for k,v in result.items() if k not in ['vertices','triangles','boxes']}, 'output triangles',len(out))
