# Decisions

This is the running decision log. Add concise dated entries when scope, architecture, or workflow changes.

## 2026-06-22

- WMCYN Online uses Unreal Engine 5.8.
- First Signal Build is the active target, not the full future product.
- Canonical First Signal target: Bring The WMCYN Crib online with two standalone VR users and one PCVR recording user, all present in the same world, with basic presence, OBS-friendly capture from the PCVR machine, and one Verbatim world marker.
- The world is the anchor. Do not model a lead participant as a permanent user type; users enter The WMCYN Crib and receive presence/capability state.
- First Signal uses two standalone VR user presences and one PCVR recording user presence.
- HumanCodeable AFCore / Advanced VR Framework has been tested and works.
- AFCore remains the baseline framework.
- Skip the broad AFCore example-map validation loop and proceed to WMCYN integration.
- Use `L_WMCYNOnline` / `L_crib` as the WMCYN target maps.
- Preserve AFCore systems for locomotion, grab, hands, menus, and base multiplayer unless a concrete gap is proven.
- Create WMCYN wrapper/child classes only where WMCYN behavior is needed.
- WMCYN-specific custom work should focus on user identity, live presence, temporary capabilities, persistent-world entry, PCVR recording capture, Verbatim marker, and world event logging.
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
- Snap the local test pawn directly to `SPAWN_FirstSignal_StandaloneVR_A` instead of adding the old 92 cm debug-floor offset.
- Do not use a coarse convex hull for the current Crib walking floor if it creates an invisible raised-floor feel. Prefer the imported floor mesh set to `Use Complex Collision As Simple` for this early validation lane.
- Latest MCP PIE result: `BP_WMCYN_QuestUserPawn0` starts near `X 14.1, Y -700, Z 553.94`, reports `MOVE_Walking`, `walkable=true`, and stands on imported Crib component `Plane_003`.
- Start a dedicated First Signal audio capability lane before identity/nameplates go too far. Voice is core to the Crib, not optional polish.
- Use hybrid audio for MVP: prove in-game voice for world presence/reference, but keep external/real mics available for final clean recording until in-game audio quality is proven.
- Do not edit AFCore for voice. If VOIP setup is needed, prefer WMCYN-owned wrapper/registration around the active WMCYN Quest pawn.
- Treat the current `Null` online subsystem as a local/LAN validation path, not the final persistent-world voice backend.
- VR Preview audio output through the Oculus virtual headphone device is good enough to continue; do not chase the WASAPI raw-mode warning unless audio output actually fails.
- Do not treat `LogVoiceEncode` alone as proof that voice works. First Signal still needs a microphone capture and user-to-user voice test.
- The OpenXR mapping-context warning is acceptable temporarily if it preserves working AFCore controller input. Clean it up only through a complete Enhanced Input migration that includes AFCore locomotion/controllers, not by registering a partial WMCYN-only context.
- First Signal product flow is now: username/password login -> persistent WMCYN Crib world -> user identity appears in-world.
- First Signal uses only username/password login and Enter World for entry into the persistent Crib.
- Login stores display name/username, presence mode (`Quest` or `PCVR`), and temporary capabilities such as `Recording` and `CanTriggerVerbatimMarker`.
- Successful login enters `L_WMCYNOnline` / The WMCYN Crib. The Crib is the world.
- Use AFCore `NameTag` if possible for the first nameplate. If that path is awkward, use the smallest WMCYN-owned nameplate wrapper rather than editing AFCore.
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
- Continue to call this a technical runtime registration helper in WMCYN docs. Product language remains: username/password login -> persistent WMCYN Crib world.

## 2026-07-12

- The VR Preview crash after the successful local mic proof was later determined to be unrelated to the WMCYN voice implementation and tied to a `DefaultEngine.ini` issue.
- Restore the complete working WMCYN voice path instead of keeping voice parked.
- Keep the implementation WMCYN-owned: `BP_WMCYN_VoiceRegistrationComponent` handles mic threshold, `CreateTalkerForPlayer`, `RegisterWithPlayerState`, hidden technical session creation, and automatic `ToggleSpeaking 1`.
- Re-enable project voice config for the First Signal audio lane: `bHasVoiceEnabled=true`, `[Voice] bEnabled=true`, and `bAndroidVoiceEnabled=True`.
- Continue treating the hidden technical session as internal Unreal voice plumbing only. The product model remains username/password login into the persistent WMCYN Crib world.
- Keep `MaxLocalTalkers=1`. UE 5.8 defines `MAX_SPLITSCREEN_TALKERS` as one on Windows, and `FVoiceEngineImpl::Init` indexes its fixed local-talker array using the configured value. The previous value of four caused repeatable memory corruption and a VR Preview access violation during `OnlineSubsystemNull` voice startup. Each First Signal device has one local user; the other users belong in the remote-talker pool.
- Close the standalone First Signal audio-capture lane: Oculus headset microphone capture, local talker registration, voice encoding, and speaking state have been proven in VR Preview.
- Keep hybrid audio as the First Signal MVP plan: in-game voice provides world presence and reference audio; external/real microphones remain the backup and final clean recording path.
- Remove automatic `online voice dump` calls from normal startup and keep `LogVoiceEngine` at `Error` by default. This suppresses the recurring single-user PIE packet/drop warnings; detailed voice dumps remain an explicit troubleshooting action.
- Move active implementation to identity/nameplates. `WBP_WMCYN_LoginJoin` now uses only username, password, and Enter World; its old selector and marker-relocation path are no longer active.
- On Enter World, submit the username through inherited `Comp_PlayerInfo_Basic.SetPlayerName`. Its existing server update and `Updated_PlayerName` dispatcher remain the networked name source.
- `BP_WMCYN_PlayerState_FirstSignal` listens to `Updated_PlayerName` and, on server authority, mirrors `Username`, `DisplayName`, the current Quest-first `PresenceMode`, and temporary `Capabilities` into WMCYN-owned replicated fields.
- Drive the existing AFCore NameTag from that same inherited player-name path; do not edit AFCore assets or create a second nameplate system unless it fails in a multi-user test.
- Do not embed AFCore `Widget_Input_TextBox` in WMCYN UMG assets. Its UE 5.8 Designer/thumbnail path through `Widget_Base` design-time theming and `AFCore_Border` can crash Slate/UMGEditor.
- Keep username and password as native Unreal `EditableTextBox` controls inside `WBP_WMCYN_LoginJoin`.
- Reuse AFCore's runtime `BP_Overlay_Widget_Keyboard` and `Widget_Keyboard_US` through the WMCYN login widget's `Widget_Base` parent instead of rebuilding a keyboard or modifying AFCore.
- Preserve the active controller's virtual-user text focus with AFCore `WidgetInteractionComponent.SetFocus`. Reuse one keyboard overlay while switching between username and password.
- Spawn the keyboard from the login root's lower edge, not from a field-relative point. The keyboard must sit fully beyond the parent `Comp_Widget` collision rectangle; a merely visible/coplanar overlay can still lose every controller trace to the login panel.
- Keep AFCore `BP_Overlay_Widget_Keyboard`, `Widget_Keyboard_US`, and the source `WidgetInteractionComponent`; the hit-plane correction belongs in the WMCYN wrapper. Final headset interaction confirmation remains the next login gate.

## Open Decisions

- Should the PCVR recording user remain on shared `BP_Pawn_VR_Char` with spectator/camera capability support, or switch to `BP_Pawn_VR_Camera` after PIE testing?
- What exact payload should the first Verbatim marker log contain?
- Where should First Signal world event logs be stored during early alpha: on-screen, local file, Unreal log, replicated event list, or a combination?
- Should First Signal in-game voice use push-to-talk or open mic plus mute?
- Which real online voice backend should replace the local `Null` validation path when persistent-world networking is ready?
- If the hidden technical session causes multiplayer/world-entry side effects later, should WMCYN replace it with a small C++/Blueprint bridge that registers local talker `0` directly?
