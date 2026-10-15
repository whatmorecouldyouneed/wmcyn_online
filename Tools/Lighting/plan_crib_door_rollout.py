"""Build reviewed leaf partitions and hinge anchors from source geometry."""
import json
import math
from analyze_rollout_geometry import ROOT, islands

sources = [('Scene_036', json.loads((ROOT/'door_world_geometry.json').read_text()))]
for name in ['Scene_029', 'Scene_031', 'Scene_032', 'Scene_033', 'Scene_045']:
    sources.append((name, json.loads((ROOT/(name+'_doors.json')).read_text())[0]))
plan = []
number = 1
for name, data in sources:
    groups = islands(data)
    bodies = [r for r in groups if len(r['triangles']) == 80]
    if name == 'Scene_036':
        bodies.sort(key=lambda r: [1299,1604,1548,1447,1391].index(r['id']))
    frames = [r for r in groups if len(r['triangles']) in (48,64)]
    assigned = {r['id']: [r] for r in bodies}
    for r in groups:
        if r in bodies or r in frames:
            continue
        body = min(bodies, key=lambda b: sum((a-c)**2 for a,c in zip(b['center'],r['center'])))
        assigned[body['id']].append(r)
    leaves = []
    for body in bodies:
        parts = assigned[body['id']]
        handles = [r for r in parts if r['min'][2] > body['min'][2]+90 and r['max'][2] < body['min'][2]+125]
        assert handles, (name,body['id'])
        handle = [sum(r['center'][a] for r in handles)/len(handles) for a in range(3)]
        axis = max((0,1),key=lambda a: body['max'][a]-body['min'][a])
        pivot = list(body['center'])
        pivot[axis] = body['max'][axis] if handle[axis] < body['center'][axis] else body['min'][axis]
        pivot[2] = body['min'][2]
        swing = 100
        if name == 'Scene_036' and body['id'] == 1299:
            pivot = [-747.5,85,460.6]
            handle = [-724,-2,571]
            swing = -100
        elif name == 'Scene_033':
            # These two leaves are baked open against a shared double frame.
            pivot[0] = -886 if body['id'] == 520 else -1102
            swing = 85 if body['id'] == 520 else -87
        ids = sorted(t for r in parts for t in r['triangles'])
        leaves.append(dict(number=number, body=body['id'], triangles=ids, pivot=pivot,
                           handle=[a-b for a,b in zip(handle,pivot)], swing=swing,
                           source_bounds=[body['min'],body['max']]))
        number += 1
    removed = {t for leaf in leaves for t in leaf['triangles']}
    remainder = sorted(set(range(len(data['triangles'])//3))-removed)
    assert len(removed)+len(remainder)==len(data['triangles'])//3
    assert sum(len(d['triangles']) for d in leaves)==len(removed)
    assert set(remainder)=={t for r in frames for t in r['triangles']}
    plan.append(dict(source=name, data=data, leaves=leaves, remainder=remainder))
    print(name, [(d['number'],len(d['triangles']),[round(x,1) for x in d['pivot']],d['swing']) for d in leaves], 'frame triangles',len(remainder))
(ROOT/'door_rollout_plan.json').write_text(json.dumps(plan,indent=2))
