# WMCYN Backend Integration Audit

## Summary

WMCYN's Firebase backend is the identity/bootstrap service for First Signal. It accepts username or email plus password and returns verified identity with the canonical Crib runtime endpoint. Firebase does not host the live world: one authoritative, always-on Unreal server supplies it. Players never create or host sessions.

## Sources Inspected

- Local folder: `C:/Users/jvred/Documents/WMCYN/wmcyn-backend-infra`
- Upstream: `https://github.com/whatmorecouldyouneed/wmcyn-backend-infra`
- Audited upstream commit: `b2f260bc5916ced61b17f98eeb9d4f5a93a93337`
- Related WMCYN web and mobile authentication clients
- No `.env.local` values or credentials were read.

The local backend checkout was repaired from upstream `main` at `b2f260b` while preserving existing local CORS changes. Active implementation is on `codex/first-signal-backend-bootstrap`. `npm ci`, ESLint, TypeScript compilation, and the focused login-identifier tests pass.

Dependency installation reported 25 audit findings: 1 low, 14 moderate, 8 high, and 2 critical. Do not run an automatic force upgrade; review the dependency graph and upgrade deliberately.

The live API currently returns `500/503` because Firebase billing is inactive. This is an expected infrastructure state confirmed by the project owner, not a source-build failure.

## Existing Capabilities

- Firebase Admin ID-token verification through Bearer authentication.
- `GET /v1/profile/me` returns UID, username, display name, avatar, email, roles, access, and timestamps.
- `GET /v1/vr/profile` returns a small Unreal-oriented identity payload.
- Firestore user profiles already carry `handle`, `displayName`, `roles`, and `hasAccess` conventions.
- Existing web/mobile clients use Firebase `signInWithEmailAndPassword` and retrieve Firebase ID tokens.
- A six-digit headset pairing/custom-token flow exists, but it is not the approved WMCYN Online login flow.
- AR scenes, anchors, live streams, product data, and other mobile services exist but do not represent an Unreal multiplayer runtime.

## Implemented First Signal Contracts

### Credential Exchange

`POST /v1/auth/login` accepts `{ "identifier": "...", "password": "..." }`. `identifier` may be a Firebase email, username, or `@username`. Username login resolves a normalized handle through server-only `loginHandles/{handleNormalized}` and then uses Firebase Identity Toolkit email/password exchange. The response contains Firebase tokens plus verified profile identity; credential failures use a common response to reduce account enumeration.

Initial users are created manually. See `Docs/FIRST_SIGNAL_USER_PROVISIONING.md` for the Firebase Authentication, `users/{uid}`, and `loginHandles/{handleNormalized}` records.

### VR Bootstrap

`GET /v1/vr/bootstrap` is Bearer-authenticated and combines verified identity with world entry data. Its implemented shape is:

```json
{
  "identity": {
    "uid": "firebase-uid",
    "username": "jared",
    "displayName": "Jared",
    "roles": [],
    "hasAccess": true
  },
  "world": {
    "worldId": "wmcyn-online",
    "locationId": "wmcyn-crib",
    "runtimeId": "first-signal-runtime",
    "host": "example-host",
    "port": 7777,
    "buildId": "first-signal-001"
  },
  "joinTicket": null
}
```

The Firebase ID token authenticates the API request and remains private in the Unreal GameInstance subsystem. The currently implemented response still returns `joinTicket: null`; a short-lived Unreal join ticket is required before secure internet entry can pass. Neither value may be replicated to other players.

### Unreal Runtime Registry

`Docs/FIRST_SIGNAL_WORLD_RUNTIME_CONTRACT.md` is now the normative contract. The backend must maintain exactly one current `worldRuntimes/wmcyn-online` record with stable world/location IDs, per-process runtime ID, public host/port, region, build/protocol compatibility, four-user capacity, status, connected-user count, heartbeat, and lease expiry.

The Unreal server registers with a server-only credential, reports ready state after `L_WMCYNOnline` loads, heartbeats every 10 seconds, and receives a 30-second lease. Stale runtimes are unavailable. Remote acceptance does not use a LAN address, player-hosted listen server, session browser, or endpoint picker.

## Security Gate Status

1. Completed: unauthenticated `/debug/env` removed.
2. Completed: `x-uid` is accepted only with the Firebase Auth emulator and explicit `ALLOW_DEV_UID_AUTH=true`.
3. Completed: rate limiting restored for public/admin/auth/pairing routes.
4. Completed: `/users`, `/profiles`, `loginHandles`, and `worldRuntimes` rules no longer expose sensitive records publicly.
5. Completed: anonymous admin mutation access removed while retaining the compatibility helper name.
6. Completed: live routes use verified request identity.
7. Completed for manual First Signal provisioning: normalized handles use a server-only lookup index. A transactional public handle-claim flow remains out of scope.
8. Pending: review and update the 25 dependency audit findings without an automatic force upgrade.
9. Completed for this pass: `.env.example` documents non-secret configuration; HTTP config dumping is removed.

## Unreal Integration

Refactor the existing WMCYN login gate into these states:

1. Awaiting credentials
2. Authenticating
3. Loading verified profile/bootstrap
4. Connecting to the Crib runtime
5. Entered world
6. Error with retry

The current login widget, AFCore keyboard, and locomotion lock remain. `UWMCYNBackendSubsystem` implements authentication/bootstrap with private token storage and a `-WMCYNBackendUrl=` override. The existing local PlayerState submission is an explicit PIE fallback only. The active login Blueprint is connected to the asynchronous native bridge.

The compiler gate is closed: UE 5.8 builds `WMCYNRuntime` with MSVC `14.50.35717` / compiler `14.50.35737`.

## Implementation Order

1. Restore Firebase billing and re-run real login/bootstrap against the hardened API.
2. Implement server-only runtime registration, ready state, heartbeat lease, and stale-runtime rejection for `worldRuntimes/wmcyn-online`.
3. Add build/protocol compatibility plus short-lived signed join-ticket issuance and Unreal-side validation.
4. Deploy one internet-reachable authoritative Unreal runtime for `L_WMCYNOnline`.
5. Connect three remote Quest clients and one remote PCVR recording client through automatic bootstrap.
6. Prove reconnect to the same logical world across transport loss and server-process restart.
