# Matched Protocol-2 Cutover

## Live Cutover Result (November 2, evening ET)

- Server run 41549189104 and Both-client run 41549193056 succeeded from 4152296.
- All client manifest SHA-256 entries verified locally. Quest versionCode
  1793668406, expected release certificate, APK plus OBB present.
- Server archive SHA-256 verified remotely before extraction:
  F300E538914D49FF462E6C979440341AA917082B391182EF6060D6965EE72D7B.
- Fresh zero-user heartbeat checked immediately before stop/switch/start.
- New server registration returned HTTP 200; canonical record online with
  protocol 2, build first-signal-001, fresh heartbeat. NRestarts=0 at first check.
- Previous live directory retained as `/opt/wmcyn/LinuxServer-pre4152296`;
  original root-only rollback copy/service unit also retained. No secrets changed.
- Exact-artifact ALPHA promotion run 41563083633 is in progress, not yet confirmed
  published. New manual workflow verifies source run/commit and package hashes;
  no rebuild required. Workflow commit ae59836 does not change runtime source.
- PCVR archive: runner `client-release/Saved/FounderRelease/`
  `wmcyn_online-PCVR-Development-41522965cac9.zip`.
- Bootstrap protocol mismatch is resolved at registration level. Fresh account
  login, network join, headset/remote camera acceptance still require testing.

The sections below record pre-cutover observations, not current deployment state.

## Verified Before Build

- Founder reports no users online and editor closed, November 2.
- Live runtime has fresh heartbeat, build `first-signal-001`, protocol 1.
- Local code uses same build ID, protocol 2; backend rejects the mismatch at
  bootstrap with `incompatible_build`. Do not lower the protocol or bypass tickets.
- API is ACTIVE, last deployed October 26. Signed identity bootstrap must still
  be smoke-tested against the new runtime before claiming entry works.
- Quest ALPHA workflow 40635673373 succeeded from ff29459 on October 28.
  Founder distribution success does not prove protocol-2 world-entry acceptance.
- VM service active; deployed directory 2.3 GB, 40 GB free. No server change yet.

## Execution Gates

1. Checkpoint only reviewed camera source, Settings widget and release docs.
2. Build Linux server with deploy=false and Both clients with publish_alpha=false
   from the exact same commit using the existing serialized GitHub Actions queue.
3. Confirm cook success, manifest commit, package integrity and Quest signing.
4. Stage server separately; preserve current LinuxServer directory and private
   service configuration for rollback. Do not use the old in-place auto-deploy.
5. Recheck connected users immediately before downtime; pause if anyone joined.
6. Switch to staged server, verify heartbeat protocol 2 and signed entry/identity.
   Verify PCVR connection and remote voice/tracking with matching clients.
7. Publish the verified Quest artifact to ALPHA and give founders update notice.
   Do not claim they have updated until hardware confirmation.
8. Keep previous server/client artifacts available. Rollback both sides together,
   never just change the Firestore protocol advertisement. Retain idle cost controls.

Build/deploy status is pending until results are added below. No server replacement,
ALPHA publication, headset acceptance or remote camera recording claimed here.

## Execution Checkpoint

- Shared source commit: `41522965cac9cc0c87efb905cfc2b1fe115b1321`, pushed to main.
- Linux Actions run: `41549189104` (deploy=false), checkout/LFS in progress.
- Both-client Actions run: `41549193056` (Development, publish_alpha=false),
  waiting behind the server job in `unreal-build` concurrency.
- Runner online, editor/Live Coding preflight passed, local disk ~270 GB free.
- Backend checkout `1be0443`: TypeScript build and 58/58 tests passed again.
- Live heartbeat connectedUsers=0 before staging. Recheck before actual cutover.
- Rollback preserved remotely at `/opt/wmcyn/rollback/protocol1-20261102/`:
  2.3 GB LinuxServer copy plus service unit, root-only directory, READY marker.
  Service remained active. Private service contents were not printed or committed.
- Do not deploy from the legacy overwrite-in-place workflow step. Stage and
  verify the new package, retain rollback, then cut over only after client success.
