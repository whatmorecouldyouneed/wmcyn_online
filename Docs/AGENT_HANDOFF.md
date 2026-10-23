# Agent Handoff

## 2026-10-23 First Interview Checkpoint

- Unreal branch `main`, starting HEAD `5d74a0a` (2026-10-14). Over 250 dirty
  status entries already existed, including staged furniture, native source,
  AFCore assets, Mimic source pawn, level, materials and DLLs. Do not reset,
  blanket-stage, or imply this work was all produced in this pass.
- Backend branch `codex/first-signal-backend-bootstrap`; existing avatar route,
  runtime route and publication/service work must remain intact. Mobile untouched.
- Read `FIRST_INTERVIEW_ROADMAP.md` for approved scope and evidence boundaries.

## This Pass

- Backend signs profile-derived username/displayName; placeholder/invalid profiles
  fail bootstrap before reservation. Unreal source switches to protocol 2 and
  binds network identity to verified ticket data. Client names cannot override it.
- Display resolver prefers actual account names over generated AP names.
- Camera source recognizes Mimic's spaced `Right Controller` component name;
  adds three local pose/FOV slots, save/select functions and synthetic assertions.
  Menu exposure, controller diagnosis in headset and operator entitlement remain.
- No AFCore/Mimic assets, map, furniture, avatar assets or config edited this pass.
- Windows had mapped source/doc files; original versions are
  preserved under `Saved/Checkpoints/FirstInterview20261023` before replacement.

## Verification / Deployment

- Follow-up: both native modules linked successfully after editor close. The
  first headless run reached tests but crashed in the synthetic camera harness:
  EndPlay was called without BeginPlay. Harness now balances lifecycle; rerun
  required. This was not a successful hardware or camera rendering test.
- Founder clarified headset microphones only for now. External mic/interface
  and isolated speaker stems are deferred; verify received VOIP mix and local
  operator mic exactly once in OBS, plus co-located echo/latency.

- Backend TypeScript build + full `npm test`: 58/58 pass on 2026-10-23.
- Native UHT + 20 compile actions passed with `-NoLink` on 2026-10-23 using
  Launcher UE 5.8 / MSVC 14.44.35228. Identity/ticket and camera test code compiled,
  but tests have not executed because the loaded DLLs have not been replaced.
  Pre-existing warning: WMCYNRuntime uses HairStrandsCore without listing the
  HairStrands plugin dependency. No new packaged PCVR,
  Quest, Linux server or remote acceptance. Cloud state not queried/deployed.
- Current open editor and project plugin manifest match Launcher UE_5.8 BuildId
  55116800. Earlier C:/UE58-only instructions are historical. Compile-only
  validation uses the matching Launcher engine; save/close before linking DLLs.
  Do not force close, hotpatch or manually change BuildId to hide incompatibility.
- Do not deploy protocol 2 server alone. Deploy additive backend signed identity
  first, then matching protocol 2 Linux server and clients as one release.
  Existing protocol 1 server cannot accept protocol 2 clients and vice versa.
- Backend identity changes checkpointed as `66c78b1` on its existing branch;
  unrelated avatar/backend work remains unstaged and preserved. No push/deploy.
- Unreal implementation remains mixed with existing source work and needs a
  reviewed scoped checkpoint. Roadmap/handoff/release docs are separately scoped;
  do not claim the worktree is clean or the implementation has been pushed.
- Attempted docs-only checkpoint was blocked twice by a concurrent Git index
  lock (active Git processes observed). Left lock and existing index untouched.
  New docs remain uncommitted; retry after that operation finishes.

## Next Exact Task

After editor closes, link WMCYNRuntime + WMCYNRuntimeEditor with the verified
matching Launcher UE_5.8 engine (recheck manifest/association first).
Run identity/ticket and handheld-camera automation. Fix failures before reopening.
Then expose three shot controls in the existing styled menu and implement the
server-derived Recording capability gate before production use. Do not rework
Jared IK or start seating offsets while the native checkpoint is unverified.

Pending: real two-account spoof/respawn/reconnect test; matching packages;
two-seat adapter; stage rights/placement; microphone inventory; real OBS rehearsal.
