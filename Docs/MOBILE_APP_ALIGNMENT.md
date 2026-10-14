# Mobile App Alignment

Date checked: 2026-10-14

## Repositories Checked

- Mobile app path: `C:/Users/jvred/Documents/WMCYN/wmcyn-mobile-app/wmcyn-mobile-app`
- Local mobile checkout: detached at `e0be1ee` (`docs: handoff to the wmcyn_online UE 5.8 project`) with modified `package-lock.json` and `yarn.lock`; this checkout is stale and must not be treated as the mobile source of truth.
- Mobile remote `origin/main`: native Swift/SwiftUI client, with Expo removed by `5fd5f14` and the current TestFlight build-number follow-up at `0b271d9`.
- Backend remote `origin/main`: `6ddce4f`, with the avatar manifest and pairing/runtime contracts present.

## What Lines Up

### Identity and Firebase

The mobile app and Unreal use the same Firebase project and production API base. Mobile Firebase Auth can sign in with email/password or a Firebase custom token. Unreal's WMCYN runtime already has the matching custom-token sign-in and verified bootstrap path.

### Pairing Contract

The shared backend flow is:

1. `POST /v1/auth/code.create` creates a short-lived six-character code.
2. An authenticated companion/browser client calls `POST /v1/auth/code.attach` with that code.
3. The VR client polls `POST /v1/auth/code.exchange`.
4. The response returns a Firebase custom token; the client exchanges it for an ID token and continues through world bootstrap.

The native iOS client has the current pairing flow under `native-ios/Sources/Pairing/` and uses the shared six-character code exchange contract. The Unreal runtime has the corresponding async pairing action. The remaining work is validating the final display/attach direction against the native client and completing the in-headset widget hookup.

## Mobile Integration Status

Native Swift/SwiftUI is now the official mobile client on the organization's `main` branch. The former Expo/React Native app was intentionally retired after native feature parity. The native client owns consent, TrueDepth capture/import, upload, status polling, preview, publish, delete, pairing, and the companion product surfaces.

The in-progress iOS client sends Firebase bearer tokens for avatar endpoints and models the capture as a Live Link Face/TrueDepth archive. That matches the backend's authenticated avatar routes and the Unreal workstation pipeline, but it does not mean that a runtime-ready MetaHuman bundle is already published for a user.

## Avatar Alignment

- Backend `origin/main` now exposes `GET /v1/avatar/manifest?platform=quest|pcvr`.
- A user without a published avatar receives a default-avatar manifest instead of a 404.
- The manifest supports `runtime_mesh` and `pak` delivery shapes.
- Unreal has manifest fetch/apply plumbing and verified WMCYN-owned exported MetaHuman assets, but the non-default publish -> login -> apply proof is still open.
- Quest and PCVR currently share the verified exported paths; no Quest-specific optimization is claimed yet.

## Mismatches To Resolve

- The local mobile checkout is detached from the current native-only `origin/main` and has modified lockfiles. Preserve those files before creating a clean checkout; do not reset the working copy blindly.
- The old Expo pairing-screen wording is no longer authoritative. Finalize Unreal's display/attach behavior against `native-ios/Sources/Pairing/` and the live backend contract.
- The mobile app proves identity transfer, not shared-world presence. World discovery, join tickets, slot assignment, runtime reconnect, nameplates, voice, and OBS remain Unreal/runtime acceptance gates.

## Recommended Next Checkpoint

1. Create a clean local checkout from the current native-only mobile `origin/main`; preserve the detached checkout's modified lockfiles separately.
2. Keep the backend pairing contract unchanged and settle the display/attach direction against the native client.
3. Test one real mobile account through pairing into Unreal.
4. Test one published avatar manifest through PCVR before making Quest-specific output claims.
