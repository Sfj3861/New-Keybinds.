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
- Functional destinations: target-level search, online level browser, My Levels, pause menu and main menu.
- Target Gameplay/Practice actions currently open search results for the configured level ID. They do not automatically start a level.
- Sequence Draft stores UI action notes only and does not replay inputs.

### Global Profiles
- Dynamic named profiles: create, select, save, apply, duplicate, rename and delete.
- Each profile serializes every registered setting, including keybinds and Smart Controls settings.
- No fixed three-slot limit and no hardcoded list of scalar settings.

## Known limitations
- All changes still require on-device playtesting on Geometry Dash Android 2.2091.
- Directly launching the configured online level without selecting a search result is not implemented.
- Smart Controls are tap buttons only; hold, multi-touch and gesture recognition are not implemented.
- English is the current UI language; Spanish, French and Russian translations are not included yet.

## Build
- Android 64-bit
- Geode SDK 6.0.0
- C++23

CI compilation proves that the project compiles, not that every runtime behavior has been tested on-device.
