"""Inventory connected source islands without changing Unreal assets."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2] / 'Saved/Diagnostics/Lighting'


def islands(data):
    vertices, triangles = data['vertices'], data['triangles']
    parents = list(range(len(vertices)))

    def find(i):
        while parents[i] != i:
            parents[i] = parents[parents[i]]
            i = parents[i]
        return i

    def union(a, b):
        parents[find(a)] = find(b)

    positions = {}
    for i, v in enumerate(vertices):
        key = tuple(round(x, 3) for x in v)
        if key in positions:
            union(i, positions[key])
        else:
            positions[key] = i
    for i in range(0, len(triangles), 3):
        union(triangles[i], triangles[i+1])
        union(triangles[i], triangles[i+2])
    groups = {}
    for i in range(0, len(triangles), 3):
        groups.setdefault(find(triangles[i]), []).append(i//3)
    result = []
    for group, ids in groups.items():
        points = [vertices[triangles[t*3+j]] for t in ids for j in range(3)]
        lo = [min(p[a] for p in points) for a in range(3)]
        hi = [max(p[a] for p in points) for a in range(3)]
        result.append(dict(id=group, triangles=ids, min=lo, max=hi, center=[(a+b)/2 for a,b in zip(lo,hi)]))
    return result


if __name__ == '__main__':
    for path in sorted(ROOT.glob('Scene_*_doors.json')):
        sections = json.loads(path.read_text())
        for index, data in enumerate(sections):
            rows = islands(data)
            (ROOT / (path.stem + '_islands_' + str(index) + '.json')).write_text(json.dumps(rows, indent=2))
            print(path.stem, index, len(data['triangles'])//3, data['material'])
            for r in rows:
                print(r['id'], len(r['triangles']), [round(x,1) for x in r['min']], [round(x,1) for x in r['max']])
