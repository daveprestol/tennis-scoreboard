# Installation Instructions

This guide explains how to install Tennis Scoreboard for OBS on another Mac without a paid Apple Developer account.

The plugin can be distributed through GitHub as a ZIP file, but because it is not signed and notarized with Apple Developer certificates, macOS may require a few manual security steps.

Users installing a compiled release ZIP do not need Xcode. Xcode is only required when building the plugin from source.

## Compatibility

Current 1.0 compatibility:

- macOS 12.0 or newer.
- Apple Silicon Macs: M1, M2, M3, M4, or newer.
- OBS Studio 31.x recommended.
- Built against OBS Studio dependency target `31.1.1`.
- Tested on macOS using the OBS native plugin format.
- Supports tennis scoring and padel/no-ad style golden point scoring.

Not currently supported/tested:

- Intel Macs, unless a separate `x86_64` or universal build is produced.
- Windows.
- Linux.
- Automatic notarized `.pkg` installation.

## What You Download From GitHub

For a user install, download this release ZIP from GitHub Releases:

```text
tennis-scoreboard.plugin.zip
```

The ZIP should contain:

```text
tennis-scoreboard.plugin
```

Do not download the source code ZIP if you only want to install the plugin. The source code ZIP does not contain a compiled `.plugin` bundle.

Older release asset name:

```text
tennis-scoreboard-macos-arm64-v1.0.0.zip
```

That ZIP also contains the plugin, but `tennis-scoreboard.plugin.zip` is the clearer installer download.

## Install Manually

1. Quit OBS Studio.

2. Download and unzip:

```text
tennis-scoreboard.plugin.zip
```

3. Open Finder.

4. In the Finder menu, click:

```text
Go > Go to Folder...
```

5. Paste this path:

```text
~/Library/Application Support/obs-studio/plugins/
```

6. If the `plugins` folder does not exist, create it.

7. Copy this folder/bundle into that folder:

```text
tennis-scoreboard.plugin
```

The final path should be:

```text
~/Library/Application Support/obs-studio/plugins/tennis-scoreboard.plugin
```

8. Open Terminal and run:

```sh
xattr -cr ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
codesign --force --deep --sign - ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
```

9. Open OBS Studio.

10. In OBS, open:

```text
Docks > Tennis Scoreboard
```

If the dock appears, the plugin loaded correctly.

## Use the Plugin

Opening the dock starts the local scoreboard server.

Use this URL to control the match from the same Mac:

```text
http://127.0.0.1:9876/config
```

Create a Browser Source in OBS and set its URL to:

```text
http://127.0.0.1:19876/score
```

If you want to control the match from another device on the same network, use the Mac's local IP address instead of `127.0.0.1`.

Example:

```text
http://192.168.1.20:9876/config
```

The OBS Browser Source can keep using:

```text
http://127.0.0.1:19876/score
```

## Game Scoring Modes

The config panel includes a `Game scoring` selector:

- `Traditional advantage`: at 40-40, a team must win advantage and then the game.
- `Golden point`: at 40-40, the overlay shows `Golden Point`; the next point wins the game.

Use `Golden point` for padel or no-ad scoring formats.

## If OBS Says the Plugin Failed to Load

Quit OBS and run:

```sh
xattr -cr ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
codesign --force --deep --sign - ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
```

Then reopen OBS.

You can also verify the local signature:

```sh
codesign --verify --deep --strict --verbose=2 ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
```

Expected result:

```text
valid on disk
satisfies its Designated Requirement
```

## If macOS Blocks the Plugin

Because this release is not Apple-notarized, macOS may block it or mark it as downloaded from the internet.

Run:

```sh
xattr -cr ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
```

Then sign it locally:

```sh
codesign --force --deep --sign - ~/Library/Application\ Support/obs-studio/plugins/tennis-scoreboard.plugin
```

This is not the same as official Apple Developer signing. It only signs the plugin locally on your Mac so OBS/macOS can load it more reliably.

## Build From Source Instead

If you want to build the plugin yourself, install:

- Xcode.
- Homebrew.
- CMake.
- Git.
- Ninja.

Install tools:

```sh
brew install cmake git ninja
```

Select Xcode:

```sh
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept
```

Build and install locally:

```sh
./scripts/install-local-macos.sh
```

Restart OBS after installation.

## Public Distribution Note

This manual install path does not require a paid Apple Developer account.

However, for a professional public installer, you need:

- Apple Developer Program membership.
- `Developer ID Application` certificate.
- `Developer ID Installer` certificate.
- A signed `.pkg`.
- Apple notarization.
- Stapled notarization ticket.

Without that, the GitHub ZIP method can still work, but users may need the manual Terminal steps above.
