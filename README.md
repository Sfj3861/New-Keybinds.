# New Keybinds

An Android-focused Geometry Dash mod that adds configurable keyboard controls.

## Current development status

**Prototype 0.3.0 — gameplay and editor keyboard milestone**

Implemented in source:

### Gameplay
- Player 1 jump input.
- Player 2 jump input.
- Place Practice Mode checkpoint.
- Delete Practice Mode checkpoint.
- Restart the current level.

### Editor
- Move selected objects up, left, down, and right.
- Rotate selected objects counter-clockwise and clockwise.
- Editor actions are forwarded through the native `EditorUI` keyboard handlers.
- Editor shortcuts check whether another keyboard delegate or text-input/IME field owns input before handling a shortcut.

Geode `keybind` settings provide key capture, saved assignments, and support for multiple keys per action.

## Current limitations

- The custom New Keybinds window and its dedicated Library button are not implemented yet.
- Navigation shortcuts are not implemented yet.
- Controller support and Smart Controls are not implemented yet.
- Keybind configuration is currently exposed through Geode's mod settings.
- Android device compatibility and runtime behavior still need to be tested on a real device.
- The repository changes have not yet been compiled against the target Android toolchain.

## Target

- Geometry Dash Android 2.2091
- Geode 5.9.0
- C++23

## Build

Requires the Geode SDK and Android NDK. Configure the `GEODE_SDK` environment variable, then build for the Android architecture supported by your device using the Geode CLI.

## License

MIT. See [LICENSE](LICENSE).
