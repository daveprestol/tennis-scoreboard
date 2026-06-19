# Tennis Scoreboard for OBS

Native macOS OBS Studio plugin for controlling and displaying a live tennis or padel scoreboard.

The plugin adds a small OBS dock that starts a local scoreboard server. The operator controls the match from a browser-based config panel, while OBS displays the scoreboard through a Browser Source.

## Features

- Native OBS plugin for macOS.
- Small OBS dock under `Docks > Tennis Scoreboard`.
- Local config panel:
  - `http://127.0.0.1:9876/config`
- Browser Source overlay:
  - `http://127.0.0.1:19876/score`
- Singles and doubles support.
- Doubles names can be entered as separate players or as `Player One/Player Two`.
- Automatic tennis scoring:
  - 0, 15, 30, 40, Deuce, Advantage, Game.
  - Games, sets, match winner.
  - Tiebreak display.
- Configurable match format:
  - Best of sets.
  - Games per set.
  - Win by games.
  - Tiebreak at.
  - Traditional advantage or golden point scoring.
- Automatic broadcast terms:
  - Match point.
  - Set point.
  - Break point.
  - Game point.
  - Deuce.
  - Golden Point.
  - Advantage / Ad.
  - Tiebreak.
  - Serving for the set.
  - Serving for the match.
- Match status notices:
  - Suspended.
  - Interrupted.
  - Delayed.
  - Retired / RET.
  - Walkover / WO.
  - Custom message.
- Visual customization:
  - Event name.
  - Event logo URL.
  - Optional logo tint.
  - Board, row, accent, text, event title, and team colors.
  - Global scoreboard opacity.
- Undo, reset game, reset set, reset match.
- Winner screen when the match is finished.

## How It Works

After OBS loads the plugin, open:

```text
Docks > Tennis Scoreboard
```

Opening the dock starts an internal HTTP server.

Use the config URL to operate the match:

```text
http://127.0.0.1:9876/config
```

Create an OBS Browser Source and set its URL to:

```text
http://127.0.0.1:19876/score
```

The config panel can also be used from another device on the same network. Use the Mac's LAN IP instead of `127.0.0.1`, for example:

```text
http://192.168.1.20:9876/config
```

Keep the OBS Browser Source on the Mac using either `127.0.0.1` or the LAN IP:

```text
http://127.0.0.1:19876/score
```

## Project Structure

```text
.
├── CMakeLists.txt
├── CMakePresets.json
├── buildspec.json
├── scripts/
│   └── install-local-macos.sh
├── docs/
│   ├── ARCHITECTURE.md
│   ├── PROCESS_TO_1_0.md
│   └── GITHUB_PROJECT_DOCUMENTATION.md
├── src/
│   ├── plugin-main.cpp
│   ├── core/
│   │   ├── MatchState.cpp
│   │   ├── MatchState.h
│   │   ├── TennisRules.cpp
│   │   └── TennisRules.h
│   ├── overlay/
│   │   ├── OverlayHttpServer.cpp
│   │   └── OverlayHttpServer.h
│   └── ui/
│       ├── ScoreboardDock.cpp
│       └── ScoreboardDock.h
├── resources/
│   ├── default-presets/
│   │   └── default-dark.json
│   └── overlay/
│       ├── config.html
│       ├── config.css
│       ├── config.js
│       ├── index.html
│       ├── scoreboard.css
│       └── scoreboard.js
└── tests/
    └── tennis-rules-tests.cpp
```

## File Guide

| File | Purpose |
| --- | --- |
| `src/plugin-main.cpp` | OBS module entry point. Registers the Tennis Scoreboard dock. |
| `src/ui/ScoreboardDock.*` | Qt dock shown inside OBS. Starts/stops the local server and handles remote actions. |
| `src/overlay/OverlayHttpServer.*` | Small HTTP server for `/config`, `/score`, static assets, state JSON, and API actions. |
| `src/core/MatchState.*` | Central match state, display labels, JSON serialization, score intelligence terms. |
| `src/core/TennisRules.*` | Scoring engine: points, games, sets, tiebreak, advantage, golden point, undo, reset, winner handling. |
| `resources/overlay/config.html` | Operator control panel UI. |
| `resources/overlay/config.css` | Styling for the config panel. |
| `resources/overlay/config.js` | Config panel behavior, autosave, actions, state polling. |
| `resources/overlay/index.html` | Scoreboard overlay HTML used by the OBS Browser Source. |
| `resources/overlay/scoreboard.css` | Scoreboard visual styling. |
| `resources/overlay/scoreboard.js` | Scoreboard rendering logic from `/state.json`. |
| `resources/default-presets/default-dark.json` | Default visual preset values. |
| `tests/tennis-rules-tests.cpp` | Pure C++ smoke tests for scoring and score terms. |
| `scripts/install-local-macos.sh` | Builds, copies, clears quarantine attributes, and locally signs the plugin for OBS. |
| `buildspec.json` | OBS Plugin Template dependency versions. |
| `CMakePresets.json` | Build presets for macOS, Windows, and Linux. The tested 1.0 target is macOS arm64. |

## Languages and Tools

### Languages

- C++17 for the OBS plugin, Qt dock, HTTP server, match state, and tennis rules.
- HTML for config and overlay documents.
- CSS for config and scoreboard styling.
- JavaScript for config actions and live overlay rendering.
- Bash for local macOS install automation.
- CMake for build configuration.

### Main Frameworks and APIs

- OBS Studio Plugin API.
- `obs-frontend-api` for OBS dock integration.
- Qt6 Widgets/Core/Network.
- CMake with Xcode generator on macOS.

## Requirements

The current tested target is macOS Apple Silicon.

Required:

- macOS 12.0 or newer.
- OBS Studio.

For installing a compiled release ZIP, users do not need Xcode.

For building from source, developers need:

- Xcode.
- Homebrew.
- CMake.

Recommended Homebrew packages:

```sh
brew install cmake git ninja
```

The local machine used for the 1.0 build had:

```text
Xcode 26.5
Homebrew 6.0.2
CMake 4.3.3
Git 2.53.0
Ninja 1.13.2
OBS dependency target: 31.1.1
```

## Build on macOS

Install Xcode from the App Store, then select it:

```sh
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept
```

Install build tools:

```sh
brew install cmake git ninja
```

Configure and build:

```sh
cmake --preset macos
cmake --build --preset macos
```

The build system downloads the OBS, Qt6, and OBS dependency packages declared in `buildspec.json`.

## Install Locally in OBS on macOS

Run:

```sh
./scripts/install-local-macos.sh
```

The script installs the plugin to:

```text
~/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin
```

It also:

- Builds the plugin.
- Copies the plugin bundle into the OBS plugin folder.
- Clears extended attributes with `xattr -cr`.
- Locally signs the plugin with `codesign --force --deep --sign -`.

Restart OBS Studio after installation.

## Verify the Plugin Signature

```sh
codesign --verify --deep --strict --verbose=2 "$HOME/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin"
```

Expected result:

```text
valid on disk
satisfies its Designated Requirement
```

## Use in OBS

1. Restart OBS Studio.
2. Open `Docks > Tennis Scoreboard`.
3. Confirm the dock says the server is running.
4. Open the config panel:

```text
http://127.0.0.1:9876/config
```

5. In OBS, create a new Browser Source.
6. Set the Browser Source URL to:

```text
http://127.0.0.1:19876/score
```

7. Set the Browser Source size as needed for your scene.
8. Control the match from `/config`.

If you update the plugin while OBS is open, restart OBS and refresh the Browser Source.

## Config Panel

The config panel is intended for live operation.

Setup fields:

- Event name.
- Event logo URL.
- Singles or doubles.
- Match format.
- Game scoring mode: traditional advantage or golden point.
- Player/team names.
- Colors.
- Logo tint.
- Scoreboard opacity.

Score controls:

- Add points.
- Change server.
- Start/finish match.
- Undo.
- Reset game/set/match.
- Trigger match notices.

## Score Intelligence Rules

The overlay shows one main score term at a time. Priority is:

1. Match point.
2. Set point.
3. Deuce.
4. Golden Point.
5. Break point.
6. Game point.
7. Advantage / Ad.
8. Tiebreak.
9. Serving for the match.
10. Serving for the set.

`Break point` replaces `Game point` when the receiver can win the game on the next point.

When golden point mode is enabled, 40-40 displays `Golden Point` instead of `Deuce`, and the next point wins the game.

## Testing

Run the core scoring tests:

```sh
clang++ -std=c++17 -Isrc tests/tennis-rules-tests.cpp src/core/TennisRules.cpp src/core/MatchState.cpp -o /tmp/tennis-rules-tests
/tmp/tennis-rules-tests
```

Expected:

```text
tennis-rules-tests passed
```

## Troubleshooting

### CMake cannot find C or C++ compilers

Make sure full Xcode is installed and selected:

```sh
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept
```

Then rebuild:

```sh
./scripts/install-local-macos.sh
```

### OBS says the plugin failed to load

Try reinstalling:

```sh
./scripts/install-local-macos.sh
```

Then restart OBS.

Verify the installed bundle:

```sh
codesign --verify --deep --strict --verbose=2 "$HOME/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin"
```

For release ZIP users, clear quarantine and sign locally:

```sh
xattr -cr ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
codesign --force --deep --sign - ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
```

### `/score` returns not found

Use the score port:

```text
http://127.0.0.1:19876/score
```

The config panel uses a different port:

```text
http://127.0.0.1:9876/config
```

### Other devices cannot open the config panel

Use the Mac's local network IP instead of `127.0.0.1`.

Example:

```text
http://192.168.1.20:9876/config
```

Also check macOS firewall/network permissions.

### Browser Source does not update after reinstall

Restart OBS and refresh the Browser Source cache.

## Current Limitations

- The 1.0 build was tested on macOS Apple Silicon.
- The plugin is locally signed for development/testing, not notarized for public macOS distribution.
- Match setup and visual settings are not persisted as a formal saved profile yet.
- Logo tint depends on CSS mask support and the image source.
- Windows/Linux presets exist from the OBS Plugin Template, but this 1.0 process focused on macOS.

## License

See `LICENSE`.
