"""Resolve original prefab material assignments and extract exact texture sources.

Run outside Unreal; requires PyYAML. Raw package content stays under Saved.
"""
import json
from pathlib import Path
import re
import tarfile
import yaml

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Saved/Diagnostics/Lighting"
audit = json.loads((OUT / "unity_source_audit.json").read_text())
assets = {a["guid"]: a for a in audit["assets"]}
source_meshes = {a["name"]: a for a in json.loads((OUT / "source_fbx_meshes.json").read_text())}
prefab = OUT / "UnityReference/Assets/Hidden Heights SGB/prefabs/Hidden Heights World.prefab"
documents = {}
parts = re.split(r"(?m)^--- !u!\d+ &(-?\d+).*\n", prefab.read_text(encoding="utf-8-sig"))
for i in range(1, len(parts), 2):
    documents[int(parts[i])] = yaml.safe_load(parts[i + 1])
bindings = {}
for doc in documents.values():
    renderer = doc.get("MeshRenderer")
    if not renderer:
        continue
    obj = documents.get(renderer["m_GameObject"]["fileID"], {}).get("GameObject", {})
    name = obj.get("m_Name")
    mesh = source_meshes.get(name)
    if not mesh:
        continue
    mats = renderer.get("m_Materials", [])
    if len(mats) != len(mesh["materials"]):
        continue
    for original, ref in zip(mesh["materials"], mats):
        if original and ref.get("guid") in assets:
            bindings.setdefault(original, set()).add(assets[ref["guid"]]["path"])
print(json.dumps({k: sorted(v) for k, v in bindings.items()}, indent=2))
(OUT / "unity_material_bindings.json").write_text(json.dumps({k: sorted(v) for k, v in bindings.items()}, indent=2))

# Explicit reviewed mappings, not fuzzy matching or Blender/Unity slot-order guesses.
# Single-slot Plane.005 confirms wood floor 1; Cube.071 confirms wood floor 2.
choices = {
    "house_atlas_1_": "textures/house atlas 1/house atlas.mat",
    "house_atlas_1__001": "textures/house atlas 1/house atlas.mat",
    "house_atlas_2": "textures/house atlas 2/hosue atlas 2.mat",
    "stone_wall": "textures/stone wall/stone wall.mat",
    "wood_floor": "textures/wood floor/wood floor 1.mat",
    "wood_floor_001": "textures/wood floor/wood floor 2.mat",
    "pool": "textures/pool/pool.mat",
    "pool_tiles": "textures/pool/pool tiles/pool tiles.mat",
    "pool_details": "textures/pool/details/pool details.mat",
    "lights": "textures/lights/lights.mat",
    "roof": "textures/roof/roof.mat",
}
by_path = {a["path"]: a for a in audit["assets"]}
manifest = {"materials": [], "water_normals": []}
extract = {}
def texture(path):
    asset = by_path[path]
    extract[asset["guid"] + "/asset"] = path
    return str(OUT / "UnityReference" / path)

for target, relative in choices.items():
    source = by_path["Assets/Hidden Heights SGB/" + relative]
    text = re.sub(r"(?m)^%.*\n|!u!\d+", "", source["source"])
    mat = yaml.safe_load(text)["Material"]["m_SavedProperties"]
    tex = {k: v for record in mat["m_TexEnvs"] for k, v in record.items()}
    floats = {k: v for record in mat["m_Floats"] for k, v in record.items()}
    item = {"target": target, "source": source["path"], "maps": {}, "roughness": 1 - floats.get("_Glossiness", 0.5)}
    for prop, key in (("BASE_COLOR", "_MainTex"), ("NORMAL", "_BumpMap"), ("METALLIC", "_MetallicGlossMap"), ("EMISSIVE_COLOR", "_EmissionMap")):
        env = tex.get(key, {})
        guid = env.get("m_Texture", {}).get("guid")
        if guid in assets and assets[guid]["path"].lower().endswith((".png", ".jpg", ".tif")):
            path = assets[guid]["path"]
            # A property label alone cannot prove the image's channel semantics.
            if prop == "METALLIC" and "metallic" not in path.lower():
                continue
            item["maps"][prop] = texture(path)
            if prop == "BASE_COLOR":
                item["uv_scale"] = env.get("m_Scale", {"x": 1, "y": 1})
    rough = [p for p in source["resolved_guids"].values() if "roughness" in p.lower()]
    if rough:
        assert len(rough) == 1
        item["maps"]["ROUGHNESS"] = texture(rough[0])
    assert "BASE_COLOR" in item["maps"], target
    manifest["materials"].append(item)
for name in ("Water_N_A.png", "Water_N_B.png"):
    manifest["water_normals"].append(texture("Assets/Hidden Heights SGB/shaders/water/" + name))
with tarfile.open(audit["package"], "r|gz") as archive:
    for member in archive:
        if member.name in extract and member.isfile():
            target = (OUT / "UnityReference" / extract[member.name]).resolve()
            assert target.is_relative_to((OUT / "UnityReference").resolve())
            data = archive.extractfile(member).read()
            if target.exists():
                assert target.read_bytes() == data, "Reference file differs; refusing overwrite"
            else:
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
(OUT / "restoration_manifest.json").write_text(json.dumps(manifest, indent=2))
print("Prepared", len(manifest["materials"]), "material mappings and", len(extract), "unique source maps")
