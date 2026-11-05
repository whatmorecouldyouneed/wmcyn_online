# Tray Session

Port of the Unity/VRChat "Joint Roller" prefab by matic954 to a synced VR mechanic. The founder
moved it from the planned interview couch set to the living room coffee table on 2026-11-04.
Decisions and scope are recorded here; do not treat this as acceptance until the checklist below
is complete.

Source: `C:\Users\jvred\WMCYN v2.0\Assets\Joints By Matic 954\Joint Roller`.
Content: `/Game/WMCYN/Interactables/TraySession`. Scripts: `Tools/TraySession/`.

## Mechanic

Tap grip to take, tap again to drop, with either hand. Items stay where released, because the
source pickups are kinematic with gravity off.

The tray is a pickup too, which the source's is not. Founder testing found the low table meant
crouching to reach. It has a 35 cm reach and pulls to the hand, and everything on it when taken
rides along. Items on it can still be taken, and always win over the tray itself.

| Action | Result |
|---|---|
| Ember touches a lit lighter or a burning joint | Lights |
| Hold trigger while burning | Inhale; burn rate doubles until 0.8 |
| Release trigger | 1.5 s exhale from the holder's head |
| Ember touches ashtray | Puts out; progress kept, can relight |
| Ember touches papers | Re-roll; resets |

Idle burn takes 200 s. A joint inhaled throughout takes 120 s: 80 s boosted, then 40 s idle.
A lighter strike fails 2 times in 9.

## Architecture

- `FWMCYNJointBurnState`: plain port of the source logic, tested directly.
- `AWMCYNSessionItem`: replicated, no physics. Held items attach to the holder's tracked hand on
  every machine, so there is no per-frame item traffic.
- `AWMCYNJointItem`, `AWMCYNLighterItem`, `AWMCYNSessionFixture`: server owns state.
- `UWMCYNItemInteractionComponent`: local input only. RPCs travel through
  `UWMCYNFirstSignalPresenceComponent`, because items have no owning connection.
- Grip priority: camera, then doors, then items.
- `UWMCYNHighEffectComponent`: local, head-locked shell, hidden from scene captures.

## Rebuilding content

Editor closed, in this order: Blender `prepare_tray_source.py`, then `build_tray_materials.py`
through `UnrealEditor-Cmd -run=pythonscript`. Meshes and textures are imported separately with
`StaticMeshTools`/`TextureTools`; Niagara systems are authored in the editor.

`place_tray_session.py` only did the first placement and refuses to run once the session exists,
because the live placement is hand-set.

Assigning a Niagara system to an inactive component restarts it in the editor preview, so
exhale clouds can appear around the joints while editing. Simulate shows the true state.

Do not batch material edits through the live MCP toolset. A failed script is rolled back by an
undo, and retrying on that material crashed the editor on 2026-11-04.

## Deviations from the source

- The high effect's script and shader are missing from the Unity project. Only its 60 s duration,
  an opacity ramp value and a rainbow material survive, so it is a reconstruction. It fires when an
  inhale starts, not on every trigger press, and has a Settings off switch.
- `firePlane` has no resolvable mesh; a plain plane is used.
- Per-pixel fBm is replaced by a tiled texture for Quest. The source's side burn band sits above
  the mesh's UV range, so only the tip glows; that band was not ported.
- All four strike sounds play. The source's range skips the last clip of each set.
- Either held item can report a contact, so a held lighter can light a resting joint.
- Ember smoke is a thin stream of soft sprites, not the source's continuous trails. A Niagara
  ribbon did not read at this scale, and sprites are cheaper on Quest. Exhale matches the source.
- Unlit tips are red because that is the source material's `_BurnColor1`. Unset primitive data
  also makes all four tips glow in the editor viewport only; Simulate shows them unlit.
- `Smoke Fogged OUT` is excluded: 812 particles/s with a 40 s life, and unused by the session.
- The source is table-top scale for a VRChat session, and that scale is kept: 49 cm tray,
  29 cm ashtray, 16.6 cm joints.

## Rights

The pack's readme says not for resale. It also contains downloaded models with no licence files.
OCB branding, a shirt texture and a stock rainbow image were replaced with WMCYN art or the pack's
own metal texture; matic954's credit panel on the pack is kept.

The founder accepted the rights risk for the closed ALPHA group on 2026-11-04. That does not cover
a public release: clear it with matic954 first, and review age rating and marketing against
Meta's content and ad rules.

## Release

Released on protocol 3 on 2026-11-04; see `Release/MATCHED_PROTOCOL3_2026_11_04.md`. The backend
compares a client's protocol with the running server's, so no backend deploy was needed.

The GitHub repo is public, so pushing these assets published them. The founder chose that on
2026-11-04, for now. It's separate from a game release, which still needs the rights note above.

## Status - 2026-11-04

- [x] Native build; 11/11 WMCYN automation tests pass (synthetic NullRHI, including burn, grip,
      server authority and the camera regression).
- [x] Meshes, textures, materials, sounds and placement in `L_WMCYNOnline`.
- [x] Niagara systems, Settings button (compiled in the Blueprint), rendered editor review.
- [x] Linux server target compiles with `-NoLink`; not linked or packaged.
- [x] Flame orientation fix built; placed lighter confirms the new card scale.
- [ ] Lit flame seen upright in a running session.
- [ ] Settings button placement and styling in a running menu.
- [ ] Local multi-client session: skipped by founder on 2026-11-04, so replication is unproven
      outside the synthetic server-authority test.
- [ ] PCVR and Quest 2 headset checks, including left hand and effect comfort.
