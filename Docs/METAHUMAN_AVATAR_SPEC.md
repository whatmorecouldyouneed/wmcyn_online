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

