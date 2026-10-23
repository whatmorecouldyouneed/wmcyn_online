# First Interview Release and Rehearsal

Protocol-2 checkpoint, 2026-10-23. This is a pending acceptance checklist, not a
deployment report. No live backend or runtime was changed by this checkpoint.

## Matched Release

1. Review dirty work by owner. Preserve pre-existing AFCore/Mimic/map changes;
   never use a blanket commit to hide them inside an identity fix.
2. Build/test backend. Ensure target accounts have a valid profile handle,
   username or displayName. Missing/placeholder-only profiles return
   `profile_identity_incomplete`, not a technical ID or email label.
3. Deploy additive signed username/displayName ticket fields first. Smoke-test
   bootstrap without logging its token, ticket, credentials or response body.
4. Build matching protocol-2 Linux server, PCVR and Quest clients from the same
   reviewed source checkpoint. Record commit, build ID and artifact hashes.
   Do not change the world-entry ticket's HMAC envelope from v1: protocolVersion
   inside the payload is 2; these version numbers serve different purposes.
5. Schedule runtime replacement and distribute clients together. Protocol-1
   clients cannot enter the protocol-2 runtime. Roll back server and clients
   together if necessary; never weaken ticket verification for compatibility.
6. Verify registration/heartbeat, canonical runtime ID and idle shutdown policy.
   Do not leave paid compute running after tests or disable idle controls globally.

## Identity Test

- Fresh login and remembered login both request new entry tickets.
- Two different accounts plus operator enter the same runtime.
- Both peers see profile names in the tag and roster; no AP/UUID labels.
- Respawn and reconnect retain identity. Logout releases controller identity.
- Modified name RPC arguments do not change server PlayerState or remote labels.
- Invalid, expired, replayed, wrong-runtime and wrong-protocol tickets fail.
- Default account remains default; no cross-account avatar reassignment.

## Camera Test

- Current shot API: SaveShot/SelectShot, slots 0 master, 1 interviewer, 2 guest.
  Slots are local to the pawn lifetime, not durable saved production presets.
- Menu exposure and trusted Recording entitlement are required before release.
- Grab/aim/release/recall seated and standing; no door also opens on grip.
- Save all shots; select each while moving and opening/closing menu. Pose and
  FOV remain fixed; headset view stays tracked. No second capture is allocated.
- Disabled camera stops capture; selecting a shot cannot silently reactivate it.
- Check lens/display orientation against OBS output and remote avatars.
- Measure GPU/frame timing with capture off/on. Current 640x360 is interaction
  validation only; choose actual recording resolution from measured headroom.

## Audio and Take

- Headset microphones only, per founder correction on 2026-10-23. Verify the
  operator receives both participants in game audio and OBS records that mix.
  If the operator also speaks, capture their headset mic exactly once; local
  voice capture alone does not prove that voice is present in the recorded mix.
- Keep backing music separate where routing permits. Do not expect to split
  mixed VOIP into isolated participant tracks later. External microphones deferred.
- Check co-located echo, headphone monitoring and whether VOIP is doubled in OBS.
- Use approved local backing media, no unstable network autoplay for rehearsal.
- Check free disk space, take name/date, recording path and a sync clap/slate.
- Short record/playback first, then 30 minutes at measured 1080p30 target.
- Verify switched shots, intelligible headset speech/music sync, no sustained
  dropped frames; play the actual output files and copy a backup.

## Test Record Template

Date / tester / layer / device / engine / client commit+artifact / server
commit+artifact / protocol / scenario / pass-fail / sanitized evidence / next fix.
Use separate entries for compile, Editor, headset, packaged PCVR, standalone
Quest and remote multiplayer. Leave unrun gates explicitly pending.
