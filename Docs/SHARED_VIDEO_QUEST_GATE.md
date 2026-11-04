# Shared Video: Quest Feasibility Gate

## Update - 2026-11-04 (later)

Channels, the AFCore adapter and panels are now implemented for direct links, ahead of this gate at
the founder's request. See `CRIB_DISPLAY_STREAM.md`. That overrides "Next Exact Work" item 5 and
the shared URL RPC, timeline sync, crash quarantine, panel, volume and decoder budget entries in
"Still unimplemented" below. **The resolver, Quest playback measurement and everything else below
remain open.**

PCVR resolving is implemented; the founder chose to validate it from the built client. Quest is not.

- `dev.ffmpegkit-maintained:yt-dlp-android:2.0.2` audited: MIT wrapper, 22.8 MB AAR, API 24+, no
  `DexClassLoader` in `classes.jar` or the Chaquopy jar, `minCompileSdk=1`. Its Python runtime still
  loads native modules, so Meta's scan is unproven.
- Its `execute()` always downloads and has no `-g`, so the planned bridge calls
  `yt_dlp.YoutubeDL.extract_info(download=False)` through Chaquopy instead.
- The founder approved adding it on 2026-11-04. `WMCYNRuntime_UPL.xml` adds the dependency and
  `AndroidThunkJava_WMCYNResolveVideo`; `WMCYNRuntime.Build.cs` picks the file up automatically.
- A resolver crash on Quest is covered by the channel's crash marker, which is set before resolving.
- Still unproven: real links on Quest 2, first-use Python startup time, and Meta's scan.

## Status - 2026-11-04

Implementation started, NOT feature complete. Full Quest and PCVR parity is
required. Do not ship PC-only YouTube/Twitch as completion.

- Baseline HEAD: 88a517d. Existing stage, tray, UI, environment and historical
  documentation changes are dirty and preserved. They were not swept into a commit.
- Local world protocol is 3 (tray RPCs); last documented live release is protocol 2.
  Live cloud state was not queried. No server/backend deployment was performed.
- Quest 3 detected via ADB. No Quest 2 or 3S device acceptance established.
- New native AWMCYNVideoProbe builds successfully in Launcher UE 5.8 Win64 Editor.
- Android ARM64 native compile/link and Gradle Development APK packaging succeeded
  (84.7 seconds). This native-build invocation did not cook the new map or stage
  matching content. APK was not installed, published or treated as acceptance.
- Isolated `/Game/WMCYN/Dev/VideoProbe/L_VideoProbe` generated successfully.
  Production Crib, autoplay Blueprint, AFCore, Mimic and account logic unchanged.

## Resolver Finding

The proposed yausername/youtubedl-android dependency declares GPL-3.0. Epic's
Unreal EULA explicitly identifies GPL among incompatible licenses. Do not embed
this wrapper in the Unreal APK without a separately reviewed compatible route.
This is a dependency adoption blocker, not a successful Meta security evaluation.

- https://github.com/yausername/youtubedl-android/blob/master/LICENSE
- https://www.unrealengine.com/eula/unreal

An alternative to evaluate is ffmpegkit-maintained/yt-dlp-android 2.0.2, whose
wrapper declares MIT and embeds Python via Chaquopy. It documents ARM64 support
and no in-app updater. Its build currently installs unpinned yt-dlp/certifi;
published artifact contents/transitives still need verification and hashes.
Its default documented API downloads media, so metadata-only extraction must
be verified rather than assuming a drop-in resolver. No dependency was installed.

- https://github.com/ffmpegkit-maintained/yt-dlp-android
- https://github.com/ffmpegkit-maintained/yt-dlp-android/blob/main/THIRD-PARTY-NOTICES.txt
- https://github.com/ffmpegkit-maintained/yt-dlp-android/blob/main/library/build.gradle

Existing Build-Clients.ps1 rejects DexClassLoader references. Passing that local
check would not establish Meta security acceptance of an embedded interpreter.
No cookies, paid anti-bot service, relay, proxy, or backend deployment is authorized.

## Implemented Diagnostic

WMCYN-owned VideoProbe runs only with `-WMCYNVideoProbe` and never on a dedicated
server. It opens two fixed public fixtures, not user-provided links:

1. Thirty seconds of video-off baseline.
2. Sixty seconds of W3C Sintel MP4.
3. Sixty seconds of Akamai Sintel HLS.
4. Close player and detach audio.

It requests ElectraPlayer, logs actual selected player, open failure/success,
media time, texture dimensions and five-second game-frame p95 windows. The map
has a fixed world-space screen and HMD camera, no login/backend or multiplayer.
Audio starts at 25 percent. This is a diagnostic, not the production AFCore adapter.
The fixtures are external public hosts and may change or fail. HLS is currently
adaptive/unrestricted: logs must establish dimensions; a production 720p30 cap
has NOT been implemented. Video timestamps/texture dimensions do not prove a
visible picture or audible sound. Game-frame p95 is not GPU/compositor telemetry.

Map generation: `Tools/Video/create_probe_map.py` using Unreal Python commandlet.
This script owns only the isolated probe map/material/texture and overwrites them.
Local standalone diagnostic launch (not packaged acceptance):

```powershell
& 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe' `
  'C:/Users/jvred/OneDrive/Documents/Unreal Projects/wmcyn_online/wmcyn_online.uproject' `
  /Game/WMCYN/Dev/VideoProbe/L_VideoProbe -game -WMCYNVideoProbe -log
```

Do not distribute the native-build APK as a new founder build: this pass does
not cook or verify matching content, sign a release, or update the online server.

## Next Exact Work

1. Review/checkpoint existing tray/stage changes separately before matched release.
2. Finish permissive resolver artifact/transitive/license audit; pin checksums and
   verify metadata-only extraction without runtime code downloads.
3. Cook the isolated probe map into a dedicated diagnostic package, verify its
   identity/content before installing, then record Quest playback and timing.
4. Validate resolution cap, Android suspend/resume, YouTube VOD/live and Twitch;
   conduct Meta security validation before adopting the resolver.
5. Only after the gate passes, implement the three authoritative channels,
   AFCore media adapter and Designer-visible panels from the approved plan.

Still unimplemented: shared URL RPCs, timeline sync, private-destination/redirect
policy, production crash quarantine, resolver execution, panels, per-channel
local volume, decoder proximity budget, remote tests and matching release builds.
The old autoplay stream remains unchanged until the replacement is validated;
its known crash/HTTP risk is not cleared by this diagnostic.
