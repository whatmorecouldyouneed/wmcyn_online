# Founder client release

## Automation

`Build VR Clients` builds PCVR and Quest Development packages on relevant main
pushes. Documentation-only changes do not trigger a cook. Manual dispatch selects
Both, Quest, or PCVR and Development/Shipping. ALPHA publication is explicit and
does not deploy/start the world server. No pull-request builds run on this trusted
self-hosted runner.

The workflow checks out the triggering commit and LFS content under the runner's
`client-release` directory. It never packages the interactive editor checkout.
Commit reviewed source/assets before expecting them in a release. The old Quest
workflow is blocked to prevent accidentally building uncommitted editor work.

All client/server builds share the `unreal-build` concurrency group. Running work
is not interrupted; GitHub replaces older pending runs when newer work queues.
Do not queue a manual release behind repeated pushes without checking its status.

## Runner setup

- Bring `wmcyn-buildbox` online; keep its workspace separate from the editor.
- Install matching Launcher UE 5.8, native toolchain, Android SDK/NDK/JDK, Git LFS,
  and Android build-tools 36.0.0. Validate a clean checkout can cook editor assets.
- Install Meta platform utility at `C:/Tools/ovr-platform-util.exe` and verify its
  `upload-quest-build` APK/OBB flags against the installed version before release.
- Repository variables: `WMCYN_SIGNING_INI`, `WMCYN_KEYSTORE` are runner file
  paths; `WMCYN_CERT_SHA256` is the established Meta certificate fingerprint.
  Signing override must contain StoreVersion and KeyStore=wmcyn-release.keystore.
- Repository secrets: `META_APP_ID`, `META_APP_SECRET`. Never commit signing
  passwords, private keys, or print their config. Back up the existing key safely.
- Version codes use UTC epoch seconds. Confirm the first generated code exceeds
  Meta's currently published code; subsequent serialized builds increase it.

## Release checks

1. Review/commit the intended source and assets. Resolve protocol-2 signed identity
   deployment with backend and matching Linux server before distributing clients.
2. Run artifact-only build. Inspect build manifest, logs and signature report.
3. Test fresh install/update, login, names, microphone permission, movement, menu,
   fallback avatar, and remote visibility/voice on matching server/client versions.
4. Run publication only for an approved revision; repeat device checks if rebuilt.
   Verify Meta processing and tester channel membership before announcing access.
5. Provide founder instructions, version and known limitations. Preserve the
   approved artifact and rollback package before 14-day Actions retention expires.

APK data remains embedded by default. If packaging requires an OBB, artifacts and
upload include it. Multiple expansion files fail closed pending explicit support.
PCVR artifacts contain the whole packaged application, not just its executable.
GitHub's downloadable artifact archive supplies the ZIP container.

## Connected repositories

Mobile main inspected October 26 is native Swift/iOS (Expo retired), including
TrueDepth, interview/camera and default-avatar behavior. Phone distribution remains
TestFlight and is separate from this Quest workflow. Backend main includes the
default avatar manifest; local signed-identity commit 66c78b1 is not established as
deployed. Do not equate default mobile mesh delivery with cooked Unreal delivery.

## Current evidence and blockers (October 26, 2026)

- Five native identity/ticket/camera tests completed successfully in the October 23
  rerun. This does not establish packaged or remote acceptance.
- New packaging scripts pass PowerShell parser validation; actual CI cook pending.
- Follow-up: runner started and GitHub confirms online. Meta utility installed
  through WinGet with installer hash verification; its APK/OBB flags verified
  using local help. Publisher also discovers this per-user installation.
- Existing release keystore validated and repository variables configured. The
  previous packaged APK is debug-signed and is not an ALPHA candidate. Packaging
  now explicitly re-signs Development APKs with the existing release key and
  verifies the pinned fingerprint. Meta's accepted certificate remains a separate
  first-publication check; no key was generated or replaced.
- Interactive checkout has extensive staged/unstaged assets and source. These have
  not been blanket committed or reverted. No new build uploaded by this pass.
- Linux server workflow still uses its historical interactive path; it shares the
  build lock, but must be migrated to an isolated source-engine checkout before
  calling a server release reproducible. No server deployment performed.
