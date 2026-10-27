# Founder Release Runbook

One-click rebuild and publish for founder client builds. Packaging success is
build evidence only; device, login, voice and multi-user behaviour are separate
explicit tests and are never inferred from a green run.

## Rebuild

GitHub Actions -> **Build VR Clients** -> Run workflow.

| Input | Choose |
| --- | --- |
| target | `Both`, `Quest` or `PCVR` |
| configuration | `Development` for iteration, `Shipping` for anything uploaded |
| publish_alpha | leave off unless this build is meant to reach founders |

Pushes to `main` that touch source, plugins, content, config, the uproject or
build scripts start the same workflow automatically as **Development,
artifact-only**. Publication is never automatic. A queued automatic build whose
commit is no longer the tip of `main` is skipped; a build already packaging or
uploading is never cancelled.

The runner is shared with **Build Linux Server** through the `unreal-build`
concurrency group, so only one Unreal build runs at a time.

## Publish to ALPHA

Run the workflow with `target: Quest`, `configuration: Shipping` and
`publish_alpha: true`. The build refuses to upload unless the APK is signed with
the pinned release certificate and carries a v2 signature. On success it writes
`release-reference.json` and preserves the approved release as a separate
90-day artifact, naming the previous approved release as the rollback target.

Invites are a dashboard action and must follow a successful upload: the ALPHA
channel starts empty, and the Administrator role does not grant access.

## Preconditions on the runner

- Unreal must be closed. The workflow fails fast rather than closing it.
- `WMCYN_SIGNING_INI`, `WMCYN_KEYSTORE` and `WMCYN_CERT_SHA256` repository
  variables must point at the runner-only signing files.
- `META_APP_ID` and `META_APP_SECRET` secrets are required only for publication.

## Tested versions

| Component | Version |
| --- | --- |
| Unreal Engine | 5.8, Launcher install at `C:\Program Files\Epic Games\UE_5.8` |
| Android build-tools | 36.0.0 (`apksigner`) |
| Min / target SDK | 29 / 35 |
| Android package | `com.wmcyn.wmcyn_online` |
| Android graphics | Vulkan only; OpenGL ES3.1 disabled |
| Windows graphics | DX11 / SM5 only; SM6 and ES31 shader formats disabled |
| World protocol | 2 |
| Default build id | `first-signal-001` |
| World capacity | 4 |

Five engine versions are installed. Only the 5.8 Launcher install matches the
project; a build against another install can exit 0 without compiling.

## Known constraints

- Meta caps the APK at 1 GB (VRC.Quest.Packaging.5). The build fails above
  1 GiB and warns between decimal 1 GB and 1 GiB, because Meta's wording does
  not say which it means.
- `bPackageDataInsideApk=True`, so no OBB is produced. Going over the cap
  requires turning that off; the build and publish scripts already handle an
  OBB once one exists.
- Quest clients do not show the MetaHuman avatar. The plugin is desktop-only,
  and MetaHuman content is excluded from the Android cook.
- Signing is applied after packaging by `apksigner`, not by Unreal's
  distribution signing, so the keystore is never passed to Gradle.
- The release keystore is unrecoverable. Losing it means a new package name and
  a reinstall for every founder.

## Not covered by a green build

Install, login, display names, movement, menu, voice, two-user visibility and
hearing, and server protocol compatibility. Record these per build; the manifest
reports `hardwareAcceptance: pending` until someone does.
