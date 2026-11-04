"""Generate the couch upholstery and throw-pillow maps.

An earlier pass left the couch body untextured on the grounds that a weave map
is near-invisible on black fabric and costs a sample per pixel. That trade was
wrong for this job: without it the couch reads as clean CG, and the target is a
photogrammetry scan of a real, worn sofa. So the body gets a full set here.

What makes fabric read as scanned rather than modelled is mostly two things at
different scales: a fine weave, and the soft irregular creases that run across
it. The creases come from ridged value noise with its domain warped, which
gives organic flowing lines instead of anything that looks tiled.

Writes, into Intermediate/WMCYN/Couch:
    T_Crib_CouchFabric_BaseColor   charcoal, weave + creases + wear blotching
    T_Crib_CouchFabric_Normal      weave + creases, OpenGL handed
    T_Crib_CouchPillow_BaseColor   dark ground, light crosshatch threads

No third-party modules; nothing in this environment has numpy or Pillow.
"""
import math
import struct
import zlib
from pathlib import Path

SIZE = 1024
PILLOW_SIZE = 512
OUT = Path(__file__).resolve().parents[2] / 'Intermediate' / 'WMCYN' / 'Couch'

# Near-black charcoal. Scanned black upholstery is never pure black; it sits
# around here once the weave and dust are in it.
FABRIC_BASE = (34, 34, 38)
# The pillow ground must stay close to the couch, only a little lighter and
# bluer. Previously it was mid-grey, which lit up far brighter than the body.
PILLOW_GROUND = (40, 40, 45)
PILLOW_THREAD = (131, 129, 122)


def write_png(path, rows, size):
    raw = bytearray()
    for row in rows:
        raw.append(0)
        raw += row

    def chunk(tag, data):
        body = tag + data
        return struct.pack('>I', len(data)) + body + struct.pack('>I', zlib.crc32(body))

    header = struct.pack('>IIBBBBB', size, size, 8, 2, 0, 0, 0)
    path.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', header)
                     + chunk(b'IDAT', zlib.compress(bytes(raw), 9))
                     + chunk(b'IEND', b''))


def hashed(i, seed):
    h = (i * 374761393 + seed * 668265263) & 0xFFFFFFFF
    h = (h ^ (h >> 13)) * 1274126177 & 0xFFFFFFFF
    return ((h ^ (h >> 16)) & 0xFFFFFF) / 0x1000000


def value_noise(freq, seed, size=SIZE):
    """Tiling value noise, smoothstepped."""
    lattice = [[hashed(y * freq + x, seed) for x in range(freq)] for y in range(freq)]
    step = freq / size
    cell, blend = [], []
    for i in range(size):
        pos = i * step
        c = int(pos)
        t = pos - c
        cell.append(c % freq)
        blend.append(t * t * (3.0 - 2.0 * t))
    out = []
    for y in range(size):
        y0, fy = cell[y], blend[y]
        r0, r1 = lattice[y0], lattice[(y0 + 1) % freq]
        row = [0.0] * size
        for x in range(size):
            x0, fx = cell[x], blend[x]
            x1 = (x0 + 1) % freq
            top = r0[x0] + (r0[x1] - r0[x0]) * fx
            bot = r1[x0] + (r1[x1] - r1[x0]) * fx
            row[x] = top + (bot - top) * fy
        out.append(row)
    return out


def octaves(bands, seed, size=SIZE):
    total = [[0.0] * size for _ in range(size)]
    for index, (freq, weight) in enumerate(bands):
        band = value_noise(freq, seed + index * 97, size)
        for y in range(size):
            trow, brow = total[y], band[y]
            for x in range(size):
                trow[x] += brow[x] * weight
    scale = 1.0 / sum(w for _, w in bands)
    for row in total:
        for x in range(size):
            row[x] *= scale
    return total


def creases(seed):
    """Ridged noise with a warped domain: soft flowing fabric creases."""
    base = octaves([(14, 1.0), (26, 0.5)], seed)
    wx = value_noise(7, seed + 311)
    wy = value_noise(7, seed + 733)
    amp = 58.0
    out = [[0.0] * SIZE for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            sx = int(x + (wx[y][x] - 0.5) * amp) % SIZE
            sy = int(y + (wy[y][x] - 0.5) * amp) % SIZE
            n = base[sy][sx]
            # Ridge transform turns noise bands into crease lines.
            r = 1.0 - abs(2.0 * n - 1.0)
            out[y][x] = r * r
    return out


def plain_weave(threads, size=SIZE):
    """Over-under weave height, one value per pixel."""
    out = [[0.0] * size for _ in range(size)]
    period = size / threads
    for y in range(size):
        fy = (y / period) % 1.0
        j = int(y / period)
        for x in range(size):
            fx = (x / period) % 1.0
            i = int(x / period)
            warp = math.sin(math.pi * fx)
            weft = math.sin(math.pi * fy)
            if (i + j) % 2 == 0:
                out[y][x] = 0.62 * warp + 0.26 * weft
            else:
                out[y][x] = 0.26 * warp + 0.62 * weft
    return out


def fabric():
    weave = plain_weave(58)
    crease = creases(4401)
    wear = octaves([(4, 1.0), (9, 0.5)], 5501)

    height, colour = [], []
    for y in range(SIZE):
        hrow = [0.0] * SIZE
        crow = bytearray()
        for x in range(SIZE):
            w, c, b = weave[y][x], crease[y][x], wear[y][x]
            hrow[x] = 0.30 * w + 0.70 * c
            # Creases sit in shadow, raised weave catches light, and broad
            # blotches stand in for dust and sitting wear.
            # Creases belong in the normal map. Leaving much of them in the
            # albedo made the fabric read as a printed damask, not as black
            # upholstery, so their tonal share stays small here.
            shade = 0.86 + 0.14 * w + 0.08 * (1.0 - c) + 0.12 * (b - 0.5)
            lint = 1.0
            if hashed(y * SIZE + x, 881) > 0.9985:
                lint = 2.4                      # occasional pale fibre
            for ch in range(3):
                value = FABRIC_BASE[ch] * shade * lint
                crow.append(max(0, min(255, int(value + 0.5))))
        height.append(hrow)
        colour.append(crow)
    write_png(OUT / 'T_Crib_CouchFabric_BaseColor.png', colour, SIZE)

    strength = 4.0
    normals = []
    for y in range(SIZE):
        up, down, here = height[(y - 1) % SIZE], height[(y + 1) % SIZE], height[y]
        row = bytearray()
        for x in range(SIZE):
            dx = (here[(x + 1) % SIZE] - here[(x - 1) % SIZE]) * strength
            dy = (down[x] - up[x]) * strength
            nx, ny = -dx, -dy               # -dy keeps V up, the OpenGL handedness
            inv = 1.0 / math.sqrt(nx * nx + ny * ny + 1.0)
            row += bytes((int((nx * inv * 0.5 + 0.5) * 255.0 + 0.5),
                          int((ny * inv * 0.5 + 0.5) * 255.0 + 0.5),
                          int((inv * 0.5 + 0.5) * 255.0 + 0.5)))
        normals.append(row)
    write_png(OUT / 'T_Crib_CouchFabric_Normal.png', normals, SIZE)


def pillow():
    """Charcoal ground with irregular off-white threads, as in the photos."""
    lines = 20
    step = PILLOW_SIZE / lines

    def field(seed):
        cover = [0.0] * PILLOW_SIZE
        for n in range(lines):
            centre = (n + 0.5 + (hashed(n, seed) - 0.5) * 0.55) * step
            width = 1.1 + hashed(n, seed + 31) * 1.8
            weight = 0.45 + hashed(n, seed + 77) * 0.55
            for x in range(int(centre - width) - 1, int(centre + width) + 2):
                d = abs(x - centre)
                if d > width:
                    continue
                v = weight * (1.0 - (d / width) ** 2)
                cover[x % PILLOW_SIZE] = max(cover[x % PILLOW_SIZE], v)
        return cover

    warp, weft = field(1201), field(2203)
    rows = []
    for y in range(PILLOW_SIZE):
        row = bytearray()
        for x in range(PILLOW_SIZE):
            t = max(warp[x], weft[y])
            grain = (hashed(y * PILLOW_SIZE + x, 97) - 0.5) * 0.10
            for ch in range(3):
                value = PILLOW_GROUND[ch] + (PILLOW_THREAD[ch] - PILLOW_GROUND[ch]) * t
                value *= 1.0 + grain
                row.append(max(0, min(255, int(value + 0.5))))
        rows.append(row)
    write_png(OUT / 'T_Crib_CouchPillow_BaseColor.png', rows, PILLOW_SIZE)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    fabric()
    pillow()
    print('WMCYN_COUCH_TEXTURES ' + str(OUT))


main()
