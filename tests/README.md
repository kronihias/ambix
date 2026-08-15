# ambix Plugin Test Suite

Python-based unit and regression tests for the ambix Ambisonic plugin suite.

Modelled after the test setup in the sibling `mcfx` project (pytest +
pedalboard + scipy + golden `.npy` files), with the same two-tier host strategy:

1. **Tier 1 — pedalboard** (in-process) for plugins with square main buses.
2. **Tier 2 — `ambix_testhost`** (a small JUCE console host built from this
   tree) for plugins whose main bus is asymmetric (encoder mono→ambi,
   decoder 4→N, binaural 4→2) or for testing at orders >= 2 (9, 16, … channels).
   pedalboard pins bus width at load time and only negotiates symmetric layouts.

---

## Setup

### 1. Create and activate a virtual environment

```bash
python3 -m venv tests/.venv
source tests/.venv/bin/activate
```

### 2. Install Python dependencies

```bash
pip install -r tests/requirements.txt
```

### 3. Build the VST3 plugins and the testhost

```bash
cmake -B _build -DBUILD_VST=OFF -DBUILD_VST3=ON -DBUILD_AU=OFF -DBUILD_STANDALONE=OFF -DBUILD_TESTHOST=ON
cmake --build _build --target ambix_converter_VST3 ambix_testhost
```

Build other plugin targets (`ambix_mirror_VST3`, `ambix_encoder_VST3`,
`ambix_binaural_VST3`, `ambix_decoder_VST3`, etc.) as the corresponding
test files require them. Tests skip cleanly if a plugin isn't built.

### 4. Fetch pluginval (optional)

```bash
python scripts/get_pluginval.py
```

Downloads Tracktion's validator into `_build/tools/`; see
[Tier 3 — pluginval](#tier-3--pluginval). `scripts/run_tests.py` does this
for you, and `test_pluginval.py` skips if the binary is absent.

---

## Running tests

```bash
source tests/.venv/bin/activate
pytest tests/ -v
```

### Update golden reference files

Run this after an **intentional** DSP change to accept the new output:

```bash
pytest tests/ --update-golden
```

---

## Structure

```
tests/
├── reference/                    # Python DSP reference implementations
│   └── converter_ref.py          # ACN/FuMa/SID maps + normalisation gains (1st order)
├── golden/                       # Stored .npy reference outputs (committed)
├── conftest.py                   # pytest fixtures, plugin paths, run_testhost helper
├── test_converter.py             # ambix_converter — 1st order (pedalboard)
├── test_converter_order2.py      # ambix_converter — 2nd order via testhost (CS, 2D flags, N3D scaling)
├── test_encoder.py               # ambix_encoder — directional SH encoding via testhost
├── test_binaural.py              # ambix_binaural — 4→2 structural checks via testhost
├── test_decoder.py               # ambix_decoder — 4→N structural checks via testhost
├── test_mirror.py                # ambix_mirror — full (l,m)-bucket coverage
├── test_rotator.py               # ambix_rotator — invariants + golden
├── test_rotator_z.py             # ambix_rotator_z — invariants + golden
├── test_maxre.py                 # ambix_maxre — invariants + golden
├── test_move.py                  # ambix_move — translation/rotation bypass + energy
├── test_warp.py                  # ambix_warp — identity at defaults + finite
├── test_widening.py              # ambix_widening — finite under modulation
├── test_directional_loudness.py  # smoke only (config-heavy)
├── test_vmic.py                  # smoke only (config-heavy)
├── test_smoke_all.py             # load + silence + finite over every built VST3
├── test_pluginval.py             # Tracktion pluginval host-compatibility run over every built VST3
├── requirements.txt
└── pytest.ini
```

`ambix_visualizer` is a JUCE GUI app, not a VST3, and is correctly absent
from `ALL_PLUGINS`.

---

## How it works

### Tier 1 — pedalboard

`pedalboard.load_plugin` is called via the `_load()` helper in
`conftest.py`. The host pins the bus width at instantiation and refuses
asymmetric layouts, so encoder/decoder/binaural fail to load and are
skipped from the smoke layer. Parameters are set with
`plugin[display_name] = float_in_0_to_1`.

### Tier 2 — ambix_testhost

A small JUCE console app (`ambix_testhost/Source/Main.cpp`, ~250 lines)
loads a VST3 at a caller-specified discrete or ambisonic-order channel
layout, sets parameters from a JSON file, processes a WAV input, and
writes the output WAV. Python calls it via `subprocess` and reads the
result through `soundfile`. See `run_testhost()` in `conftest.py`.

The testhost negotiates the bus layout by trying
`AudioChannelSet::ambisonic(order)` first for widths matching
(order+1)² and falling back to `discreteChannels(N)` otherwise. That
matters because JUCE's VST3 wrapper rejects `discreteChannels` for
ambisonic widths.

### Tier 3 — pluginval

`test_pluginval.py` runs [Tracktion's pluginval](https://github.com/Tracktion/pluginval)
over every VST3 in `_build/vst3/` (including the fixed-input
`ambix_encoder_i2/i4/i6/i8` variants) — one parametrized test per plugin,
failing on a non-zero exit code. Where the tests above check *what the DSP
computes*, pluginval checks *how the plugin behaves in a host*: repeated
cold/warm instantiation, bus-layout negotiation, sample-rate and block-size
changes mid-stream, `processBlock` without `prepareToPlay`, parameter
thread-safety, state save/restore round-trips, editor open/close, and (at
strictness 10) parameter fuzzing.

The validator binary is fetched once from the pinned GitHub release into
`_build/tools/pluginval-<version>/`:

```bash
python scripts/get_pluginval.py
```

`scripts/run_tests.py` does this automatically, so CI needs no extra step.
If the binary is missing (e.g. no network), the pluginval tests skip.

Environment knobs:

| Variable | Default | Meaning |
| --- | --- | --- |
| `PLUGINVAL` | — | Use this validator binary instead of downloading |
| `PLUGINVAL_VERSION` | `v1.0.4` | Release tag to download |
| `PLUGINVAL_STRICTNESS` | `10` | 1–10; 5 is pluginval's own default |
| `PLUGINVAL_SKIP_GUI` | `1` | Set to `0` to include editor tests |
| `PLUGINVAL_TIMEOUT_MS` | `300000` | Per-test inactivity timeout (`-1` disables) |
| `PLUGINVAL_RANDOM_SEED` | — | Fix the seed to replay a fuzz failure |
| `PLUGINVAL_REQUIRED` | — | Fail instead of skipping if the binary is missing (set in CI) |

Editor tests are off by default because `ambix_encoder`'s editor creates an
OpenGL context (`ambix_encoder/Source/SphereOpenGL.cpp`), which hosted CI
runners don't reliably provide. On a machine with a real display:

```bash
PLUGINVAL_SKIP_GUI=0 pytest tests/test_pluginval.py -v
```

Full validator logs land in `_build/pluginval-logs/`; CI uploads that
directory as an artifact when the job fails.

### Golden files

On the first run of each regression test the plugin output is saved to
`tests/golden/<name>.npy`. Subsequent runs compare against it. Commit
the golden files alongside the code so CI can detect regressions.

Current goldens:

- `converter_fuma_n3d_flip_noise.npy` — ACN/SN3D → FuMa/N3D + L/R mirror
- `rotator_yaw60_pitch40_roll55_noise.npy` — non-trivial 3-axis rotation
- `rotator_z_55deg_noise.npy` — Z-only rotation at ~+18°
- `maxre_order1_apply1_noise.npy` — max-rE weighting at 1st order

---

## Coverage notes

Most tests run at 1st order (4 channels) because pedalboard can't host
the universal builds at wider widths. The 2nd-order `test_converter_order2.py`
uses the testhost and exercises code paths that are identity at 1st
order: Condon-Shortley phase flip, `InputIs2D`/`OutputIs2D` channel
packing, and the order-2 row of the SN3D↔N3D scaling table.

`ambix_directional_loudness`, `ambix_vmic`, and `ambix_decoder` (without
a preset) depend on runtime configuration that's outside this suite's
scope; their dedicated test files only assert structural invariants
(load, silence, finite output, correct shape). Functional DSP coverage
there would need a committed configuration fixture and is left as
future work.
