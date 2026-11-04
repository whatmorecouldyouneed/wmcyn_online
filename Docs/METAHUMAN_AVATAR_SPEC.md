# MetaHuman Avatar Spec

Last updated: 2026-07-28

## Purpose

This doc captures the WMCYN Online avatar pipeline target for the post-First-Signal lane:

- user scans face on iPhone
- backend accepts and tracks the job
- Unreal-based build step turns the capture into a real MetaHuman-derived avatar
- Quest and PCVR clients log into the persistent world and load the user's active avatar

This is not the current First Signal acceptance gate. First Signal still prioritizes shared-world presence, login, voice, and OBS capture using the proven native Mimic pawn path.

## Product Goal

WMCYN Online should eventually let an internal user:

1. scan their face on iPhone
2. preview the generated result
3. publish it
4. log into the persistent WMCYN world
5. appear in-world using their active avatar on Quest or PCVR

Audience for v1 is internal only: founders and creators, not public self-serve signup.

## Current State

The mobile/backend side is already far ahead of the Unreal side:

- iOS consent -> capture -> upload -> poll -> preview -> publish exists
- avatar deletion exists
- backend avatar job endpoints exist
- pairing-code auth exists
- the actual MetaHuman build step is still stubbed
- `/v1/avatar/manifest` is not implemented yet
- Unreal client avatar fetch/apply is not implemented yet

For Unreal, the user-facing proof path still needs to be built.

## Constraints That Matter

- MetaHuman identity solve and Mesh to MetaHuman are Unreal Editor plugin workflows, not a simple backend API call.
- The automated build step therefore needs a real Unreal project plus editor automation, not just cloud functions.
- Epic's pipeline still depends on Epic-hosted services for parts of autorigging/texture generation.
- Full-fidelity MetaHumans are too heavy for standalone Quest as-is.
- We need different output variants for Quest and PCVR.

## UE-Side Ownership

The Unreal repo owns these decisions and implementations:

1. how the client signs in inside VR
2. how the client asks for the active avatar manifest
3. how the client downloads, caches, mounts, and applies avatar content
4. what runtime representation is used for Quest versus PCVR
5. what exact manifest shape the backend must return

## Sign-In Decision

Primary VR sign-in path should be pairing code, not typed password.

Why:

- password entry in-headset is slow and annoying
- the pairing flow already exists and is verified on the phone
- it better fits the "persistent world" product shape

Email/password remains a fallback path for dev, desktop, and recovery scenarios.

## Runtime Loading Decision

Use a phased approach:

### Phase A: prove the loop with runtime-imported avatar content

Goal:

- verify phone -> backend -> manifest -> download -> apply in headset
- keep implementation cheap enough to move quickly

Recommended form:

- simplified rig-compatible avatar asset
- shared runtime skeleton/animation contract
- WMCYN-owned loader path in `Plugins/WMCYNRuntime`

This may not preserve full MetaHuman fidelity, but it closes the end-to-end product loop sooner.

### Phase B: ship-quality runtime mounting via cooked platform bundles

Goal:

- load actual cooked MetaHuman-compatible content
- preserve higher-fidelity materials, meshes, and platform-specific variants

Recommended form:

- per-avatar cooked pak/iostore bundle
- per-platform output variants
- mount at login or after background download

This is the intended long-term path.

## Manifest Decision

The backend should expose one resolved manifest per requested platform:

`GET /v1/avatar/manifest?platform=quest`

or

`GET /v1/avatar/manifest?platform=pcvr`

Why this shape:

- each client only needs one platform variant
- keeps payload smaller
- makes caching clearer
- lets backend decide the correct active variant for that client

### Response contract

```json
{
  "manifestVersion": 1,
  "avatarId": "avt_123",
  "avatarVersion": 7,
  "platform": "quest",
  "isDefaultAvatar": false,
  "displayName": "Jared",
  "delivery": {
    "type": "runtime_mesh",
    "url": "https://...",
    "sha256": "sha256:...",
    "sizeBytes": 12345678,
    "mountPoint": null,
    "assetRoot": "/Game/WMCYN/Avatars/Runtime/avt_123_v7",
    "skeletalMeshPath": "/Game/WMCYN/Avatars/Runtime/avt_123_v7/SK_Avatar",
    "animClassPath": "/Game/WMCYN/Avatars/Runtime/Common/ABP_WMCYN_Avatar",
    "dnaUrl": null,
    "dnaSha256": null
  },
  "generatedAtUtc": "2026-07-28T00:00:00Z",
  "cacheKey": "avt_123_v7_quest",
  "expiresAtUtc": "2026-07-29T00:00:00Z"
}
```

### Notes on the contract

- `delivery.type` must support both `runtime_mesh` and `pak`
- `avatarVersion` is the content version users care about
- `cacheKey` is the client-side invalidation key
- `dnaUrl` is optional and should only be present when the chosen runtime path actually needs it
- `displayName` is informational only; authoritative world identity still comes from login/PlayerState

## No-Avatar Behavior

Do not return `404` when a user has no published avatar.

Return `200` with a default manifest instead:

- `isDefaultAvatar = true`
- `avatarId = "default"`
- a stable default Quest/PCVR representation

Why:

- the world always needs a visible avatar path
- it avoids special-case client failure states during login
- it supports gradual rollout while generation is still internal-only

## Quest vs PCVR Output

We need two avatar variants per published avatar:

- Quest: reduced LODs, simpler materials, mobile-safe hair/cards, aggressive memory discipline
- PCVR: higher-fidelity output suitable for OBS and closer-range viewing

The manifest endpoint should resolve the right variant by requested platform.

## Clothing Is Catalog-Derived, Not Manifest-Delivered

Decided 2026-10-22, while putting clothes on the first account-bound avatar.

The manifest carries exactly two mesh paths, body and head. Garments do **not**
travel over it, and no replicated field names them. Instead the pre-cooked
accept catalog in `WMCYNFirstSignalPresenceComponent.cpp` carries optional
`Outfit` and `Shoes` paths per family, read off the family only *after* its
body/head/anim paths have matched. Clothing is part of the avatar's identity,
so it rides with the family identity.

Consequences, all of them deliberate:

- No new manifest field, no new wire field, no new trust boundary. Asset paths
  still originate only in C++, so the authority check continues to reject any
  client-supplied path (`stage=authority result=rejected
  reason=unverified_client_selection`).
- A family with no garments is valid and stays valid - the 52b64e54 bake has
  none. A family that *names* a garment it cannot load is a broken family and
  aborts the whole swap, so a missing garment can never leave a half-dressed
  avatar.
- Garments are driven by `SetLeaderPoseComponent(AvatarBodyVisual)`, not by an
  AnimBlueprint of their own. They share the body skeleton, so copying its
  evaluated pose is both correct and one AnimInstance cheaper per garment.
- Changing an avatar's clothes requires a client build. That is acceptable only
  while avatars are pre-cooked; the moment delivery becomes real downloads, a
  `precooked` delivery type and per-slot manifest paths both become necessary.

### The garments are Y-up, and that is fine

Both garments' reference poses differ from the body's by exactly one global
rotation: every bone maps `(x, y, z) -> (x, z, -y)`, carried entirely by a 90
degree roll on the `root` bone, with a fitted residual of **0.00 cm** across
all 11 sampled bones and all 342 bones present. The garments are Y-up (a
Maya/Unity source convention) while the body is Z-up.

This does **not** need fixing, and the asset bounds that first raised it - the
outfit is 165 cm tall on Y where the body is 156 cm on Z - are a red herring.
A leader-pose follower skins as `v_mesh * inv(C_ref) * C_leader`. The garment's
vertices and its reference pose are both Y-up, so the rotation appears on both
sides and cancels:

    v_Yup * inv(C_body * R) * C_leader  ==  v_Zup * inv(C_body) * C_leader

The test that matters is therefore *not* whether a garment's bind pose matches
the body's. It is whether the garment's bind is self-consistent with its own
vertex data and whether the difference is a pure global rotation.
`Tools/AvatarPipeline/check_jared_bind_pose.py` fits one across the 24
determinant-+1 axis permutations and reports the residual; a residual that does
*not* fit is the real failure, because per-bone discrepancies do not cancel.

Grooms (hair, beard, eyebrows, eyelashes) exist in the source import and are
still **not** brought across. Strand grooms need rebuilt binding assets and are
expensive in VR. Deferred, not forgotten.

## Appearance Faults Found In The First Clothed Preview (2026-10-22)

Three faults, three unrelated causes. Recorded because two of them look like
something they are not, and both cost a wrong first diagnosis.

**Garments looked like wet plastic with white speckling.** Cause: the ORM map
`RiggedFit_DefaultMaterial_OcclusionRoughnessMetallic_1001` was imported as
`TC_Default` with `sRGB=true`. An ORM map is data, not colour. Gamma-decoding
it drags roughness down (glossy everywhere) and makes metallic noisy
(speckling). BaseColor and Normal were already correct, so this one texture was
the entire garment problem. **Check texture import flags before reading a
material graph** - assets from non-Unreal sources routinely arrive mis-flagged.

**Eyes rendered as a flat opaque slab.** Cause: the MetaHuman eye stack puts an
occlusion shell and a lacrimal edge in front of the eyeball, both meant to be
translucent; here they resolved opaque. The eyeball materials underneath were
fine. Fix: hide those two sections with `M_WMCYN_Hidden` (unlit, opacity 0)
rather than repairing the MetaHuman translucency stack. Stripping occlusion and
wetness layers is standard for headset-targeted MetaHumans, and they contribute
nothing at conversational distance.

**The source face materials are the broken ones.** `MI_EyeL_Baked`,
`MI_Face_Eyelashes` and friends under `/Game/MetaHumans/Jared/Face/Materials/`
have `None` for every texture parameter. The *preview copies* carry the real
baked textures. Do not "restore" a preview material to its source equivalent -
the source is emptier.

### Grooms

Hair, eyebrows and goatee are `UGroomComponent`s attached to `AvatarHeadVisual`
and listed per-family in the catalog as groom+binding pairs. Eyelashes are
deliberately excluded: the head mesh carries eyelash geometry in its own
section, so a groom would double them.

A `UGroomBindingAsset` targets one specific skeletal mesh. The source bindings
target `SKM_Jared_FaceMesh`, but the pawn renders `SKM_Jared_Head_Preview`. The
bindings are therefore duplicated into the preview folder and **retargeted**,
which is only valid because the preview head is a byte-duplicate of the source
face mesh. `prepare_jared_grooms.py` compares vertex counts and refuses to
retarget if they ever diverge - at that point the bindings must be rebuilt, not
repointed. A mismatched binding renders as hair floating beside the avatar.

Hair is the only strand-only groom (`Hair_S_360Waves` has no `CardsMesh`
variant); eyebrows and goatee ship with card LODs. PCVR only, consistent with
Quest being deferred for this avatar. Hair cards are the right answer when
Quest returns.

## Recommended UE Runtime Architecture

Keep avatar work WMCYN-owned and layered around the proven First Signal pawn.

Recommended ownership:

- current locomotion/tracking/body source of truth remains `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal`
- add a WMCYN avatar application layer around that pawn
- put runtime download/cache/manifest logic in `Plugins/WMCYNRuntime`
- do not edit AFCore assets to introduce avatar loading

Suggested components:

- `UWMCYNAvatarManifestSubsystem` or extension inside `UWMCYNBackendSubsystem`
- `UWMCYNAvatarRuntimeSubsystem` for cache, download, and mount/import
- `UWMCYNAvatarPresentationComponent` on the active pawn for apply/swap/fallback behavior

## Editor Build Step

For the internal cohort, do not start with a GPU fleet.

Start with one workstation:

- UE project installed and working
- MetaHuman plugins configured
- GPU available
- headless or scripted editor automation against the real project

That workstation can:

1. pull a queued capture
2. run the solve/build flow
3. generate Quest and PCVR outputs
4. upload build artifacts
5. mark the avatar publishable

Only build queueing/worker orchestration after the internal loop proves out.

## Known External Blockers

- Epic licensing/compliance for large-scale automatic end-user generation should be confirmed directly before public rollout
- Epic-hosted services remain part of the pipeline
- the physical HQ office banner width still needs a real-world measurement for unrelated AR marker work

## Success Criteria

This lane is successful when:

1. a real internal user publishes an avatar from the phone flow
2. Unreal signs in with pairing code
3. Unreal fetches the user's manifest
4. Unreal downloads/caches the correct platform variant
5. the player spawns into the persistent world with that avatar applied
6. fallback/default avatar behavior is clean when no custom avatar is active

