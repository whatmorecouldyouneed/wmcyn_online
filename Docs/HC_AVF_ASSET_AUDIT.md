# HC AVF Asset Audit

Generated from filesystem inspection of the local WMCYN Online Unreal project. Unreal `.uasset` and `.umap` files are binary, so this audit infers purpose from folder names, asset names, config references, and surrounding project structure only. No Unreal assets were modified.

## Summary

- HumanCodeable / Advanced Framework content is present under `Content/AFCore`.
- No project `Plugins` directory was found; AFCore appears to be imported as project content rather than a local plugin folder.
- Project maps present outside AFCore: `Content/Levels/L_WMCYNOnline.umap` and `Content/Levels/L_crib.umap`.
- Current config points `EditorStartupMap` and `GameDefaultMap` at `/Game/AFCore/Examples/Maps/Map_Example_Components`.
- Current config points `GlobalDefaultGameMode` at `/Game/AFCore/Blueprints/Game/BP_GameMode_Main.BP_GameMode_Main_C`.
- Current config points `GameInstanceClass` at `/Game/AFCore/Blueprints/Game/BP_GameInstance_Main.BP_GameInstance_Main_C`.
- Packaging currently cooks multiple AFCore example maps and `DirectoriesToAlwaysCook=(Path="/Game/AFCore/AnimBP/Hand")`.
- Target platforms are Android and Windows. `bStartInVR=True`, OpenXR is enabled, Android package settings include Quest2, and input mappings include Oculus/Valve/Vive/MixedReality/Cosmos controller axes and actions.

## Highest Priority Findings for First Signal

1. AFCore already provides core GameMode, GameInstance, PlayerController, PlayerState, desktop pawns, VR pawns, multiplayer components, interaction components, UI/HUD components, VR hand assets, and example maps.
2. The project is currently configured to start in an AFCore example map, not `L_WMCYNOnline`.
3. For First Signal, use AFCore as the framework baseline, then map WMCYN-specific identity, presence modes, temporary capabilities, PCVR recording, Verbatim marker, and Crib world logic around it.
4. Do not build custom locomotion, grab, or menu systems until the AFCore demo maps have been manually inspected in-editor.

## Candidate Assets

| Asset name | File path | Category | Why it may matter | Recommendation |
|---|---|---|---|---|
| BP_GameMode_Main | `Content/AFCore/Blueprints/Game/BP_GameMode_Main.uasset` | GameMode | Config already uses this as `GlobalDefaultGameMode`; likely central AFCore gameplay bootstrap. Binary contents not inspected. | Use as baseline; investigate manually before replacing. |
| BP_GameInstance_Main | `Content/AFCore/Blueprints/Game/BP_GameInstance_Main.uasset` | GameInstance | Config already uses this as `GameInstanceClass`; likely manages framework lifecycle/runtime state. Binary contents not inspected. | Use or wrap; keep unless a WMCYN-specific GameInstance is proven necessary. |
| BP_PlayerController_Main | `Content/AFCore/Blueprints/Game/BP_PlayerController_Main.uasset` | Controller | Main AFCore player controller candidate for both PC and VR input routing. Binary contents not inspected. | Use as baseline; wrap WMCYN role/world-runtime behavior outside it if possible. |
| BP_PlayerState_Main | `Content/AFCore/Blueprints/Game/BP_PlayerState_Main.uasset` | Multiplayer | PlayerState is relevant for display names, presence modes, capabilities, and replicated world runtime identity. Binary contents not inspected. | Investigate manually; likely wrap/extend for WMCYN display name, presence mode, and capabilities. |
| BP_Pawn_Base | `Content/AFCore/Blueprints/Pawn/BP_Pawn_Base.uasset` | Pawn | Base pawn class likely shared by desktop/mobile/VR variants. Binary contents not inspected. | Investigate manually; avoid editing directly. |
| BP_Pawn_Base_Screen | `Content/AFCore/Blueprints/Pawn/BP_Pawn_Base_Screen.uasset` | Pawn | Screen-oriented pawn variant may be useful for non-VR fallback/debug views. Binary contents not inspected. | Secondary for First Signal. |
| BP_Pawn_Desktop | `Content/AFCore/Blueprints/Pawn/Desktops/BP_Pawn_Desktop.uasset` | Pawn | Desktop pawn candidate for non-VR PC fallback/debug flows. Binary contents not inspected. | Secondary for First Signal; current recorder is PCVR, not desktop. |
| BP_Pawn_Desktop_Camera | `Content/AFCore/Blueprints/Pawn/Desktops/BP_Pawn_Desktop_Camera.uasset` | Pawn | Explicit desktop camera pawn candidate for fallback OBS/debug capture workflow. Binary contents not inspected. | Secondary fallback/reference; do not treat as main PCVR recording baseline. |
| BP_Pawn_Desktop_Char | `Content/AFCore/Blueprints/Pawn/Desktops/BP_Pawn_Desktop_Char.uasset` | Pawn | Desktop character pawn may support non-VR fallback/debug testing. Binary contents not inspected. | Secondary for First Signal. |
| BP_Pawn_VR | `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR.uasset` | Pawn | Main VR pawn candidate for standalone VR users and PCVR recording user. Binary contents not inspected. | High priority: use/investigate for First Signal VR presences. |
| BP_Pawn_VR_Camera | `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Camera.uasset` | Pawn | VR camera-specific pawn variant may handle HMD/camera rig behavior. Binary contents not inspected. | Investigate manually; likely useful for avatar/head presence. |
| BP_Pawn_VR_Char | `Content/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Char.uasset` | Pawn | VR character pawn variant may include body/avatar representation. Binary contents not inspected. | High priority: inspect for head/hand presence. |
| BP_Pawn_Assymetric | `Content/AFCore/Blueprints/Pawn/Assymetric/BP_Pawn_Assymetric.uasset` | Pawn | Asymmetric pawn set may support alternate/fallback PC+VR mixed control flows. Binary contents not inspected. | Secondary for First Signal; inspect only if PCVR recording cannot cover capture needs. |
| BP_Pawn_Assymetric_Camera | `Content/AFCore/Blueprints/Pawn/Assymetric/BP_Pawn_Assymetric_Camera.uasset` | Pawn | Asymmetric camera pawn may help understand AFCore camera-feed patterns. Binary contents not inspected. | Secondary reference/fallback for recording support. |
| BP_Pawn_Assymetric_Char | `Content/AFCore/Blueprints/Pawn/Assymetric/BP_Pawn_Assymetric_Char.uasset` | Pawn | Asymmetric character pawn may support mixed desktop/VR runtime presence. Binary contents not inspected. | Investigate manually. |
| Comp_Controls | `Content/AFCore/Blueprints/Components/Pawn/Comp_Controls.uasset` | Locomotion | Likely central movement/input component for pawns. Binary contents not inspected. | High priority: investigate for smooth locomotion. |
| Comp_Touch | `Content/AFCore/Blueprints/Components/Pawn/Comp_Touch.uasset` | Interaction | Touch/contact component may support VR hand or controller interactions. Binary contents not inspected. | Investigate manually. |
| Comp_Teleport | `Content/AFCore/Blueprints/Components/Interaction/Comp_Teleport.uasset` | Locomotion | Teleport interaction component. First Signal wants smooth locomotion, but teleport may be fallback/comfort option. Binary contents not inspected. | Wrap/keep as optional; do not prioritize over smooth locomotion. |
| Comp_Grab | `Content/AFCore/Blueprints/Components/Interaction/Comp_Grab.uasset` | Interaction | Core grab component candidate. Binary contents not inspected. | Use existing framework option; avoid custom grab system. |
| Comp_Gaze | `Content/AFCore/Blueprints/Components/Interaction/Comp_Gaze.uasset` | Interaction | Gaze targeting may support VR UI and hands-free interaction. Binary contents not inspected. | Investigate manually. |
| Comp_Latch | `Content/AFCore/Blueprints/Components/Interaction/Comp_Latch.uasset` | Interaction | Latching interaction may support handles, doors, sliders, and held UI. Binary contents not inspected. | Investigate manually. |
| Comp_Overlap | `Content/AFCore/Blueprints/Components/Interaction/Comp_Overlap.uasset` | Interaction | Overlap interaction likely useful for triggers and world events. Binary contents not inspected. | Wrap for Verbatim marker/world triggers if suitable. |
| Comp_Select | `Content/AFCore/Blueprints/Components/Interaction/Comp_Select.uasset` | Interaction | Selection component likely useful for UI and object interaction. Binary contents not inspected. | Investigate manually. |
| Comp_Replication | `Content/AFCore/Blueprints/Components/Multiplayer/Comp_Replication.uasset` | Multiplayer | Replication component candidate for shared world runtime state. Binary contents not inspected. | High priority: investigate for same-world presence replication. |
| Comp_Replication_Actor | `Content/AFCore/Blueprints/Components/Multiplayer/Comp_Replication_Actor.uasset` | Multiplayer | Actor replication helper. Binary contents not inspected. | Investigate manually. |
| Comp_Replication_Component | `Content/AFCore/Blueprints/Components/Multiplayer/Comp_Replication_Component.uasset` | Multiplayer | Component replication helper. Binary contents not inspected. | Investigate manually. |
| Comp_RPC | `Content/AFCore/Blueprints/Components/Multiplayer/Comp_RPC.uasset` | Multiplayer | RPC helper likely important for networked interactions and marker events. Binary contents not inspected. | High priority: wrap for Verbatim marker event if appropriate. |
| BP_LoginCredentials | `Content/AFCore/Blueprints/Multiplayer/BP_LoginCredentials.uasset` | Multiplayer | Login credential object may support user identity. Binary contents not inspected. | Investigate manually; do not assume backend auth. |
| BP_LoginCredentials_Simple | `Content/AFCore/Blueprints/Multiplayer/BP_LoginCredentials_Simple.uasset` | Multiplayer | Simpler identity/login option may fit First Signal better than full auth. Binary contents not inspected. | High priority: investigate for display name and user presence. |
| BP_SessionInfo | `Content/AFCore/Blueprints/Multiplayer/BP_SessionInfo.uasset` | Multiplayer | Runtime state object candidate. Binary contents not inspected. | Investigate for world runtime metadata. |
| BP_SessionInfo_Simple | `Content/AFCore/Blueprints/Multiplayer/BP_SessionInfo_Simple.uasset` | Multiplayer | Framework runtime world info candidate. Binary contents not inspected. | High priority: investigate for persistent-world entry flow. |
| Comp_HUD | `Content/AFCore/Blueprints/Components/UI/Comp_HUD.uasset` | HUD | HUD component candidate for framework display/UI. Binary contents not inspected. | Investigate for minimal First Signal overlay. |
| Comp_PawnUI | `Content/AFCore/Blueprints/Components/UI/Comp_PawnUI.uasset` | Widget | Pawn-attached UI component candidate for names and presence/capability labels. Binary contents not inspected. | Wrap for display name and presence/capability labels if suitable. |
| Comp_Widget | `Content/AFCore/Blueprints/Components/UI/Comp_Widget.uasset` | Widget | General widget component for 3D/in-world UI. Binary contents not inspected. | Use existing UI system before custom widgets. |
| BP_PawnUI_Element | `Content/AFCore/Blueprints/UI/PawnUI/BP_PawnUI_Element.uasset` | Widget | Likely a pawn label/UI element. Binary contents not inspected. | High priority for display name and presence/capability labels. |
| Widget_NameTag | `Content/AFCore/Blueprints/Widgets/Pawn/Widget_NameTag.uasset` | Widget | Confirmed AFCore pawn identity widget. It is referenced by the VR, mobile, and desktop character pawns and binds the owning pawn's PlayerState `Comp_PlayerInfo_Basic.Updated_PlayerName` dispatcher to its AFCore text control. | Use through the WMCYN native-pawn adapter; do not edit the AFCore asset. |
| Widget_Tag_Simple | `Content/AFCore/Blueprints/UI/SimpleTag/Widget_Tag_Simple.uasset` | Widget | Confirmed generic board/signage presentation. It contains a themed border and `Text_SimpleTag`, exposes `UpdateTagText`, and is referenced only by `BP_Board_Tag`. | Use for generic world labels, not player identity. |
| Widget_SelectionWindow | `Content/AFCore/Blueprints/UI/SelectionWindow/Widget_SelectionWindow.uasset` | Widget | Selection/menu UI may support role/runtime/debug selection. Binary contents not inspected. | Investigate, but keep First Signal UI minimal. |
| Widget_Keyboard | `Content/AFCore/Blueprints/UI/Keyboard/Widgets/Widget_Keyboard.uasset` | Widget | In-world keyboard may support entering username/password in VR. Binary contents not inspected. | Investigate if in-world text entry is needed. |
| Widget_Overlay_Notification | `Content/AFCore/Blueprints/UI/Overlay/Widgets/Widget_Overlay_Notification.uasset` | Widget | Notification overlay may report marker events or join status. Binary contents not inspected. | Wrap lightly if needed. |
| Widget_Welcome_Main | `Content/AFCore/Blueprints/UI/WelcomeScreen/Widget_Welcome_Main.uasset` | Widget | Existing welcome/menu flow. Binary contents not inspected. | Investigate but do not let menu work block First Signal. |
| BP_Helper_Spectator | `Content/AFCore/Blueprints/Helpers/BP_Helper_Spectator.uasset` | Other | Spectator helper may support PCVR recording capture/monitoring or fallback camera feeds. Binary contents not inspected. | High priority: investigate for OBS/camera workflow, but keep PCVR as main embodiment. |
| Comp_Spectator | `Content/AFCore/Blueprints/Components/Misc/Comp_Spectator.uasset` | Other | Spectator component may support capture/monitoring views. Binary contents not inspected. | High priority: investigate for recording support. |
| BP_Manager_Spectator | `Content/AFCore/Blueprints/Manager/BP_Manager_Spectator.uasset` | Other | Spectator manager may coordinate camera/spectator state. Binary contents not inspected. | High priority: investigate. |
| BP_Helper_TeleportIndicator | `Content/AFCore/Blueprints/Helpers/BP_Helper_TeleportIndicator.uasset` | Locomotion | Teleport indicator helper. Binary contents not inspected. | Keep as fallback/comfort support. |
| BP_Manager_TeleportLocation | `Content/AFCore/Blueprints/Manager/BP_Manager_TeleportLocation.uasset` | Locomotion | Teleport location manager. Binary contents not inspected. | Investigate only after smooth locomotion baseline. |
| DA_ControllerVR_OculusPreset | `Content/AFCore/DataAsset/Presets/VRControls/General/DA_ControllerVR_OculusPreset.uasset` | Controller | Meta/Oculus standalone headset control preset candidate. Binary contents not inspected. | High priority for standalone headset controls. |
| DA_ControllerVR_Default | `Content/AFCore/DataAsset/Presets/VRControls/General/DA_ControllerVR_Default.uasset` | Controller | Default VR control preset. Binary contents not inspected. | Use as fallback baseline. |
| PDA_ControllerVR | `Content/AFCore/DataAsset/Presets/VRControls/PDA_ControllerVR.uasset` | Controller | Primary data asset class for VR controller presets, registered in AssetManager settings. Binary contents not inspected. | Investigate manually. |
| SK_Hand_Epic_Left / SK_Hand_Epic_Right | `Content/AFCore/Meshes/Pawn/Hand/Epic/` | Pawn | Hand meshes and physics assets support basic left/right hand presence. Binary contents not inspected. | Use/investigate for placeholder hand presence. |
| SK_Hand_Oculus_L / SK_Hand_Oculus_R | `Content/AFCore/Meshes/Pawn/Hand/Oculus/` | Pawn | Oculus hand meshes likely aligned with Meta/Oculus standalone controllers/hands. Binary contents not inspected. | High priority for standalone headset placeholder hands. |
| AnimBP_Hand_Epic_Left / Right | `Content/AFCore/AnimBP/Hand/` | Pawn | Hand animation blueprints are explicitly cooked, indicating they are expected runtime assets. Binary contents not inspected. | High priority for hand presence. |
| Anim_Hand_*_Grab_* | `Content/AFCore/Animations/Hand/Left/` and `Content/AFCore/Animations/Hand/Right/` | Interaction | Many named grab poses for controller, pinch, joystick, plate, pole, stick, etc. Binary contents not inspected. | Use existing grab pose library; avoid custom hand animation initially. |
| Map_Example_Components | `Content/AFCore/Examples/Maps/Map_Example_Components.umap` | Demo Map | Current startup/default map; likely safest first AFCore demo to open. Binary contents not inspected. | Use as first manual validation map. |
| Map_Example_Controls | `Content/AFCore/Examples/Maps/Map_Example_Controls.umap` | Demo Map | Likely demonstrates input/controller control setup. Binary contents not inspected. | High priority for controls validation. |
| Map_Example_Locomotion | `Content/AFCore/Examples/Maps/Map_Example_Locomotion.umap` | Demo Map | Likely demonstrates movement/locomotion. Binary contents not inspected. | High priority for smooth locomotion checkpoint. |
| Map_Example_Quest | `Content/AFCore/Examples/Maps/Map_Example_Quest.umap` | Demo Map | Explicit Quest-named example map, strongly relevant to standalone headset validation. Binary contents not inspected. | High priority for standalone headset validation. |
| Map_Example_Interactions | `Content/AFCore/Examples/Maps/Map_Example_Interactions.umap` | Demo Map | Likely demonstrates grab/select/latch/overlap interactions. Binary contents not inspected. | Inspect before building interaction features. |
| Map_Example_HandsOn | `Content/AFCore/Examples/Maps/Map_Example_HandsOn.umap` | Demo Map | Likely demonstrates hand interaction examples. Binary contents not inspected. | High priority for hand/grab validation. |
| Map_Example_LaserController | `Content/AFCore/Examples/Maps/Map_Example_LaserController.umap` | Demo Map | Likely demonstrates laser pointer/controller UI interactions. Binary contents not inspected. | Investigate for VR UI selection. |
| Map_Example_Menu | `Content/AFCore/Examples/Maps/Map_Example_Menu.umap` | Demo Map | Menu example may contain runtime/role/menu patterns. Binary contents not inspected. | Investigate, but keep First Signal menu minimal. |
| Map_Example_UserInterfaces | `Content/AFCore/Examples/Maps/Map_Example_UserInterfaces.umap` | Demo Map | UI demo likely shows widgets/overlays/in-world panels. Binary contents not inspected. | Investigate for display name, presence/capability UI, marker feedback. |
| Map_Template | `Content/AFCore/Maps/Map_Template.umap` | Demo Map | Framework template map may be a better base than examples. Binary contents not inspected. | Investigate as possible clean template. |
| Map_Empty | `Content/AFCore/Maps/Map_Empty.umap` | Demo Map | Config transition map. Binary contents not inspected. | Use only as transition/blank baseline. |
| L_WMCYNOnline | `Content/Levels/L_WMCYNOnline.umap` | Demo Map | Project-specific likely canonical main map requested by First Signal planning. Binary contents not inspected. | Use as target main map after framework validation. |
| L_crib | `Content/Levels/L_crib.umap` | Demo Map | Project-specific WMCYN Crib environment candidate. Binary contents not inspected. | High priority: inspect in editor for First Signal setting. |
| BP_Jared | `Content/Blueprints/Characters/BP_Jared.T3D` | Pawn | Only non-AFCore project Blueprint-like text export found; likely character/avatar placeholder. | Investigate manually; possible avatar placeholder, but not an AVF framework asset. |

## Input and Platform Notes

- `DefaultInput.ini` includes keyboard/mouse, gamepad, Oculus Touch, Valve Index, Vive, MixedReality, Cosmos, and motion controller mappings.
- Relevant actions include `Select`, `ActivateRotation`, `Pause`, `MotionControllerTriggerLeft/Right`, `MotionControllerGripLeft/Right`, `ThumbstickPressLeft/Right`, `MotionControllerMenu`, `Ping`, `PushToTalk`, and `VOIPLoop`.
- Relevant axes include `Forward`, `Right`, `Up`, `LookUp`, `LookRight`, `Zoom`, `ThumbstickLeft_X/Y`, `ThumbstickRight_X/Y`, `TriggerAxisLeft/Right`, and `Grip1AxisLeft/Right`.
- Project settings target Mobile/Scalable and Android/Windows, with `bStartInVR=True` and Quest2 packaging enabled. That is favorable for First Signal's standalone headset + PCVR target, though target hardware should still be manually validated.

## Recommended First Signal Asset Path

1. Open AFCore demo maps manually in this order: `Map_Example_Quest`, `Map_Example_Locomotion`, `Map_Example_Controls`, `Map_Example_Interactions`, `Map_Example_UserInterfaces`, then `Map_Example_Components`.
2. Validate `BP_Pawn_VR`, `BP_Pawn_VR_Char`, `BP_Pawn_VR_Camera`, Oculus hand meshes, and hand AnimBPs for standalone VR users and the PCVR recording user.
3. Validate `BP_Helper_Spectator`, `Comp_Spectator`, `BP_Manager_Spectator`, and video/camera helpers for OBS-friendly capture from the PCVR machine. Treat desktop/asymmetric camera pawns as fallback/reference only.
4. Validate `BP_SessionInfo_Simple`, `BP_LoginCredentials_Simple`, `Comp_Replication`, and `Comp_RPC` for persistent-world entry, display names, presence/capabilities, and the Verbatim marker event.
5. Move First Signal toward `L_WMCYNOnline` and `L_crib` only after the AFCore framework classes are confirmed in-editor.

## Use / Wrap / Ignore Guidance

- Use: AFCore GameMode, GameInstance, PlayerController, VR pawn families, controller presets, hand assets, and example maps as the baseline.
- Wrap: PlayerState/login/replication/UI pieces for WMCYN display name, presence modes, temporary capabilities, PCVR recording, Verbatim marker, and world events.
- Investigate manually: all `.uasset` and `.umap` candidates listed above because their internal Blueprint graphs/classes were not inspected.
- Ignore for First Signal: broad AFCore textures/materials/sounds/demo props unless a selected demo map or pawn depends on them.

## Unknowns and Risks

- MCP currently does not expose editor asset graph, actor, or level inspection tools, so this audit cannot prove Blueprint parent classes, component wiring, replication settings, or map contents.
- `.uasset` and `.umap` files are binary; conclusions are name/path/config based.
- The project currently defaults to AFCore example maps rather than `L_WMCYNOnline`; switching maps/settings should be planned separately and done only after manual editor confirmation.
- Online subsystem is currently `Null`, so standalone headset + PCVR same-world testing may work only as local/LAN-style testing until a real online subsystem path is chosen.
