# Decisions

This is the running decision log. Add concise dated entries when scope, architecture, or workflow changes.

## 2026-06-22

- WMCYN Online uses Unreal Engine 5.8.
- First Signal Build is the active target, not the full future product.
- Canonical First Signal target: Bring The WMCYN Crib online with two standalone VR users and one PCVR recording user, all present in the same world, with basic presence, voice, and OBS-friendly capture from the PCVR machine.
- The world is the anchor. Do not model a lead participant as a permanent user type; users enter The WMCYN Crib and receive presence/capability state.
- First Signal uses two standalone VR user presences and one PCVR recording user presence.
- HumanCodeable AFCore / Advanced VR Framework has been tested and works.
- AFCore remains the baseline for compatible reusable framework systems and reference behavior. The native Mimic pawn is the First Signal body, tracking, locomotion, footsteps, and floor-behavior source of truth.
- Skip the broad AFCore example-map validation loop and proceed to WMCYN integration.
- Use `L_WMCYNOnline` / `L_crib` as the WMCYN target maps.
- Preserve AFCore systems for locomotion, grab, hands, menus, and base multiplayer unless a concrete gap is proven.
- Create WMCYN wrapper/child classes only where WMCYN behavior is needed.
- WMCYN-specific custom work should focus on verified user identity, live presence, persistent-world entry, voice, PCVR recording capture, and the handheld camera path. Verbatim is deferred.
- MCP initially exposed AgentSkill tools only. Unreal Editor Toolset access was later enabled, exposing scene/actor/asset-level tools; still do not modify Unreal assets or maps unless explicitly asked.
- `Specs/001-avf-class-selection/AFCORE_PRESENCE_BASELINE_SELECTION.md` is the current draft plan for choosing AFCore presence baselines before creating WMCYN wrappers.

## 2026-06-23

- Use `BP_Pawn_VR_Char` as the initial First Signal presence baseline for both standalone VR users and the PCVR recording user.
- Use AFCore `BP_PlayerPosition` actors as the initial level spawn markers in `L_WMCYNOnline`.
- Keep placed `BP_Pawn_VR_Char` instances as `PREVIEW_` actors tagged `WMCYN_PreviewOnly` until the runtime spawn flow is wired.
- Name the three initial spawn lanes `StandaloneVR_A`, `StandaloneVR_B`, and `PCVR_Recording`; do not revive old lead/visitor/operator terminology.
- Scaffold the first WMCYN entry widget as `/Game/WMCYN/UI/WBP_WMCYN_LoginJoin`.
- The login widget was only an early scaffold and is superseded by the 2026-07-10 persistent-world login decision.

## 2026-06-24

- Add a temporary local First Signal entry manager at `/Game/WMCYN/Core/BP_WMCYN_FirstSignalEntryManager`.
- Place one manager actor in `L_WMCYNOnline` as `BP_WMCYN_FirstSignalEntryManager_01`.
- Keep First Signal entry local-only for now: the old display-name/presence-slot UI values are superseded and should not become replicated identity, auth, or world-entry state.
- Use tags on the existing AFCore `BP_PlayerPosition` actors as the temporary spawn lookup path: `WMCYN_Spawn_StandaloneVR_A`, `WMCYN_Spawn_StandaloneVR_B`, and `WMCYN_Spawn_PCVR_Recording`.
- Let the login widget move the local pawn to the selected spawn marker for this pass. Do not treat this as the final same-world multiplayer entry architecture.
- Keep `WMCYN_PreviewOnly` pawns as editor helpers and hide/disable them at runtime from the entry manager.

## 2026-06-25

- Stabilize editor VR Preview before deeper First Signal behavior work. The renderer crash path is mitigated by keeping editor startup out of forced VR and using DX11 for the current PCVR test lane.
- `L_WMCYNOnline` had AFCore `BP_PlayerPosition` spawn markers but no standard Unreal `PlayerStart`, so VR Preview could spawn at an engine/default location before the login relocation fired.
- Add one standard `PlayerStart` named `PLAYERSTART_FirstSignal_Default` at `SPAWN_FirstSignal_StandaloneVR_A` as the deterministic initial entry point.
- The original First Signal spawn area was under/inside upper Crib collision, which could push the VR pawn into a roof-like surface. Move the First Signal spawn strip to nearby floor samples around `X 260`.
- Do not use screen-space `AddToViewport` UMG inside headset tests. Keep `WBP_WMCYN_LoginJoin` for later, but replace it with a world-space VR panel before using it in VR.
- Place `BP_WMCYN_VRPreviewStabilizer_01` in `L_WMCYNOnline` to set HMD tracking origin to `LocalFloor` and reset orientation/position on BeginPlay for the VR Preview lane.
- If tracking remains offset after this, handle it as a focused pawn/tracking-origin investigation, not as a framework rewrite.
- Retire the three placed `PREVIEW_FirstSignal_*` mannequin actors from the live level. The editor scene should show one Jared preview presence, not three local role mannequins.
- Do not place imported `/Game/JaredMetaHuman/Content/MetaHumans/Jared/BP_Jared` directly in `L_WMCYNOnline`; its `LiveLinkSetup` graph has stale `ABP_MH_LiveLink` / `LLink_Face_Subj` compile issues.
- Use `/Game/WMCYN/Pawns/BP_WMCYN_JaredPreview_Quest` as the static Jared preview actor for now. It is a WMCYN-owned duplicate with LiveLink disabled for preview.
- Quest-first does not mean AFCore's mobile/touch pawn family. The AFCore `BP_Pawn_Mobile_Char_Example_Quest` asset is a thin mobile example wrapper.
- The AFCore `BP_Pawn_VR_Char_Example_Quest` asset is also a thin example wrapper, parented to `BP_Pawn_VR_Char`, with demo `PawnUI` behavior. Use it as reference, not as the WMCYN runtime parent.
- For standalone headset work, use WMCYN child classes of the AFCore VR pawn family.
- Create `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` as the current WMCYN runtime pawn hook, parented to AFCore `BP_Pawn_VR_Char`.
- Create `/Game/WMCYN/Data/DA_WMCYN_FirstSignal_Level` as the WMCYN Level Data asset and set its `default_Pawn_VR` to `BP_WMCYN_QuestUserPawn`.
- Add `BP_MapInfo_WMCYN_FirstSignal` to `L_WMCYNOnline` and point its `levelKey` at `DA_WMCYN_FirstSignal_Level`. Leave menu/lobby keys and desktop/mobile pawn defaults unset until their paths are intentionally chosen.
- Repair the imported Jared source Blueprint only where it blocks editor play: remove the stale `ABP_MH_LiveLink` / `LLink_Face_Subj` branch from `LiveLinkSetup` and repoint obvious body/face/clothing mesh refs to the actual imported `/Game/JaredMetaHuman/Content/...` assets.
- Keep the level using the WMCYN-owned duplicate `/Game/WMCYN/Pawns/BP_WMCYN_JaredPreview_Quest`, not the imported source `BP_Jared`, for the current static preview.
- Treat the current Jared visual as rough editor presence. The nested MetaHuman import still has missing `/Game/MetaHumans/...` soft references and should not become the runtime Quest avatar path until the import is cleaned or a Quest-safe visual wrapper is chosen.
- In-viewport PIE is allowed as a compile/stability smoke test, but it currently spawns AFCore `BP_Pawn_Mobile_Char_C_0`. Do not interpret that as failure of the `default_Pawn_VR` path until VR Preview/headset testing checks `BP_WMCYN_QuestUserPawn`.

## 2026-07-07

- Pivot First Signal avatar/presence away from Jared MetaHuman preview work.
- Remove the placed Jared preview actor from `L_WMCYNOnline`; keep MetaHuman/Jared assets only as deferred content for a later avatar-fidelity ticket.
- Use `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` as the active First Signal Quest/VR runtime pawn hook.
- Keep `BP_WMCYN_QuestUserPawn` parented to AFCore `/Game/AFCore/Blueprints/Pawn/VR/BP_Pawn_VR_Char` so AFCore remains responsible for VR movement, controller/head presence, replication support, and the existing name-tag UI path.
- Accept AFCore/default head, torso, hand/controller, capsule, mannequin, or simple placeholder visuals for First Signal. Basic working presence is the target; avatar fidelity comes later.
- Do not spend current First Signal time on Jared MetaHuman materials, grooms, clothing post-process, LiveLink, ARKit, face animation, or Quest-ready MetaHuman optimization.
- In-viewport PIE is still only a compile/runtime smoke test, but it now uses the WMCYN First Signal GameMode/Controller wrapper path and spawns `BP_WMCYN_QuestUserPawn`. Quest/VR Preview hardware testing must still confirm left analog movement, head/controller presence, comfort, and PCVR visibility.

## 2026-07-08

- Use WMCYN-owned GameMode and PlayerController wrapper assets for First Signal level ownership instead of editing AFCore framework assets.
- `BP_WMCYN_GameMode_FirstSignal` is the level GameMode wrapper for `L_WMCYNOnline`.
- `BP_WMCYN_PlayerController_FirstSignal` inherits AFCore `BP_PlayerController_Main`, forces the VR pawn selection lane, and syncs the runtime WMCYN Quest pawn into AFCore's `playingPawn` reference.
- Keep First Signal local test entry deterministic by using `SPAWN_FirstSignal_StandaloneVR_A` as the first local standalone VR spawn.
- Keep spawn marker indices simple: `StandaloneVR_A = 0/default`, `StandaloneVR_B = 1`, `PCVR_Recording = 2`.
- Use `BP_WMCYN_VRPreviewStabilizer` as a temporary WMCYN-owned VR Preview stabilization hook. It may reset HMD tracking and snap the current local WMCYN Quest pawn to `WMCYN_Spawn_StandaloneVR_A`.
- This stabilizer snap is not the final multiplayer spawn/runtime system. It is a local First Signal test-lane fix to stop the pawn from starting at the PCVR marker and collision-shoving above the Crib.
- AFCore assets remain untouched for this fix.

## 2026-07-09

- Treat the imported Crib FBX scene as visual scenery for First Signal collision, not as the authoritative gameplay collision source.
- Use a WMCYN-owned visible static mesh debug floor in `L_WMCYNOnline` for the current walkable test lane.
- Do not rely on plain `BlockingVolume` actors or component-only collision slabs for this AFCore VR pawn lane unless a future headset test proves the pawn responds to them correctly.
- Keep the debug floor visible until VR Preview confirms the pawn lands on it; hide or replace it with polished invisible collision only after the hardware path passes.
- If VR Preview still falls through after the WMCYN pawn defaults report walking/blocking in PIE, treat the next investigation as "VR Preview is using or overriding a different runtime pawn/movement path," not as another floor-geometry pass.
- Keep the collision pass narrow: stabilize the First Signal spawn/movement lane before adding identity, nameplates, Verbatim, PCVR recording tooling, or more UI.
- Imported `/Game/Environments/WMCYN_Crib/Scene*` static mesh assets may have their generated/simple collision disabled for this milestone; AFCore assets remain untouched.
- Because hardware VR still fell while MCP PIE reported a valid walkable floor, add a temporary WMCYN-only floor guard to `BP_WMCYN_VRPreviewStabilizer`. It forces the WMCYN Quest pawn back to `MOVE_Walking` and rescues only extreme Z drift below the debug floor or above the current test lane.
- Treat the floor guard as a First Signal hardware validation safety rail, not final locomotion or collision architecture. Remove or replace it after the Crib collision lane is proven stable in headset.
- Jared's headset retest confirmed the floor guard fired with `WMCYN FloorGuard: rescued Quest pawn below floor`. This was a useful diagnostic, then superseded by the 2026-07-10 removal of the height rescue.
- Next gate is a headset retest: the WMCYN Quest pawn should stay on the Crib floor during stick locomotion without falling through or being pushed above the world.
- Do not assume a placed Crib component is gameplay-ready just because its component collision profile blocks. Inspect the static mesh BodySetup too.
- Imported Crib meshes can have no simple collision shapes while still being set to `Use Simple Collision As Complex`; that creates a physically hollow floor for CharacterMovement.
- Initially treat `/Game/Environments/WMCYN_Crib/Scene_096` (`Plane_102`) as the current spawn-floor collision candidate. This was superseded on 2026-07-10 when its convex hull created an invisible raised-floor feel.
- The temporary debug floor and WMCYN floor guard were validation scaffolding. They are superseded by the 2026-07-10 real-floor pass.

## 2026-07-10

- If the Crib floor holds but the user visibly hovers, inspect WMCYN stabilizer spawn height before adding more collision. The old debug-floor path lifted the pawn root to `Z 649.15`, which is too high for the real Crib floor.
- Remove the `WMCYN FloorGuard` height rescue from `BP_WMCYN_VRPreviewStabilizer`; keep only the HMD reset, spawn-marker snap, and temporary `MOVE_Walking` safety.
- Snap the local test pawn directly to the complete `SPAWN_FirstSignal_StandaloneVR_A` transform instead of adding the old 92 cm debug-floor offset. Reapply marker rotation as well as location after HMD reset so native pawn initialization cannot change the approved entry facing.
- Do not use a coarse convex hull for the current Crib walking floor if it creates an invisible raised-floor feel. Prefer the imported floor mesh set to `Use Complex Collision As Simple` for this early validation lane.
- Latest MCP PIE result: `BP_WMCYN_QuestUserPawn0` starts near `X 14.1, Y -700, Z 553.94`, reports `MOVE_Walking`, `walkable=true`, and stands on imported Crib component `Plane_003`.
- Start a dedicated First Signal audio capability lane before identity/nameplates go too far. Voice is core to the Crib, not optional polish.
- Use hybrid audio for MVP: prove in-game voice for world presence/reference, but keep external/real mics available for final clean recording until in-game audio quality is proven.
- Do not edit AFCore for voice. If VOIP setup is needed, prefer WMCYN-owned wrapper/registration around the active WMCYN Quest pawn.
- Treat the current `Null` online subsystem as a local/LAN validation path, not the final persistent-world voice backend.
- VR Preview audio output through the Oculus virtual headphone device is good enough to continue; do not chase the WASAPI raw-mode warning unless audio output actually fails.
- Do not treat `LogVoiceEncode` alone as proof that voice works. First Signal still needs a microphone capture and user-to-user voice test.
- The OpenXR mapping-context warning is acceptable temporarily if it preserves working AFCore controller input. Clean it up only through a complete Enhanced Input migration that includes AFCore locomotion/controllers, not by registering a partial WMCYN-only context.
- First Signal product flow is now: username-or-email/password login -> persistent WMCYN Crib world -> verified user identity appears in-world.
- First Signal uses only one identifier field, password, and Enter World for entry into the persistent Crib.
- Login stores display name/username, presence mode (`Quest` or `PCVR`), and temporary capabilities such as `Recording` and `CanTriggerVerbatimMarker`.
- Successful login enters `L_WMCYNOnline` / The WMCYN Crib. The Crib is the world.
- Use AFCore's built-in pawn name tag for player display names: `/Game/AFCore/Blueprints/Widgets/Pawn/Widget_NameTag`. `Widget_Tag_Simple` is generic board/signage UI, not the pawn identity widget.
- Next implementation layer is identity/nameplate/presence only after the active VR runtime path stays stable in headset.
- Use `/Game/WMCYN/Core/BP_WMCYN_PlayerState_FirstSignal`, a child of AFCore `BP_PlayerState_Main`, as the first replicated WMCYN identity/presence store.
- First Signal PlayerState fields start as `Username`, `DisplayName`, `PresenceMode`, and `Capabilities`.
- `BP_WMCYN_GameMode_FirstSignal` owns the WMCYN PlayerState assignment. AFCore PlayerState is not edited.
- `WBP_WMCYN_LoginJoin` has been visually corrected to username/password/Enter World, but its internal graph cleanup is a follow-up implementation task.
- Add WMCYN-owned `/Game/WMCYN/Audio/BP_WMCYN_VoiceRegistrationComponent` instead of editing AFCore voice logic. It registers the WMCYN Quest pawn's `VOIPTalker` with PlayerState during BeginPlay.
- Add WMCYN-owned `WMCYN_VOIPTalker` to `BP_WMCYN_QuestUserPawn` and point the inherited AFCore `vOIPComponent` reference at it. This keeps voice setup in the WMCYN child path.
- Do not consider First Signal voice passed just because the VOIPTalker registers. The next proof is Quest mic capture, user-to-user audio, PCVR monitoring, and OBS capture.
- After headset `ToggleSpeaking 1` still returned `StartLocalProcessing(0) returned 0xFFFFFFFF`, extend the WMCYN voice registration component to call Unreal's `CreateTalkerForPlayer` and lower mic threshold before the next headset retest. If that does not change the error, treat the blocker as below the Blueprint talker layer.
- Treat OpenXR push-to-talk as an input migration issue, not an AFCore issue. Do not globally register a partial WMCYN Enhanced Input context unless it also includes the full AFCore controller/locomotion action set. Keep the WMCYN `IA_WMCYN_PushToTalk` / `IMC_WMCYN_FirstSignal_VR` assets parked and preserve the working AFCore legacy mappings for First Signal hardware validation.
- For the immediate voice proof, auto-run `ToggleSpeaking 1` from the WMCYN-owned voice registration component after VOIPTalker registration. This avoids touching AFCore input while the team tests whether Unreal local voice capture works at all.
- Track the tiny visible player/avatar as a separate First Signal avatar scale bug. Current evidence says actor scale, capsule height, AFCore head/torso component scale, and `worldToMeters` are normal, so do not fix it by changing global world scale.

## 2026-07-11

- Latest headset log proves the WMCYN auto-speaking path fires: `CreateTalkerForPlayer`, `RegisterWithPlayerState`, and `ToggleSpeaking 1` all run.
- `StartLocalProcessing(0) returned 0xFFFFFFFF` remains after auto-toggle, so the blocker is below the VOIPTalker/button layer.
- UE 5.8 source inspection shows `StartLocalVoiceProcessing(0)` returns failure when local user `0` is not the voice engine's registered owning user.
- `OnlineSubsystemNull` normally registers local talkers when a technical online session registers local players. The user-facing product model remains one persistent WMCYN world with no session picker/access code, but the implementation may still need an invisible technical session or WMCYN-owned local-talker registration bridge for Unreal voice.
- Do not globally register the partial WMCYN Enhanced Input context just to fix push-to-talk. Controllers and locomotion working through AFCore legacy input are more important for the current gate.
- Added explicit OnlineSubsystem voice diagnostic keys: `MaxLocalTalkers=1`, `MaxRemoteTalkers=8`, `VoiceNotificationDelta=0.200000`, and `bDuckingOptOut=true`.
- Implement the voice unblock as a WMCYN-owned hidden technical OnlineSubsystem session inside `BP_WMCYN_VoiceRegistrationComponent`, not as AFCore edits and not as a user-facing session/lobby/access-code flow.
- The AFCore documentation notes that AFCore already has multiplayer/session UI/data patterns and that WMCYN should inspect/reuse framework paths before building custom networking. The implementation follows that by using Unreal/AFCore-compatible session plumbing only as an internal voice registration helper.
- In-viewport PIE now proves the missing layer: hidden session creation registers local talker `0`, after which `StartLocalProcessing(0)` returns `0x00000000`.
- Headset VR Preview now also proves local voice capture: the Oculus virtual headset mic is selected, local talker `0` registers, local voice processing starts, voice frames encode, and local TALKING/NOTTALKING events fire.
- After that proof, VR Preview began crashing on startup with the crash stack passing through `OnlineSubsystemUtils` and `OnlineSubsystemNull`. Park automatic hidden session creation, `ToggleSpeaking 1`, and `online voice dump` on BeginPlay until VR Preview stability is reconfirmed.
- A follow-up in-viewport smoke run still crashed through `OnlineSubsystemUtils` / `OnlineSubsystemNull` before WMCYN voice registration messages. Fully park the WMCYN voice registration component and temporarily disable Unreal voice config until VR Preview stability is restored.
- Reintroduce local talker registration later as an explicit isolated pass, not as automatic BeginPlay behavior.
- Continue to call this a technical runtime registration helper in WMCYN docs. Product language remains: username-or-email/password login -> persistent WMCYN Crib world.

## 2026-07-12

- The VR Preview crash after the successful local mic proof was later determined to be unrelated to the WMCYN voice implementation and tied to a `DefaultEngine.ini` issue.
- Restore the complete working WMCYN voice path instead of keeping voice parked.
- Keep the implementation WMCYN-owned: `BP_WMCYN_VoiceRegistrationComponent` handles mic threshold, `CreateTalkerForPlayer`, `RegisterWithPlayerState`, hidden technical session creation, and automatic `ToggleSpeaking 1`.
- Re-enable project voice config for the First Signal audio lane: `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.
- Continue treating the hidden technical session as internal Unreal voice plumbing only. The product model remains username-or-email/password login into the persistent WMCYN Crib world.
- Keep `MaxLocalTalkers=1`. UE 5.8 defines `MAX_SPLITSCREEN_TALKERS` as one on Windows, and `FVoiceEngineImpl::Init` indexes its fixed local-talker array using the configured value. The previous value of four caused repeatable memory corruption and a VR Preview access violation during `OnlineSubsystemNull` voice startup. Each First Signal device has one local user; the other users belong in the remote-talker pool.
- Close the standalone First Signal audio-capture lane: Oculus headset microphone capture, local talker registration, voice encoding, and speaking state have been proven in VR Preview.
- Keep hybrid audio as the First Signal MVP plan: in-game voice provides world presence and reference audio; external/real microphones remain the backup and final clean recording path.
- Remove automatic `online voice dump` calls from normal startup and keep `LogVoiceEngine` at `Error` by default. This suppresses the recurring single-user PIE packet/drop warnings; detailed voice dumps remain an explicit troubleshooting action.
- Move active implementation to identity/nameplates. `WBP_WMCYN_LoginJoin` now uses only username, password, and Enter World; its old selector and marker-relocation path are no longer active.
- On Enter World, submit the username through inherited `Comp_PlayerInfo_Basic.SetPlayerName`. Its existing server update and `Updated_PlayerName` dispatcher remain the networked name source.
- `BP_WMCYN_PlayerState_FirstSignal` listens to `Updated_PlayerName` and, on server authority, mirrors `Username`, `DisplayName`, the current Quest-first `PresenceMode`, and temporary `Capabilities` into WMCYN-owned replicated fields.
- Drive AFCore `Widget_NameTag` through its intended `Comp_PlayerInfo_Basic.Updated_PlayerName` path while retaining WMCYN/Unreal PlayerState identity fields; do not edit AFCore assets or create a second visual nameplate system unless it fails in a multi-user test.
- Do not embed AFCore `Widget_Input_TextBox` in WMCYN UMG assets. Its UE 5.8 Designer/thumbnail path through `Widget_Base` design-time theming and `AFCore_Border` can crash Slate/UMGEditor.
- Keep username and password as native Unreal `EditableTextBox` controls inside `WBP_WMCYN_LoginJoin`.
- Reuse AFCore's runtime `BP_Overlay_Widget_Keyboard` and `Widget_Keyboard_US` through the WMCYN login widget's `Widget_Base` parent instead of rebuilding a keyboard or modifying AFCore.
- Preserve the active controller's virtual-user text focus with AFCore `WidgetInteractionComponent.SetFocus`. Reuse one keyboard overlay while switching between username and password.
- Spawn the keyboard from the login root's lower edge, not from a field-relative point. The keyboard must sit fully beyond the parent `Comp_Widget` collision rectangle; a merely visible/coplanar overlay can still lose every controller trace to the login panel.
- Keep AFCore `BP_Overlay_Widget_Keyboard`, `Widget_Keyboard_US`, and the source `WidgetInteractionComponent`; the hit-plane correction belongs in the WMCYN wrapper. Final headset interaction confirmation remains the next login gate.
- Keep the world-space VR login gate in `Game Only` input mode while separately ignoring locomotion input. AFCore's controller Select path must continue receiving trigger presses after an editable field gains virtual-user focus.
- Give native WMCYN login fields explicit dark normal/focused/read-only foreground colors and placeholders; do not inherit AFCore's light parent foreground onto white field backgrounds.

## 2026-07-13

- Headset testing confirms the existing AFCore runtime keyboard now selects and types into both native WMCYN username/password fields. Do not replace this path with a custom keyboard.
- Keep one WMCYN-owned login submission path. `BTN_EnterWorld.OnClicked` and password `OnTextCommitted(OnEnter)` both call `SubmitLogin`.
- A successful login must explicitly close the AFCore keyboard overlay and destroy the owning world-space entry manager after restoring game input and locomotion. `RemoveFromParent` alone is insufficient for a widget hosted by an actor `WidgetComponent`.
- Resolve VR login field selection from the pointer event's screen position and each native field's cached geometry, with password checked before username. Do not use persistent `IsHovered` state to choose the target field because a stale virtual-user hover can redirect password clicks to username.
- Use AFCore `Widget_NameTag` and the WMCYN replicated identity path for First Signal display names. AFCore's `NameTag` terminology is correct for pawn identity; `SimpleTag` refers to a separate generic board label.
- Correct listen-server possession timing in the WMCYN child pawn with a delayed local-hidden/remote-visible refresh of the inherited AFCore NameTag. This is a WMCYN compatibility wrapper, not an AFCore asset change.
- Assign presence slots on server authority with a GameMode-owned monotonic counter: `0 = StandaloneVR_A`, `1 = StandaloneVR_B`, and `2 = PCVR_Recording`.
- Keep `BP_WMCYN_VRPreviewStabilizer` spawn correction authority-only. Network clients must accept the indexed server spawn instead of snapping themselves before PlayerState replication arrives.
- Register every replicated pawn's `VOIPTalker` with that pawn's own PlayerState, but run local session creation, microphone setup, and speaking activation only for the locally controlled pawn.
- Keep the PCVR recording user on the shared AFCore VR pawn family until an actual OBS/capture test proves it insufficient; inspect `BP_Pawn_VR_Camera` only after that failure.
- A three-client listen-server PIE run is the current desktop proof for indexed presence, replicated WMCYN metadata, AFCore remote NameTags, and per-world local voice ownership. It does not close the separate-device Quest-to-Quest hearing or OBS acceptance gates.
- Treat the earlier tiny-player report as incomplete body coverage, not a global scale failure. The active AFCore head and torso meshes have normal human dimensions and all relevant transforms remain at scale `1.0`; together they form only a roughly 90 cm bust with no pelvis or legs.
- Do not enlarge the AFCore head/torso placeholders. Scaling those meshes would distort proportions without creating a full body.
- Do not assign AFCore `SK_Framework_Movement_Mannequin` or `AnimBP_Mannequin` as a complete VR body by itself. The AFCore Animation Blueprint has idle/run/jump states but no HMD/controller-driven full-body IK.
- Add WMCYN-owned `/Game/WMCYN/Dev/AvatarMirror/BP_WMCYN_AvatarMirror`, based on AFCore's existing render-target mirror pattern, and place `DEV_AvatarMirror_Tracking` behind StandaloneVR_A. No AFCore asset is edited.
- Use the mirror check to decide between a lightweight WMCYN three-point tracked body proxy and a later full-body IK wrapper. Final avatar fidelity must not block the basic replicated-presence gate.
- Select WMCYN-owned children of AFCore's left/right hand controller classes from `BP_WMCYN_QuestUserPawn`. Preserve AFCore's controller interaction, laser, teleport, finger, and replication components instead of rebuilding hands.
- Use AFCore's `SK_Framework_Movement_Mannequin` and unmodified `AnimBP_Mannequin` only as the mesh and stable locomotion/base pose. They do not own VR tracking.
- Implement First Signal VR tracking with WMCYN-owned `WMCYN_VRBodyMesh`, `WMCYN_VRBodyRig`, and `CR_WMCYN_VRBody`. Head and hands come from inherited AFCore tracking components.
- Do not infer pelvis and foot locations from the HMD for First Signal. Bypass the full-body PBIK pass and let AFCore's input pose own the torso and legs; head-to-pelvis solving distorted the chest and created unwanted knee behavior.
- Solve each arm separately with WMCYN two-bone IK. Use a world-space elbow pole derived from the shoulder/hand midpoint plus a body-relative outward/back bias; do not use a fixed world-direction pole vector.
- Keep AFCore head and WMCYN hand actors as the visible endpoints; hide the mannequin head and hand bones to avoid duplicate geometry.
- Treat the headset-confirmed right arm as the tracked-body calibration reference. Correct the left arm by mirroring the skeleton-axis signs required by the mannequin hierarchy; do not retune the working right arm.
- Correct the head-in-torso issue by offsetting only the inherited visible AFCore head component from the WMCYN pawn wrapper. Do not move the HMD camera, camera proxy, capsule, torso, or AFCore source asset.
- A second unpossessed WMCYN pawn in PIE is the current remote-render proof: the full adult-scale body renders from another user's view. Local mirror articulation and real multi-device replication still require headset evidence.
- Do not edit AFCore assets for this body pass. All controller selection, mesh ownership, Control Rig logic, and compatibility behavior remain under `/Game/WMCYN`.

## 2026-07-14

- The initial entries below document the superseded AFCore-derived Mimic experiment. They remain as failure history and rollback context, not as instructions for the production player pawn.
- Use the migrated Mimic Pro VR IK system as a WMCYN-owned body layer, not as a replacement pawn, GameMode, input stack, or locomotion system.
- Keep `/Game/WMCYN/Pawns/BP_WMCYN_QuestUserPawn` unchanged as the stable rollback point. The active validation pawn is `/Game/WMCYN/Pawns/Experimental/BP_WMCYN_QuestUserPawn_Mimic`, a true child of that pawn.
- Keep AFCore responsible for locomotion, HMD/controller tracking, interaction, hand actors, voice, identity, spawn handling, and multiplayer behavior. Mimic receives those tracking references and drives only the Manny body.
- Use a WMCYN-owned child component, `BP_WMCYN_MimicBodyComponent`, for UE 5.8 compatibility guards. Do not edit the migrated Mimic source component for WMCYN-specific behavior.
- Attach the Mimic component to `WMCYN_VRBodyMesh` during construction so its BeginPlay can discover the skeletal mesh, `ABP_VRBody`, and `CR_VRBody` in the order the package expects.
- Superseded: the initial integration passed the body mesh as Mimic's `VR Origin`. Direct sample comparison showed this was semantically wrong; AFCore `CameraHandle` is now passed as the tracking origin.
- Leave the solver idle when no HMD is active. Ordinary PIE remains a runtime/inheritance smoke test; VR Preview is the authoritative IK validation lane.
- Keep the prior WMCYN arm-only Control Rig implementation on the base pawn as rollback until the Mimic mirror, Quest performance, replication, and floor-contact gates pass.
- GameMode currently selects the Mimic child for the next hardware test. Rolling back requires only restoring `BP_WMCYN_GameMode_FirstSignal.DefaultPawnClass` to `BP_WMCYN_QuestUserPawn`.
- Disable `Owner No See` only on the Mimic child's body mesh at runtime. Use Manny's attached head like the sample, hide the separate AFCore static bust, and keep only Manny's duplicate hand bones hidden.
- Supersede the fixed 6 cm lift with a WMCYN visual floor anchor: place `WMCYN_VRBodyMesh` at AFCore `getPawnGroundLocation` plus the current 12 cm sole clearance after AFCore ticks. Do not use Mimic's sample `Dynamic Adjustments`, which would also modify capsule size, actor position, VR origin, movement speed, and input.
- Reject the detection-only seated pass. Hardware testing proved that merely labeling a low HMD as seated leaves the camera in the torso and drives Mimic into a folded-knee pose.
- Reject the follow-up per-tick `CameraHandle` rewrite as well. Hardware testing showed the view was far off. WMCYN must not derive a standing offset from AFCore capsule half-height or fight AFCore's own handle correction.
- Treat Mimic's sample as the source of truth for later seated calibration: it uses a dedicated `Height Offset`, the rig's approximately 168.06 cm `FullBody_Component` marker, and an explicit calibration action. Adapt that structure in a WMCYN-owned layer only after the restored AFCore camera baseline passes.
- Pass AFCore `CameraHandle`, not the Manny body mesh, into Mimic's `VR Origin` reference.
- Use Manny's attached skeletal head like the sample and hide the separate AFCore static head/torso bust. Keep Manny's duplicate hand bones hidden because AFCore/WMCYN hand actors remain the hand endpoints.
- Use an isolated duplicate map and WMCYN child pawn to test Mimic's native body implementation before adapting more calibration code into the First Signal pawn. Do not replace the active First Signal GameMode or modify `L_WMCYNOnline` for this diagnostic.
- Override only the native test child's incompatible capsule defaults. The imported one-centimeter custom physics capsule falls through the repaired Crib floor; the test child uses the already-proven AFCore 26 cm `Pawn` capsule while leaving Mimic's camera, `VR Origin`, `Height Offset`, body, input, and calibration graphs unchanged.

### Current Native-Pawn Decision

- Headset validation of the native Mimic lane passes after left-thumbstick calibration: camera/body alignment, full-body presentation, locomotion, footsteps, and Crib-floor collision work correctly.
- Promote the proven child to `/Game/WMCYN/Pawns/BP_WMCYN_UserPawn_FirstSignal` and make it the First Signal player source of truth. Do not continue tuning the AFCore-derived Mimic wrapper toward the same result.
- Preserve native Mimic body, tracking hierarchy, seated calibration, locomotion, hand/body solve, and footsteps. Build WMCYN identity, login, voice, nameplate, indexed spawn, capabilities, and pose replication around it with WMCYN-owned adapters.
- Use `BP_WMCYN_UserPawn_FirstSignal` as the shared default avatar/pawn for standalone VR and PCVR recording users. Consider a camera-specific PCVR pawn only if an OBS capture test proves the shared native pawn insufficient.
- Keep the AFCore-derived Quest pawn and isolated native test map as rollback/diagnostic paths until the production pawn passes login, voice, replicated pose, and three-device gates.
- Use Mimic's existing procedural foot-target system as the first walking-animation pass. `ABP_VRBody` already updates `CalculateFeetTargets` from HMD/world velocity, so do not add a competing AFCore locomotion AnimBP or custom walk-cycle system before headset testing.
- `BP_WMCYN_UserPawn_FirstSignal` is now the production `DefaultPawnClass` in `BP_WMCYN_GameMode_FirstSignal`; `L_WMCYNOnline` no longer uses the AFCore-derived experimental body lane.
- Keep indexed slot assignment in the existing WMCYN GameMode/PlayerState contract, but perform pawn placement through generic Pawn/Character APIs. Do not restore AFCore `BP_Pawn_Base`, `playingPawn`, or `PawnReady` assumptions around the native pawn.
- Use the WMCYNRuntime `WMCYN_FirstSignalPresence` component as the native-pawn adapter for identity, VOIPTalker registration, remote-only nameplate rendering, and replicated head/hand transforms. Do not edit AFCore or the imported Mimic parent.
- Login identity is server-authoritative. `SubmitLocalFirstSignalIdentity` sends a reliable pawn-owned RPC; the server sanitizes and writes `Username`, `DisplayName`, and standard PlayerName. Password text is never stored, and clients cannot assign their own capabilities.
- Replicate head, left-controller, and right-controller relative transforms at 20 Hz with owner skipping and remote interpolation. Character movement remains responsible for root locomotion replication.
- Three-client production PIE is the desktop acceptance proof for distinct indexed spawns, possession, remote nameplate visibility, native voice registration, and tracked-pose transport; a focused two-client probe separately proves both direct server identity submission and the client-to-server identity RPC. Physical Quest/PCVR testing remains required for comfort, hardware input, hearing, performance, and OBS.
- Keep `OnlineSubsystemNull` local talker `1..3` warnings classified as PIE split-screen probe noise while real local talker `0` registers successfully. Do not mistake those warnings for separate-device voice acceptance.
- Preserve Mimic's existing trigger-to-widget-click graphs. Fix the native pawn's WMCYN login compatibility in `WMCYN_FirstSignalPresence` by changing its inherited widget interaction trace channel from `Pawn` to `Visibility`, extending interaction distance to 750 cm, and keeping hit testing enabled. Do not duplicate the trigger graph or edit the imported pawn.
- AFCore's keyboard sends characters through the overlay's source `WidgetInteractionComponent`. Because the native Mimic pawn does not populate AFCore `Comp_Widget.lastWidgetInteractionComponent`, register a WMCYN-owned focus-only interaction with the login widget host before every keyboard spawn or field switch. Give it a separate virtual user and disable hit testing so visible pointer clicks cannot steal text focus between letters.
- Attach the local right-hand pointer at zero offset to the native pawn's tracked `RightAim` motion-controller component. Use the OpenXR controller aim pose as the interaction source; do not derive laser direction from animated finger or hand bones. Keep its cyan debug line thin and scoped to the login gate, then hide it immediately after successful login.
- Enable and refresh the local runtime nameplate immediately after identity submission, but retain `Owner No See` so it renders in the avatar mirror and remote-user views without floating in front of the owner's HMD camera.
- Position every runtime nameplate from the replicated HMD camera plus a fixed world-up offset. Keep pitch and roll locked to zero and billboard only around world yaw; use player yaw when the viewer has no horizontal separation. Use `25 cm` text after the oversized headset diagnostic confirmed visibility and orientation.
- Replace the temporary WMCYN `TextRenderComponent` with a WMCYN-owned `WidgetComponent` hosting AFCore `Widget_NameTag`. Preserve the approved `25 cm` width, HMD anchor, yaw-only billboard, owner-hidden mirror/remote visibility, and replicated identity source. Mirror successful login into AFCore `Comp_PlayerInfo_Basic.PlayerName` and trigger its existing RepNotify dispatcher. Load the widget softly during `BeginPlay`; eager native-constructor loading occurs before UE 5.8 Typed Elements registration and prevents editor startup.
- Host `Widget_NameTag` with AFCore `Comp_Widget` and `DA_Theme_Default`, matching AFCore's character pawn template. A plain Unreal `WidgetComponent` bypasses AFCore's `FL_UI`/`M_UIMaster` theme initialization and can render the tag as white text on a white block. Keep the compatibility host WMCYN-owned and do not modify `M_UIMaster` or AFCore widget assets.
- Reinforce the login gate in the WMCYN native-pawn adapter. Ignore locomotion and stick-look input and stop current character movement until local identity submission succeeds; do not disable HMD/controller tracking or trigger input needed to use the menu.
- Treat the three indexed `BP_PlayerPosition` actors as WMCYN post-possession transform anchors. Keep their transforms/capsules unchanged and suppress only the misleading per-instance `BAD SIZE` editor sprite; do not scale the pawn, move the Crib, or alter AFCore's source `BP_PlayerPosition` asset.
- Treat yaw `0` on `SPAWN_FirstSignal_StandaloneVR_A` as the approved runtime entry facing. The login gate intentionally locks stick turning, so the delayed stabilizer must reapply the complete indexed marker transform after HMD reset. Reconcile the default PlayerStart yaw with that marker before packaging.
- Treat the first Quest package attempt as blocked by the local UE 5.8 installation, not by project content. Enable the engine's Android optional component and configure the matching Android toolchain before changing project packaging settings.
- Move Verbatim out of the First Signal acceptance gate. It remains a possible stretch feature after login, shared-world presence, voice, PCVR/OBS capture, and the initial camera path are proven.
- Reuse the existing Firebase backend for identity and profile bootstrap rather than inventing a second account system. Existing clients use Firebase email/password and the backend verifies Firebase ID tokens.
- Do not reuse the backend's six-digit headset pairing flow for WMCYN Online First Signal. The approved product flow remains direct credentials -> verified identity -> automatic Crib world entry.
- Keep passwords, Firebase tokens, and join tickets out of replicated Unreal state. Only verified UID, username/display name, presence mode, slot, and server-assigned capabilities may reach PlayerState.
- Treat the current Unreal login submission as a local prototype until it authenticates asynchronously. Movement may unlock only after authentication, profile/bootstrap resolution, and successful world entry.
- Use a hidden PC listen server for the first same-LAN three-device proof if needed. This is temporary infrastructure and must not add a user-facing host/session flow.
- The persistent product target is an authoritative Unreal world runtime discovered through the backend. Add explicit world runtime registration/heartbeat and authenticated entry/bootstrap contracts before claiming internet multiplayer.
- Repair or replace `C:/Users/jvred/Documents/WMCYN/wmcyn-backend-infra` from upstream before backend implementation. Its local Git metadata and route source tree are incomplete, while upstream `main` at audit commit `b2f260b` builds cleanly.
- Harden the backend before reactivation: remove public config debug output, disable `x-uid` in production, restore rate limiting, review public profile reads and anonymous admin-route access, and address dependency audit findings.
- First Signal's credential field accepts username, `@username`, or email plus password. Initial users are created manually in Firebase Authentication, `users/{uid}`, and the server-only `loginHandles/{handleNormalized}` index.
- Keep `WBP_WMCYN_LoginJoin` as the single login surface. `WBP_WMCYN_BootLogin` had zero referencers and is retired; do not create a second login state machine.
- Reuse AFCore `Widget_Multiplayer_Players` and `Widget_Multiplayer_PlayerField` through WMCYN child `WBP_WMCYN_WhosHere`; do not reimplement player enumeration or rows.
- Reuse AFCore `Widget_Settings_Audio` and its scaffold Apply/Reset path through WMCYN child `WBP_WMCYN_Settings_Audio`.
- Keep AFCore Graphics settings PCVR-only until reviewed. Do not expose the full scalability page on Quest before hardware performance validation.
- Treat the missing MSVC `14.50.35717` toolchain as the current Unreal C++ verification blocker. Do not alter project configuration to work around an engine compiler requirement.

## Open Decisions

- What machine hosts the first same-LAN listen-server proof, and how will Quest builds receive its address without exposing a session picker?
- What is the first production Unreal world-server host and deployment target after the LAN proof?
- Should First Signal in-game voice use push-to-talk or open mic plus mute?
- Which real online voice backend should replace the local `Null` validation path when persistent-world networking is ready?
- If the hidden technical session causes multiplayer/world-entry side effects later, should WMCYN replace it with a small C++/Blueprint bridge that registers local talker `0` directly?
