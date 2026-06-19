# Tennis Scoreboard OBS Plugin Architecture

## Overview

The plugin is a native OBS Studio module for macOS. It registers a small Qt dock, starts two local HTTP servers, and serves a browser-based control panel plus a browser-source scoreboard overlay.

```text
OBS Studio
  -> Native C++ plugin
  -> Qt dock: Docks > Tennis Scoreboard
  -> Local HTTP server
     -> /config on port 9876
     -> /score on port 19876
  -> MatchState + TennisRules domain layer
  -> Packaged HTML/CSS/JS overlay resources
```

## Runtime Flow

1. OBS loads `tennis-scoreboard.plugin`.
2. `src/plugin-main.cpp` registers the dock.
3. Opening the dock starts `OverlayHttpServer`.
4. The user operates the match from `http://127.0.0.1:9876/config`.
5. OBS displays `http://127.0.0.1:19876/score` as a Browser Source.
6. Config actions post JSON to `/api/action`.
7. `ScoreboardDock` updates `TennisRules` and `MatchState`.
8. `/state.json` returns a `MATCH_STATE_UPDATED` JSON snapshot.
9. `scoreboard.js` polls the state and renders the overlay.

## Current Modules

- `src/plugin-main.cpp`: OBS module entry point and dock registration.
- `src/ui/ScoreboardDock.*`: small OBS dock, server controls, remote action handling.
- `src/overlay/OverlayHttpServer.*`: local HTTP server, static asset serving, `/state.json`, `/api/action`.
- `src/core/MatchState.*`: state model, format, teams, theme, labels, JSON serialization, score intelligence.
- `src/core/TennisRules.*`: scoring engine for traditional advantage and golden point, games, sets, tiebreaks, undo, reset, winner handling.
- `resources/overlay/config.*`: browser-based operator UI.
- `resources/overlay/index.html`, `scoreboard.css`, `scoreboard.js`: OBS Browser Source overlay.
- `resources/default-presets/*`: default visual preset seed data.
- `tests/tennis-rules-tests.cpp`: pure C++ smoke tests for scoring behavior.

## Ports

- Config panel: `9876`.
- Scoreboard overlay: `19876`.

Both servers also serve shared assets and `/state.json` as needed.

## Build Notes

The macOS preset enables `obs-frontend-api` and Qt because `obs_frontend_add_dock_by_id` requires frontend integration and the dock is a QWidget.

```sh
cmake --preset macos
cmake --build --preset macos
```

The template's build scripts fetch matching OBS, Qt6, and prebuilt dependencies from the versions declared in `buildspec.json`.

## Distribution Notes

For open source distribution without Apple Developer notarization, publish a GitHub Release ZIP containing the compiled `tennis-scoreboard.plugin` bundle and direct users to `instructions.md`.

For frictionless public macOS distribution, create a signed and notarized `.pkg` using Apple Developer certificates.

## Future Improvements

1. Persist match setup and visual settings between OBS sessions.
2. Add packaged release automation for GitHub Releases.
3. Add universal macOS builds if Intel Mac support is required.
4. Add signed/notarized macOS installer support for professional distribution.
