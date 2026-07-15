# First Signal World Runtime Contract

Status: deployment contract; implementation pending  
Last updated: 2026-07-15

## Product Rule

The WMCYN Crib is one canonical always-on world. Players do not host it, create it, browse sessions, or choose between copies. Firebase authenticates the user and tells the client how to enter the current Unreal runtime. The Unreal server owns the live world, replicated presence, movement, and voice transport.

First Signal capacity is four concurrent presences:

| Slot | Mode | Required capability |
|---|---|---|
| `StandaloneVR_A` | `Quest` | world presence |
| `StandaloneVR_B` | `Quest` | world presence |
| `StandaloneVR_C` | `Quest` | world presence |
| `PCVR_Recording` | `PCVR` | `Recording` |

The current Unreal implementation uses numeric indices `0`, `1`, `2`, and `3` in that order. Remote entry must eventually reserve by mode/slot rather than relying on connection order.

## Canonical Runtime Record

Firebase stores one authoritative document at `worldRuntimes/wmcyn-online`. It represents the currently serving process for the stable logical world `wmcyn-online` and location `wmcyn-crib`.

```json
{
  "worldId": "wmcyn-online",
  "locationId": "wmcyn-crib",
  "runtimeId": "uuid-for-this-server-process",
  "status": "starting|online|draining|offline",
  "host": "public-dns-or-ip",
  "port": 7777,
  "region": "us-east",
  "buildId": "first-signal-001",
  "protocolVersion": 1,
  "capacity": 4,
  "connectedUsers": 0,
  "startedAt": "server timestamp",
  "heartbeatAt": "server timestamp",
  "leaseExpiresAt": "server timestamp"
}
```

- `worldId` and `locationId` stay stable across restarts.
- `runtimeId` changes for every server process/epoch.
- `host` must be internet reachable. Never publish `localhost`, a private LAN address, or an editor-only endpoint.
- Bootstrap returns the runtime only when status is `online`, its lease is current, and the client build/protocol is compatible.
- A stale heartbeat makes the world unavailable. Firebase must not return a stale endpoint as healthy.

## Server Registration And Heartbeat

The Unreal server authenticates with a server-only deployment credential. No Quest or PCVR build receives that credential.

1. On startup, the server calls `POST /v1/vr/runtime/register` with world/location IDs, runtime ID, public endpoint, build/protocol, region, and capacity.
2. Firebase verifies the server credential and atomically replaces the singleton runtime record with status `starting`.
3. After `L_WMCYNOnline` is ready to accept users, the server reports status `online`.
4. The server calls `POST /v1/vr/runtime/heartbeat` every 10 seconds with runtime ID, status, and connected-user count.
5. Each accepted heartbeat extends `leaseExpiresAt` by 30 seconds. A heartbeat with an old runtime ID cannot overwrite a newer process.
6. Graceful shutdown sets `draining`, rejects new joins, then sets `offline`. A crash is detected by lease expiry.

Registration and heartbeat endpoints are idempotent for the same runtime ID and reject client Firebase ID tokens as server authentication.

## Login And Bootstrap

The client flow is fixed:

1. User enters username or email plus password.
2. `POST /v1/auth/login` returns Firebase credentials and verified profile identity.
3. Client calls authenticated `GET /v1/vr/bootstrap` with its `buildId`, `protocolVersion`, and a mode hint of `Quest` or `PCVR`.
4. Firebase checks access, runtime health, compatibility, and available mode/slot capacity.
5. Firebase returns verified identity, the canonical runtime endpoint, a server-authoritative presence reservation, and a short-lived signed join ticket.
6. Unreal connects automatically. There is no session picker or user-facing host/join flow.

Required bootstrap shape:

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
    "runtimeId": "current-runtime-uuid",
    "host": "crib.example.com",
    "port": 7777,
    "buildId": "first-signal-001",
    "protocolVersion": 1
  },
  "presence": {
    "mode": "Quest",
    "slot": "StandaloneVR_A",
    "capabilities": []
  },
  "joinTicket": "short-lived-signed-ticket"
}
```

The ticket expires within 60 seconds and is bound to `uid`, `worldId`, `runtimeId`, `buildId`, protocol version, presence mode/slot, capabilities, expiry, and a replay-resistant ticket ID. The Unreal server validates it before finalizing PlayerState presence. Passwords, Firebase tokens, server credentials, and join tickets must never be replicated or written to normal logs.

Until ticket validation is implemented, internet entry is not security-complete even if direct networking succeeds.

## Reconnect

- On transport loss, freeze locomotion, keep the local identity in memory, and show a small reconnect state rather than returning to a session menu.
- Retry bootstrap with bounded backoff and obtain a fresh join ticket; never reuse an expired ticket.
- If the same runtime is healthy, reconnect to it. If the server restarted, bootstrap returns the new runtime ID for the same logical world.
- The server should reserve the disconnected user's slot for 60 seconds and reclaim it by verified UID when possible. After expiry, normal slot assignment resumes.
- Server restart changes `runtimeId`, not `worldId`. The product remains one world.
- After reconnect, restore authoritative PlayerState identity, nameplate, capabilities, tracking replication, voice registration, and the assigned spawn/last-safe location.

## Failure Responses

- `401`: invalid or expired user authentication.
- `403`: authenticated user lacks WMCYN world access.
- `409`: client build/protocol incompatible or requested mode has no slot.
- `503`: canonical runtime is starting, offline, draining, or heartbeat-stale.

Clients may retry `503` and transient network failures. They must not invent a local world or expose a host button as fallback.

## Remote Acceptance Gate

- Three packaged Quest clients in different locations authenticate and enter slots A, B, and C.
- One packaged PCVR client in a fourth location enters `PCVR_Recording`.
- All four see distinct names and replicated body/head/hand movement.
- All four hear the intended in-game voice path; OBS records PCVR video and reference audio.
- Disconnect/reconnect restores the same verified identity without a session picker.
- A server restart publishes a new runtime ID and clients can bootstrap back into the same logical Crib world.
- Stale or incompatible runtime records never produce a successful join.
