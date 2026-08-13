# Backend Handoff: Avatar Manifest and UE Integration

Last updated: 2026-07-28

## Purpose

This doc tells the backend what the Unreal client expects for avatar bootstrap after login.

The missing backend piece is the active avatar manifest returned to a logged-in WMCYN client.

## Current Backend State

Already live or already verified outside Unreal:

- `POST /v1/avatar/scan.uploadUrl`
- `POST /v1/avatar/scan.create`
- `GET /v1/avatar/scan.status`
- `POST /v1/avatar/publish`
- `DELETE /v1/avatar`
- pairing code create/exchange

Still missing for Unreal:

- `GET /v1/avatar/manifest`

## Endpoint

`GET /v1/avatar/manifest?platform=quest`

or

`GET /v1/avatar/manifest?platform=pcvr`

Auth:

- required
- `Authorization: Bearer <Firebase ID token>`

## Why platform-specific

The Unreal client should ask for exactly one resolved variant:

- Quest client gets Quest-safe output
- PCVR client gets PCVR output

This avoids downloading or describing the wrong platform assets.

## Required Query Params

- `platform`: `quest` or `pcvr`

Optional future params:

- `avatarVersionHint`
- `deliveryPreference`

Do not block initial implementation on those optional params.

## Success Response

Return `200 OK` with a resolved manifest, even when the user has no custom avatar yet.

Example:

```json
{
  "manifestVersion": 1,
  "avatarId": "avt_123",
  "avatarVersion": 7,
  "platform": "quest",
  "isDefaultAvatar": false,
  "displayName": "Jared",
  "delivery": {
    "type": "runtime_mesh",
    "url": "https://cdn.example/avatar/avt_123/quest/runtime_mesh_v7.zip",
    "sha256": "sha256:abc123",
    "sizeBytes": 12345678,
    "mountPoint": null,
    "assetRoot": "/Game/WMCYN/Avatars/Runtime/avt_123_v7",
    "skeletalMeshPath": "/Game/WMCYN/Avatars/Runtime/avt_123_v7/SK_Avatar",
    "headSkeletalMeshPath": "/Game/WMCYN/Avatars/Runtime/avt_123_v7/SK_Avatar_Head",
    "animClassPath": "/Game/WMCYN/Avatars/Runtime/Common/ABP_WMCYN_Avatar",
    "dnaUrl": null,
    "dnaSha256": null
  },
  "generatedAtUtc": "2026-07-28T00:00:00Z",
  "cacheKey": "avt_123_v7_quest",
  "expiresAtUtc": "2026-07-29T00:00:00Z"
}
```

## Delivery Types

The contract must support both:

- `runtime_mesh`
- `pak`

Interpretation:

- `runtime_mesh`: Unreal downloads/imports runtime avatar content using a WMCYN-owned runtime path
- `pak`: Unreal downloads a cooked bundle and mounts it

The backend should not assume only one forever.

## Default Avatar Behavior

When the user has no published avatar, return `200` with a default manifest:

```json
{
  "manifestVersion": 1,
  "avatarId": "default",
  "avatarVersion": 1,
  "platform": "quest",
  "isDefaultAvatar": true,
  "displayName": "Default",
  "delivery": {
    "type": "pak",
    "url": "https://cdn.example/avatar/default/quest/default_v1.pak",
    "sha256": "sha256:def456",
    "sizeBytes": 9876543,
    "mountPoint": "../../../wmcyn_online/Content/Paks/DownloadedAvatars/default_v1/",
    "assetRoot": "/Game/WMCYN/Avatars/Default/Quest",
    "skeletalMeshPath": "/Game/WMCYN/Avatars/Default/Quest/SK_DefaultAvatar",
    "headSkeletalMeshPath": "/Game/WMCYN/Avatars/Default/Quest/SK_DefaultAvatar_Head",
    "animClassPath": "/Game/WMCYN/Avatars/Runtime/Common/ABP_WMCYN_Avatar",
    "dnaUrl": null,
    "dnaSha256": null
  },
  "generatedAtUtc": "2026-07-28T00:00:00Z",
  "cacheKey": "default_v1_quest",
  "expiresAtUtc": "2026-07-29T00:00:00Z"
}
```

Do not use `404` for "no avatar yet".

## Error Cases

Use normal auth/contract failures only:

- `401` invalid or missing auth
- `400` invalid platform
- `500` internal manifest/build state failure

Avoid `404` for uncustomized users.

## Backend Data Expectations

The backend should track, per published avatar:

- active avatar id
- current avatar version
- availability by platform (`quest`, `pcvr`)
- delivery type
- artifact URL
- hash/checksum
- size
- optional DNA sidecar URL/hash
- generated timestamp
- cache key

## Unreal Expectations

The UE client will:

1. authenticate
2. fetch manifest for the current platform
3. compare `cacheKey`
4. skip download if cached and matching
5. otherwise download and apply
6. fall back to default-avatar behavior if the returned manifest says `isDefaultAvatar = true`

## Out of Scope for First Implementation

- public self-serve avatar generation scale
- multi-variant differential patching
- granular LOD manifest fan-out
- public marketplace-style CDN invalidation logic

The first goal is one working resolved manifest per platform.
