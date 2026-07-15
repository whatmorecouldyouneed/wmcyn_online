# AFCore MVP Reuse Audit

The implementation-status matrix is maintained in `Docs/AFCORE_MVP_REUSE_MATRIX.md`.

## Purpose

Keep WMCYN product behavior WMCYN-owned while reusing AFCore framework systems that already match the First Signal requirements. Do not edit `/Game/AFCore/*`.

## Active Reuse

| Need | AFCore source | WMCYN use |
| --- | --- | --- |
| World-space widget hosting and theme | `Comp_Widget`, `Widget_Base`, `DA_Theme_Default` | Host the login and runtime identity UI with AFCore theme initialization. |
| VR text entry | `BP_Overlay_Widget_Keyboard`, `Widget_Keyboard_US` | Keep the proven AFCore keyboard with native WMCYN editable fields. |
| Display name | `Widget_NameTag`, `Comp_PlayerInfo_Basic` | Drive the AFCore NameTag from verified WMCYN PlayerState identity. |
| Player roster | `Widget_Multiplayer_Players`, `Widget_Multiplayer_PlayerField` | Candidate for a WMCYN-owned `Who's Here` wrapper. The active WMCYN PlayerState already inherits `BP_PlayerState_Main` and owns `Comp_PlayerInfo_Basic`. |
| Settings apply/reset | `Widget_Scaffold_Settings` | Candidate for WMCYN settings confirmation and reset behavior. |
| Audio settings | `Widget_Settings_Audio` | Candidate for Master, Music, SFX, Voice, and audio-quality controls. Verify packaged voice routing separately. |

## Keep WMCYN-Owned

- `WBP_WMCYN_LoginJoin` remains the branded persistent-world login surface.
- Keep native Unreal username/password fields. Do not insert AFCore `Widget_Input_TextBox`; its UE 5.8 Designer path has caused repeatable Slate/UMGEditor crashes.
- Login submission, backend authentication, verified identity, world bootstrap, and locomotion gating remain WMCYN responsibilities.
- `WBP_WMCYN_BootLogin` had no referencers and was deleted on 2026-07-14. `WBP_WMCYN_LoginJoin` is the only active login surface.

## Multiplayer Widgets

| AFCore widget | Decision | Reason |
| --- | --- | --- |
| `Widget_Multiplayer_Login_Simple` | Reference only | One-button EOS login flow with no username/password fields and no referencers. It is not the WMCYN Firebase credential screen. |
| `Widget_Multiplayer_Players` | Reused | WMCYN child `WBP_WMCYN_WhosHere` inherits its refresh timer and PlayerState enumeration. Runtime menu exposure remains pending. |
| `Widget_Multiplayer_PlayerField` | Reused through Players | Reads `Comp_PlayerInfo_Basic` player name and ping, matching the existing identity bridge. No WMCYN row duplicate was created. |
| `Widget_Multiplayer_Host_Simple` | Exclude | Session-host semantics conflict with the persistent world and it depends on unstable `Widget_Input_TextBox`. |
| `Widget_Multiplayer_Servers` / `Server` | Exclude | Server-browser behavior is infrastructure, not a user-facing WMCYN flow. |
| `Widget_Multiplayer` | Exclude | Composes host, server browser, and session UI. |
| `Widget_Multiplayer_Leave` | Exclude | `Leave Session` is not the product language or desired flow. |
| `Widget_Multiplayer_Logout` | Wrap later | May support Sign Out after WMCYN defines token clearing, disconnect, and return-to-login behavior. |

## Settings Widgets

- WMCYN child `WBP_WMCYN_Settings_Audio` now inherits AFCore `Widget_Settings_Audio`, including scaffold Apply/Reset and AFCore sound-class overrides.
- Use selected WMCYN wrappers instead of exposing all of `Widget_Settings` unchanged.
- Quest First Signal: Audio only at first.
- PCVR First Signal: Audio plus a reviewed subset of Graphics.
- Do not expose unrestricted resolution scale, shadow, post-process, or effects controls on Quest before performance validation.
- General currently centers on language; defer it.
- Controls and Gameplay are mostly framework shells; defer them until concrete comfort and movement options are defined.

## Future Camera Reuse

AFCore contains useful foundations in `Comp_Spectator`, `BP_Manager_Spectator`, `BP_Helper_Spectator`, `BP_SpectatorCamera`, `BP_CameraDisplay`, `BP_Recorder`, and `BP_Pawn_VR_Camera`. `Widget_HUD_Frame` is a layout container suitable for a viewfinder overlay, not a photo-capture implementation.

The physical camera should be WMCYN-owned while reusing AFCore interaction and spectator/render-target behavior. Quest image export and PCVR streamer routing remain custom platform work. Camera implementation starts only after the shared-world hardware gate.

## Refactor Order

1. Thin the active login widget around one WMCYN authentication/bootstrap call while preserving the proven AFCore keyboard.
2. Keep the old local identity path available only as an explicit development fallback.
3. Host `Widget_Multiplayer_Players` in a small WMCYN `Who's Here` surface and verify three replicated names.
4. Host AFCore Audio settings and Apply/Reset behavior in a WMCYN settings wrapper.
5. Add platform filtering before exposing Graphics settings.
6. Run VR login, keyboard, locomotion gate, NameTag, roster, and audio-setting regressions.
