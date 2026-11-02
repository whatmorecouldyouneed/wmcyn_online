# Matched Protocol-2 Cutover

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
