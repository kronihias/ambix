# ambiX — Ambisonic Plug-in Suite

Cross-platform Ambisonic VST/VST3/LV2 plug-ins and standalone applications for macOS, Windows, and Linux.

- VST3 plug-ins automatically adjust to the track channel count and switch Ambisonic order (VST2 are pre-built for a fixed order/channel count).
- Uses the **ambiX convention**: ACN channel order, SN3D normalization, full periphony (3D), excluding the √(1/4π) factor
- Spherical harmonics are computed recursively; the maximum order is defined at compile time — in practice limited by the host's channel count (N+1)² or CPU
- Built on the [JUCE](https://www.juce.com) framework (GPLv3)
- Designed to complement the [mcfx multichannel plug-in suite](http://www.matthiaskronlachner.com/?p=1910)

**Convention reference:** [Nachbar et al., Ambisonics Symposium 2011](http://iem.kug.ac.at/fileadmin/media/iem/projects/2011/ambisonics11_nachbar_zotter_sontacchi_deleflie.pdf)

---

## Table of Contents

- [ambiX — Ambisonic Plug-in Suite](#ambix--ambisonic-plug-in-suite)
	- [Table of Contents](#table-of-contents)
	- [Plug-ins](#plug-ins)
		- [ambix\_binaural](#ambix_binaural)
		- [ambix\_decoder](#ambix_decoder)
		- [ambix\_converter](#ambix_converter)
		- [ambix\_directional\_loudness](#ambix_directional_loudness)
		- [ambix\_encoder](#ambix_encoder)
		- [ambix\_move](#ambix_move)
		- [ambix\_maxre](#ambix_maxre)
		- [ambix\_mirror](#ambix_mirror)
		- [ambix\_rotator](#ambix_rotator)
		- [ambix\_rotator\_z](#ambix_rotator_z)
		- [ambix\_vmic](#ambix_vmic)
		- [ambix\_warp](#ambix_warp)
		- [ambix\_widening](#ambix_widening)
	- [Standalone Applications](#standalone-applications)
		- [ambix\_visualizer](#ambix_visualizer)
	- [Prerequisites for Building](#prerequisites-for-building)
	- [How to Build](#how-to-build)
	- [LV2 Plug-ins](#lv2-plug-ins)
	- [Known Problems](#known-problems)
	- [Changelog](#changelog)

---

## Plug-ins

Further reading: [LAC 2013 paper](http://lac.linuxaudio.org/2013/papers/51.pdf), [ICSA 2014 paper](http://iaem.at/Members/zotter/publications/2014_KronlachnerZotter_AmbiTransformationEnhancement_ICSA.pdf)

### ambix_binaural

Binaural decoder with impulse responses for various real-world loudspeaker setups and venues.

**Preset search paths:**

| Platform | Path |
|----------|------|
| Windows | `C:\Users\<username>\AppData\Roaming\ambix\binaural_presets\` |
| macOS | `~/Library/ambix/binaural_presets/` |
| Linux | `~/ambix/binaural_presets/` |

---

### ambix_decoder

Same as `ambix_binaural` but without convolution — loudspeaker signals are sent directly to the outputs (single-band decoding).

Uses the same preset format and search paths as `ambix_binaural`.

The IEM AllRADecoder is easier [https://plugins.iem.at/docs/allradecoder/](https://plugins.iem.at/docs/allradecoder/) to work with nowadays.

---

### ambix_converter

Converts between different Ambisonic standards on the fly (including FuMa ↔ ambiX and 2D ↔ 3D), allowing multiple standards within one project.

---

### ambix_directional_loudness

Amplifies, attenuates, or filters specific regions of the spherical sound field. Press `S` to toggle solo on the last selected filter.

---

### ambix_encoder

Panning plug-in with a configurable number of input channels. A width parameter spreads channels equally along the azimuth.

- OSC remote control with configurable custom ID; the track name is included in outgoing OSC messages for easier identification
- Opt+drag to adjust the width of multiple sources simultaneously
- Automatically advertises itself on the local network (Zeroconf) so the **ambix_visualizer** can discover it without manual IP/port configuration

**Outgoing (sent to configured target):**

| Message | Arguments |
|---------|-----------|
| `/ambi_enc` | `id(int) track_name(str) distance(float) azimuth(float) elevation(float) size(float) peak(float) rms(float) [reply_port(int)]` |

Azimuth and elevation in degrees (−180…+180); distance is currently unused (always 2.0); size 0…1; peak and rms linear 0…1 (0 dBFS = 1.0); `reply_port` is appended only when the receiver is bound.

**Incoming (received on configured port):**

| Message | Arguments | Range |
|---------|-----------|-------|
| `/ambi_enc_set` | `id(int) distance(float) azimuth(float) elevation(float) size(float)` | az/el deg −180…+180, size 0…1 |
| `/ambi_enc_subscribe` | `uuid(str) reply_port(int) [name(str)] [visualizer_ip(str)]` | Zeroconf subscription from visualizer |
| `/ambi_enc_unsubscribe` | `uuid(str)` | Remove a Zeroconf subscription |

---

### ambix_move

6DoF-like soundfield translation and rotation.

- Translates the listener viewpoint by ±5 m in x, y, z on a virtual sampling sphere (configurable reference radius 1–5 m)
- Rotation follows the same Yaw / Pitch / Roll + quaternion convention as `ambix_rotator`
- The full 6DoF transformation is computed as a single matrix multiply at runtime (decode → remap directions + apply 1/r gain → re-encode)
- Independent enable toggles for translation and rotation
- OSC control on a configurable UDP port (default 7130), useful for binaural playback with 6DoF head-tracking:

| Message | Arguments | Range |
|---------|-----------|-------|
| `/rotation` | `pitch yaw roll` | deg, −180…+180 each |
| `/quaternion` | `qw qx qy qz` | −1…+1 each |
| `/xyz` | `x y z` | metres, −5…+5 each |
| `/translation` | `x y z` | alias of `/xyz` |
| `/6dof` | `x y z qw qx qy qz` | metres + quaternion |

---

### ambix_maxre

Applies (or reciprocally applies) spherical max-rE weighting to suppress sidelobes, as described in Zotter & Frank — *"All-Round Ambisonic Panning and Decoding"*.

---

### ambix_mirror

Inverts or mirrors the sound field about the x, y, or z axis.

---

### ambix_rotator

Rotation around the x, y, and z axes (including quaternion input). OSC control on a configurable UDP port:

| Message | Arguments | Range |
|---------|-----------|-------|
| `/rotation` | `pitch yaw roll` | deg, −180…+180 each |
| `/quaternion` | `qw qx qy qz` | −1…+1 each |
| `/head_pose` | `user_id x y z pitch yaw roll` | id (int), position (ignored), deg |

---

### ambix_rotator_z

Rotation around the z axis only.

---

### ambix_vmic

Virtual microphone: outputs the selected region of the sound field as an audio signal rather than an Ambisonic stream.

---

### ambix_warp

Warps the sound field towards the equator, poles, front, or back. The GUI supports mouse dragging and undo/redo.

---

### ambix_widening

Frequency-dependent rotation around the z axis for source widening or diffuse early reflections. The GUI supports mouse dragging gestures.

Article: [http://dx.doi.org/10.14279/depositonce-12](http://dx.doi.org/10.14279/depositonce-12) — created with Franz Zotter and Matthias Frank.

---

## Standalone Applications

### ambix_visualizer

Real-time 3D source position visualizer — runs as a macOS desktop app or an **iOS/iPadOS** app (iPad recommended).

- Displays source positions from one or more `ambix_encoder` instances on a 2D Hammer-Aitoff or interactive 3D sphere projection
- Automatically discovers `ambix_encoder` instances on the local network via Zeroconf (no manual IP/port setup needed); also accepts manual OSC subscriptions for use with other encoders (e.g. IEM MultiEncoder)
- Listens on a configurable UDP port (default 9000)

See [`ambix_visualizer/README.md`](ambix_visualizer/README.md) for build instructions, iOS deployment, and network setup details.

---

## Prerequisites for Building

CMake, a working build environment, FFTW3, and Eigen 3 are required. Standalone applications additionally require the ASIO SDK for Windows ASIO support.

**Linux** — install dependencies (Debian/Ubuntu):

```bash
sudo apt install libasound2-dev libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev \
    libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev mesa-common-dev \
    libfftw3-dev \
    libzita-convolver3 \
    libzita-convolver-dev \
    libeigen3-dev
```

**macOS** — install dependencies via MacPorts:

```bash
sudo port install eigen3-devel
```

**Windows** — download FFTW3 and Eigen manually (see their respective project pages).

Clone the repository including submodules:

```bash
git clone --recurse-submodules https://github.com/kronihias/ambix/
```

---

## How to Build

Use **cmake-gui** or **cmake/ccmake** from the terminal.

1. Initialise submodules (if not cloned with `--recurse-submodules`):
    ```bash
    git submodule init
    git submodule update
    ```

2. Create a build folder and configure:
    ```bash
    mkdir build
    cd build
    ccmake ..
    ```

3. Adjust parameters to your needs, then build:
    ```bash
    make -j$(nproc)
    ```

4. Find the binaries in `ambix/BUILD/_bin/` and copy them to your system plug-in folder:

    | Format | macOS | Windows | Linux |
    |--------|-------|---------|-------|
    | VST2 | `/Library/Audio/Plug-Ins/VST` | `C:\Program Files\Steinberg\VstPlugins` | `/usr/lib/lxvst` or `/usr/local/lib/lxvst` |

---

## LV2 Plug-ins

1. Enable the `BUILD_LV2` CMake flag before building.
2. In the `lv2-ttl-generator/` folder, run `make` to compile the `lv2_ttl_generator` tool.
3. From the `_bin` folder (e.g. `ambix/BUILD/_bin`), run:
    ```bash
    ./../../lv2-ttl-generator/generate-ttl.sh
    ```
    This generates all required `.ttl` files. Copy the `.lv2` folders from `ambix/BUILD/_bin/lv2` to `/usr/lib/lv2`.

---

## Known Problems

- Documentation is sparse — refer to the papers linked above for background.
- GUI-less plug-ins cannot be used as standalones.
- Linux: LV2 plug-ins don't show the GUI; VST plug-in GUIs may crash — use the host's built-in GUI or the standalone version via Jack.

---

## Changelog

### v0.4.0 (2026-04-19) <!-- omit in toc -->

- VST3 support: all plug-ins now ship as VST3 with a single binary that automatically adjusts to the track's Ambisonic order — VST3 is recommended going forward, VST2 is considered legacy for backward compatibility
- **New:** `ambix_move` — 6DoF-like soundfield transformation: translate the listener position (x/y/z) and rotate using the same Yaw/Pitch/Roll + quaternion convention as `ambix_rotator`; independent enable toggles for translation and rotation; OSC control on a configurable port, useful for binaural playback with 6DoF head-tracking (small head movements)
- **New:** `ambix_visualizer` — standalone macOS/Win and iOS/iPadOS/Android app that displays source positions from `ambix_encoder` instances on a 2D or 3D sphere in real time, also compatible with IEM MultiEncoder
- `ambix_warp`: new interactive GUI with mouse dragging and undo/redo
- `ambix_widening`: new interactive GUI with mouse dragging gestures
- `ambix_encoder`: Zeroconf network advertisement for automatic discovery by `ambix_visualizer`; track name included in OSC messages; opt+drag to adjust multiple source widths simultaneously
- `ambix_binaural`: performance improvements from mcfx_convolver
- `ambix_directional_loudness`: press `S` to toggle solo on the last selected filter
- `ambix_rotator`: fix initialisation issue
- Update to JUCE 8

### v0.3.0 (2022-04-16) <!-- omit in toc -->

New builds optimised for Apple Silicon and 64-bit Intel Mac; Windows 64-bit. Update to JUCE 7; removed soxr dependency.

### v0.2.10 (2020-02-06) <!-- omit in toc -->

`ambix_binaural`: fix dropouts/artifacts for hosts sending incomplete block sizes (e.g. Adobe, Steinberg); option to store preset within the project (enables DAW project exchange without external files); allow exporting stored preset as a `.zip` file.

### v0.2.9 (2019-06-12) <!-- omit in toc -->

`ambix_warp`: fixed crash.

### v0.2.8 (2017-05-20) <!-- omit in toc -->

`ambix_binaural`: improved FFTW thread-safety to fix startup crash when other plug-ins use FFTW. `ambix_rotator`: add quaternion input.

### v0.2.7 (2017-03-22) <!-- omit in toc -->

`ambix_binaural`: improved performance, various bugfixes. `ambix_rotator`: improved performance. `ambix_encoder`: adjustable custom ID for remote control.

### v0.2.6 (2016-04-08) <!-- omit in toc -->

`ambix_converter`: fixed scaling of the W channel when converting from/to FuMa. `ambix_binaural`: convolution engine fix, preset dir save fix. `ambix_vmic`: GUI fix.

### v0.2.5 (2015-12-06) <!-- omit in toc -->

Removed liblo dependency. `ambix_encoder`: improved GUI performance. `ambix_decoder`: fixed binaural L/R impulse response swap, added volume control, indicate loaded preset in menu. `ambix_directional_loudness`: improved solo/window behaviour (multiple filters now pass through when soloed), 8 filters by default. `ambix_rotator`: GUI and OSC rewrite. `ambix_vmic`: GUI update, 8 outputs by default.

### v0.2.4 (2015-07-19) <!-- omit in toc -->

Improved efficiency for `ambix_binaural`.

### v0.2.3 (2014-12-27) <!-- omit in toc -->

Multichannel encoder displays actual source positions; AudioMulch compatibility for saving settings.

### v0.2.2 (2014-08-18) <!-- omit in toc -->

Encoder flickering fix; added OSC settings; new control modes: right-click for relative source movement, Shift to freeze elevation, Ctrl to freeze azimuth.

### v0.2.1 (2014-04-17) <!-- omit in toc -->

Fixed VST identifier for Plogue Bidule compatibility.

### v0.2.0 (2014-03-30) <!-- omit in toc -->

Added `ambix_widening`; JUCE update; encoder GUI panning fix.

### (2014-03-15) <!-- omit in toc -->

Fixed binaural decoder crash during configuration unloading.

### (2014-02-19) <!-- omit in toc -->

Warping curve 2 adjusted slightly (ICSA paper); warping pre-emphasis added; `ambix_encoder`: fixed abs(elevation) > 90°.

### (2014-02-13) <!-- omit in toc -->

Added `ambix_maxre`.

### v0.1.0 (2014-01-24) <!-- omit in toc -->

First release.

---

**Thanks to:** Institute of Electronic Music and Acoustics Graz, Franz Zotter, Winfried Ritsch, Martin Rumori, Florian Hollerweger, Peter Plessas, IOhannes Zmölnig, Thomas Musil, Gerriet K. Sharma, Matthias Frank, Fons Adriaensen, Jörn Nettingsmeier, Filipe Coelho (DISTRHO project), Daniel Rudrich (IEM Plug-ins), Music Innovation Studies Centre of the Lithuanian Academy of Music and Theatre, Ricardas Kabelis, Mantautas Krukauskas, Tadas Dailyda, Sebastian Grill, the surrsound and Linux audio communities.

---

&copy; 2013–2026 Matthias Kronlachner — m.kronlachner@gmail.com
