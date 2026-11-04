# Persistent Sign-In And Development Workflow

Date: 2026-10-16
Status: Windows native foundation and Designer-visible login/settings controls implemented. Credential-store automation and local UI smoke checks pass. Quest persistence and real-account end-to-end acceptance remain pending. Not a shipped feature.

## Order

1. Remembered sign-in, explicit sign-out/switch account, force-login regression.
2. Explicit offline local-world test mode that cannot grant online access.
3. Developer menu, connection status, test presets and sanitized diagnostics.
4. Adaptive UI styling from ADAPTIVE_UI_SPIKE.md.

## Existing System

WMCYNBackendSubsystem receives Firebase ID and refresh tokens from login/pairing, then requests the backend bootstrap and a short-lived join ticket. Previously tokens existed only in memory. AFCore supplies reusable UI/keyboard/settings behavior; its ordinary save/settings storage is not a secure credential vault. Preserve that UI behavior and the native Mimic pawn.

## Implemented Foundation

- Windows Credential Manager generic credentials, local Windows-user scope and local-machine persistence (not roaming). Scope is separated by backend URL and Firebase API key. Stores only refresh credential and a display label; never password, ID token or join ticket. No plaintext disk fallback.
- Explicit opt-in SetRememberSignIn(true), then ordinary login. Save occurs after valid bootstrap. Unsupported platforms remain memory-only. Quest Android Keystore implementation is pending, not silently replaced with SharedPreferences plaintext.
- HasRememberedSignIn/GetRememberedDisplayLabel expose availability and an untrusted display-only label. The label never establishes identity or capabilities.
- ContinueRememberedSignIn on the existing async login action calls RestoreSignInAndLoadBootstrap, reusing normal completion/travel behavior. Restore does not use the local PIE identity fallback.
- Restore exchanges the credential through Firebase Secure Token API, persists token rotation, then obtains a new bootstrap/ticket. Bootstrap identity must match the authenticated UID. No cached server address/ticket is accepted as restored authentication.
- Revoked/expired/disabled/deleted-account refresh errors clear saved credentials and request login. Network/service errors retain the credential for explicit retry. Bootstrap 401 clears credentials. Restore/bootstrap requests have 15-second timeouts.
- Authentication generations discard stale login/pairing/refresh/bootstrap callbacks after sign-out or subsystem shutdown. Shutdown does not delete remembered credentials.
- -WMCYNForceLogin bypasses saved sign-in and persistence and disables the async action's PIE identity fallback. It does not erase the saved credential merely by running a password login. Explicit SignOut still deletes it.
- Storage failure is logged by stage/result without credential values. A failed deletion is logged and restore is suppressed for this subsystem lifetime; deletion failure must be surfaced by the final UI before claiming the account was forgotten on disk.

## Remaining Integration

- The live WBP_WMCYN_LoginJoin was inspected and retains AFCore Widget_Base inheritance. Designer controls CHK_RememberSignIn, ROW_RememberedSignIn, BTN_ContinueSaved and BTN_SwitchAccount are bound by WMCYNRememberedSignInControls from the existing presence initialization path. Credential fields and keyboard graphs are preserved.
- Settings has a Designer-owned SignOutButton, wired through its existing native WMCYN parent. It clears authentication and travels to the local Crib, leaving the remote world. Runtime-registration hosts are excluded to avoid stopping the shared world. Real remote disconnect validation remains pending.
- Switch Account cancels pending authentication, forgets saved access and clears both credential fields. Credential deletion failure leaves restore suppressed and exposes a retry through Switch Account instead of claiming the device was successfully cleared.
- Optional -WMCYNAutoContinue is development-only, runs after login controls are initialized, and still requires saved credentials plus Firebase refresh/bootstrap. -WMCYNForceLogin takes precedence. Automatic entry remains untested.
- Android: use Android Keystore-backed encryption in private app storage, exclude encrypted credentials from backup, handle invalidation/uninstall/rotation and test on Quest. Do not enable persistence until this is implemented and validated.
- Add mocked HTTP tests for rotation, auth rejection, transient failures, UID mismatch, and sign-out racing pending requests. Define cancellation completion for the UI as part of its wiring.
- The previous automatic PIE identity fallback is removed. -WMCYNLocalIdentity now requires non-shipping, standalone and no active runtime registration; force-login/backend-auth flags override it. A complete offline test preset/menu is still pending.

## Verification

Windows UE 5.8 compile/link checks passed. WMCYN.Auth.SecureCredentialRoundTrip ran in UnrealEditor-Cmd and passed (one test), covering missing/read/write/rotation/deletion/empty-token handling with an isolated synthetic credential target. Evidence: Saved/Logs/PersistentSignInTests.log.

The editor MCP endpoint was reachable directly on localhost even though the chat's deferred Unreal tools were unavailable. Both WMCYN widgets compiled and saved; the editor was closed only after confirming no dirty packages and no PIE, then restarted with the linked DLL. No AFCore/Mimic edits, APK package, backend write or server deployment were performed. Existing unrelated dirty assets remain untouched.

Local PIE verifies the new checkbox is present, the saved-login row is collapsed with no credential, and the native controls are bound. A synthetic Switch Account delegate click cleared the password and restored the credential form. Screenshots: Saved/Diagnostics/PersistentSignIn/login_controls_verified.png and continue_layout_preview.png. The latter is a presentation-only preview with a sample label, not evidence of authenticated restored identity. Offscreen ticking was enabled only on the transient widget host for capture, not saved to the asset.

No real password, refresh credential or server ticket was supplied during these tests. Real-account restart/restore, revocation, reconnect, remote sign-out, automatic entry and Quest hardware tests remain open.

Acceptance before calling the lane complete:

1. Opt in and log in normally. Restart application; Continue shows the correct label without submitting a password.
2. Continue performs Firebase refresh and a fresh server bootstrap. Join-ticket verification and account authority remain unchanged.
3. Sign Out/Switch Account remove saved access; a late HTTP completion cannot restore it. A connected player leaves the world on sign-out.
4. Force-login testing shows credentials even with a saved sign-in and cannot enter via the PIE fallback.
5. Revocation returns to login; offline/503 leaves an explicit retry option without an automatic request loop.
6. Login, keyboard, voice, nameplate, locomotion lock/unlock, pairing and multiplayer regressions pass on PCVR and Quest.

Sources: https://firebase.google.com/docs/reference/rest/auth and https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-credwritew
