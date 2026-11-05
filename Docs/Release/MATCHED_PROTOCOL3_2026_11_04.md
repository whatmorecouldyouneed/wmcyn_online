# Matched Protocol-3 Cutover

Follows `MATCHED_PROTOCOL2_2026_11_02.md`.

## Live Cutover Result (November 4, evening ET)

- Quest ALPHA run 41938845060 uploaded the exact verified package as build 1703678144684832. Meta's
  post-upload validation was still running when this was recorded, so availability is unconfirmed.
- Fresh heartbeat showed zero users and protocol 2 immediately before the switch.
- Canonical record then showed online, protocol 3, build first-signal-001, fresh heartbeat.
  Registration returned 200, NRestarts=0, no fatal log lines.
- Previous server retained at `/opt/wmcyn/LinuxServer-pre0e0a80e`.
- Fresh login, network join, voice, and all shared video and tray behavior are still untested.

## Verified Build

- Source: `0e0a80e0b474b37b6844888aacb8f8e192d396f2` on `feature/protocol3-tray-and-video`.
- Server run 41928874583 and Both-client run 41928886479 succeeded from that commit.
- Quest: versionCode 1793835273, protocol 3, APK 150,521,818 bytes plus OBB, release certificate
  matches `WMCYN_CERT_SHA256`, all manifest hashes verified.
- Quest APK holds the resolver bridge, YtDlp and Chaquopy, arm64 only, no `DexClassLoader`.
- PCVR: `wmcyn_online-PCVR-Development-0e0a80e0b474.zip`, with `yt-dlp.exe` at the pinned checksum.
- Client artifacts expire after 14 days, so ALPHA promotion needs them before 2026-11-18.

## Staged Server

- SHA-256 76355A846ED9F4D020A2794922B9CC49047A6340AB5E700722982E3D1E005635, verified on the VM.
- Staged beside the live server, owned by `wmcyn`, permissions matching the live directory. Windows
  `tar` drops execute bits, so staged copies need `chmod`.
- Live `Saved` folders hold only a log and generated config, so nothing carries over.

## Rollback

ALPHA went first, so a Meta rejection would have left the live world unchanged. Rollback both sides together: restore `LinuxServer-pre0e0a80e` and the prior ALPHA build.

Not yet tested: real YouTube or Twitch links on either platform, Meta's scan with the Python
runtime, panels in a headset, and multi-user sync.
