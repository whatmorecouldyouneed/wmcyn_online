# AFCore WMCYN Implementation Notes

Source: Advanced Framework - Core 4.1 Documentation, HumanCodeable, May 5, 2021.  
Use with `Docs/HC_AVF_ASSET_AUDIT.md` and `Docs/AFCORE_DOCUMENTATION_INDEX.md`.

These are WMCYN-specific notes distilled from the AFCore documentation. They are not a replacement for checking the actual Blueprint assets in Unreal before edits.

## Core Architecture Takeaways

- AFCore is intentionally component-driven. Prefer adding or wrapping framework components over building deep WMCYN class hierarchies.
- AFCore game classes handle level transition, pawn navigation, settings, and persistent application data.
- `BP_GameMode_Main` declares the other game classes, but in multiplayer it is server-only. Avoid putting WMCYN client-facing presence/display/UI logic directly in GameMode.
- `BP_GameInstance_Main` persists across level changes and stores current level data assets. It is a candidate location to observe or bridge WMCYN world runtime metadata, but avoid replacing it casually.
- `BP_Player_Controller_Main` spawns, possesses, and navigates pawns. Its `Pawn Select` mode can choose pawns dynamically or force VR/Desktop/Mobile.
- `BP_PlayerState_Main` handles per-player multiplayer info. The docs say most player settings are now handled by the Player Info component.

## Minimum Level Setup

AFCore expects each framework level/world map to include:

- `BP_MapInfo` with a Level Data Asset reference.
- `BP_PlayerPosition` for pawn spawn location/rotation.
- Optional `BP_PostProcess` for post-process highlighting.

For WMCYN First Signal, do not just drop pawns into `L_WMCYNOnline` and hope the framework finds them. The manual path is:

1. Identify whether `L_WMCYNOnline` is already acting as the AFCore world map.
2. Confirm a `BP_MapInfo` actor exists or create a WMCYN-safe equivalent only after review.
3. Confirm the Level Data Asset points to the right world map and streamed maps.
4. Use player position actors for standalone VR users and the PCVR recording user spawn intent.

## Level Data Asset Implications

The Level Data Asset is central. It can specify:

- title/subtitle/transition metadata
- default desktop pawn
- default VR pawn
- default mobile pawn
- world map
- maps to stream/load

This is likely the cleanest place to connect `L_WMCYNOnline`, `L_crib`, and presence baselines without changing AFCore internals.

## Presence Baseline Direction

### Standalone VR Users

Start from AFCore VR pawn classes and controller presets.

Relevant doc concepts:

- VR pawn is selected when an HMD is present.
- VR pawn mainly acts as the player vantage point and input relay.
- Motion controllers and motion components implement most VR interaction behavior.
- Controls component routes real controller input through preset data assets to motion controller functions.

Likely WMCYN baseline to inspect:

- `BP_Pawn_VR`
- `BP_Pawn_VR_Char`
- Oculus/VR controller preset data assets
- hand meshes and hand AnimBPs
- `Comp_Controls`

### PCVR Recording User

Start from the AFCore VR pawn family, then add or wrap a recording capability if needed. The PCVR recording user is still a VR presence, not a desktop operator.

Relevant doc concepts:

- VR pawn is selected when an HMD is present, including PCVR.
- The PCVR machine can provide the OBS capture surface.
- Spectator and video components may still help with capture, monitoring, or alternate camera feeds.
- Desktop/asymmetric camera assets are secondary fallback/reference options, not the main First Signal embodiment.

Likely WMCYN baseline to inspect:

- `BP_Pawn_VR`
- `BP_Pawn_VR_Char`
- `BP_Pawn_VR_Camera`
- `Comp_Spectator`
- `BP_Helper_Spectator`
- `BP_Manager_Spectator`
- `Video Component`

## Identity and Display Names

AFCore already has a Player Info component. The docs describe it as saving per-player info outside the PlayerState, including:

- player color
- player name for multiplayer scenarios

For First Signal, start display names, presence mode, and temporary capabilities from this path:

1. Inspect `BP_PlayerState_Main`.
2. Inspect the Player Info component asset found in AFCore.
3. Decide whether WMCYN needs a child/wrapper for `DisplayName`, `PresenceMode`, and `Capabilities`.
4. Avoid a separate identity system until PlayerInfo is proven insufficient.

## Multiplayer and UI Caution

The docs explicitly caution that widgets do not replicate within themselves. Widget actions can trigger replicated actor functions, but the widget UI should be treated as client-side presentation unless AFCore has a specific replicated flow around it.

Implications for WMCYN:

- Do not make the first Verbatim marker depend on custom replicated widget state.
- Prefer a simple replicated actor/component event or server-routed function for marker logging.
- For shared in-world choices, inspect AFCore widget-less UI paths first.
- Selection Menu and Radial Menu are worth inspecting because AFCore designed them to work better in multiplayer VR contexts.

## Persistent World Entry Flow

The docs show existing multiplayer UI patterns. Treat these as framework references, not product UX targets for First Signal:

- multiplayer pallet widget
- multiplayer HUD widget
- framework world-entry button
- server list refresh
- IP search bar
- player field with player name, ping, mute

For First Signal, inspect these assets before building a WMCYN custom lobby:

- `Widget_Initial_Pallet_Multiplayer`
- `Widget_HUD_Multiplayer`
- `BP_LoginCredentials_Simple`
- `BP_SessionInfo_Simple`
- `Comp_Replication`
- `Comp_RPC`

Keep the first persistent-world entry path simple: username/password login, enter The WMCYN Crib, display connected users, capture from the PCVR machine, and record one marker.

## Verbatim Marker Direction

Best first fit from the docs:

- Trigger Component for activating a function.
- Name/Values/State components for simple runtime state.
- Replication/RPC components for multiplayer event flow.
- Player Info / PlayerState for who triggered the marker.

Avoid building a new menu framework for marker capture. The first marker can be a small actor/component path with a visible debug confirmation.

## Manual Review Checklist Before Creating WMCYN Blueprints

- Confirm `BP_GameMode_Main`, `BP_GameInstance_Main`, and `BP_Player_Controller_Main` are still the active baseline.
- Inspect whether `BP_Player_Controller_Main` uses `Dynamic`, `ForceVR`, `ForceDesktop`, or another pawn selection mode.
- Find the Level Data Asset used by the current target map.
- Confirm the Level Data Asset's default VR pawn class for standalone VR and PCVR users.
- Inspect `BP_PlayerPosition` usage for multiple players and player indexes.
- Inspect Player Info component fields and replication behavior.
- Inspect spectator/video/camera behavior in a running PIE run before creating any WMCYN recording wrapper.
- Inspect multiplayer pallet/HUD widgets, but keep UI replication concerns in mind.

## First Signal Recommendation

The next implementation planning step should be `Specs/001-avf-class-selection`: choose the AFCore baseline classes for standalone VR users and the PCVR recording user, then list only the smallest WMCYN wrapper classes needed for display name, presence mode, capabilities, persistent-world entry, and Verbatim data.
