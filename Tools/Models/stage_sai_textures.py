"""Resolve the Sai desk materials to textures and stage them for Unreal.

Run after Tools/Models/extract_sai_desk.py:

    python Tools/Models/stage_sai_textures.py

Unity keeps a material's albedo as a GUID in the .mat file, and the filename it
points at only appears in the .meta sidecar next to the texture. So this walks
tex/*.meta to build a GUID -> filename map, reads each .mat's _MainTex through
it, and copies the result to an ASCII name. The source filenames are Japanese
and full of commas, which neither Unreal asset names nor the editor bridge
handle well.

Materials with no _MainTex are flat colours. Unity serialises _Color in gamma
space, so those are converted to linear exactly once here, the same rule the
rug and couch materials follow. Do not convert again when authoring the Unreal
material: its vector parameters are already linear.

Writes tex/T_Sai_*.png and sai_material_table.json into
Intermediate/WMCYN/SaiDesk/, which is the input for the Unreal-side import.
"""
import json
import re
import shutil
import struct
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding='utf-8')

SAI = Path('C:/Users/jvred/WMCYN v2.0/Assets/Sai')
OUT = Path(__file__).resolve().parents[2] / 'Intermediate' / 'WMCYN' / 'SaiDesk'
TEX = OUT / 'tex'
# The project holds decor to 1K and everything else to 2K on Quest; the source
# atlases are 4K.
CAP_DEFAULT = 2048


def srgb_to_linear(c):
    """The sRGB EOTF, not a 2.2 power curve. Applied exactly once."""
    return round(c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4, 6)


def guid_map():
    out = {}
    for meta in (SAI / 'tex').rglob('*.meta'):
        text = meta.read_text(encoding='utf-8', errors='ignore')
        found = re.search(r'guid:\s*([0-9a-f]{32})', text)
        if found:
            out[found.group(1)] = meta.name[:-5]
    return out


def png_width(path):
    head = path.read_bytes()[:32]
    if head[:8] == b'\x89PNG\r\n\x1a\n' and head[12:16] == b'IHDR':
        return struct.unpack('>II', head[16:24])[0]
    return 0


def main():
    TEX.mkdir(parents=True, exist_ok=True)
    mapping = json.loads((OUT / 'sai_material_map.json').read_text(encoding='utf-8'))
    guids = guid_map()
    table = {}

    for slug, original in sorted(mapping.items()):
        mat = SAI / 'Mat' / (original + '.mat')
        if not mat.exists():
            # Botan is an FBX-only material with no Unity counterpart.
            table[slug] = {'kind': 'flat', 'linear': [0.25, 0.25, 0.25],
                           'note': 'no Unity .mat; neutral fallback'}
            continue
        text = mat.read_text(encoding='utf-8', errors='ignore')
        main_tex = re.search(r'_MainTex:.*?guid:\s*([0-9a-f]{32})', text, re.S)
        if main_tex and guids.get(main_tex.group(1)):
            src = SAI / 'tex' / guids[main_tex.group(1)]
            dst = TEX / ('T_Sai_' + slug[4:] + src.suffix.lower())
            shutil.copyfile(src, dst)
            width = png_width(dst)
            table[slug] = {'kind': 'tex', 'file': dst.name, 'src_px': width,
                           'max': min(width, CAP_DEFAULT) or CAP_DEFAULT,
                           'source': src.name}
        else:
            colour = re.search(
                r'_Color:\s*\{r:\s*([\d.]+),\s*g:\s*([\d.]+),\s*b:\s*([\d.]+)', text)
            rgb = [float(v) for v in colour.groups()] if colour else [0.25] * 3
            table[slug] = {'kind': 'flat', 'srgb': rgb,
                           'linear': [srgb_to_linear(c) for c in rgb]}

    (OUT / 'sai_material_table.json').write_text(
        json.dumps(table, ensure_ascii=False, indent=2), encoding='utf-8')
    for slug, entry in sorted(table.items()):
        if entry['kind'] == 'tex':
            print('%-20s TEX  %-26s %4dpx -> %d' %
                  (slug, entry['file'], entry['src_px'], entry['max']))
        else:
            print('%-20s FLAT linear=%s %s' %
                  (slug, entry['linear'], entry.get('note', '')))
    print('WMCYN_SAI_TEXTURES %s' % TEX)


main()
