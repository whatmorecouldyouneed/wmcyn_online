"""Compare the garments' reference poses against the body's.

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=Tools/AvatarPipeline/check_jared_bind_pose.py

`prepare_jared_clothing.py` flagged that the garments' bounds put their long
axis on Y (the outfit spans 165cm on Y) while the body's is on Z. Bounds alone
cannot settle it: they can be stale from an earlier import. The reference
skeleton can, because that is what the skinning actually uses.

A follower bound with SetLeaderPoseComponent takes the *leader's* bone
transforms but skins with its *own* inverse-bind matrices:

    v_skinned = v_mesh * inv(C_ref) * C_leader

So the question is NOT whether the garment's reference pose matches the body's.
It is whether the garment's reference pose is consistent with the garment's own
vertex data, and whether the difference from the body is a *pure global
rotation*. If it is, the rotation appears in both v_mesh and inv(C_ref) and
cancels, and the garment is worn correctly:

    v_Yup * inv(C_body * R) * C_leader == v_Zup * inv(C_body) * C_leader

A difference that is NOT a single global rotation - per-bone discrepancies, a
different hierarchy, missing bones - does not cancel, and that is the failure
worth catching. This script fits a global rotation across all shared bones and
reports the residual.

Reads only - assigns nothing, saves nothing.
"""
import itertools
import json
from pathlib import Path

import unreal


def sign_of_permutation(perm):
    """+1 for an even permutation, -1 for an odd one."""
    sign = 1
    for i in range(len(perm)):
        for j in range(i + 1, len(perm)):
            if perm[i] > perm[j]:
                sign = -sign
    return sign

PREVIEW = '/Game/WMCYN/AvatarPipeline/Runtime/Jared/Preview'

# Spine and extremities: enough to see both the up axis and the facing.
BONES = ['root', 'pelvis', 'spine_01', 'spine_05', 'neck_01', 'head',
         'clavicle_l', 'hand_l', 'thigh_l', 'foot_l', 'ball_l']

MESHES = [
    ('body', f'{PREVIEW}/SKM_Jared_Body_Preview'),
    ('outfit', f'{PREVIEW}/SKM_Jared_Outfit_Preview'),
    ('shoes', f'{PREVIEW}/SKM_Jared_Shoes_Preview'),
]


def main():
    bridge = getattr(unreal, 'WMCYNRetargetEditorLibrary', None)
    report = {'poses': {}, 'verdict': {}}
    if bridge is None:
        report['error'] = 'bridge_unavailable'
    else:
        for label, path in MESHES:
            mesh = unreal.load_asset(path)
            if mesh is None:
                report['poses'][label] = {'error': 'missing', 'path': path}
                continue
            report['poses'][label] = json.loads(
                bridge.describe_reference_pose(mesh, BONES))

        # The body is the reference. For every bone both meshes have, compare
        # component-space translation; a consistent axis swap is the signature
        # of an unconverted up axis, and a near-zero delta means the bind is
        # already shared and the bounds warning was a false alarm.
        body = report['poses'].get('body', {}).get('refpose', {})
        for label in ('outfit', 'shoes'):
            other = report['poses'].get(label, {}).get('refpose', {})
            deltas, shared = {}, []
            for bone, value in body.items():
                mine = other.get(bone)
                if not mine or mine.get('missing') or value.get('missing'):
                    continue
                shared.append(bone)
                deltas[bone] = [round(m - b, 2)
                                for m, b in zip(mine['loc'], value['loc'])]
            worst = max((max(abs(v) for v in d) for d in deltas.values()), default=None)

            # Try to explain the whole difference as one global rotation. The 24
            # signed axis permutations with determinant +1 cover every 90-degree
            # rotation, which is what an unconverted up axis produces. A fit
            # here means the difference cancels under leader-pose skinning.
            fit, residual = None, None
            for perm in itertools.permutations(range(3)):
                for signs in itertools.product((1, -1), repeat=3):
                    if sign_of_permutation(perm) * signs[0] * signs[1] * signs[2] != 1:
                        continue  # a reflection, not a rotation
                    worst_here = 0.0
                    for bone in shared:
                        src = body[bone]['loc']
                        dst = other[bone]['loc']
                        for axis in range(3):
                            worst_here = max(
                                worst_here, abs(signs[axis] * src[perm[axis]] - dst[axis]))
                    if residual is None or worst_here < residual:
                        fit, residual = (perm, signs), worst_here

            explained = residual is not None and residual < 1.0
            report['verdict'][label] = {
                'shared_bones': shared,
                'missing_from_garment': sorted(
                    b for b in body if b not in shared and not body[b].get('missing')),
                'max_component_delta_cm': worst,
                'global_rotation_fit': {
                    'axis_map': [['X', 'Y', 'Z'][a] for a in fit[0]] if fit else None,
                    'signs': list(fit[1]) if fit else None,
                    'residual_cm': round(residual, 3) if residual is not None else None,
                },
                'deltas': deltas,
                'reading': (
                    'no bones in common - cannot compare' if not shared
                    else 'bind pose is identical to the body'
                    if worst is not None and worst < 1.0
                    else 'bind pose differs by ONE GLOBAL ROTATION only - this '
                         'cancels under leader-pose skinning, garment is fine'
                    if explained
                    else 'bind pose differs in a way a global rotation does NOT '
                         'explain - leader-pose will deform the garment'),
            }

    out = Path(unreal.Paths.project_saved_dir(), 'Diagnostics/Avatars')
    out.mkdir(parents=True, exist_ok=True)
    (out / 'jared_bind_pose_check.json').write_text(json.dumps(report, indent=2))
    print('WMCYN_BIND_POSE ' + json.dumps(report, indent=2))


main()
