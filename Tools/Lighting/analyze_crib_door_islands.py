"""Find welded-position islands, including disconnected frame and handle pieces."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[2]/'Saved/Diagnostics/Lighting'
data=json.loads((root/'door_world_geometry.json').read_text())
vertices=data['vertices'];triangles=data['triangles']
parents=list(range(len(vertices)))
def find(i):
    while parents[i]!=i:
        parents[i]=parents[parents[i]];i=parents[i]
    return i
def union(a,b): parents[find(a)]=find(b)
positions={}
for i,v in enumerate(vertices):
    key=tuple(round(x,3) for x in v)
    if key in positions: union(i,positions[key])
    else: positions[key]=i
for i in range(0,len(triangles),3):
    union(triangles[i],triangles[i+1]);union(triangles[i],triangles[i+2])
groups={}
for i in range(0,len(triangles),3):groups.setdefault(find(triangles[i]),[]).append(i//3)
report=[]
for group,ids in groups.items():
    points=[vertices[triangles[t*3+j]] for t in ids for j in range(3)]
    low=[min(p[a] for p in points) for a in range(3)];high=[max(p[a] for p in points) for a in range(3)]
    report.append(dict(id=group,triangles=ids,min=low,max=high,center=[(a+b)/2 for a,b in zip(low,high)]))
(root/'door_islands.json').write_text(json.dumps(report,indent=2))
for r in report:
    if r['max'][2]>450 and r['min'][2]<850:
        print(r['id'],len(r['triangles']),[round(v,1) for v in r['min']],[round(v,1) for v in r['max']])
