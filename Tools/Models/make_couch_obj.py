"""Build the Abbotsford couch, and its throw pillow, as OBJ files.

Target is a photogrammetry-scan read of the real sofa, not a clean product
render. Three things do that work here:

  * every piece is a superellipsoid, so foam edges round off instead of
    chamfering;
  * neighbouring cushions are cut wider than their share of the width, so they
    press together into a crease rather than leaving a slot between them;
  * every vertex is pushed along its normal by two octaves of value noise, and
    each cushion gets its own seed plus a little rotation and scale jitter, so
    no two are identical and none is mathematically smooth.

UVs are scaled in centimetres rather than per piece, so the weave stays the same
physical size whether it lands on an arm or a seat cushion. Each vertex takes
the projection plane its normal points most strongly at; the switch happens on
the rounded edges where a seam does not show.

The pillow is written as its own mesh, centred on the origin and unrotated, so
it is placed and posed as separate actors.

Envelope stays the Freedom Abbotsford 4.5 Seater (3 Piece): 110 x 324 x 87 cm,
seat top at 45. No third-party modules; nothing here has numpy.
"""
import math
from pathlib import Path

OUT = Path(__file__).resolve().parents[2] / 'Intermediate' / 'WMCYN' / 'Couch'
TILE_CM = 28.0                  # one texture tile covers this much surface


def sgn_pow(t, e):
    if t == 0.0:
        return 0.0
    return math.copysign(abs(t) ** e, t)


def hash3(i, j, k, seed):
    h = (i * 374761393 + j * 668265263 + k * 2246822519 + seed * 3266489917) & 0xFFFFFFFF
    h = (h ^ (h >> 13)) * 1274126177 & 0xFFFFFFFF
    return ((h ^ (h >> 16)) & 0xFFFFFF) / 0x1000000 - 0.5


def noise3(x, y, z, seed):
    """Smooth trilinear value noise, used to rough up otherwise perfect forms."""
    i, j, k = math.floor(x), math.floor(y), math.floor(z)
    fx, fy, fz = x - i, y - j, z - k
    fx = fx * fx * (3.0 - 2.0 * fx)
    fy = fy * fy * (3.0 - 2.0 * fy)
    fz = fz * fz * (3.0 - 2.0 * fz)
    total = 0.0
    for dk in (0, 1):
        wk = fz if dk else 1.0 - fz
        for dj in (0, 1):
            wj = fy if dj else 1.0 - fy
            for di in (0, 1):
                wi = fx if di else 1.0 - fx
                total += hash3(i + di, j + dj, k + dk, seed) * wi * wj * wk
    return total


def cluster(t, power=1.8):
    """Remap [0,1] so samples bunch toward the middle of the span.

    A superellipsoid with a low exponent is nearly a box: almost the whole
    angular range maps onto the flat faces, and the rounded edge is squeezed
    into a few degrees. Sampled uniformly, that edge gets one or two segments
    and reads as a chamfer. Each quadrant's midpoint is where its edge lives,
    so clustering there spends the vertices on the only curved part.
    """
    d = 2.0 * t - 1.0
    return 0.5 + 0.5 * math.copysign(abs(d) ** power, d)


def lon_angle(i, nu):
    """Longitude, with the four vertical edges resolved."""
    t = 4.0 * i / nu
    q = min(int(t), 3)
    return -math.pi + (q + cluster(t - q)) * math.pi / 2.0


def lat_angle(j, nv):
    """Latitude, with the top and bottom edge rings resolved."""
    t = 2.0 * j / nv
    q = min(int(t), 1)
    return -math.pi / 2.0 + (q + cluster(t - q)) * math.pi / 2.0


def rotate(p, pitch_deg, yaw_deg):
    x, y, z = p
    if pitch_deg:
        a = math.radians(pitch_deg)
        ca, sa = math.cos(a), math.sin(a)
        x, z = x * ca + z * sa, -x * sa + z * ca
    if yaw_deg:
        a = math.radians(yaw_deg)
        ca, sa = math.cos(a), math.sin(a)
        x, y = x * ca - y * sa, x * sa + y * ca
    return [x, y, z]


class Mesh:
    def __init__(self):
        self.v = []
        self.vt = []
        self.vn = []
        self.groups = {}

    def piece(self, material, centre, half, e1, e2, nu, nv,
              pitch=0.0, yaw=0.0, seed=0, rough=0.0, sag=0.0, planar_uv=False):
        hx, hy, hz = half
        grid = []
        for j in range(nv + 1):
            lat = lat_angle(j, nv)
            cv, sv = math.cos(lat), math.sin(lat)
            row = []
            for i in range(nu + 1):
                lon = lon_angle(i, nu)
                cu, su = math.cos(lon), math.sin(lon)
                lx = hx * sgn_pow(cv, e1) * sgn_pow(cu, e2)
                ly = hy * sgn_pow(cv, e1) * sgn_pow(su, e2)
                lz = hz * sgn_pow(sv, e1)

                # Outward direction, normalised in half-extent space.
                nx, ny, nz = lx / hx, ly / hy, lz / hz
                length = math.sqrt(nx * nx + ny * ny + nz * nz) or 1.0
                nx, ny, nz = nx / length, ny / length, nz / length

                if sag and lz > 0.0:
                    # Seat cushions dip where people sit.
                    dip = (1.0 - (lx / hx) ** 2) * (1.0 - (ly / hy) ** 2)
                    lz -= sag * max(0.0, dip) * (lz / hz)

                if rough:
                    d = (noise3(lx / 9.0, ly / 9.0, lz / 9.0, seed) * 1.4
                         + noise3(lx / 3.0, ly / 3.0, lz / 3.0, seed + 17) * 0.6) * rough
                    lx += nx * d
                    ly += ny * d
                    lz += nz * d

                if planar_uv:
                    uv = [0.5 + 0.5 * ly / hy, 0.5 + 0.5 * lz / hz]
                else:
                    # Project on whichever plane this vertex faces most squarely,
                    # in centimetres, so weave density is uniform across pieces.
                    ax, ay, az = abs(nx), abs(ny), abs(nz)
                    if az >= ax and az >= ay:
                        uv = [lx / TILE_CM, ly / TILE_CM]
                    elif ay >= ax:
                        uv = [lx / TILE_CM, lz / TILE_CM]
                    else:
                        uv = [ly / TILE_CM, lz / TILE_CM]

                world = rotate([lx, ly, lz], pitch, yaw)
                row.append(len(self.v))
                self.v.append([world[0] + centre[0], world[1] + centre[1],
                               world[2] + centre[2]])
                self.vt.append(uv)
                self.vn.append([0.0, 0.0, 0.0])
            grid.append(row)

        faces = self.groups.setdefault(material, [])
        for j in range(nv):
            for i in range(nu):
                a, b = grid[j][i], grid[j][i + 1]
                c, d = grid[j + 1][i + 1], grid[j + 1][i]
                if j != 0:
                    faces.append((a, b, c))
                if j != nv - 1:
                    faces.append((a, c, d))

    def finish(self):
        for faces in self.groups.values():
            for a, b, c in faces:
                pa, pb, pc = self.v[a], self.v[b], self.v[c]
                u = [pb[k] - pa[k] for k in range(3)]
                w = [pc[k] - pa[k] for k in range(3)]
                n = [u[1] * w[2] - u[2] * w[1],
                     u[2] * w[0] - u[0] * w[2],
                     u[0] * w[1] - u[1] * w[0]]
                for idx in (a, b, c):
                    for k in range(3):
                        self.vn[idx][k] += n[k]
        for n in self.vn:
            length = math.sqrt(n[0] ** 2 + n[1] ** 2 + n[2] ** 2)
            if length > 1e-9:
                n[0] /= length
                n[1] /= length
                n[2] /= length
            else:
                n[2] = 1.0

    def write(self, obj_path, mtl_name):
        lines = ['# WMCYN Crib', 'mtllib ' + mtl_name]
        for x, y, z in self.v:
            lines.append('v %.4f %.4f %.4f' % (x, y, z))
        for u, w in self.vt:
            lines.append('vt %.5f %.5f' % (u, w))
        for x, y, z in self.vn:
            lines.append('vn %.5f %.5f %.5f' % (x, y, z))
        for material, faces in self.groups.items():
            lines.append('usemtl ' + material)
            lines.append('g ' + material)
            for a, b, c in faces:
                lines.append('f %d/%d/%d %d/%d/%d %d/%d/%d'
                             % (a + 1, a + 1, a + 1, b + 1, b + 1, b + 1,
                                c + 1, c + 1, c + 1))
        obj_path.write_text('\n'.join(lines) + '\n')

    def report(self, label):
        lo = [min(p[k] for p in self.v) for k in range(3)]
        hi = [max(p[k] for p in self.v) for k in range(3)]
        tris = sum(len(f) for f in self.groups.values())
        print('WMCYN_%s size=%.1fx%.1fx%.1f range_x=[%.1f,%.1f] tris=%d groups=%s'
              % (label, hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2],
                 lo[0], hi[0], tris, sorted(self.groups)))


def build_couch():
    m = Mesh()
    m.piece('Ply', (0, 0, 3.5), (51, 157, 3.5), 0.07, 0.07, 24, 12,
            seed=11, rough=0.08)
    m.piece('Fabric', (0, 0, 18.5), (55, 162, 11.5), 0.13, 0.13, 40, 20,
            seed=23, rough=0.40)
    for n, side in enumerate((-1, 1)):
        m.piece('Fabric', (0, side * 153, 38), (55, 9, 22), 0.15, 0.15, 40, 20,
                seed=31 + n * 7, rough=0.50, yaw=side * 0.4)
    for n, y in enumerate((-97, 0, 97)):
        m.piece('Fabric', (-44, y, 58.5), (10.5, 51, 28.5), 0.19, 0.19, 40, 20,
                pitch=-4.0 - n * 0.6, yaw=(n - 1) * 0.7,
                seed=51 + n * 13, rough=0.65)
    # Half-width 48 against a 97 cm pitch leaves a 1 cm gap that the rounded
    # edges open into a shallow seam, and each cushion sits a few millimetres
    # off its neighbour. Overlapping them at this low an exponent made all three
    # tops coplanar, so the seat read as one plank with no cushions in it.
    for n, y in enumerate((-97, 0, 97)):
        m.piece('Fabric', (6.5, y, 37.3 + n * 0.35), (46.5, 48, 7.5), 0.17, 0.17,
                40, 20, yaw=(n - 1) * 0.5, pitch=(n - 1) * 0.4,
                seed=71 + n * 13, rough=0.60, sag=1.0 + n * 0.25)
    m.finish()
    m.write(OUT / 'SM_Crib_CouchAbbotsford.obj', 'SM_Crib_CouchAbbotsford.mtl')
    (OUT / 'SM_Crib_CouchAbbotsford.mtl').write_text(
        'newmtl Fabric\nKd 0.09 0.09 0.10\n\nnewmtl Ply\nKd 0.76 0.65 0.49\n')
    m.report('COUCH')


def build_pillow():
    """One throw pillow, centred and upright. Posed per actor in the level."""
    m = Mesh()
    m.piece('Pillow', (0, 0, 0), (8, 24, 24), 0.17, 0.17, 44, 24,
            seed=301, rough=0.40, planar_uv=True)
    m.finish()
    m.write(OUT / 'SM_Crib_CouchPillow.obj', 'SM_Crib_CouchPillow.mtl')
    (OUT / 'SM_Crib_CouchPillow.mtl').write_text('newmtl Pillow\nKd 0.18 0.18 0.20\n')
    m.report('PILLOW')


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    build_couch()
    build_pillow()


main()
