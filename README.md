# New Keybinds

A dedicated configuration hub for Geometry Dash Android 2.2091 and Geode SDK 6.0.0.

## Implemented code paths

### Custom Keybinds
- Dedicated in-mod configuration pages for Gameplay, Editor and Navigation.
- Captures physical keyboard input and modifier combinations (Ctrl, Shift and Alt).
- Supports Add and Replace modes, clearing assignments, and up to six bindings per action.
- Detects duplicate chords within the same input context and prevents assigning a conflicting chord.
- Saves directly to the action settings consumed by the event dispatcher. The Geode keybind settings popup is not used by this UI.
- Gameplay: P1/P2 jump, practice checkpoint place/delete, restart, and target-level search.
- Editor: move selected objects and rotate left/right.
- Navigation: online level browser, main menu, pause menu and My Levels.

### Smart Controls
- Separate P1 and P2 touch buttons, with independent enable switches.
- Button size, opacity and position can be adjusted.
- Buttons send a balanced tap (press + release); no held state is left behind.
- Position and visibility are applied when the next level's UILayer is created.

### Instant Actions
- Gameplay and Practice actions use the configured target ID.
- A saved level is launched through Geometry Dash's LevelInfoLayer Play flow. If it is not locally downloaded, the mod requests the download and launches it when the level data is ready.
- Practice Mode is enabled immediately after Geometry Dash creates the PlayLayer.
- Online level browser, My Levels, pause menu and main menu remain available.
- Sequence recording stores timestamped actions routed through New Keybinds keybinds and Smart Controls.
- While recording during gameplay, a floating red SAVE STEPS button stores the captured sequence. The sequence panel can start a recording, review the saved steps or clear them.
- Saved sequences are records only; automatic replay of saved sequences is not implemented.

### Interface
- The Mods Library entry uses a round Geode-style icon button.
- Popups retain the standard top-right close (X) control.
- Smart Controls size and opacity use draggable sliders.
- The hub has English, Spanish, French and Russian language selectors. The translations cover the main hub, Smart Controls and Instant Actions; some legacy labels in the detailed keybind/profile pages may remain English.

### Global Profiles
- Dynamic named profiles: create, select, save, apply, duplicate, rename and delete.
- Each profile serializes every registered setting, including keybinds and Smart Controls settings.
- No fixed three-slot limit and no hardcoded list of scalar settings.

## Known limitations
- All changes still require on-device playtesting on Geometry Dash Android 2.2091.
- Direct level launch and Practice Mode launch need in-game testing on Android 2.2091, including online download callbacks.
- Recording currently captures New Keybinds keybind events, its Smart Controls taps and the Instant Actions shortcuts; it does not yet capture every native touch, mouse or keyboard event in Geometry Dash.
- Saved sequences are not automatically replayed.
- Smart Controls are tap buttons only; hold, multi-touch and gesture recognition are not implemented.

## Build
- Android 64-bit
- Geode SDK 6.0.0
- C++23

CI compilation proves that the project compiles, not that every runtime behavior has been tested on-device.

/Known Issues — Beta 1

- Editor crashes: Pressing certain custom keybinds in the editor may cause a crash.
- Practice Mode (Android): The Z and X keybinds for Practice Mode currently do not work on Android.
- Editor features: Hitboxes and Ignore Damage currently only activate in the editor.

These issues are known and may be addressed in future updates. This is a beta release, so bugs and unexpected behavior may occur./
