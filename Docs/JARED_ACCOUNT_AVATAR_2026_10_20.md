# Jared Account Avatar Follow-Up

## Verified Failure

- The October 20 Continue-as-Jared run establishes a login UID successfully.
- Both recorded PCVR manifest responses report `source=default`. This is not a
  remembered-sign-in or nameplate failure.
- Read-only Firestore inspection confirms the expected account owns its active
  avatar job. Both profile and job are `preview_ready`; the job has no body/head
  runtime paths and no PCVR delivery object.
- The WMCYN Jared preview body and head now resolve their respective MetaHuman
  plugin skeletons in the running editor.
- Network clients previously discarded published selections because a
  ticket-bound server resolver was absent.

## Scope and Changes

The approved source is `/Game/MetaHumans/Jared`, presented through WMCYN copies.
Do not reassign the older test account's MetaHuman geometry or DNA.

The server implementation now requests `/v1/vr/runtime/avatar` using its deployment
credential and the UID/platform from a verified join ticket. Responses are
discarded after logout or identity change. Published body/head/animation paths
must match the existing explicit runtime catalog before replication. Client
asset-path submissions remain rejected. Failed requests have a bounded retry.
The dedicated server need not instantiate presentation meshes.

This is source implementation, not yet a deployed or headset-accepted fix.
The working Mimic pawn, calibration, locomotion and AFCore assets are unchanged.

Validation: UnrealHeaderTool and the module-only `WMCYNRuntime` compile passed
on October 20 (`UnrealEditor Win64 Development -Module=WMCYNRuntime -NoLink`).
After the user saved and closed Unreal, the module-only build without `-NoLink`
also passed and linked `UnrealEditor-WMCYNRuntime.dll`. No live Firestore
documents were written. `git diff --check` passed for the native edits.

The first startup check exposed a stale Launcher build ID in the generated plugin
module manifest. Both WMCYNRuntime and WMCYNRuntimeEditor were rebuilt against
`C:/UE58`, then the generated manifest was aligned with that engine's build ID.
The second commandlet startup passed with zero errors (11 existing warnings).
The saved Jared body has 342 matching bones and the head 875, with no parent
mismatches and both skeleton assignments intact. Report:
`Saved/Diagnostics/Avatars/jared_skeleton_check.json`.
This NullRHI check does not validate rendered materials or headset animation.

## Remaining Gates

1. Native compile/link and headless startup validation are complete.
2. Validate Jared's complete visible assembly: skin, eyes, clothing, head alignment
   and animation. Skeleton assignment alone does not prove presentation quality.
3. Confirm all required Jared assets are included in the PCVR cook.
4. Publish a truthful account-owned PCVR delivery. Do not use the existing
   `publish-jared-precooked.mjs --apply` unchanged: it explicitly names a missing
   storage object. A cooked-in delivery contract or real supported delivery
   artifact is required; cloud asset streaming is outside this pass.
5. Verify the server resolver endpoint is deployed, then build/deploy matching
   server and PCVR client code. Keep Quest fallback until separately validated.

## Acceptance

## Implemented Account-Catalog Follow-Up

- Firebase Admin read-only lookup confirms the explicitly approved catalog UID
  belongs to [account email] and is enabled.
- `WMCYNAccountAvatarCatalog` supplies a PCVR-only cooked-in selection when that
  verified account's backend manifest is default. A published backend selection
  still wins. This does not mark the Firestore scan job published or invent a URL.
- Both authenticated local resolution and ticket-bound server resolution use
  the same catalog. No display-name-based selection or arbitrary client paths.
- Four WMCYN eye/eyelash material copies now reference the available baked maps.
- Jared's head uses a WMCYN Copy Pose From Mesh AnimBlueprint attached at the
  body's component origin, not a second head-bone translation. Original AFCore,
  Mimic and source MetaHuman assets are untouched.
- Windows packaging explicitly includes the Jared preview folder; Quest remains
  gated. This rule has not yet been proven by a fresh packaged cook.
- Both native modules compile/link. Head animation commandlet completes with
  zero errors. `WMCYN.Avatar.AccountCatalog` automation passes (unknown account,
  display-name rejection, Quest gating, approved PCVR selection, no fake URL).
- Full garment/groom assembly and rendered/headset validation remain pending.
  The hosted server and distributed clients have NOT been rebuilt/deployed.
  Online Continue-as-Jared cannot be accepted from these local tests alone.

## Hardware Acceptance

Continue as Jared, then look in the mirror. Logs must show established identity,
published manifest, accepted catalog paths, successful mesh loads and visual
application. A second account with no publication must retain its default.
On a remote peer, verify Jared's selection and tracked movement are visible.
Disconnect during resolution and confirm no stale response changes another pawn.
Never log tokens, passwords, signed URLs or full manifest bodies.

Remote visibility, complete materials and packaged acceptance remain unverified.

## Live Pose Repair (2026-10-22)

- Live PIE inspection confirmed Jared body/head are selected, but the hidden
  native `SKM_Manny_Simple` source uses `AlwaysTickPose`. This does not guarantee
  refreshed component-space bones while hidden. The user reports an offset,
  non-standing body after the first left-stick movement.
- WMCYN source now saves the native tick policy, enables
  `AlwaysTickPoseAndRefreshBones` while the replacement is active, and restores
  the saved policy on default fallback or component EndPlay. Explicit tick
  prerequisites order native body -> presentation body -> head.
- Native rebuild/restart and headset acceptance are pending. No retarget,
  clothing, camera, capsule, calibration, AFCore or Mimic asset changes in this fix.
- Run `Tools/AvatarPipeline/capture_jared_live_pose.py` in PIE before movement,
  after left-stick movement, and while seated/standing. It writes only sanitized
  component/bone measurements to `Saved/Diagnostics/Avatars`.
- Compare source and target head/pelvis/feet over movement. If pose alignment
  still fails with fresh source bones, create an isolated Jared retargeter using
  the actual Manny Simple source; do not edit the historical shared retargeter.
  Outfit/shoes remain gated on tracking acceptance; remote validation is separate.

### Headset Result After Rebuild

- Both WMCYN native modules compiled and linked successfully on 2026-10-22.
  Project reopened with the matching C:/UE58 source-built editor.
- Jared confirms the avatar now tracks and locomotion is correct in headset.
  This is local user-reported acceptance, not remote-peer or Quest acceptance.
- Preserve the working pose bridge. No new retargeter or camera offset is needed
  based on this test.
- Later October 22 work added WMCYN outfit/shoe leader-pose components and
  hair/eyebrow/goatee components. Preparation, bind-pose, appearance and groom
  material reports exist under Saved/Diagnostics/Avatars. Do not redo these as
  missing implementation. Complete headset appearance, remote visibility and
  fresh packaged acceptance remain unverified.
- October 23 decision: preserve this work; use the default avatar wherever Jared
  has not passed target-device/remote acceptance. Full scan delivery is deferred.
