# AFCore Documentation Index

Source PDF: `C:/Users/jvred/Downloads/Advanced_Framework_Documentation.pdf`

Document title: Advanced Framework - Core 4.1 Documentation  
Author/source shown in PDF: HumanCodeable  
Date shown in PDF: May 5, 2021  
PDF created locally: June 22, 2026  
Pages: 78

## Extraction Note

The PDF itself has no extractable text layer. It renders correctly as images, but automated PDF text extraction returns zero characters.

An OCR/pasted-text copy was later provided in Codex attachment storage and used to create `Docs/AFCORE_WMCYN_IMPLEMENTATION_NOTES.md`. Read that WMCYN-focused note first, then use this index to find source sections in the PDF when manual verification is needed.

For future Codex work, use this index to find the relevant PDF sections, then verify details directly in the PDF or Unreal Editor before changing assets.

## High-Value Sections for WMCYN First Signal

### Framework Setup

- Introduction: page 5
- File Structure - Core Framework: page 6
- Game Classes: page 7
- Level Setup: page 8
- Navigation: page 8
- Changing Levels: pages 9-10
- Streaming Level Manager: page 10

Use these before changing WMCYN map startup, level loading, or GameMode/GameInstance assumptions.

### Interaction Components

- Interaction Components: page 11
- Select Components: page 12
- GazeView Components: page 13
- Grab Components: page 15
- Latch Components: page 17
- Overlap Components: page 18
- Teleport Component: page 19

Use these before adding custom grab, select, gaze, overlap, or teleport behavior. First Signal should reuse AFCore interaction paths where possible.

### State, Snapping, and Replication Components

- State Components: page 20
- Name Component: page 23
- Trigger Component: page 25
- Values Component: page 25
- Snapping Components: page 26
- Multiplayer Components: page 30
- Base Replication Component: page 30
- Actor Replication Component: page 30
- Component Replication Component: page 31

Use these before implementing WMCYN display names, presence/capability labels, replicated state, Verbatim marker triggers, or shared world runtime objects.

### UI and Pawn Components

- UI Components: page 31
- Menu Pallet Component: page 31
- Panel-based Menu Components: page 31
- Widget Component: page 32
- Window Component: page 33
- Mini Tag Component: page 33
- Pawn Components: page 34
- Controls Component: page 34
- Radial Menu Component: page 36
- Touch Component: page 36
- HUD Component: page 37
- Player Info Component: page 37

Use these before building First Signal display names, presence labels, recording UI, or in-world UI.

### Spectator, Camera, and Production Flow

- Spectator Component: page 39
- Video Component: page 40
- Pawn Hierarchy: page 45
- Startup: page 46
- Virtual Reality: page 46
- VR Pawn: page 46
- Motion Controllers: page 47
- Motion Components: page 49
- Presets: page 53
- Asymmetric Game Play: page 53
- Desktop: page 54
- Mobile Devices: page 55

Use these before choosing the AFCore baseline for standalone VR users and the PCVR recording user.

### Multiplayer and Data

- Multiplayer: page 56
- Client Server Models and Ownership: page 56
- Replication: page 56
- State Components and Interaction Components: page 57
- Non-predetermined Movement: page 57
- Spawning and Deleting of Actors: page 57
- User Interfaces: page 57
- Primary Data Assets: page 59
- Level Data Asset: page 60
- Panel Data Asset: page 61
- Gesture Data Assets: page 62
- Data Tables: page 63
- Structs: page 63
- Enums: page 65

Use these before changing persistent-world entry, spawn behavior, player ownership, presence/capability persistence, or data-driven setup.

### User Interfaces

- Widgets: page 67
- Widget Buttons: page 68
- Pallet Widgets: page 68
- HUD Widgets: page 69
- Widget Based UI: page 70
- Info Window: page 71
- Panel Based Menu: page 71
- Pallets: page 72
- Tiny Display: page 72
- Mini Tags: page 72
- HUD: page 73
- Widget-less UI: page 73
- Selection Menu: page 73
- Radial Menu: page 74

Use these before adding presence UI, display name UI, Verbatim feedback UI, or recording UI.

## WMCYN Use Guidance

- Keep AFCore as the baseline and wrap WMCYN behavior around it.
- Do not edit AFCore assets directly until the relevant PDF section and the asset behavior have both been checked.
- For standalone VR users, inspect the VR pawn, motion controller, motion component, preset, hand, and multiplayer sections first.
- For the PCVR recording user, start with the same VR pawn family, then inspect spectator, video, HUD, and UI sections for capture/recording support.
- For Verbatim marker work, inspect trigger/state components, replication components, player info, and multiplayer ownership first.

## Next Useful Manual Check

Open the PDF beside Unreal and inspect pages 45-57 while reviewing these AFCore assets in the Content Browser:

- `BP_Pawn_VR`
- `BP_Pawn_VR_Char`
- `BP_Pawn_VR_Camera`
- `Comp_Spectator`
- `BP_Manager_Spectator`
- `Comp_Replication`
- `Comp_RPC`
- `BP_PlayerState_Main`

The immediate goal is to pick presence baselines and recording capability support, not to build new framework systems.
