# iPhone VCam Interview Spike

Date: 2026-10-14
Status: discovery only; no Unreal assets or maps changed.

## The Idea

Build a WMCYN Crib set that matches Jared's living room closely enough to stage interviews in the virtual room. Three Quest users and one PCVR user remain normal persistent-world participants. An iPhone becomes a handheld production-camera controller aimed at the virtual couch, while Unreal renders the Crib, avatars, lighting, and audio. OBS records the PC/Unreal output.

The clean first version is:

```text
physical living room reference
        -> measured / dressed Unreal Crib set
        -> Unreal VCam actor driven by iPhone ARKit pose
        -> PCVR/Unreal render of remote avatars
        -> OBS recording
```

The phone is initially controlling the virtual camera. It is not yet a phone AR camera that composites Unreal avatars over live phone video.

## Spatial Scan + Astra Object Extraction

Yes, we can extend this into a scan-to-set-dressing workflow:

```text
iPhone room scan
    -> metric room/scene data
    -> Astra identifies and separates objects
    -> cleanup / UVs / pivots / collision / LODs
    -> selected WMCYN-owned Unreal assets
    -> Crib set dressing and camera calibration
```

The important division of labor is:

- The scan is the source of spatial truth: dimensions, placement, room scale, and camera path.
- Astra is the interpretation layer: object identification, segmentation, reconstruction suggestions, and deciding what should become a separate asset.
- Blender or another controlled asset-cleanup step remains the production boundary before anything enters the Unreal project.
- Unreal owns the final materials, collision, LODs, lighting behavior, naming, and runtime performance.

Apple RoomPlan can produce parametric room data and USD/USDZ exports for supported LiDAR devices, while a dense photogrammetry/object scan can provide more visual detail. RoomPlan is useful for walls, floors, openings, and broad furniture placement; a dedicated object scan or RealityScan-style capture is better for a couch, chair, table, poster, or prop that needs recognizable surface detail. [Apple RoomPlan](https://developer.apple.com/documentation/roomplan) [RealityScan](https://www.unrealengine.com/en-US/realityscan)

### What “cut out” can mean

1. **3D object extraction:** isolate the couch, lamp, table, or wall decoration from the scan and bring it into Unreal as its own mesh. This is the useful version for the Crib.
2. **2D image cutout:** remove a background from a photo and use the result as a poster, decal, card, or billboard. This is cheaper, but it is not a walkable or interactable 3D object.

The first pilot should be one signature object, preferably the couch or a recognizable wall prop. We should not replace the whole Crib with a raw scan. Room scans often have missing backs, holes, noisy geometry, fused objects, inconsistent materials, and too many triangles for Quest. Astra can help separate and rebuild the scene, but the result still needs scale checks, watertight geometry where needed, UV/material cleanup, pivots, collision, LODs, and a Quest performance review.

### How it helps the interview idea

The scan can give us a measured couch origin, seat spacing, wall line, and operator camera position. Those become the calibration anchors for the Unreal set and the iPhone VCam. That lets us shoot a virtual interview with the same spatial rhythm as the living room while keeping the Crib's networked avatars, lighting, voice, and OBS output.

The best pipeline is therefore hybrid:

- Scan the real room for dimensions and composition.
- Use Astra to extract only the high-value objects that make the room feel like Jared's living room.
- Let Noah build the intentional set dressing and lighting around those anchors.
- Keep the current Crib shell, collision, and multiplayer layout stable.
- Use the resulting anchors to calibrate the iPhone-driven virtual camera.

### Privacy and asset boundary

A living-room scan contains private spatial information. Keep the raw scan outside the Unreal repository and only bring in approved, cleaned exports. The Unreal repo should receive the selected WMCYN-owned meshes and metadata, not the full raw scan or unnecessary views of the home.

### Recommended scan pilot

1. Scan the living room and export the room structure.
2. Capture one separate high-quality pass of the couch or one signature prop.
3. Use Astra to label/separate that object.
4. Clean the result and verify real-world scale outside Unreal.
5. Import it under a future WMCYN-owned scanned-prop folder.
6. Place it beside the current Crib couch as an A/B comparison.
7. Keep whichever version gives the best interview silhouette, performance, and visual identity.

This proves the idea without making scan reconstruction a prerequisite for First Signal.

## Why It Fits What We Have

- The Crib already has a recognizable couch/conversation area and named production-camera lanes.
- The persistent world already supports three Quest presences plus one PCVR recording presence.
- The native Mimic pawn remains the body/tracking source of truth for headset users.
- The PCVR user already carries the recording capability and is the natural owner of camera controls.
- AFCore already has spectator, camera, HUD, and recording references that should be inspected before new systems are created.
- The native iOS app remains the account/avatar/pairing companion. It does not need to become the camera app for the first proof.

## Unreal Camera Route

Epic's UE 5.8 VCam workflow is the closest existing fit:

1. Enable and validate `VirtualCamera`, `LiveLink`, and `Take Recorder` in a disposable camera test map.
2. Place a VCam/Cine Camera actor aimed at the Crib couch.
3. Run Epic's Live Link VCAM app on the iPhone.
4. Connect the phone and Unreal machine on the same network.
5. Use the phone's ARKit motion tracking to drive camera position and rotation.
6. Calibrate the physical couch/room origin to the virtual couch/room origin.
7. Record the PC/Unreal output through OBS; optionally record the camera transform with Take Recorder for repeatable reshoots.

Epic's official workflow uses a VCam actor, Live Link, Pixel Streaming/connection tooling, and the Live Link VCAM iOS app. The project currently lists Live Link-related and Apple ARKit support, but `VirtualCamera`, `TakeRecorder`, and the exact VCam connection plugins are not explicitly listed in `wmcyn_online.uproject`. This is a plugin-validation task, not permission to edit the project file yet.

References:

- [Controlling a Virtual Camera Actor using Live Link](https://dev.epicgames.com/documentation/en-us/unreal-engine/controlling-a-virtual-camera-actor-using-live-link-in-unreal-engine)
- [Using Multiple Virtual Cameras](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-multiple-virtual-cameras-in-unreal-engine)
- [Unreal VCam Tools and Configuration](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-vcam-tools-and-configuration)

## Physical Room to Crib Set

### Living Room Photo References (2026-10-14)

Jared supplied four reference photos. These establish appearance and relative layout; they do not provide a metric scan or enough coverage to extract complete 3D furniture. The phone measurements below establish initial scale; remaining dimensions and concealed surfaces are unverified.

| Reference | Observed features | Use in the interview set |
|---|---|---|
| Photo 1 | Black modular couch beneath a large WMCYN banner; colorful flower cushions, green rug, floor lamp, red racing seat, window at the right | Main interview backdrop. Start with the couch/banner/rug composition and a camera facing the couch. |
| Photo 2 | Large window with blinds, desk/monitor workstation, wall-mounted TV, office chair, racing setup | Adjacent production corner. Candidate location for a virtual video-player screen after shot composition is settled. |
| Photo 3 | Open kitchen, white upper cabinets, wood lower cabinets, countertop, poster beside the couch area | Context for a wider establishing shot; secondary to the couch interview setup. |
| Photo 4 | Entry passage, doors, counter edge and stools | Circulation reference. Measure clearance before defining physical operator movement or virtual furniture placement. |

The first blockout should include the couch wall, couch silhouette, rug footprint, window opening, and lamp. Model the banner as a separate surface using the original artwork where available. Add the workstation, screen, kitchen, and entry details after the primary interview shot works.

Use a simple couch proxy before attempting detailed object extraction. Dark upholstery, rug fibers, blinds, and reflective appliances may need targeted capture or manual modeling; their production quality cannot be inferred from these photos. Keep decorative geometry separate from simple floor/furniture collision.

### Initial Phone Measurements (2026-10-14)

| Dimension | Jared's measurement | Converted centimeters | Rounded blockout value |
|---|---|---|---|
| Couch overall width | 6 ft 11 in | 210.82 cm | 211 cm |
| Usable seating width, left to right | 6 ft | 182.88 cm | 183 cm |
| Ceiling height above floor | 9 ft | 274.32 cm | 274 cm |
| WMCYN flag height | 3 ft 7 in | 109.22 cm | 109 cm |
| WMCYN flag width | 6 ft | 182.88 cm | 183 cm |
| WMCYN flag bottom edge above floor | Approximately 4 ft 3 in | 129.54 cm | 130 cm |

Derived flag placement: top edge approximately 238.76 cm above the floor, leaving approximately 35.56 cm to the measured ceiling. Use rounded values of 239 cm top height and 35 cm ceiling clearance for the first blockout; do not treat these derived values as independent measurements.

Use these as initial phone-measured estimates. Converted decimals express unit conversion, not measurement accuracy. At Unreal's default centimeter scale, build the couch proxy 211 units wide and the ceiling 274 units above the floor; retain 183 units as the usable seating span. Keep the proven pawn scale and world-to-meters setting unchanged. These measurements apply to the living-room interview set within the Crib, not to the entire existing Crib shell.

Next measurements to collect, in centimeters:

- Room width and depth.
- Couch depth, seat height, back height, and usable seat positions.
- Couch-to-wall offset and couch-to-camera distance.
- Window position and size; flag dimensions and approximate mounting height are recorded above.
- Rug footprint, operator starting position, and clear path to the entry.

For calibration, use floor level below the couch center as the set origin and record a forward direction toward the camera. Each headset and the iPhone has its own tracking origin; matching the room visually will not automatically align their poses. Validate one seated headset user against a measured virtual seat, then align the VCam at a known physical camera mark. Preserve Mimic's proven seated calibration and check that the virtual body matches the intended seated interview posture.

Four Quest headsets at HQ are available hardware for the planned setup. Current First Signal allocation remains three Quest users plus one PCVR user. Four simultaneous Quest participants plus a separate recording client would need an explicit capacity/slot change and validation.

Next concrete handoff: these references plus the measurements above -> a couch-area blockout for Noah -> one seated-avatar/camera alignment check -> furniture and lighting pass. Photo files remain in the original attachments and are not copied into the repository by this documentation pass.

The living room should be treated as a measured production reference, not as a new runtime architecture.

- Measure couch width, seat spacing, camera height, wall distances, and the primary interview sightline.
- Photograph the room under the intended interview lighting.
- Preserve a clear couch anchor and camera-safe background in the Crib.
- Let Noah's UE 5.6 set-dressing work own furniture, lighting, posters, and decorative placement.
- Import that content into UE 5.8 through the existing branch/LFS workflow, then integrate it into a WMCYN-owned environment layer.
- Keep the imported Crib FBX and existing level ownership boundaries intact.
- Add a small WMCYN calibration actor later, with named points such as `CAMCAL_PhysicalCouchOrigin`, `CAMCAL_VirtualCouchOrigin`, and `CAMCAL_OperatorStart`.

The goal is not millimeter-perfect world reconstruction. The goal is a believable, repeatable interview composition with stable camera framing.

## Two Possible Products

### Recommended first product: virtual camera

The iPhone moves the Unreal camera. The phone screen is a control surface; OBS records the rendered virtual scene. This uses Unreal's existing virtual-production path and keeps avatars, lighting, voice, and networking in the system we already have.

### Later product: phone AR/composite camera

The iPhone camera shows the physical room and composites or streams the virtual avatars into it. That is a different feature: ARKit world tracking, camera passthrough, rendering/compositing, latency handling, and likely a separate capture/export path. It should not be mixed into the first VCam spike.

## Recommended Order Relative to Crib Polish

Lighting-first update (2026-10-14): Jared identified general Crib darkness as a
blocker to scouting and placement. Complete the basic visibility pass scoped in
`Docs/CRIB_BASIC_LIGHTING_PLAN.md` before the couch blockout. This is functional
room lighting at existing ceiling fixtures; final interview lighting remains a
later composition pass.

### Unreal Location Scout (2026-10-14)

- MCP confirmed the open map is `/Game/Levels/L_WMCYNOnline`; PIE was stopped.
- Jared clarified the intended reference entrance is the driveway-side entrance, with the interview area to the right when entering.
- Candidate inspected: the empty lower-level lounge beside the pool terrace, with a broad rectangular opening, a large window on the far wall, and a side opening toward the exterior spiral stairs. This is a proposed location pending Jared's visual confirmation of the screenshot; no front-door actor label identifies the route unambiguously.
- Screenshot: `Saved/Diagnostics/InterviewSetScout/Crib_LowerLounge_Candidate_20261014_Unlit.png`. Captured in Unlit mode for readable geometry; it does not demonstrate finished lighting. Lit mode was restored after the scout.
- Reproducible screenshot viewpoint: world location `(0, -500, 240)` cm; rotation `(Pitch -5, Yaw 180, Roll 0)` degrees; viewport FOV `90` degrees. These are editor-camera coordinates, not a new spawn or a tested floor anchor.
- This room is substantially dark in Lit mode. A focused interview lighting pass will be required.
- Four exploratory collision traces around `(-1200, -500)` returned no hits, including downward/upward traces. Floor collision, ceiling clearance, and wall distances here remain unverified; the existing spawn-area collision proof does not establish this room as walkable.

We have enough reference for the first visual blockout: 211 cm overall couch width, 183 cm usable seating span, a 183 x 109 cm banner starting roughly 130 cm above the floor, and a 274 cm target living-room ceiling height. Couch depth, seat height, back height, and room footprint remain to be measured; provisional values must be labeled as estimates.

Suggested first layout: put the couch/banner against a solid wall segment, keep the large window and terrace passage clear, place the rug in front, and use the broad room opening as the first camera approach. Verify fit with simple proxies before choosing final placement. The living-room ceiling measurement belongs to the interview set and does not authorize resizing the existing Crib shell.

Next pass: confirm the pictured room -> couch/banner/rug blockout -> local floor and seated-pose check -> a wide interview camera frame. A complete scan and detailed object extraction can follow this initial fit check.

The scan/VCam idea should come **before full decoration**, but only as a thin calibration pass. Do not build the entire scan-reconstruction pipeline before making the room visually useful.

1. Capture enough of the living room to establish the couch, seat spacing, wall/background line, and operator camera height.
2. Place simple proxy furniture and camera marks in the Crib.
3. Run the disposable VCam smoke test against those proxies.
4. Let Noah dress the real furniture, posters, props, and lighting around the proven composition anchors.
5. Re-test the VCam shot with the dressed room.
6. Add video players and other presentation systems once the camera frame, screen locations, lighting, and collision are stable.

This order prevents a beautifully decorated room from later being rebuilt because the couch, player spacing, or camera sightline is wrong. It also keeps video-player work from becoming a distraction while the interview composition is still moving.

## Proposed Stages

### Stage 0: editor proof

- Do not touch `L_WMCYNOnline`.
- Create or use a disposable camera test map.
- Confirm the VCam actor appears and receives iPhone motion.
- Confirm the view can frame a static couch proxy.

### Stage 1: Crib camera proof

- Add one WMCYN-owned camera wrapper or camera actor in the existing production-camera folder.
- Point it at the actual Crib couch.
- Add one calibration transform and one saved camera bookmark.
- Confirm a seated two-person shot and a wider four-person shot.

### Stage 2: live interview proof

- Start the canonical world normally.
- Connect one iPhone as the camera controller.
- Have three Quest users and one PCVR user enter.
- Confirm all avatars remain visible while the iPhone camera moves.
- Record a 30-second OBS clip with clean framing and hybrid audio.

### Stage 3: repeatability

- Record camera motion or bookmarks with Take Recorder.
- Reopen the same shot and reproduce the framing.
- Add a camera capability gate so only the PCVR recording user or an explicitly authorized operator can drive it.

## Risks and Open Questions

- VCam may be editor-first in the first spike; packaged runtime support must be verified before promising a standalone production app.
- Phone-to-PC network discovery, firewall rules, Pixel Streaming, and Live Link ports must be tested on the actual HQ network.
- A moving iPhone camera needs a calibration origin; raw ARKit pose should not be applied directly to world coordinates without scale/origin handling.
- The physical room and virtual Crib will diverge unless the couch, camera height, and background anchors are measured deliberately.
- Four headset users plus a live VCam viewport may expose GPU or encoder limits on the PCVR machine.
- The camera operator should not become a fifth world participant unless we intentionally choose that behavior.

## Spike Recommendation

Proceed with the VCam route first. It lines up with the current Unreal/OBS architecture, reuses Epic's supported virtual-production tools, and keeps the native mobile app focused on accounts, pairing, and avatars. The next implementation ticket should be a disposable `WMCYN_VCam_Smoke` map and plugin validation, followed by one real couch shot. Do not add custom iPhone AR compositing until that camera shot is stable.
