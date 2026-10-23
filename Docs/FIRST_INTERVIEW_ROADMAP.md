# First Interview Readiness

Approved implementation roadmap, 2026-10-23. Read `AGENT_HANDOFF.md` for the
current implementation checkpoint. This document supersedes old LAN/listen-host
acceptance instructions, not the historical test evidence.

## Production Decisions

- Interviewer and guest together at HQ, in the existing couch/banner/rug set.
- One canonical internet Crib. No participant-hosted rooms or access codes.
- Three standalone Quest users and one PCVR operator remain the world target;
  the first recording gate is two Quest participants plus PCVR operator.
- One switched video program: master, interviewer, guest. OBS owns recording.
- Headset microphones only for now (founder correction, 2026-10-23). Record the
  received in-world voice mix; do not assume external microphones or isolated
  participant tracks. Prepared backing track plus live vocals still needs a
  quality/latency rehearsal. External clean recording is a later upgrade.
- Proven default Mimic avatar is the fallback. Preserve Jared; full scan delivery
  and avatar fidelity do not block an episode.
- Preserve AFCore and Mimic assets, tracking, calibration, locomotion and collision.

## Current Evidence

- User confirmed Jared tracking and locomotion in local VR Preview after the
  October 22 rebuild. Outfit/shoe leader-pose and groom components now exist.
  Appearance completeness, fresh packaged and remote results are unverified.
- Jared is an approved PCVR cooked-in account catalog selection, not proof of
  scan-to-runtime delivery. `delivery.url` is parsed but not downloaded/mounted.
- Historical packaged Quest/PCVR runs proved mutual visibility and hearing.
  They do not certify the changed October 23 source or all four users together.
- Camera already has one shared 640x360 target, local grip/release, recall, FOV
  and spectator routing. No encoder or verified production recording exists.
- Native Mimic remains the player. AFCore BP_Chair inspection did not establish
  a usable seat system; seated calibration alone is not a seated avatar pose.
- Current editor set includes couch, banner, rug and workstation; flower meshes
  and placement have reports. Do not relocate or redress the approved set.
- Mobile/backend upload, jobs, publication and ownership exist. The inspected
  detached mobile checkout has Live Link Face upload, not verified current
  unmerged TrueDepth work. Latest cloud publication state was not queried.

## Readiness Matrix

Effort is engineering time excluding hardware coordination. A code change is
not an acceptance result.

| Area | Priority / approach | Acceptance and dependencies | Estimate / confidence |
| --- | --- | --- | --- |
| Login / world | P0, Firebase refresh/bootstrap/tickets | Matching server and clients; two accounts plus operator, reconnect | 1-2 days / medium |
| Identity | P0, signed profile names -> PlayerState/AFCore | No AP/UUID labels; spoof attempts fail; names survive respawn/reconnect remotely | 1-2 days / high |
| Avatars | P0 default, P1 Jared | Default works; Jared outfit/materials/remote/cook separately pass; no inferred Quest support | 0.5-1 day default, 2-4 Jared / medium |
| Seating | P0, WMCYN adapter | Two server-reserved anchors, proximity, sit/stand, disconnect release, head/hands preserved | 3-5 days / medium-low |
| Camera | P0, one capture / three poses | Grip/release/recall, seated/standing, door exclusion, no headset changes | 2-3 days / medium |
| Audio | P0, headset microphones / VOIP mix | Both voices audible, echo/monitoring check, operator mic captured once; music separate where routing permits | 1-2 days / medium |
| Recording | P0, OBS switched spectator | Measured 1080p30, 30-minute file playback, sync, no sustained drops | 1-2 days / medium |
| Set | P0 readiness, P1 polish | Seated sightlines, route downstairs, collision, autoplay safety | 1-2 days / medium |
| Stage / screen | P0 minimal approved media | Rights/placement, performer mark, access, light, synced track and recorded performance | 2-4 days / medium-low |
| DJ | P1 props, P2 live transport | Rights/placement first; meshes are not audio transport | 1-2 days visual / low |
| Avatar preview | P1, equipped visual only | Capture only while open, rotate preview not pawn, Quest performance | 2-3 days / medium-low |
| iPhone VCam | P2 isolated spike | Existing capture integration; packaged network proof separate from PIE | 2-3 days / medium-low |

## Sprint Order

1. **Identity and checkpoint:** preserve dirty work, bind server names to signed
   backend profile, reject placeholders, coordinate protocol/build rollout.
2. **Recordable camera:** repair controller/grip path, expose three save/select
   shot slots in existing menu, test short OBS take with real microphones.
   Static presets remain useful if handheld polish slips.
3. **Two-person seating:** one couch seat locally, then two exclusive server
   occupants with remote pose visibility. Gaming chair uses same adapter later.
4. **Performance:** approve stage and asset rights; disable unstable autoplay;
   one preloaded prepared track plus headset vocals; assess quality and latency.
5. **Rehearsal:** internet-connected two Quest + PCVR, 30-minute conversation,
   downstairs transition, performance, reconnect, file playback and backup.

Identity and camera source work can proceed independently. Final camera framing
depends on seating. Stage import waits for approval. Never disable cloud cost
controls globally; check runtime lease/idle policy before the scheduled rehearsal.

## Contracts

**Identity:** Firebase account profile supplies username/displayName in the
HMAC-signed ticket. Protocol 2 server validates and retains that identity per
controller, applies it after pawn creation/respawn, and does not accept arbitrary
network client names. Local standalone preview is not online authentication.

**Seats:** server validates proximity and occupancy, replicates seat ID and
occupant, releases on stand/disconnect. WMCYN presentation adapter preserves
Mimic head/hand tracking. Never lower the headset to fake sitting.

**Camera:** local operator prop, one render target/capture. World-space shot
poses and FOV. Production controls require server-derived Recording capability;
local ownership/Windows checks alone are not entitlement. No replicated prop yet.

**Media:** approved track ID, playing/paused state and authoritative start time;
clients preload/seek/correct drift. Server coordinates without media decoding.
A replicated boolean alone does not synchronize a backing track.

**Recording:** OBS captures spectator program and headset voice mix; slate,
take naming, free-space check, playback and backup. 640x360 upscaled to 1080p is
not production acceptance. No in-game encoder or recording backend this sprint.

## Acceptance Record

For every test record: layer, date, device, engine/client/server build, scenario,
result, evidence location, remaining issue. Layers are **compile**, **Editor**,
**headset**, **packaged PCVR**, **standalone Quest**, **remote multiplayer**.
Do not translate an Editor or synthetic result into device acceptance.

- [ ] Same runtime; verified names after login, respawn and reconnect.
- [ ] Altered name RPC cannot change public identity; invalid tickets rejected.
- [ ] Two occupied couch seats; no double reservation; safe stand/disconnect.
- [ ] Usable default avatars; optional Jared accepted independently.
- [ ] Three selectable shots; framing fixed during movement/menu use.
- [ ] Remote avatars visible in preview and OBS spectator capture.
- [ ] Both headset voices and backing music audible without doubled VOIP; no
  claim of isolated speaker stems from a mixed game-audio capture.
- [ ] Safe route downstairs and short prepared-track performance recorded.
- [ ] 30-minute take playable, synchronized, backed up, editable audio/video.

## Remaining Founder Inputs

Exact stage/DJ placement and approved model/media rights. Audio hardware is
headset microphones for now. `stage.fbx`, `HandMIC.fbx` and VDJ booth meshes are candidates only;
no confirmed reuse license from inspected folders. Omit bowling and smoking props.

P1: bounded Jared appearance, set polish, gaming chair, preview/production UI.
P2: iPhone camera, shared camera ownership if needed, live DJ transport.
P3: automated scan delivery, cloud asset streaming, advanced customization.

## Evidence and References

- `Plugins/WMCYNRuntime/Source/WMCYNRuntime/Private/WMCYNFirstSignalPresenceComponent.cpp`
- `Plugins/WMCYNRuntime/Source/WMCYNRuntime/Private/WMCYNCameraToolComponent.cpp`
- `Docs/JARED_ACCOUNT_AVATAR_2026_10_20.md`, `Docs/AVATAR_WORKSTATION_RUNBOOK.md`
- Backend `functions/src/routes/vr.ts`, `services/joinTicket.ts` and `worldIdentity.ts`.
- [OBS isolated audio tracks](https://obsproject.com/kb/multiple-audio-track-recording-guide)
- [Epic VCam](https://dev.epicgames.com/documentation/unreal-engine/virtual-cameras-in-unreal-engine?lang=en-US)
