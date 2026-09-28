# Tennis Scoreboard for OBS

Native OBS Studio plugin for controlling and displaying a live tennis/padel scoreboard. The primary release target is macOS, with one universal plugin for Apple Silicon and Intel.

The plugin adds a small OBS dock that starts a local server. Operators control the match from a browser config panel, and OBS displays the scoreboard through a Browser Source.

## Quick Links

- User install guide: `instructions.md`
- Full project documentation: `docs/GITHUB_PROJECT_DOCUMENTATION.md`
- Development process notes: `docs/PROCESS_TO_1_0.md`
- Architecture notes: `docs/ARCHITECTURE.md`

## Features

- OBS dock: `Docks > Tennis Scoreboard`.
- Config panel: `http://127.0.0.1:9876/config`.
- Browser Source overlay: `http://127.0.0.1:19876/score`.
- Singles and doubles support.
- Doubles names as separate players or `Player One/Player Two`.
- Traditional advantage scoring.
- Golden point scoring for padel/no-ad formats.
- Automatic match terms: `Match point`, `Set point`, `Break point`, `Game point`, `Deuce`, `Golden Point`, `Advantage / Ad`, `Tiebreak`, `Serving for the set`, `Serving for the match`.
- Winner screen, custom notices, suspended/interrupted/delayed/RET/WO states.
- Visual controls for colors, event logo, optional logo tint, and scoreboard opacity.

## User Installation

Users do not need Xcode if they download the compiled macOS release:

```text
tennis-scoreboard-1.0.1-macos-universal.zip
```

After unzipping it, copy this bundle into OBS's plugins folder:

```text
tennis-scoreboard.plugin
```

See `instructions.md` for the manual macOS install steps.

The same GitHub Release also provides separate Windows and Ubuntu packages. Native OBS plugins cannot use one binary across all operating systems: macOS loads a `.plugin`, Windows loads a `.dll`, and Linux loads a `.so`.

## Build From Source

Building from source on macOS requires Xcode and CMake:

```sh
cmake --preset macos
cmake --build --preset macos
```

Install the local development build into OBS:

```sh
./scripts/install-local-macos.sh
```

After installation, restart OBS Studio and open `Docks > Tennis Scoreboard`.

## Test

```sh
ctest --test-dir build_macos -C RelWithDebInfo --output-on-failure
```

Expected:

```text
tennis-rules-tests passed
```
