# AFCore MVP Reuse Matrix

## Rule

Reuse AFCore where its behavior matches First Signal. Keep WMCYN product policy, Firebase authentication, persistent-world entry, and native Mimic pawn adaptation in WMCYN-owned code/assets. Do not edit `/Game/AFCore/*`.

| First Signal need | AFCore source | WMCYN surface | Status | Do not duplicate |
| --- | --- | --- | --- | --- |
| World-space UI host and theme | `Comp_Widget`, `Widget_Base`, `DA_Theme_Default` | Active login and runtime NameTag hosts | Active and previously headset-tested | Widget hosting, theme initialization, or UI master materials |
| VR keyboard | `BP_Overlay_Widget_Keyboard`, `Widget_Keyboard_US` | `WBP_WMCYN_LoginJoin` with native `EditableTextBox` fields | Active and previously headset-tested | Keyboard layout, key dispatch, or overlay placement system |
| Credential UI | AFCore keyboard/overlay only | `WBP_WMCYN_LoginJoin` | Consolidated active surface; label now accepts username or email | AFCore EOS login or a second boot-login widget |
| Firebase authentication and world bootstrap | None | `UWMCYNBackendSubsystem` + native login bridge | Built with MSVC 14.50 and connected to the active login; PIE fallback proven, real service proof waits on Firebase billing | Tokens in PlayerState or client-assigned roles/capabilities |
| Display name | `Widget_NameTag`, `Comp_PlayerInfo_Basic` | `WMCYN_FirstSignalPresence` bridge | Active; three-client PIE identity proof exists | A second WMCYN text/nameplate system |
| Who's Here roster | `Widget_Multiplayer_Players`, `Widget_Multiplayer_PlayerField` | `WBP_WMCYN_WhosHere` child | Live in the runtime menu; PIE roster displayed the submitted identity; headset refresh test pending | Custom player enumeration, row creation, refresh timer, name, or ping row |
| Audio settings | `Widget_Settings_Audio`, `Widget_Scaffold_Settings`, AFCore sound classes/mix | `WBP_WMCYN_Settings_Audio` child | Live in the runtime menu; headset voice-volume Apply/Reset test pending | Master/Music/SFX/Voice sliders or Apply/Reset behavior |
| Graphics settings | `Widget_Settings_Graphics` | No active WMCYN wrapper | PCVR-only candidate; excluded from Quest until performance validation | Full scalability UI on Quest |
| Runtime menu shell | AFCore scaffold, vertical tabs, widget switcher, and theme | `WBP_WMCYN_RuntimeMenu` hosted by `WMCYN_FirstSignalPresence` | Active after login; PIE open/close and visual roster proof pass | A second menu framework or hard-loaded AFCore class in a native constructor |
| Voice presence | AFCore/Unreal VOIP conventions plus AFCore voice sound class | `WMCYN_FirstSignalPresence` | Local capture path proven; separate-device hearing pending | A second local voice capture/session path |
| Player body and locomotion | AFCore remains a reference; native Mimic is the proven body source | `BP_WMCYN_UserPawn_FirstSignal` | Active source of truth | Custom locomotion, footsteps, tracking hierarchy, or duplicate body solver |
| Quest/PCVR camera foundation | AFCore spectator/camera helpers and `Widget_HUD_Frame` | Future WMCYN physical camera | Discovery only, after shared-world gate | Capture UX or platform export before hardware presence passes |

## Excluded AFCore Surfaces

- `Widget_Multiplayer_Login_Simple`: EOS one-button login, not Firebase username-or-email/password.
- `Widget_Multiplayer_Host_Simple`, Servers, Server, Leave: user-facing session/host semantics conflict with the persistent-world product model.
- `Widget_Input_TextBox`: repeatable UE 5.8 Slate/UMGEditor crash path; keep native Unreal text boxes.
- Full `Widget_Settings` on Quest: exposes Graphics and unrelated pages before platform validation.

## Current Gate

The implementation gate is complete: the wrappers compile, preserve AFCore ownership, and are exposed from the post-login runtime menu. The remaining gate is a headset regression for pointer interaction, roster refresh, Audio Apply/Reset, keyboard/login, themed NameTag, and locomotion unlock.
