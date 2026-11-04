# Crib Display: Autoplaying Live Stream

## Current Design (2026-11-04)

Shared channels replaced the autoplay actor. `WMCYN_DisplayAutoplay` is gone from the level, and
everything below History is old. AFCore has no link input or replicated link:
`Comp_Video.SourceURL` isn't replicated and `Widget_VideoControl` is only a play button.

| Channel | Screen | Mirror | Default |
|---|---|---|---|
| `LivingRoom` | `BP_Display_C_1` | none | Rick and Morty stream |
| `Stage` | `BP_Screen_C_1` | DJ booth `DISP` slot | Blank |
| `Basement` | `BP_Screen_C_7` | none | Blank |

- `AWMCYNVideoChannel` replicates the link. Each channel owns its player and texture, set into
  `Comp_Video` by reflection, so rooms can't overwrite each other through AFCore's shared `MT_Video`.
- The DJ screen uses the stage's material, so it can't drift and needs no second decoder.
- `BP_WMCYN_VideoPanel` hosts `WBP_WMCYN_VideoPanel` in an AFCore `Comp_Widget`. The widget was
  duplicated from the login panel to keep its keyboard functions. Controls are Designer-owned and
  bound by name in `AWMCYNVideoPanel`.
- Submits go through `ServerSetVideoChannel`, with a 3-second cooldown per user across channels.
- Volume is local, per channel, and saved. Quest decodes only the nearest playing channel in 15 m.
- A link that crashes a client is skipped once on next launch. Submitting it again retries it.

**Link policy limits:** direct http and https links are accepted, except local and private
addresses. The check only sees the submitted link, so a public address that redirects somewhere
private can't be filtered through Electra.

**YouTube and Twitch:** each client resolves the page link itself through `WMCYNVideoResolver`, so
direct URLs are never replicated. PCVR packages a pinned, checksummed `yt-dlp.exe` via
`Tools/Video/Get-YtDlp.ps1`; run that script to use it in a local editor. Quest shows "aren't
available on Quest yet" until its dependency is approved, per `SHARED_VIDEO_QUEST_GATE.md`.

Files seek to the shared start time. Live streams stay at their live edge.

**Verified 2026-11-04 in Simulate:** living room stream through the channel, stage, basement and
DJ mirror with a temporary test clip, panels rendering, 17/17 WMCYN automation, Linux compile only.
**Not verified:** any real YouTube or Twitch link (yt-dlp has never run in this project), typing
and clicking on a panel, a real submit in a running session, audio by ear, two clients, packaged
builds, or anything on Quest.

## History

2026-10-17. The wall display in `L_WMCYNOnline` plays a live HLS stream on world load.

## Reuse

No new media assets. The level already contained AFCore's `BP_Display`
(`/Game/AFCore/Examples/Blueprints/Samples/BP_Display`), whose `Comp_Video`
component is a complete video rig: it builds the media source, constructs the
MediaPlayer, binds AFCore's `MT_Video` media texture, applies the video material
to the screen face and wires the `MediaSound` component, all at runtime from two
properties. Only the instance `sourceURL` was changed; `videoSource` was already
`StreamMediaSource`. No AFCore asset was edited.

- Stream: `http://adultswim-vodlive.cdn.turner.com/live/rick-and-morty/stream.m3u8`
- Multi-variant HLS, H.264 + AAC, 416x234 up to 1920x1080.

## Autoplay

AFCore starts video from interaction, not on load: `Comp_Video` BeginPlay only
broadcasts `Video_PlayingStateChanged`, and nothing binds it. The actual play path
is `OnRep_Playing -> SetPlay -> getMediaPlayer -> Play`, so an edit-time
`Playing = true` does not start anything.

`/Game/WMCYN/Core/BP_WMCYN_DisplayAutoplay` is a WMCYN-owned actor placed in the
level as `WMCYN_DisplayAutoplay`. On BeginPlay, when not a dedicated server, it
finds every `BP_Display` and calls AFCore's own `Component_Bool_Set(true)` on its
`Comp_Video`. That is AFCore's public state-component entry point, so the bool
replicates and every client plays through the normal AFCore path.

The dedicated-server guard keeps the headless GCP runtime from opening a video
decoder. Note that because `Component_Bool_Set` is authority-routed, a client
call still sets the replicated value on the server; the server has no media
player plugins in a Linux server build, so this is expected to no-op there, but
it has not been verified on the deployed runtime.

## Audio (2026-10-19)

`BP_Display_C_1` was visually playing the stream but silent. Two causes:

1. AFCore binds MediaSound components by **component tag**: `FLib_Utility`
   `getAllRelevantComponents` matches tag `MediaSound`. The placed
   `MediaSound` component on `BP_Display_C_1` had no tag, so AFCore's own
   `AssignMediaSound` never targeted it. Fixed by adding the `MediaSound` tag to
   the component instance; persisted in `L_WMCYNOnline.umap`.
2. The WMCYN `BP_WMCYN_DisplayAutoplay` audio repay sourced its player from
   `MT_Video -> GetMediaPlayer`, but that shared texture's player is `None`
   (AFCore binds a different internal texture). The graph now takes the player
   directly from the cast `Comp_Video` (`Class|CompVideo|GetMediaPlayer`) and
   calls `Media|MediaSoundComponent|SetMediaPlayer` on each found
   MediaSoundComponent at ~1s and ~4s after BeginPlay. Engine-verified via a
   same-graph `GetMediaPlayer` echo (`WMCYN APIECHO VALID after set`).

Note: reflective `mediaPlayer` probes through Unreal MCP returned `None` even
while the engine-side bind was active — treat the C++ API echo as authoritative,
not stale property-panel views across PIE sessions.

## Stream stability warning

The `adultswim-vodlive.cdn.turner.com` HLS stream crashed the editor once
(2026-10-19): Electra decoder assert `BitReader.GetRemainingByteLength() >=
payloadSize` at `ElectraUtilsMPEGVideo.cpp:96`, thread
`ElectraPlayer::Video decoder`, from a corrupt NAL on the wire. Recurrence is
possible while this URL is the source. For long sessions prefer a more stable
stream or a local file; no user control exists yet.

## Verified

Simulate-in-editor: Electra opened the URL, adaptively upswitched 736k -> 1236k ->
1736k -> 2500k bps, and reported `Playback started`. No `LogMedia` or Electra
errors. Capture: `Saved/Diagnostics/Lighting/Display_Stream_PIE.png`.

Audio bind chain executes and is engine-verified (see above); human
headset/speaker confirmation pending.

ElectraPlayer, AndroidMedia and WmfMedia are all already enabled, so no plugin or
packaging change was needed.

## Not Verified

Headset playback, Quest decode cost and frame impact, audio levels and
attenuation, behaviour on the deployed dedicated server, and stream longevity
(a third-party live URL can change or geo-restrict without notice). There is no
user control: the stream starts and runs, by request.
