#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OBS_PLUGIN_DIR="$HOME/Library/Application Support/obs-studio/plugins"
PLUGIN_NAME="tennis-scoreboard.plugin"

export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake is missing. Install it with: brew install cmake"
  exit 1
fi

if ! /usr/bin/xcodebuild -version >/dev/null 2>&1; then
  cat <<'MSG'
Xcode is required to build OBS plugins on macOS with the official OBS plugin template.

Install Xcode from the App Store, then run:
  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
  sudo xcodebuild -license accept

After that, run this script again.
MSG
  exit 1
fi

cd "$ROOT_DIR"

cmake --preset macos
cmake --build --preset macos

mkdir -p "$OBS_PLUGIN_DIR"

SOURCE_PLUGIN="$ROOT_DIR/build_macos/rundir/RelWithDebInfo/$PLUGIN_NAME"
if [[ ! -d "$SOURCE_PLUGIN" ]]; then
  SOURCE_PLUGIN="$ROOT_DIR/build_macos/RelWithDebInfo/$PLUGIN_NAME"
fi

if [[ ! -d "$SOURCE_PLUGIN" ]]; then
  echo "Build finished, but $PLUGIN_NAME was not found in the expected build output."
  exit 1
fi

rm -rf "$OBS_PLUGIN_DIR/$PLUGIN_NAME"
cp -R "$SOURCE_PLUGIN" "$OBS_PLUGIN_DIR/"
xattr -cr "$OBS_PLUGIN_DIR/$PLUGIN_NAME"
codesign --force --deep --sign - "$OBS_PLUGIN_DIR/$PLUGIN_NAME"

echo "Installed $PLUGIN_NAME to:"
echo "  $OBS_PLUGIN_DIR/$PLUGIN_NAME"
echo
echo "Restart OBS Studio, then open Docks > Tennis Scoreboard."
