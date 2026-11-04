"""Slice the PC/desk setup out of the Sai's Room VRChat FBX.

    blender -b --factory-startup --python Tools/Models/extract_sai_desk.py

The source is not a desk asset, it is a whole room: 368 mesh objects and 31
materials covering bed, closet, walls, door, curtains and heater as well as the
desk. Object names are mostly generic Blender primitives (立方体.047, 円柱.019,
NURBSパス.012), so there is nothing to select by name. What does identify the
desk is the material an object uses and where it sits in the room, and that is
what this classifies on.

Writes two FBX plus a material map into Intermediate/WMCYN/SaiDesk/. Nothing in
the source folder is modified; the material renames below happen in memory and
are never saved back to the .blend or the original FBX.
"""
import json
import math
import sys
from pathlib import Path

import bmesh
import bpy

sys.stdout.reconfigure(encoding='utf-8')

SOURCE = Path("C:/Users/jvred/WMCYN v2.0/Assets/Sai/Sai's room v2.0.fbx")
OUT = Path(__file__).resolve().parents[2] / 'Intermediate' / 'WMCYN' / 'SaiDesk'

# Japanese material names carrying commas are unusable as Unreal asset names,
# so they are renamed to slugs before export and the mapping is written out for
# the Unreal-side importer to resolve textures with.
SLUGS = {
    '机,プリンタ,時計,充電台,机下の棚,スピーカ': 'Sai_Desk',
    'モニタ5枚': 'Sai_Monitors',
    'キーボード_ao': 'Sai_Keyboard',
    'pc,usb,サブスマホ,touchcontroller,サブhmd,マウス': 'Sai_PC',
    'UV電源回り,ルータ,サブpc,本2冊,watch,iphone,アル': 'Sai_Clutter',
    '電源周り、本、ホームスピーカなど': 'Sai_Clutter2',
    'Chair': 'Sai_Chair',
    '黒ケーブル': 'Sai_CableBlack',
    '薄灰色コード': 'Sai_CableGrey',
    'コントローラー': 'Sai_Controller',
    'mic,hmd,tracker,usbhub,bs,cablefixed,battery': 'Sai_Gear',
    'mic,hmd,tracker,usbhub,bs,cablefixed,battery.001': 'Sai_Gear2',
    'huti': 'Sai_ScreenHuti',
    'kurobuti': 'Sai_ScreenKurobuti',
    'movie': 'Sai_ScreenMovie',
    'ura': 'Sai_ScreenUra',
    'Botan': 'Sai_ScreenBotan',
    'いろいろ': 'Sai_Misc',
    'Material.002': 'Sai_Misc2',
}

# Drop these by name, whatever else the rules decide. This is the hook for
# removing individual props by hand: add a name, re-run the script, re-import.
# Names come from sai_desk_contents.md, which this script writes next to the
# FBX every run.
DENY_OBJECTS = set()

# Room shell and furniture that is not part of the desk, dropped outright.
DENY = {
    '床', '壁_ao', '枠', 'ドア', 'ドア.001', 'ベッド', 'エアコン',
    'カーテン,エアコン,チェスト,ゴミ箱,照明',
    'クローゼット,ストーブ,扇風機,タンス',
    'クローゼット,ストーブ,扇風機,タンス.001',
    'ワットチェッカ,モニタ用電源タップ,,ルータ',
}
CHAIR = {'Chair'}
# HMD stands, trackers, base stations, the controller: mounted around the room
# rather than on the desk, and 63k triangles of it, which was 39% of the first
# extraction. Off by default. Setting INCLUDE_GEAR keeps only the pieces that
# sit low and inside the desk box, which is roughly the USB hub and the HMD on
# its stand rather than the wall-mounted base stations.
GEAR = {'mic,hmd,tracker,usbhub,bs,cablefixed,battery',
        'mic,hmd,tracker,usbhub,bs,cablefixed,battery.001',
        'コントローラー'}
INCLUDE_GEAR = False
GEAR_MAX_Z = 1.10

# Cables run the length of the room. A centre-inside test keeps any run that
# happens to be balanced over the desk, and those trail metres out of the set
# and up to the ceiling. These have to be wholly inside the box to come across,
# which keeps the short runs around the desk and drops the room-spanning ones.
CABLES = {'黒ケーブル', '薄灰色コード'}

# Outlets, switches, faceplates, power strips and the HMD wall mount are fixed
# to the room's walls. The walls are dropped, so these were left hanging in mid
# air once the set was placed in the Crib. Anything thin and flush against a
# wall plane goes with the wall. The desk touches the same back wall but is far
# too deep to trip the thickness test.
WALL_FLUSH = 0.20          # metres of depth; thicker than this is furniture
SMALL_PROP = 0.60          # metres; at or below this, require full containment
WALL_PLANES = (('hi', 1, 2.62),     # back wall, +Y
               ('lo', 0, -0.98))    # side wall, -X

# Desk cluster, in metres, in the room's own coordinates. This is also what
# trims the cable runs: they span the whole room, and only the lengths whose
# centre falls in here come across.
BOX_MIN = (-1.05, 0.70, 0.00)
BOX_MAX = (1.05, 2.78, 1.75)


def world_bounds(obj):
    pts = [obj.matrix_world @ v.co for v in obj.data.vertices]
    if not pts:
        return None
    lo = [min(p[i] for p in pts) for i in range(3)]
    hi = [max(p[i] for p in pts) for i in range(3)]
    return lo, hi


def inside(centre, lo=BOX_MIN, hi=BOX_MAX):
    return all(lo[i] <= centre[i] <= hi[i] for i in range(3))


def triangles(obj):
    return sum(len(p.vertices) - 2 for p in obj.data.polygons)


def wall_mounted(lo, hi):
    """Thin and flush against a room wall, so it falls with the wall."""
    for side, axis, plane in WALL_PLANES:
        edge = hi[axis] if side == 'hi' else lo[axis]
        beyond = edge >= plane if side == 'hi' else edge <= plane
        if beyond and (hi[axis] - lo[axis]) <= WALL_FLUSH:
            return True
    return False


def classify(obj):
    """Return 'desk', 'chair' or None."""
    if obj.name in DENY_OBJECTS:
        return None
    mats = [s.material.name for s in obj.material_slots if s.material]
    bounds = world_bounds(obj)
    if bounds is None:
        return None
    lo, hi = bounds
    centre = [(lo[i] + hi[i]) / 2 for i in range(3)]

    if any(m in CHAIR for m in mats):
        return 'chair'
    if mats and all(m in DENY for m in mats):
        return None
    if wall_mounted(lo, hi):
        return None
    if any(m in GEAR for m in mats):
        if not INCLUDE_GEAR:
            return None
        return 'desk' if inside(centre) and centre[2] <= GEAR_MAX_Z else None
    if any(m in CABLES for m in mats):
        return 'desk' if inside(lo) and inside(hi) else None
    # Small props have to be wholly inside the region. A centre test lets a
    # rail or bracket sit half outside, and whatever it was fixed to in the
    # room is gone, so it ends up hanging in space. Large pieces are exempt:
    # the desk itself legitimately spans the whole area.
    if max(hi[i] - lo[i] for i in range(3)) <= SMALL_PROP:
        return 'desk' if inside(lo) and inside(hi) else None
    return 'desk' if inside(centre) else None


def fix_normals(objs):
    """Make every face point out of its solid.

    Large parts of this asset have inconsistent winding - the PC case alone had
    1,445 of 3,115 faces pointing inward. Unity and Blender's solid view do not
    cull backfaces by default, so it looked fine there and nobody ever noticed.
    Unreal does cull, so those faces vanish and you see straight into the inside
    of the model: the PC rendered as an open shell with its fan and motherboard
    on show. Recalculating outward is the fix; the alternative, two-sided
    materials, costs shader time on Quest and only hides the problem.

    Returns the objects that actually had faces flipped.
    """
    touched = []
    for obj in objs:
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        before = [f.normal.copy() for f in bm.faces]
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        bm.normal_update()
        flipped = sum(1 for old, f in zip(before, bm.faces) if old.dot(f.normal) < 0)
        if flipped:
            bm.to_mesh(obj.data)
            obj.data.update()
            touched.append((obj.name, flipped, len(bm.faces)))
        bm.free()
    return touched


def unmirror(objs):
    """Cancel the winding flip that mirrored objects pick up on export.

    Six objects here, the PC among them, carry a negative-determinant
    transform. Their faces are wound correctly in local space, but the mirroring
    reverses them in world space. Blender's solid view and Unity both ignore
    that because neither culls backfaces by default. Unreal does, so the PC's
    outer shell was culled away and you looked straight into its fan and
    motherboard.

    Reversing the faces here cancels the flip. The transform is left alone:
    baking it out would be the tidier fix but would disturb the parenting this
    hierarchy relies on.
    """
    done = []
    for obj in objs:
        if obj.matrix_world.determinant() >= 0:
            continue
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        bmesh.ops.reverse_faces(bm, faces=bm.faces)
        bm.to_mesh(obj.data)
        obj.data.update()
        bm.free()
        done.append(obj.name)
    return done


def settle_or_drop(objs, tol=0.012):
    """Lower anything hanging in space onto the surface under it.

    A gap-under test does not work here: plenty of parts legitimately have
    nothing beneath them, because they are cantilevered - a USB stick in the
    PC's side, the monitor-arm segments, a dongle hanging on a cable. They are
    fine because they touch something that touches something that reaches the
    floor. So the test is connectivity: build a touch graph over the bounding
    boxes, take connected components, and treat any component that never
    reaches floor level as adrift.

    Adrift is not the same as unwanted. A phone hanging over the desk wants
    putting down, not deleting - the first version of this deleted one of a
    pair of phones and left its twin, which was worse than the bug. So each
    adrift component is lowered onto the highest grounded surface beneath its
    footprint, and only dropped when there is nothing below it at all.

    Returns (kept, dropped, settled).
    """
    boxes = [(o, *world_bounds(o)) for o in objs if world_bounds(o)]

    def touching(a, b):
        return all(min(a[2][i], b[2][i]) + tol >= max(a[1][i], b[1][i])
                   for i in range(3))

    n = len(boxes)
    adjacent = {i: set() for i in range(n)}
    for i in range(n):
        for j in range(i + 1, n):
            if touching(boxes[i], boxes[j]):
                adjacent[i].add(j)
                adjacent[j].add(i)

    seen, components = set(), []
    for start in range(n):
        if start in seen:
            continue
        stack, comp = [start], []
        seen.add(start)
        while stack:
            k = stack.pop()
            comp.append(k)
            for m in adjacent[k]:
                if m not in seen:
                    seen.add(m)
                    stack.append(m)
        components.append(comp)

    grounded_ids = set()
    adrift = []
    for comp in components:
        if any(boxes[i][1][2] <= 0.03 for i in comp):
            grounded_ids.update(comp)
        else:
            adrift.append(comp)

    kept = [boxes[i][0] for i in sorted(grounded_ids)]
    dropped, settled = [], []
    for comp in adrift:
        lo = [min(boxes[i][1][k] for i in comp) for k in range(3)]
        hi = [max(boxes[i][2][k] for i in comp) for k in range(3)]
        under = [boxes[i][2][2] for i in grounded_ids
                 if min(boxes[i][2][0], hi[0]) > max(boxes[i][1][0], lo[0])
                 and min(boxes[i][2][1], hi[1]) > max(boxes[i][1][1], lo[1])
                 and boxes[i][2][2] <= lo[2] + 0.003]
        if not under:
            dropped.extend(boxes[i][0] for i in comp)
            continue
        drop_by = lo[2] - max(under) - 0.002      # 2 mm of overlap so it seats
        members = {boxes[i][0] for i in comp}
        for obj in members:
            if obj.parent in members:
                continue                          # a parent carries its children
            m = obj.matrix_world.copy()
            m.translation = (m.translation[0], m.translation[1],
                             m.translation[2] - drop_by)
            obj.matrix_world = m
        settled.append(([boxes[i][0].name for i in comp], drop_by * 100))
        kept.extend(members)
    return kept, dropped, settled


def report(label, objs):
    if not objs:
        print('WMCYN_SAI %s EMPTY' % label)
        return
    lo = [math.inf] * 3
    hi = [-math.inf] * 3
    tris = 0
    bymat = {}
    for o in objs:
        b = world_bounds(o)
        if not b:
            continue
        t = triangles(o)
        tris += t
        for s in (o.material_slots or []):
            if s.material:
                e = bymat.setdefault(s.material.name, [0, 0])
                e[0] += 1
                e[1] += t
        for i in range(3):
            lo[i] = min(lo[i], b[0][i])
            hi[i] = max(hi[i], b[1][i])
    for name in sorted(bymat, key=lambda k: -bymat[k][1]):
        c, t = bymat[name]
        print('WMCYN_SAI   %-6s %-48s objs=%-4d tris=%d' % (label, name[:48], c, t))
    print('WMCYN_SAI %-6s objs=%-4d tris=%-7d size=(%.2f,%.2f,%.2f) min=(%.2f,%.2f,%.2f)'
          % (label, len(objs), tris, hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2],
             lo[0], lo[1], lo[2]))
    return lo, hi


def export(objs, path):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.export_scene.fbx(
        filepath=str(path),
        use_selection=True,
        apply_unit_scale=True,
        global_scale=1.0,
        apply_scale_options='FBX_SCALE_NONE',
        axis_forward='-Z',
        axis_up='Y',
        use_mesh_modifiers=True,
        mesh_smooth_type='FACE',
        bake_space_transform=False,
        add_leaf_bones=False,
        path_mode='STRIP',
        object_types={'MESH'},
    )


def write_inventory(groups):
    """List what came across, so individual props can be found and named.

    The merged mesh gives no per-object handle in the editor, so this is how
    you work out what something is called before adding it to DENY_OBJECTS.
    Positions are centimetres from the mesh pivot, which sits centred on the
    desk at floor level: +X toward the room, +Y toward the back wall, +Z up.
    """
    lines = ['# Sai desk contents', '',
             'Add a name to `DENY_OBJECTS` in `Tools/Models/extract_sai_desk.py`',
             'to remove that object, then re-run the script and re-import.', '',
             '| object | group | material | size cm | centre cm (x,y,z) | tris |',
             '|---|---|---|---|---|---|']
    rows = []
    for group, objs in groups.items():
        for obj in objs:
            bounds = world_bounds(obj)
            if not bounds:
                continue
            lo, hi = bounds
            size = [(hi[i] - lo[i]) * 100 for i in range(3)]
            centre = [(lo[i] + hi[i]) / 2 * 100 for i in range(3)]
            mats = [s.material.name for s in obj.material_slots if s.material]
            rows.append((-max(size), obj.name, group, (mats or ['-'])[0],
                         size, centre, triangles(obj)))
    rows.sort()
    for _, name, group, mat, size, centre, tris in rows:
        lines.append('| `%s` | %s | %s | %.0fx%.0fx%.0f | %.0f, %.0f, %.0f | %d |'
                     % (name, group, mat, size[0], size[1], size[2],
                        centre[0], centre[1], centre[2], tris))
    path = OUT / 'sai_desk_contents.md'
    path.write_text(chr(10).join(lines) + chr(10), encoding='utf-8')
    print('WMCYN_SAI inventory=%s rows=%d' % (path.name, len(rows)))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(SOURCE))

    groups = {'desk': [], 'chair': []}
    dropped = 0
    for obj in [o for o in bpy.data.objects if o.type == 'MESH']:
        where = classify(obj)
        if where is None:
            dropped += 1
        else:
            groups[where].append(obj)
    groups['desk'], adrift, settled = settle_or_drop(groups['desk'])
    dropped += len(adrift)
    for names, cm in settled:
        print('WMCYN_SAI settled %-34s by %.1fcm' % (','.join(names)[:34], cm))
    if adrift:
        print('WMCYN_SAI floating_dropped=%s' % [o.name for o in adrift][:12])
    print('WMCYN_SAI dropped=%d' % dropped)
    desk_bounds = report('desk', groups['desk'])
    report('chair', groups['chair'])
    if not groups['desk'] or not groups['chair']:
        raise SystemExit('one of the groups came out empty; check the box and deny list')

    # Both groups share one origin, taken from the desk: centred in XY and
    # sitting on the floor. The chair therefore keeps its true position
    # relative to the desk while still being its own actor, so dropping both at
    # the same transform reassembles the setup.
    lo, hi = desk_bounds
    offset = (-(lo[0] + hi[0]) / 2, -(lo[1] + hi[1]) / 2, -lo[2])
    moving = set(groups['desk']) | set(groups['chair'])
    for obj in moving:
        if obj.parent in moving:
            continue            # a parent carries its children with it
        m = obj.matrix_world.copy()
        m.translation = (m.translation[0] + offset[0],
                         m.translation[1] + offset[1],
                         m.translation[2] + offset[2])
        obj.matrix_world = m
    print('WMCYN_SAI offset=(%.3f,%.3f,%.3f)' % offset)

    every = groups['desk'] + groups['chair']
    flipped = fix_normals(every)
    print('WMCYN_SAI normals_fixed=%d objects' % len(flipped))
    for name, n, total in sorted(flipped, key=lambda r: -r[1])[:8]:
        print('WMCYN_SAI   flipped %-22s %d/%d faces' % (name[:22], n, total))
    mir = unmirror(every)
    if mir:
        print('WMCYN_SAI unmirrored=%s' % mir[:10])

    used = {}
    for objs in groups.values():
        for obj in objs:
            for slot in obj.material_slots:
                if slot.material:
                    used[slot.material.name] = slot.material
    mapping = {}
    for name, mat in sorted(used.items()):
        slug = SLUGS.get(name)
        if not slug:
            slug = 'Sai_' + ''.join(c if c.isalnum() else '_' for c in name)[:28]
        mapping[slug] = name
        mat.name = slug
    (OUT / 'sai_material_map.json').write_text(
        json.dumps(mapping, ensure_ascii=False, indent=2), encoding='utf-8')
    print('WMCYN_SAI materials=%d -> %s' % (len(mapping), ', '.join(sorted(mapping))))

    write_inventory(groups)
    export(groups['desk'], OUT / 'SM_Crib_SaiDesk.fbx')
    export(groups['chair'], OUT / 'SM_Crib_SaiChair.fbx')
    print('WMCYN_SAI wrote %s' % OUT)


main()
