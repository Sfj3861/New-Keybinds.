# New Keybinds

An Android-focused Geometry Dash mod that adds configurable keyboard controls.

## Current development status

**Prototype 0.2.0 — keyboard gameplay milestone**

Implemented in source:

- Player 1 jump input.
- Player 2 jump input.
- Place Practice Mode checkpoint.
- Delete Practice Mode checkpoint.
- Restart the current level.
- Key assignments are declared as Geode `keybind` settings, so Geode provides key capture, persistence, and support for multiple keys per action.
- Checkpoint and restart actions are forwarded to Geometry Dash's existing keyboard handlers.

## Current limitations

- The custom New Keybinds window and its Library button are not implemented yet.
- Editor shortcuts and navigation shortcuts are not implemented yet.
- Controller support and Smart Controls are not implemented yet.
- The current keybind controls are still exposed through Geode's mod settings.
- Android device compatibility and runtime behavior still need to be tested on a real device.
- This repository change has not been compiled against the target Android toolchain yet.

## Target

- Geometry Dash Android 2.2091
- Geode 5.9.0
- C++23

## Build

Requires the Geode SDK and Android NDK. Configure the `GEODE_SDK` environment variable, then build for the Android architecture supported by your device using the Geode CLI.

## License

MIT. See [LICENSE](LICENSE).
