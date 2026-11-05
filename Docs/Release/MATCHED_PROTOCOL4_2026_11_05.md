# Matched Protocol-4 Cutover

Follows `MATCHED_PROTOCOL3_2026_11_04.md`.

## Live Result (November 5, morning ET)

- Source: `7264cdffd279e2b77f2052baba10aacb1bebfd94` on `main`.
- Quest ALPHA run 42021401974 uploaded the exact verified package as build 1704172501302063. Meta's
  post-upload validation was still running when this was recorded, so availability is unconfirmed.
- Zero users and protocol 3 immediately before the switch.
- Canonical record then showed online, protocol 4, build first-signal-001, fresh heartbeat.
  Registration returned 200, NRestarts=0, no fatal lines, no missing assets.
- Previous server retained at `/opt/wmcyn/LinuxServer-pre7264cdf`.

## Verified Build

- Server run 42009368974 and Both-client run 42009333879 succeeded from that commit.
- Quest: versionCode 1793874804, protocol 4, APK 150,538,202 bytes plus OBB, arm64 only, all manifest
  hashes verified, release certificate matches `WMCYN_CERT_SHA256`.
- PCVR: `wmcyn_online-PCVR-Development-7264cdffd279.zip`, installed locally to
  `Documents/WMCYN/Clients/protocol4-7264cdf`.
- Server package SHA-256 61DC13975A59B819A795E6452C3619945014914D4812373E04D7A0BF7DA2EDAE, verified
  on the VM. `scp` dropped on the first attempt from this PC, so large uploads go in pieces with a
  hash check before unpacking.
- Client artifacts expire after 14 days.

## Rollback

Restore `LinuxServer-pre7264cdf` and the prior ALPHA build together. Old clients can't join protocol 4.

## Not Yet Tested

Everything in the polish pass is proven by automation only. Headset acceptance is still pending for
gravity, turning, crouch, remote feet, the Quest Delete key, reconnect, Respawn, TV volume, and Quest
frame rate. Meta's scan outcome is also unconfirmed.
