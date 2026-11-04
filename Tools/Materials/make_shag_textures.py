"""Generate the tiling fiber textures for the lime shag rug material.

Writes two 1024 PNGs next to each other and prints the linear-space lime tones
the material's vector parameters expect.

No third-party modules: neither the system Python nor the one Unreal ships has
numpy or Pillow, so the noise, the strand scatter and the PNG container are all
built here. Both maps tile seamlessly; every lattice and every strand wraps.

    R = strand height   high-frequency fibre streaks
    G = clump height    the tufts fibres gather into
    B = colour variation  broad lighter/darker patches of dye

The normal map is written OpenGL-style (V up), matching the project's other
normal maps, so it imports with flip_green_channel enabled like they do.
"""
import math
import struct
import zlib
from pathlib import Path

SIZE = 1024
OUT = Path(__file__).resolve().parents[2] / 'Intermediate' / 'WMCYN' / 'ShagRug'
# Lime tones as 8-bit sRGB, sampled to read as a vivid yellow-green shag:
# bright fibre tips, the body of the pile, and the shadowed base between tufts.
TONES = dict(tip=(178, 222, 58), body=(140, 190, 38), base=(78, 112, 26))


def srgb_to_linear(channel):
    """The sRGB EOTF, not a 2.2 power curve. Applied exactly once."""
    c = channel / 255.0
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def write_png(path, rows):
    """Minimal 8-bit RGB PNG writer. rows is SIZE bytearrays of SIZE*3 bytes."""
    raw = bytearray()
    for row in rows:
        raw.append(0)              # filter type 0, no prediction
        raw += row

    def chunk(tag, data):
        body = tag + data
        return struct.pack('>I', len(data)) + body + struct.pack('>I', zlib.crc32(body))

    header = struct.pack('>IIBBBBB', SIZE, SIZE, 8, 2, 0, 0, 0)
    path.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', header)
                     + chunk(b'IDAT', zlib.compress(bytes(raw), 9))
                     + chunk(b'IEND', b''))


def hash2(x, y, seed):
    """Deterministic lattice hash in [0, 1)."""
    h = (x * 374761393 + y * 668265263 + seed * 2246822519) & 0xFFFFFFFF
    h = (h ^ (h >> 13)) * 1274126177 & 0xFFFFFFFF
    return ((h ^ (h >> 16)) & 0xFFFFFF) / 0x1000000


def value_noise(freq, seed):
    """Tiling value noise at `freq` cells across the image, smoothstepped."""
    lattice = [[hash2(x, y, seed) for x in range(freq)] for y in range(freq)]
    step = freq / SIZE
    # Per-axis cell index and eased blend weight, shared by both axes.
    cell, blend = [], []
    for i in range(SIZE):
        pos = i * step
        c = int(pos)
        t = pos - c
        cell.append(c % freq)
        blend.append(t * t * (3.0 - 2.0 * t))
    out = []
    for y in range(SIZE):
        y0, fy = cell[y], blend[y]
        r0, r1 = lattice[y0], lattice[(y0 + 1) % freq]
        row = [0.0] * SIZE
        for x in range(SIZE):
            x0, fx = cell[x], blend[x]
            x1 = (x0 + 1) % freq
            top = r0[x0] + (r0[x1] - r0[x0]) * fx
            bot = r1[x0] + (r1[x1] - r1[x0]) * fx
            row[x] = top + (bot - top) * fy
        out.append(row)
    return out


def octaves(bands, seed):
    """Sum of value-noise bands as (freq, weight) pairs, normalised to [0, 1]."""
    total = [[0.0] * SIZE for _ in range(SIZE)]
    for index, (freq, weight) in enumerate(bands):
        band = value_noise(freq, seed + index * 97)
        for y in range(SIZE):
            trow, brow = total[y], band[y]
            for x in range(SIZE):
                trow[x] += brow[x] * weight
    scale = 1.0 / sum(weight for _, weight in bands)
    for row in total:
        for x in range(SIZE):
            row[x] *= scale
    return total


def strand_field(flow, count, seed):
    """Scatter fibres as additive streaks, direction steered by the flow field.

    Real shag reads as fibres that lean together in local swirls rather than in
    one combed direction, so each strand takes its heading from low-frequency
    noise and then jitters off it.
    """
    field = [[0.0] * SIZE for _ in range(SIZE)]
    for i in range(count):
        cx = hash2(i, 11, seed) * SIZE
        cy = hash2(i, 23, seed + 5) * SIZE
        jitter = (hash2(i, 31, seed + 9) - 0.5) * 1.5
        angle = flow[int(cy) % SIZE][int(cx) % SIZE] * math.tau + jitter
        # Long fibres: a shag pile is ~4 cm of yarn, so streaks stay long
        # relative to the strand spacing instead of reading as short bristle.
        length = 11.0 + hash2(i, 43, seed + 13) * 19.0
        bright = 0.35 + hash2(i, 57, seed + 17) * 0.65
        dx, dy = math.cos(angle), math.sin(angle)
        steps = int(length * 2.0)
        for s in range(steps):
            t = s / steps
            # Fibres are brightest at the tip and fade into the pile at the root.
            weight = bright * (0.25 + 0.75 * t)
            px = cx + dx * length * t
            py = cy + dy * length * t
            ix, iy = int(px) % SIZE, int(py) % SIZE
            field[iy][ix] += weight
            # A second tap one pixel across keeps strands from aliasing to dots.
            field[iy][(ix + 1) % SIZE] += weight * 0.45
    peak = max(max(row) for row in field) or 1.0
    for row in field:
        for x in range(SIZE):
            row[x] = min(1.0, row[x] / peak * 1.8)
    return field


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    flow = value_noise(6, 4001)
    # Tufts at two scales: the clumps themselves and the gaps dividing them.
    clump = octaves([(8, 1.0), (16, 0.55), (32, 0.3)], 1301)
    variation = octaves([(3, 1.0), (7, 0.45)], 2203)
    strand = strand_field(flow, 90000, 3301)

    packed, height = [], []
    for y in range(SIZE):
        row = bytearray()
        hrow = [0.0] * SIZE
        for x in range(SIZE):
            s, c, v = strand[y][x], clump[y][x], variation[y][x]
            row += bytes((int(s * 255.0 + 0.5), int(c * 255.0 + 0.5), int(v * 255.0 + 0.5)))
            # Surface the material parallaxes and lights against: broad tufts
            # carrying finer fibres.
            hrow[x] = c * 0.55 + s * 0.45
        packed.append(row)
        height.append(hrow)
    write_png(OUT / 'T_Crib_RugShag_Fibers.png', packed)

    strength = 7.0
    normals = []
    for y in range(SIZE):
        up, down = height[(y - 1) % SIZE], height[(y + 1) % SIZE]
        here = height[y]
        row = bytearray()
        for x in range(SIZE):
            dx = (here[(x + 1) % SIZE] - here[(x - 1) % SIZE]) * strength
            dy = (down[x] - up[x]) * strength
            nx, ny, nz = -dx, -dy, 1.0     # -dy: V up, the OpenGL convention
            inv = 1.0 / math.sqrt(nx * nx + ny * ny + 1.0)
            row += bytes((int((nx * inv * 0.5 + 0.5) * 255.0 + 0.5),
                          int((ny * inv * 0.5 + 0.5) * 255.0 + 0.5),
                          int((nz * inv * 0.5 + 0.5) * 255.0 + 0.5)))
        normals.append(row)
    write_png(OUT / 'T_Crib_RugShag_Normal.png', normals)

    print('WMCYN_SHAG_TEXTURES ' + str(OUT))
    for name, rgb in TONES.items():
        linear = [round(srgb_to_linear(c), 6) for c in rgb]
        print('WMCYN_SHAG_TONE %s srgb=%s linear=%s' % (name, list(rgb), linear))


main()
