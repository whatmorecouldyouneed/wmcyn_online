"""Run with Blender --background --factory-startup --python; no asset writes."""
import json
from pathlib import Path
import bpy
import numpy as np

root = Path(__file__).resolve().parents[2] / "Saved/Diagnostics/Lighting"
bpy.ops.import_scene.fbx(filepath=str(root / "UnityReference/Assets/Hidden Heights SGB/fbx/Hidden_Heights.fbx"))
audit = json.loads((root / "door_audit.json").read_text())
results = []
def signature(points):
    points = np.asarray(points, dtype=float)
    points = np.unique(np.round(points, 5), axis=0)
    dist = np.linalg.norm(points[:, None, :] - points[None, :, :], axis=2)
    dist = np.sort(dist[np.triu_indices(len(points), 1)])
    return points, dist / dist[-1]
for row in audit:
    name = row["component"].replace("poly_", "poly.")
    obj = bpy.data.objects[name]
    original, a = signature([v.co[:] for v in obj.data.vertices])
    imported, b = signature([v for section in row["sections"] for v in section["vertices"]])
    match = len(a) == len(b) and float(np.max(np.abs(a - b))) < 0.0001
    results.append({"component": row["component"], "source_unique_vertices": len(original), "unreal_unique_vertices": len(imported), "shape_matches_up_to_rigid_transform_and_scale": bool(match), "maximum_normalized_distance_error": float(np.max(np.abs(a - b))) if len(a) == len(b) else None})
(root / "door_source_comparison.json").write_text(json.dumps(results, indent=2))
print(json.dumps(results, indent=2))
