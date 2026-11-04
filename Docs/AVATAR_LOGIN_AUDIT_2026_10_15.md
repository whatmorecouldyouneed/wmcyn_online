# Avatar Login Audit: PCVR

## Account Decision

Test account: `[account email]`. User explicitly chose to keep the native default until an account-matched Unreal output exists. Do not rebind the earlier test-account MetaHuman, rebuild a character, or publish placeholder success.

## Verified Boundaries

Read-only Firebase Auth and Firestore inspection on 2026-10-15 found:

- The profile's active job `bee53df8-5a2f-4a7f-ae61-f25b110ee4fc` is `preview_ready`, with no PCVR delivery or Unreal object paths. The profile retains an older publication timestamp/version, but its status is also `preview_ready`.
- Five earlier jobs are marked published. The latest published job `e63457c2-d04e-4432-a389-7e4ab7fe287a` has neither PCVR delivery nor generic Unreal mesh/animation paths. A published scan/preview is not evidence of a cooked PCVR avatar.
- The only local runtime output directory is `WMCYNAvatar_52b64e54`. Its corresponding published job belongs to a different Firebase UID. No ownership reassignment was performed.
- The older publishing tool writes `pcvrRuntimeMeshSkeletalMeshPath` and related platform-prefixed fields. The backend `origin/main` manifest resolver reads `skeletalMeshPath`, `headSkeletalMeshPath`, and `animClassPath`, and requires platform delivery before selecting a published avatar. This contract mismatch also affects the old test asset.
- The backend checkout is on `codex/first-signal-backend-bootstrap`, not main. Its source lacks the newer manifest service. Main was inspected with `git show`, without switching branches or touching unrelated backend work.

**First failure: account publication/output availability, before Unreal mesh loading.** The inspected main resolver would choose default for this account. The actual authenticated manifest HTTP response and deployed revision were not verified in this audit; a successful default response also depends on the backend's default-URL signing.

## Runtime Corrections

- Manifest request timeout: 15 seconds. Failure releases the request guard and retries after 5 then 10 seconds, capped at three attempts per pawn/account. Account changes reset the state and restore native visuals; stale account callbacks are ignored.
- Validate response platform and default flag; published delivery must be `runtime_mesh` with a mesh path. URL parsing does not imply downloading or mounting. No streaming was added.
- Default means keep/restore the native Mimic visual. Do not attempt to load backend placeholder `/Game/WMCYN/Avatars/Default` assets.
- Published local-authority application is restricted to the existing pre-cooked body/head/animation catalog. Body, head, and animation must all load before switching visuals. Fixed sibling head-path derivation, which previously changed the package suffix but not the object suffix.
- Network clients retain native default and request canonical default from the server. The server rejects nondefault or nonempty client avatar payloads, never loads them, and only replicates its own default selection. Unreal RPC ownership and controller/pawn consistency are preserved.
- This intentionally closes the insecure client-path submission route; it is **not** a completed server-side published-avatar resolver. A ticket-UID-bound backend lookup and an account-matched cooked catalog are still required to enable published avatars online.
- Structured `WMCYN_AVATAR` logs cover UID-established (without UID value), manifest transport/contract outcomes, selection, path acceptance/rejection, mesh loading, and visual application. No passwords, bearer tokens, signed delivery URLs, or biometric capture data are logged.

No Firestore writes, backend deployment, framework asset edits, pawn replacement, or Quest scope expansion.

## Build Status

UE 5.8 UnrealBuildTool compilation and full module linking passed on 2026-10-15 (`-Module=WMCYNRuntime -NoHotReloadFromIDE`, without `-NoLink`). Unreal was closed after confirming there were no dirty content or map packages. Both `UnrealEditor-WMCYNRuntime.lib` and `UnrealEditor-WMCYNRuntime.dll` linked successfully. Authenticated manifest HTTP capture and two-client runtime acceptance remain pending. No new APK, PCVR executable or Linux server was produced or deployed.

## PCVR Acceptance

Run only after linking the updated runtime DLL and restarting the editor/client; a compile-only check does not update the running application.

1. Enter `L_WMCYNOnline` and log in with the test account. Expect `stage=login_uid result=established` and `stage=selection source=default` if the live manifest agrees with the audited records.
2. Confirm `stage=visual result=success source=native_default`. The existing full-body Mimic avatar, hands, locomotion, login, voice and nameplate must remain functional. No older test-account face should appear.
3. Simulate a manifest transport failure in a controlled test environment. Expect at most three attempts, with scheduled retries then `retry=exhausted`; movement and native presence must remain usable.
4. On matching server/client builds, connect a second PCVR client. Confirm each sees the other's native default, tracking and nameplate, and both retain voice. **Not yet executed.**
5. Security regression: from an owning client, submit arbitrary body/head/animation/cache strings through the avatar RPC. Expect `stage=authority result=rejected reason=unverified_client_selection`, no loads, and no replicated selection change. Cross-owner RPCs must remain rejected by Unreal. **Not yet executed.**
6. Published-avatar acceptance remains blocked until this account has its own validated PCVR output, publication fields match the live manifest contract, assets are cooked into matching builds, and the server resolves selection using the authenticated join-ticket UID. Only then test remote custom-avatar visibility; Quest follows afterward.
