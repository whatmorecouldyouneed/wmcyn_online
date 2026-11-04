# Adaptive UI Implementation Pilot

Date: 2026-10-16. Scope: approved spike priorities 1-4, not a replacement menu framework.

## Implemented

- Settings visual revision: neutral charcoal surfaces, 8px control corners, subtle 1px borders, consistent regular-weight Roboto at 24px (20px status text). This is a native UMG interpretation of the requested shadcn-like direction, not a web UI dependency or an imported Inter/Geist font.
- `DA_WMCYN_Theme_Neutral` is a WMCYN-owned duplicate of AFCore's theme with gradients disabled and compact typography. Runtime menu uses this separate theme; nameplates and login keep their existing theme. Missing menu theme falls back to the previous AFCore theme.
- Theme map values are applied with `Tools/UI/refine_neutral_theme.py` and read back: 28px display, 24px other categories, 8px corners. ObjectTools JSON map setters returned success without applying these maps; use the script for those fields.
- Audio visual overrides target AFCore's decorative progress bar/knob and Apply/Reset instances after initialization. Existing slider delegates and AFCore handlers remain in place. No framework asset changes.

- Designer-visible WMCYN login surface and button styling; existing editable fields, keyboard and authentication graphs retained.
- Scrollable WMCYN Settings with mic toggle/status, existing mirror visibility toggle, panel reset/pin, connection summary, and per-person listener-local volume.
- Existing AFCore Audio controls and respawn/sign-out handlers retained. Native slider presentation overrides are applied at runtime, without editing AFCore assets.
- Camera pilot: on-demand 640x360 local capture, reset in front of the user, field-of-view control and Windows spectator output with prior output restored when disabled.
- Camera controls collapse when inactive. Empty remote roster explicitly reports no other users connected.
- No pawn possession, Mimic tracking/IK, controller binding or online authentication changes in this UI pass.

## Verification

- Login visual revision: imported licensed Inter 4.1 Regular/Medium/SemiBold with a WMCYN composite font, dark rounded fields, sentence-case labels, improved spacing and a light primary button. Native fields, password masking, keyboard types, widget names and Blueprint graphs preserved. Before/after: `Saved/Diagnostics/AdaptiveUI/login_before.png` and `login_after.png`. Runtime inspection confirms both inputs use `F_Inter`; Blueprint compile passes. No real login submission or headset keyboard regression performed in this visual pass.

- Neutral Settings revision: fresh native rebuild and restart pass. Actual runtime render target inspected at `Saved/Diagnostics/AdaptiveUI/settings_neutral.png`; compact header/navigation, neutral buttons, thin audio tracks and readable Apply/Reset confirmed. Settings was selected programmatically for this screenshot, so the navigation selection highlight is not a navigation acceptance test. Headset pointer/drag/hover validation remains open. Login migration is intentionally deferred for visual review.

- Clean WMCYNRuntime Editor compilation succeeds.
- Local standalone PIE screenshot confirms actual runtime menu contains the new controls. This uses a development identity, not a Firebase or remote-world acceptance run.
- Camera render-target smoke screenshot confirms a nonblank scene image.
- Screenshots: `Saved/Diagnostics/AdaptiveUI/`. These are intermediate evidence, not final visual acceptance; slider/contextual-control polish followed the initial screenshots.
- Automation coverage: `WMCYN.UI.LocalToolBoundaries` checks listener gain isolation, mute, bounds/NaN handling and inactive camera allocation/output boundaries. Credential regression: `WMCYN.Auth.SecureCredentialRoundTrip`.
- Both automation tests passed in a fresh unattended Editor process (exit 0), recorded in `Saved/Logs/AdaptiveUITests.log`. NullRHI run emitted OpenXR API-version warnings; it does not validate headset rendering or input.

## Open Gates And Limitations

- Headset: login/keyboard focus, B menu, pointing/scrolling, mic toggle, respawn, pin/reset and movement regression.
- Remote peers: listener-local volume must affect only the selected remote user. Current gain lasts for that talker instance, not across pawn replacement or reconnect.
- Connection summary is basic, not a complete reconnect/error-state dashboard. Mic requested/idle is not proof of remote audibility.
- Mirror toggle currently hides the existing mirror; capture-cost suspension is not implemented or claimed.
- Camera is a stationary preview pilot, not a grabbable phone, photo export or production OBS recording system. Capture continues while enabled; turn it off when unused. Current capture excludes world-space widgets, including nameplates.
- Quest packaging/performance and PCVR spectator restoration require hardware testing. No new client/server deployment in this pass.
- Respawn remains in the existing Settings section; quick-row placement, reduced-motion preferences and a dedicated UI state gallery remain follow-up polish.
- No AFCore, Mimic or map assets were edited by this pass. Pre-existing dirty framework/map changes remain untouched.

## Acceptance Sequence

1. Login with keyboard; verify fields, submission and movement unlock are unchanged.
2. Open Settings with B. Toggle mic and verify local status plus remote audibility with another user.
3. Select each remote user and reduce volume; verify other users are unaffected.
4. Pin, close/reopen and reset the panel. Verify readable placement and controller hits.
5. Enable camera, adjust framing, toggle desktop output, then disable it; verify headset view never changes and desktop output restores.
6. Test mirror and respawn, then sign out and verify the login gate returns.
