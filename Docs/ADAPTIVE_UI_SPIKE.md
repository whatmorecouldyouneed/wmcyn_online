# Adaptive Tint / Spatial Depth UI Spike

Date: 2026-10-16
Status: user approved priorities 1-4. WMCYN-owned implementation pilot is underway; see `ADAPTIVE_UI_IMPLEMENTATION.md` for verified progress and open gates.

Priority: persistent sign-in remains the first functional priority (see PERSISTENT_SIGN_IN.md); real-account restore and Quest secure storage are still pending. Menu work does not close those authentication gates. The candidate tables below retain the original research proposal; they are not implementation acceptance claims.

## Scope

Apply a consistent, readable visual treatment to the existing WMCYN login, runtime menu, roster, and settings. This is a presentation pass, not a login, input, or framework rewrite. Next.js is deferred. There is no storefront in this pass. Remembered sign-in remains a separate functional task; a visual refresh does not implement it.

Preserve native Mimic tracking and movement, Firebase authentication and world-entry verification, controller pointers, AFCore keyboard dispatch, Quest keyboard behavior, locomotion gating, roster identity, and audio Apply/Reset/respawn behavior. Do not edit AFCore or Mimic assets.

## Observed Reuse

The existing AFCore reuse matrix records these integrations. The persistent-sign-in pass inspected login/settings hierarchy and added Designer-visible controls; the remaining surfaces still need live inspection before visual changes:

| Existing surface | Preserve | Allowed visual change |
| --- | --- | --- |
| WBP_WMCYN_LoginJoin | Native editable fields, credential submission, pairing, AFCore keyboard integration | Background, typography, spacing, button states |
| WBP_WMCYN_RuntimeMenu | Existing open/close action and navigation | WMCYN-owned surface and navigation styling |
| WBP_WMCYN_WhosHere | AFCore Players/PlayerField reuse | WMCYN-owned outer styling |
| WBP_WMCYN_Settings_FirstSignal | Native WMCYN settings class and bindings | Outer styling without changing its parent |
| WBP_WMCYN_Settings_Audio | AFCore Audio and Apply/Reset reuse | Overrides supported by the existing theme path |

The native settings class uses AudioSettings and RespawnButton bindings, with an optional SignOutButton binding added for remembered sign-in. Login remains a child of AFCore Widget_Base; settings retains its native WMCYN parent. Blanket reparenting risks losing behavior. Prefer composition and existing theme hooks.

## Platform Research

Reviewed 2026-10-16 using product documentation, project-hosted wikis and developer announcements. This is desk research, not a hands-on headset comparison. Recommendations below are WMCYN design judgments, not claims that another platform supplies an Unreal-compatible plugin. Wiki pages can change and some are incomplete.

| Platform | Documented pattern | WMCYN opportunity | Boundary |
| --- | --- | --- | --- |
| VRChat | Separate Quick Menu and Action Menu; the latter supports directional selection and optional trigger confirmation. Camera controls include mode, zoom, reset and contextual options. [V1-V3] | Keep common actions shallow; put detailed camera controls with the camera rather than among account settings. | Borrow interaction principles, not its world/instance browser, avatar-expression system or entire radial input scheme. |
| Basis VR | Settings document rebinding/reset, microphone modes, explicit input modes and an in-game logging console. Its framework is Unity/C#, with an MIT-licensed core. [B1-B3] | Clear control mappings, reliable Reset, and a small developer status panel would reduce testing friction. | Not a UE plugin or replacement for AFCore/Mimic. Inspect individual dependency licenses before any code reuse. |
| Resonite | Facets customize Dash/anchors; equipped tools change contextual actions. Dev Tool exposes scene inspection and gizmos. [R1-R3] | Small reusable tool panels and context-specific camera controls; a restricted developer toolbox for approved test actions. | Do not build a runtime world editor, scripting environment or arbitrary object inspector for First Signal. |
| NeosVR | Legacy UIX documentation describes reusable canvases, layouts, buttons and sliders underpinning Dash and Scene Inspector. [N1] | Maintain a WMCYN UI component/state gallery so visual and input regressions are easy to reproduce. | Historical design reference only. This review does not verify current service availability or supported SDK integration. UIX is not UMG. |
| Helios | Developer notes describe per-user volume, camera view options and progressive audio UI replacement. The developers announced indefinite suspension and January 15, 2024 shutdown. [H1] | Incrementally replace surfaces while preserving behavior; test volume persistence and camera output explicitly. | Assumes the social VR product Steam app 1047640, not similarly named products. Historical Unreal reference, not a maintained dependency recommendation. |

### Proposed Menu Shape

Keep right-controller B as the existing open/close entry point. Do not introduce hold/double-tap gestures or change controller bindings during the style pilot. The first panel is placed predictably in reach/view, not attached to a moving fingertip. Preserve pointer orientation and keyboard focus behavior.

Suggested structure, pending approval:

| Surface | Contents | Existing integration to retain |
| --- | --- | --- |
| Quick actions | Mic toggle/status, Mirror, Respawn; Camera once its existing path is audited | Existing voice, mirror, respawn and camera handlers, not replacement systems |
| Who's Here | Display names; select a person for local mute/volume if the current voice path supports it | AFCore Players/PlayerField plus verified WMCYN identity |
| Audio | Master/voice controls and tested Apply/Reset; actual mic state | Existing WMCYN settings wrapper and audio bindings |
| Tools | Camera controls; later interview utilities | Existing production tooling after inspection |
| Settings | Readability/reduced motion preferences, account/sign-out; PC-only options only on PC | Existing settings and secure sign-in implementation |
| Development (non-shipping) | Status, approved test locations, respawn, sanitized diagnostics, test preset indication | WMCYN-owned helpers; no arbitrary server console or asset editing |

One persistent Crib remains the product model. No host controls, access codes, lobby browser or create/join-session menu. A hidden transport session is not a user-facing world choice. Signing out is separate from closing the menu.

### Tool Candidates And Effort

Relative effort is provisional until existing handlers are inspected; it is not a delivery estimate. Every row is proposed, not implemented by this spike.

| Candidate | Priority / effort | Benefit and prerequisite |
| --- | --- | --- |
| Consistent surfaces + UI state gallery | Pilot / small-medium | One Designer-visible preview of fields, buttons, toggles, sliders and error/focus states before migrating live screens. |
| Quick mic, mirror and respawn actions | Next / small-medium | Fewer steps for repeated headset checks. Reuse working handlers; verify mirror render cost and mic toggle semantics. |
| Connection/status strip | Next / medium | Distinguish signed in, connecting, world connected, reconnecting, muted, permission denied and capture active. Derive from real events; capture active does not prove remote users hear audio. |
| Local per-person mute/volume | Next / medium | Useful for interviews and echo troubleshooting. Requires stable identity-to-talker mapping and an audit of the AFCore/voice APIs; keep changes local to the listener. |
| Development presets + diagnostics | Separate workflow pass / medium | Local environment, Online PCVR, Login regression; copy sanitized build/runtime facts. Offline identity must never enter the online world. Force-login must exercise normal server verification. |
| Camera tool panel | Later / medium-large | Open/close, reset pose, framing/zoom and explicit photo versus PCVR output controls. Inspect existing camera first; measure capture cost and verify Quest save path/permissions. Do not claim OBS is recording merely because output is enabled. |
| Pin or reposition a tool panel | Later experiment / medium | Helpful during an interview, but needs recover-to-default placement, seated reach and occlusion testing. |
| Wrist/radial menu, editable dashboard, live scene inspector | Defer / large | Introduces input conflicts, accidental actions and maintenance beyond current needs. No general-purpose creator tools in this pass. |

### What Adaptive Means Here

Recommendation: adapt to context and capabilities, not continuously sample the world to change menu colors. Use fixed high-contrast tokens, bounded focus/press transitions and restrained layering. Quest gets only supported controls; PCVR may expose capture options. Development-only controls never appear in shipping builds. Pending operations disable duplicate submission and expose failure/retry states without leaking credentials.

Use symbols with hover/focus labels for tools, sliders for levels, toggles for binary states and tabs for views. Do not depend on color alone for mic/error state. Keep hit areas stable across changing labels, long names and loading states. Avoid nested decorative panels, continuous gaze-following motion and translucent layers behind text.

## Reuse And Validation Plan

1. Inventory each proposed action's existing WMCYN/AFCore handler, ownership, input binding and platform support. Unknown support stays unknown, not replaced speculatively. Start from AFCORE_MVP_REUSE_MATRIX.md and the live widgets.
2. Keep UMG Designer-visible composition, existing widget parents and named bindings. Use WMCYN-owned theme/material assets. Do not introduce a new UI framework, marketplace dependency or generalized action registry without demonstrated need.
3. Build an isolated style/state gallery only after approval. Compare opaque and near-opaque surfaces in both dark and bright Crib areas. Record panel dimensions/distance, render target size, frame timing and screenshots on the actual devices.
4. Pilot login, then runtime menu. Run remembered-login and forced-login regressions, pairing, continuous keyboard typing, field switching, Enter/close, menu reopening, movement gating, audio, nameplates and respawn. Preserve native Mimic seated behavior.
5. Test local controls with remote peers: mute/volume must affect only the intended listener/talker, opening menus must not alter other users, and server-authoritative account/avatar rules must remain unchanged. Never infer remote success from a local UI indicator.
6. Set a measured performance budget from the existing menu baseline before implementation. Capture menu closed/open and mirror/camera on/off separately on Quest and PCVR; do not spend the Quest budget on decorative blur or always-running scene captures.

## Approval Proposal

Recommended first approval: **style/state gallery, then a visual-only login/menu pilot** using existing behavior. Keep persistent-sign-in acceptance separate and ahead of the live migration.

Second approval: **quick actions + truthful connection/mic status + per-person audio** after handler inspection. This is functional scope, not silently included in a material refresh.

Later: camera contextual controls, optional pinned panels and development tools as separate passes. Defer radial/wrist navigation and runtime world editing. This deliberately borrows VRChat's quick access, Basis's explicit controls and Resonite's contextual tools without replacing WMCYN's working systems.

Await user confirmation of these scopes before editing runtime code, widgets, materials or input mappings.

### Research Sources

- [V1 - VRChat Action Menu](https://docs.vrchat.com/docs/action-menu)
- [V2 - VRChat Quick Menu](https://wiki.vrchat.com/wiki/Quick_Menu/en)
- [V3 - VRChat Camera](https://wiki.vrchat.com/wiki/Camera)
- [B1 - Basis settings and controls](https://docs.basisvr.org/en/docs/controls)
- [B2 - Basis framework architecture](https://github.com/BasisVR/Basis/blob/developer/PHILOSOPHY.md)
- [B3 - Basis contribution and license guidance](https://github.com/BasisVR/Basis/blob/developer/CONTRIBUTING.md)
- [R1 - Resonite Facets](https://wiki.resonite.com/Facets)
- [R2 - Resonite Tools](https://wiki.resonite.com/Tools)
- [R3 - Resonite Dev Tool](https://wiki.resonite.com/Dev_Tool)
- [N1 - Neos UIX documentation](https://wiki.neosvr.com/UIX)
- [H1 - Helios developer announcements, including shutdown and 2023 changes](https://steamcommunity.com/app/1047640/announcements/)

## Corrections To The Submitted Design

- Name new code FWMCYNUIAdaptiveTokens/UWMCYNAdaptiveSurface rather than implying it is supplied by AFCore. Use WMCYNRUNTIME_API, not YOURGAME_API.
- UBorder::GetDynamicMaterial exists in the installed UE 5.8 headers, but the border must first have a material brush. Cache the MID with a transient UObject reference; do not load assets in a native class constructor.
- UI materials use Final Color and Opacity outputs. Use a UV-space rounded-rectangle signed-distance mask with derivative antialiasing, not scene pixel depth. Verify available material functions instead of assuming GenerateShapes_RoundedRectangle exists.
- Use one parameter contract: BaseColor, CornerRadius, BorderWidth, BorderColor, SurfaceSize, SurfaceOpacity, InteractionIntensity. The supplied code's InteractionHighlight does not match its specified InteractionIntensity.
- FLinearColor interpolation is linear-light blending, not CSS color-mix(in srgb). Define shared colors as sRGB hex tokens; blend in sRGB then convert once for Unreal. Test the resulting swatch numerically. Do not promise identical headset and browser appearance because their display pipelines differ.
- FInterpTo is smoothing, not a physical spring. Update only while interaction transitions are active; avoid permanent ticks on every decorative surface. Support reduced motion.
- A one-pixel border is an initial render-target value, not a guaranteed readable headset edge. Tune at the actual panel size, draw resolution, distance and Quest resolution. Start with adjustable 1-2 logical-pixel borders and test contrast in the Crib.
- Keep opaque or near-opaque dark panels as the baseline. Spatial depth means restrained layers and readable silhouettes, not scene blur, refraction, or expensive translucent stacks.
- The zero-texture constraint applies to surface fills, borders and corners. Preserve the existing WMCYN logo and font assets; do not reinterpret it as removing branding or font rasterization.

## Staged Implementation

1. Inspect live parents, named controls, theme initialization, focus behavior and widget component draw sizes. Record screenshots of login, keyboard, pairing, menu, roster and settings before changes. Confirm no unsaved work is overwritten.
2. Add WMCYN-owned shared tokens and one isolated surface preview. Use accent #007AFF, tint 0.15 and base #050508 as the supplied starting palette, with explicit foreground, muted text, focus, disabled and error colors. Corner radius 12 is the requested initial value, not a requirement to round every control.
3. Import static Geist Sans Regular/Medium and Geist Mono Regular from the official distribution into /Game/WMCYN/UI/Fonts. Preserve the SIL OFL license with source files. Validate glyph fallback, password masking, packaging and headset readability; Mono is for codes/status, not all labels.
4. Create /Game/WMCYN/UI/Materials/M_AdaptiveSurface_Base with the agreed procedural mask and parameter contract. Compare tall, wide and compact surfaces; aspect changes must not stretch corners or borders. Preview normal, focused, pressed, disabled and error states before adopting it.
5. Apply the surface to login first without replacing named inputs or changing delegates. Keep widgets visible and editable in the UMG Designer. Decorative layers must not intercept pointer hits. Preserve the keyboard overlay position and focus ownership.
6. After login acceptance, apply the same treatment to the runtime menu and WMCYN-owned wrappers. Prefer AFCore theme overrides for inherited internals. Do not duplicate functioning sliders, roster logic or keyboard keys just for appearance.
7. Package and test PCVR and Quest before making the new style the default. Keep a reversible original-style option during the pilot. No web refactor or server deployment is required for this visual pass.

## Acceptance

- Designer, desktop PIE and headset show username/email, password, pairing, and Enter World without clipping or overlap.
- Typing remains continuous; switching fields, backspace, Enter, close, masked passwords and validation errors work. No credentials appear in logs.
- Login locks movement until successful entry; menu open/close, respawn, roster names, audio sliders and Apply/Reset retain behavior.
- Pointer hover, click and focus remain distinct. Decorative surfaces cannot block keyboard or control interaction.
- No constructor-time widget loading or recurrence of Slate/UMG crashes. Compile and reload each changed widget before proceeding.
- No AFCore/Mimic edits. No material shader errors in packaged PCVR or Quest. Measure menu-open GPU/frame timing against the unchanged baseline; do not assume procedural materials are free.
- Screenshots verify layout, but headset checks establish text readability, stereo comfort and pointer usability.

## Current Execution Boundary

The initial spike reviewed repository guidance, AFCore reuse records, UI filenames, settings code and installed UBorder headers. Subsequent persistent-sign-in work inspected login/settings and added functional controls with local UI smoke evidence; it did not implement this visual design. This research extension changes documentation only. No adaptive material graph, font import, platform migration or headset visual comparison has been executed. Existing uncommitted source/asset work is separate and must be preserved. Do not mark proposed features or headset acceptance complete.

References:
- Existing project guidance: Docs/AFCORE_MVP_REUSE_MATRIX.md and Docs/HC_AVF_ASSET_AUDIT.md.
- Epic UBorder API: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/UMG/UBorder
- Official Geist distribution and OFL notice: https://vercel.com/font
