# Avatar Implementation Plan

Last updated: 2026-07-28

## Status

This is a planned next lane, not the current First Signal gate.

Current First Signal priority remains:

- persistent-world login
- four-user shared presence
- voice
- PCVR capture
- network validation

Avatar fidelity should not interrupt those gates.

## Goal

After First Signal stabilizes, add phone-scanned user avatars to the persistent WMCYN world.

## Phase 0: lock the contract

Done in docs:

- define the manifest shape
- choose platform-specific manifest requests
- choose fallback default-avatar behavior
- choose phased runtime loading strategy

Deliverables:

- `Docs/METAHUMAN_AVATAR_SPEC.md`
- `Docs/BACKEND_HANDOFF_AVATAR.md`

## Phase 1: VR pairing-code login path

Priority:

- implement or expose the pairing-code login UX in Unreal as the preferred in-headset auth path

Scope:

- six-character code entry
- polling `code.exchange`
- Firebase token acquisition
- same post-auth path as existing credential login

Exit:

- headset login works cleanly without typing email/password

## Phase 2: default-avatar manifest loop

Build the client lane before custom avatars exist in-world.

Scope:

- call `GET /v1/avatar/manifest?platform=...`
- accept `200` default-avatar response
- cache by `cacheKey`
- apply a default avatar representation to the active pawn

Exit:

- a logged-in Quest or PCVR user can fetch a manifest and apply a stable default avatar path

## Phase 3: prototype custom avatar runtime path

Preferred first proof:

- `delivery.type = runtime_mesh`

Scope:

- WMCYN-owned runtime download/import loader
- one shared animation/skeleton contract
- safe fallback when content is missing or invalid

Exit:

- one internal user's published avatar can appear in-world on at least one platform

## Phase 4: real cooked platform bundles

Move to ship-quality asset delivery.

Preferred target:

- `delivery.type = pak`

Scope:

- per-avatar cooked Quest bundle
- per-avatar cooked PCVR bundle
- mount/apply at runtime
- client cache invalidation via `cacheKey`

Exit:

- Quest and PCVR both load their intended platform variants from real cooked content

## Phase 5: editor automation build step

Scope:

- one GPU workstation
- headless/editor-scripted build job
- run against the real `wmcyn_online` project
- emit publishable Quest and PCVR artifacts

Exit:

- capture -> job -> artifact -> manifest -> in-world avatar works end to end for an internal tester

## Recommended UE Ownership

Keep this lane WMCYN-owned:

- runtime fetch/cache logic in `Plugins/WMCYNRuntime`
- avatar application logic in a WMCYN-owned component on the active First Signal pawn
- no AFCore asset edits by default

Suggested future classes:

- `UWMCYNAvatarRuntimeSubsystem`
- `UWMCYNAvatarPresentationComponent`
- `UWMCYNAvatarManifestSubsystem` or extension of `UWMCYNBackendSubsystem`

## Open Technical Work

1. add pairing-code login UI in Unreal
2. decide final runtime representation for default avatar
3. implement `GET /v1/avatar/manifest` client call
4. implement cache folder and invalidation by `cacheKey`
5. define how mounted/imported content maps onto `BP_WMCYN_UserPawn_FirstSignal`
6. decide whether Quest and PCVR share the same animation contract
7. identify the exact workstation that will run the editor build step

## Explicit Non-Goals Right Now

- public release scale
- avatar customization UI beyond publish/select
- final wardrobe system
- public creator economy
- switching the current First Signal pawn/body source of truth away from the proven native Mimic path

