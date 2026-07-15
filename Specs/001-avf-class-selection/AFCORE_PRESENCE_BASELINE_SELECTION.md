# AFCore Presence Baseline Selection

Status: WMCYN Quest pawn hook active; headset locomotion/collision path now validated for the current Crib floor lane  
Scope: First Signal Build only  
Last updated: 2026-07-10

## Canonical First Signal Target

Bring The WMCYN Crib online with two standalone VR users and one PCVR recording user, all present in the same world, with basic presence, voice, and OBS-friendly capture from the PCVR machine. Verbatim is retained below only as a deferred technical reference.

## Decision Summary

Use AFCore VR pawn families as the baseline for all First Signal users.

- Standalone VR users: start from `BP_Pawn_VR_Char`. MCP inspection shows it is a child of `BP_Pawn_VR` and adds visible presence-related components including head/torso meshes and a name tag component.
- PCVR recording user: start from the same `BP_Pawn_VR_Char` baseline unless in-editor testing proves `BP_Pawn_VR_Camera` is better. MCP inspection shows `BP_Pawn_VR_Camera` is also a child of `BP_Pawn_VR`, but no obvious standalone recording system was visible from property-level inspection.
- Recording support: inspect and test `Comp_Spectator`, `BP_Helper_Spectator`, and `BP_Manager_Spectator` in-editor/PIE before creating a custom recording pawn.
- Identity/presence/capabilities: use the WMCYN-owned child `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal`, parented to AFCore `BP_PlayerState_Main`, for the first replicated identity layer.
- Persistent-world entry flow: inspect AFCore login/replication/runtime assets before building custom networking.

Do not create old lead/visitor-specific pawn classes. The PCVR recording user is a normal user presence with recording capability, not a permanent camera-operator class.

## Implementation Passes

- 2026-06-23: `BP_Pawn_VR_Char` was placed in `L_WMCYNOnline` as three preview presences:
  - `PREVIEW_FirstSignal_StandaloneVR_A`
  - `PREVIEW_FirstSignal_StandaloneVR_B`
  - `PREVIEW_FirstSignal_PCVR_Recording`
- AFCore `BP_PlayerPosition` was placed in `L_WMCYNOnline` as three spawn markers:
  - `SPAWN_FirstSignal_StandaloneVR_A`
  - `SPAWN_FirstSignal_StandaloneVR_B`
  - `SPAWN_FirstSignal_PCVR_Recording`
- 2026-06-25: the three `PREVIEW_FirstSignal_*` mannequin preview actors were removed from the live level.
- 2026-07-07: Jared MetaHuman preview work was removed from the active First Signal path. Use the AFCore/default WMCYN Quest pawn path for basic runtime presence.
- 2026-06-25: `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` was created as a WMCYN-owned child of AFCore `BP_Pawn_VR_Char`.
- The AFCore `BP_Pawn_Mobile_Char_Example_Quest` asset was inspected and is a mobile/touch/AR example wrapper, not the current standalone headset runtime baseline.
- The AFCore `BP_Pawn_VR_Char_Example_Quest` asset was inspected and is a thin demo wrapper around `BP_Pawn_VR_Char`; its visible extra graph logic is demo `PawnUI` behavior. Use it as reference, not as the WMCYN runtime parent.
- 2026-06-25: `/Game/WMCYN/Data/DA_WMCYN_FirstSignal_Level` was created with `default_Pawn_VR` set to `BP_WMCYN_QuestUserPawn`.
- 2026-06-25: `BP_MapInfo_WMCYN_FirstSignal` was added to `L_WMCYNOnline` with `levelKey` set to the WMCYN First Signal Level Data asset.
- In-viewport PIE now starts without the Jared Blueprint compile blocker, but non-HMD in-viewport PIE currently spawns AFCore `BP_Pawn_Mobile_Char_C_0`.
- Current required proof is multi-user persistent-world validation: two Quest users and one PCVR recording user appear in The WMCYN Crib with basic presence/name.

## Product Model

Users are persistent identities. Presence is the live in-world embodiment. Capabilities are temporary permissions or functions granted to a presence during the world runtime.

First Signal needs:

- `PresenceMode`: `Quest`
- `PresenceMode`: `PCVR`
- `Capability`: `Recording`
- `Capability`: `CanTriggerVerbatimMarker`

The WMCYN Crib is the anchor. The world is not framed as a disposable private copy.

Implementation update, 2026-07-10:

- `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal` exists and is assigned by `BP_WMCYN_GameMode_FirstSignal`.
- Initial replicated fields: `Username`, `DisplayName`, `PresenceMode`, `Capabilities`.
- Login graph wiring and nameplate binding are still pending.

## AFCore Baseline Candidates

## MCP Inspection Findings

Read-only MCP inspection was run against the candidate assets. No Unreal assets were modified.

### VR Pawn Family

- `BP_Pawn_VR_Char`
  - Asset class: `BP_Pawn_VR_Char_C`
  - Parent: `BP_Pawn_VR_C`
  - Adds/contains presence-facing keys including `nameTag`, `sMesh_Framework_Head`, and `sMesh_Framework_Torso`.
  - Also exposes inherited VR/runtime keys including `comp_Controls`, `motionControllerLeft`, `motionControllerRight`, `mainCamera`, `cameraHandle`, `comp_Replication_Actor`, `comp_Replication_Component_Camera`, controller replication components, `hMD`, `vOIPComponent`, `trackingLevel`, and `spawnAtEyeLevel`.
  - Default values observed: `hMD = OpenXR`, `trackingLevel = GroundLevel`, `spawnAtEyeLevel = false`, `useGazeView = true`.
- `BP_Pawn_VR`
  - Asset class: `BP_Pawn_VR_C`
  - Parent: `BP_Pawn_Base_C`
  - Variables observed: `HMD`, `AssymetricPawn`, `Controllers`, `ManualControl`, `MoveFocus`, `ThumbstickThreshold`, `LastThumbstickDirection_Left`, `LastThumbstickDirection_Right`.
  - Exposes core VR keys including controls, motion controllers, camera, replication, VOIP, HMD, and asymmetric pawn support.
  - Default values observed: `hMD = OpenXR`, `trackingLevel = GroundLevel`, `spawnAtEyeLevel = false`, `useGazeView = true`.
- `BP_Pawn_VR_Camera`
  - Asset class: `BP_Pawn_VR_Camera_C`
  - Parent: `BP_Pawn_VR_C`
  - Exposes the same core VR keys as `BP_Pawn_VR`.
  - Default values observed: `hMD = OpenXR`, `trackingLevel = GroundLevel`, `spawnAtEyeLevel = true`, `useGazeView = true`.

Initial read: `BP_Pawn_VR_Char` is the best first baseline for visible standalone VR presence. `BP_Pawn_VR_Camera` remains a useful PCVR recording candidate, but property inspection did not prove it should replace the shared character baseline.

### Controls

- `Comp_Controls`
  - Parent: `ActorComponent`
  - Variables include `DefaultPreset`, `ControlsPresets`, teleport settings, movement settings, rotation settings, UI functions, and left/right motion controller classes.
  - Default preset observed: `DA_ControllerVR_OculusPreset`.
  - Movement observed: `movementStyle = Fluent`, `movementDirection = DirectionFromCamera`, `movementSpeed = 0.8`.
  - Rotation observed: `rotationType = Fluent`.
  - UI functions observed: `RadialMenu1`, `RadialMenu2`.

Initial read: this is the right first controls path for standalone headset and PCVR users. Do not build custom locomotion yet.

### Identity and Presence

- `BP_PlayerState_Main`
  - Parent: `/Script/Engine.PlayerState`
  - Variables observed: `PlayerColor`, `PlayerIndex`, `StateUpdate_PlayerIndex`.
  - Relevant keys observed: `comp_PlayerInfo_Basic`, `playerColor`, `playerIndex`, `playerId`, `bIsSpectator`, `onPawnSet`.
  - Default values observed: `playerIndex = 0`, `playerId = 0`, `bIsSpectator = false`.

Initial read: PlayerState remains the first place to inspect for replicated display name, presence mode, and capabilities. The Player Info component path exists but still needs in-editor inspection.

### Recording/Capture Support

- `Comp_Spectator`
  - Parent: `CameraComponent`
  - Variables observed: `SpectatorHelper`, `Active`, `SpectatorScreenSize`, `RenderTarget`, `SpectatorCoordinator`, `Key`, `SpectatorCameraActivated`.
  - Default screen size observed: `1920 x 1080`.
  - Defaults observed: `active = false`, `bLockToHmd = true`, `bUsePawnControlRotation = false`.
- `BP_Helper_Spectator`
  - Parent: `Actor`
  - Variables observed: `PostProcessSettings`, `FOVAngle`, `MasterActor`.
- `BP_Manager_Spectator`
  - Parent: `Actor`
  - Variables observed: `SpectatorComponents`, `ActiveIndex`, `SpectatorMap`, `CurrentlyActiveSpectator`, `PreferredScreenMode`, `NewSpectatorActivated`.
  - Default `preferredScreenMode` observed: `EntireScreen`.

Initial read: spectator support looks relevant for OBS/capture, especially because `Comp_Spectator` is a camera component with render target/screen-size concepts. It should be tested before inventing a WMCYN recording pawn.

### Persistent World Entry and Events

- `BP_LoginCredentials_Simple`
  - Parent: `BP_LoginCredentials_C`
  - No member variables observed at this level.
- `BP_SessionInfo_Simple`
  - Parent: `BP_SessionInfo_C`
  - Variables observed: `NumberOfPlayers`, `Level`.
- `Comp_Replication`
  - Parent: `ActorComponent`
  - Variables observed: `Enabled`, `InterpSpeed`, `Tolerance`, tick intervals, `CurrentOwner`, `NewTransformSet`, `ReplicatedTransform`, `AttachedPhysicsHandles`.
- `Comp_RPC`
  - Parent: `ActorComponent`
  - Variables observed: `ServerRPCCommand`, `MulticastRPCCommand`, `MulticastEvent`.

Initial read: `Comp_RPC` is the most obvious first candidate for a simple Verbatim marker event path. `BP_SessionInfo_Simple` can likely hold or expose minimal world runtime info, but needs graph/PIE review.

### UI / Nameplate Support

- `Comp_PawnUI`
  - Parent: `ActorComponent`
  - Variables observed: `PawnUIElements`, `Distances`, `SpacingAngle`, `ReferencePosition`, `PitchAngle`, `HeightDelta`, `WidgetCommand_Client`, `ElementStateChanged`, movement tolerances.
  - Default distance bands observed: `Far = 200`, `Medium = 100`, `Message = 70`, `Near = 50`.
- `BP_PawnUI_Element`
  - Parent: `Actor`
  - Variables observed: `Widget`, `Distance`, `PawnUIComponent`, `TargetTransform`, `LastWidth`.

Initial read: AFCore already has a pawn UI/nameplate path. Use this before creating custom display-name widgets.

### Standalone VR Users

Primary candidates:

- `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Char.uasset`
- `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR.uasset`
- `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Camera.uasset`

Supporting candidates:

- `Content/AFCore/Blueprints/Components/Pawn/Comp_Controls.uasset`
- `Content/AFCore/DataAsset/Presets/VRControls/General/DA_ControllerVR_OculusPreset.uasset`
- `Content/AFCore/DataAsset/Presets/VRControls/General/DA_ControllerVR_Default.uasset`
- AFCore Oculus/Epic hand meshes
- AFCore hand animation blueprints

Recommendation:

Use `BP_Pawn_VR_Char` as the first baseline because First Signal needs basic visible presence, not only a camera/input rig. If `BP_Pawn_VR_Char` is too heavy or visually wrong in PIE, use `BP_Pawn_VR` with the smallest WMCYN placeholder presence layer.

### PCVR Recording User

Primary candidates:

- `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Char.uasset`
- `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Camera.uasset`
- `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR.uasset`

Recording support candidates:

- `Content/AFCore/Blueprints/Components/Misc/Comp_Spectator.uasset`
- `Content/AFCore/Blueprints/Helpers/BP_Helper_Spectator.uasset`
- `Content/AFCore/Blueprints/Manager/BP_Manager_Spectator.uasset`
- AFCore video/camera helpers if present in the Blueprint graph

Recommendation:

Keep the PCVR recording user embodied as VR first. Start with the shared `BP_Pawn_VR_Char` baseline unless `BP_Pawn_VR_Camera` proves better in PIE. Treat spectator/video/camera assets as capability support for capture or monitoring. Treat desktop/asymmetric camera pawns as fallback/reference only, not the First Signal baseline.

## Identity, Presence, and Capability State

Primary candidates:

- `Content/AFCore/Blueprints/Game/BP_PlayerState_Main.uasset`
- AFCore Player Info component/path
- `Content/AFCore/Blueprints/Components/UI/Comp_PawnUI.uasset`
- `Content/AFCore/Blueprints/UI/PawnUI/BP_PawnUI_Element.uasset`

Minimal state for First Signal:

```json
{
  "user_id": "dev_user_01",
  "display_name": "Jared",
  "presence_mode": "Quest",
  "capabilities": ["CanTriggerVerbatimMarker"],
  "avatar_id": "placeholder_avatar_01"
}
```

PCVR recording user example:

```json
{
  "user_id": "dev_recorder_01",
  "display_name": "Recorder",
  "presence_mode": "PCVR",
  "capabilities": ["Recording", "CanTriggerVerbatimMarker"],
  "avatar_id": "placeholder_avatar_recorder"
}
```

Recommendation:

Do not build a full identity system yet. First inspect whether AFCore PlayerState/PlayerInfo already gives enough replicated display name and player identity behavior to wrap.

## Persistent World Entry and Replication

Primary candidates:

- `Content/AFCore/Blueprints/Multiplayer/BP_LoginCredentials_Simple.uasset`
- `Content/AFCore/Blueprints/Multiplayer/BP_SessionInfo_Simple.uasset`
- `Content/AFCore/Blueprints/Components/Multiplayer/Comp_Replication.uasset`
- `Content/AFCore/Blueprints/Components/Multiplayer/Comp_RPC.uasset`

Recommendation:

Use AFCore's simple login/replication/runtime path as the first inspection path. Keep public matchmaking, backend auth, and dedicated server architecture out of First Signal unless local/LAN-style persistent-world testing cannot satisfy the milestone.

## Spawn Markers

Use presence/capability names for new documentation and future manual marker naming:

- `MARKER_FirstSignal_Quest_A`
- `MARKER_FirstSignal_Quest_B`
- `MARKER_FirstSignal_PCVR_Recording`

If older marker actors already exist in the level, do not rename them automatically. Rename manually only when the level is intentionally being edited.

## Verbatim Marker Baseline

Likely first path:

- Use a simple world event payload.
- Trigger via a small replicated action path or AFCore RPC/trigger component.
- Store enough information to identify the user, presence mode, capabilities, world, map, and timestamp.

First payload shape:

```json
{
  "type": "verbatim_marker",
  "timestamp_utc": "...",
  "world": "The WMCYN Crib",
  "world_runtime_id": "first-signal-local-runtime",
  "triggered_by_user": "...",
  "presence_mode": "Quest|PCVR",
  "capabilities": ["Recording", "CanTriggerVerbatimMarker"],
  "map": "L_crib",
  "note": "First Signal marker"
}
```

Do not make the first marker depend on custom replicated widget state.

## Manual Unreal Review Checklist

Before creating WMCYN Blueprints or C++ wrappers:

- [ ] Open `BP_Pawn_VR_Char` and confirm parent class, components, visible body/head/hand behavior, and multiplayer suitability.
- [ ] Open `BP_Pawn_VR` and confirm whether it is a cleaner baseline than `BP_Pawn_VR_Char`.
- [ ] Open `BP_Pawn_VR_Camera` and confirm whether it has useful PCVR recording/camera behavior.
- [ ] Inspect `Comp_Controls` and the VR controller preset data assets for standalone headset input.
- [ ] Inspect hand meshes and hand AnimBPs for placeholder hand presence.
- [ ] Inspect `BP_PlayerState_Main` and Player Info fields for display name and player data.
- [ ] Inspect `Comp_PawnUI` and `BP_PawnUI_Element` for nameplate/presence label potential.
- [ ] Inspect `BP_LoginCredentials_Simple`, `BP_SessionInfo_Simple`, `Comp_Replication`, and `Comp_RPC`.
- [ ] Inspect `Comp_Spectator`, `BP_Helper_Spectator`, and `BP_Manager_Spectator` for recording/capture support.
- [x] Confirm the Level Data Asset path that controls default VR pawn selection for `L_WMCYNOnline`.
- [ ] Confirm whether `BP_PlayerPosition` can support the three First Signal spawn intents.

## Do Not Build Yet

- No custom locomotion system.
- No custom grab system.
- No custom menu framework.
- No desktop-only camera operator pawn.
- No old lead/visitor-specific pawn classes.
- No public matchmaking.
- No full backend auth.
- No dedicated server requirement.
- No avatar customization system.

## Proposed Wrapper Plan

Create WMCYN wrappers only after manual review proves the exact need.

Likely future wrappers:

- `BP_WMCYN_Pawn_Quest` only if AFCore VR pawn needs WMCYN-specific display/presence wiring.
- `BP_WMCYN_Pawn_PCVRRecording` only if recording capability cannot be cleanly attached to the shared VR baseline.
- `BP_WMCYN_PlayerState` or a replicated component only if AFCore PlayerState/PlayerInfo cannot hold `DisplayName`, `PresenceMode`, and `Capabilities` cleanly.
- `BP_WMCYN_VerbatimMarker` or equivalent only for the first structured world marker event.

Prefer composition/data over class multiplication.

## Next Checkpoint

Manual Unreal visual/PIE inspection should answer these three questions:

1. Does `BP_Pawn_VR_Char` look acceptable as the baseline for both standalone VR users?
2. Is the PCVR recording user the same pawn plus `Recording` capability, or does `BP_Pawn_VR_Camera` provide a better base in actual capture?
3. Can AFCore PlayerState/PlayerInfo carry display names, presence modes, and capabilities without replacing AFCore game classes?

After those answers, create the smallest WMCYN wrapper implementation plan.
