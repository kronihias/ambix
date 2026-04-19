# ambix Visualizer

A real-time source visualizer that receives OSC messages from
ambix_encoder plugins running in your DAW and displays source positions on a
2D (Hammer-Aitoff) or 3D sphere projection.

Compatible with ambix_encoder (via automated network discovery), and IEM Multiencoder (manual port/ip setup)

## Requirements

- CMake ≥ 3.22
- JUCE 8 (included in the repo as `../JUCE` relative to this directory)
- **macOS/iOS**: Xcode with Command Line Tools
- **Linux**: GCC or Clang + the usual JUCE Linux dependencies (`libasound`,
  `libfreetype`, `libx11`, etc.)

---

## Building — macOS (desktop)

```bash
cmake -S . -B _build -DCMAKE_BUILD_TYPE=Release
cmake --build _build --config Release
```

The app bundle lands at `_build/ambix_visualizer_artefacts/Release/ambix Visualizer.app`.

---

## Building — iOS (device)

### Prerequisites

1. **Full Xcode** from the App Store (not just Command Line Tools — the iOS SDK
   lives inside Xcode.app).
2. **CMake ≥ 4.0** — earlier versions do not recognise Xcode 16+.
   ```bash
   brew install cmake   # or: brew upgrade cmake
   ```
3. On first use, let Xcode install its support packages:
   ```bash
   sudo xcodebuild -runFirstLaunch
   ```
   This step is required if you see xcodebuild complaining about a missing
   plug-in (`IDESimulatorFoundation`, `DVTDownloads`, etc.).
4. An **Apple Developer account** (free tier is enough for on-device testing).
   Find your 10-character Team ID at
   [developer.apple.com](https://developer.apple.com) → Account →
   Membership Details → *Team ID* field.

### Configure

```bash
cmake -S . -B _build-ios -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=XXXXXXXXXX \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic
```

Replace `XXXXXXXXXX` with your Team ID.

### Build and run

**Option A — from the command line:**
```bash
cmake --build _build-ios --config Debug
```

**Option B — from Xcode (recommended for on-device deployment):**
```bash
open _build-ios/ambix_visualizer.xcodeproj
```
Select your iPad as the run destination in the toolbar and press Run (▶).

### Device setup (iOS 16+)

iPads running iOS 16 or later require **Developer Mode** to be enabled before
Xcode can install and run apps:

1. Settings → Privacy & Security → Developer Mode → turn on.
2. Reboot when prompted.
3. Connect the iPad to your Mac via USB. Tap "Trust" on the iPad when asked.

### Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `No CMAKE_C_COMPILER could be found` | xcodebuild plug-in failed to load | `sudo xcodebuild -runFirstLaunch` |
| Run button greyed out in Xcode | No signing team set | Re-run cmake with `-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=...` |
| Run button greyed out in Xcode | Developer Mode off | Settings → Privacy & Security → Developer Mode |
| App installs but crashes immediately | iPad name contains non-ASCII chars | Rename the iPad to ASCII-only in Settings → General → About → Name |

### iOS Simulator

```bash
cmake -S . -B _build-iossim -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphonesimulator \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
    -DCMAKE_OSX_ARCHITECTURES=arm64

open _build-iossim/ambix_visualizer.xcodeproj
```

No Team ID is needed for the simulator.

---

## Building as part of the ambix suite

When this directory is added as a subdirectory by the repo-root CMakeLists.txt,
JUCE is already present as a target and the `project()` call here is skipped.
The resulting app bundle is copied to `${BIN_DIR}/standalone/` alongside the
other ambix plugin standalones.

---

## Network setup

The visualizer listens on a UDP port (default **9000**) for OSC messages sent
by ambix_encoder. Configure your encoder plugin to send to the IP address shown
in the app's top bar and the port shown in Settings.

The local network permission prompt appears on the first launch on iOS — tap
**Allow** to let the app receive OSC packets on your local Wi-Fi network.
