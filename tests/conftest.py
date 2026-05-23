"""
pytest fixtures for ambix plugin tests.

Two host backends are used:

- **pedalboard** (tier 1): square-bus plugins only (converter, mirror,
  rotator(_z), maxre, move, warp, widening, vmic, directional_loudness).
  Plugins are loaded once per test session (scope="session") to avoid
  repeated disk I/O and plugin initialisation overhead.

- **ambix_testhost** (tier 2): subprocess CLI that hosts plugins with
  asymmetric main buses — needed for ambix_encoder (1→4), ambix_decoder
  (4→N) and ambix_binaural (4→2), which pedalboard refuses to load.

VST3s are expected at `<repo>/_build/vst3/` and the testhost at
`<repo>/_build/testhost/ambix_testhost`, built via:

    cmake -B _build -DBUILD_VST3=ON -DBUILD_TESTHOST=ON
    cmake --build _build --target ambix_<plugin>_VST3 ambix_testhost
"""

import os
import sys
from typing import Optional

import numpy as np
import pytest

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VST3_DIR  = os.path.join(REPO_ROOT, "_build", "vst3")
_TESTHOST_EXE = "ambix_testhost.exe" if os.name == "nt" else "ambix_testhost"
TESTHOST_BIN = os.path.join(REPO_ROOT, "_build", "testhost", _TESTHOST_EXE)

GOLDEN_DIR = os.path.join(os.path.dirname(__file__), "golden")


def vst3(name: str) -> str:
    return os.path.join(VST3_DIR, f"{name}.vst3")


def vst3_load_path(bundle_path: str) -> str:
    # Pedalboard on Windows fails to recognise a JUCE-built VST3 bundle dir
    # (no desktop.ini marker), but loads the inner DLL fine. Resolve to it.
    if sys.platform == "win32" and os.path.isdir(bundle_path):
        inner = os.path.join(bundle_path, "Contents", "x86_64-win",
                             os.path.basename(bundle_path))
        if os.path.isfile(inner):
            return inner
    return bundle_path


# ---------------------------------------------------------------------------
# pedalboard plugin fixtures
# ---------------------------------------------------------------------------

class PluginWrapper:
    """Wraps a pedalboard plugin to support display-name item access.

    pedalboard >= 0.9 removed __getitem__/__setitem__ from VST3Plugin.
    Tests use plugin["Param Name"] = value; this wrapper restores that
    by mapping display names to Python attribute names at construction time.
    Setting a parameter writes its raw_value (normalised 0..1 float) to match
    the plugins' legacy JUCE setParameter API.
    """

    def __init__(self, plugin):
        self._plugin = plugin
        self._name_map = {p.name: k for k, p in plugin.parameters.items()}

    def __setitem__(self, display_name: str, value):
        py_name = self._name_map.get(display_name)
        if py_name is None:
            raise KeyError(
                f"No parameter {display_name!r}. "
                f"Available: {sorted(self._name_map)}"
            )
        self._plugin.parameters[py_name].raw_value = float(value)

    def __getitem__(self, display_name: str):
        py_name = self._name_map.get(display_name)
        if py_name is None:
            raise KeyError(display_name)
        return self._plugin.parameters[py_name].raw_value

    def __call__(self, *args, **kwargs):
        return self._plugin(*args, **kwargs)

    def __getattr__(self, name):
        return getattr(self._plugin, name)


def _load(name: str):
    try:
        import pedalboard
    except ImportError:
        pytest.skip("pedalboard not installed — run: pip install pedalboard")
    path = vst3(name)
    if not os.path.exists(path):
        pytest.skip(f"Plugin not built: {path}")
    try:
        return PluginWrapper(pedalboard.load_plugin(vst3_load_path(path)))
    except ValueError as e:
        # pedalboard requires plugins to support num_inputs == num_outputs.
        # ambix_binaural (4→2), ambix_decoder (4→64) and the universal
        # ambix_encoder (mono→ambi) all advertise asymmetric main buses
        # and fail at instantiation with this error.
        if "does not support" in str(e) and "channel output" in str(e):
            pytest.skip(f"{name}: pedalboard cannot host asymmetric I/O — {e}")
        raise


# List of every plugin in the suite. Used by the smoke-test module to load
# each one as a parametrized fixture.
ALL_PLUGINS = [
    "ambix_binaural",
    "ambix_converter",
    "ambix_decoder",
    "ambix_directional_loudness",
    "ambix_encoder",
    "ambix_maxre",
    "ambix_mirror",
    "ambix_move",
    "ambix_rotator",
    "ambix_rotator_z",
    "ambix_vmic",
    "ambix_warp",
    "ambix_widening",
]


@pytest.fixture(scope="session")
def plugin_converter():
    return _load("ambix_converter")


@pytest.fixture(scope="session")
def plugin_mirror():
    return _load("ambix_mirror")


@pytest.fixture(scope="session")
def plugin_rotator():
    return _load("ambix_rotator")


@pytest.fixture(scope="session")
def plugin_rotator_z():
    return _load("ambix_rotator_z")


@pytest.fixture(scope="session")
def plugin_move():
    return _load("ambix_move")


@pytest.fixture(scope="session")
def plugin_warp():
    return _load("ambix_warp")


@pytest.fixture(scope="session")
def plugin_widening():
    return _load("ambix_widening")


@pytest.fixture(scope="session")
def plugin_maxre():
    return _load("ambix_maxre")


@pytest.fixture(scope="session")
def plugin_encoder():
    return _load("ambix_encoder")


@pytest.fixture(scope="session")
def plugin_binaural():
    return _load("ambix_binaural")


@pytest.fixture(scope="session")
def plugin_decoder():
    return _load("ambix_decoder")


@pytest.fixture(scope="session")
def plugin_directional_loudness():
    return _load("ambix_directional_loudness")


@pytest.fixture(scope="session")
def plugin_vmic():
    return _load("ambix_vmic")


# ---------------------------------------------------------------------------
# Common test signals
# ---------------------------------------------------------------------------

SR = 48000
BLOCK = SR  # 1 s of audio at 48 kHz

# 1st-order Ambisonics: 4 channels in ACN order (W, Y, Z, X).
N_AMBI_CH = 4


# ---------------------------------------------------------------------------
# Tier-2 host: ambix_testhost CLI (asymmetric main buses)
# ---------------------------------------------------------------------------

def run_testhost(plugin_name: str,
                 audio: np.ndarray,
                 in_channels: int,
                 out_channels: int,
                 params: Optional[dict] = None,
                 fs: int = SR) -> np.ndarray:
    """
    Run audio through `plugin_name` via the ambix_testhost CLI.

    `audio`        : (in_channels, N) float32 numpy array.
    `params`       : dict mapping plugin parameter display name to a
                     normalised [0, 1] float.

    Returns (out_channels, N) float32. Skips cleanly if the testhost
    binary or required Python deps are missing.
    """
    import json
    import subprocess
    import tempfile

    try:
        import soundfile as sf
    except ImportError:
        pytest.skip("soundfile not installed — run: pip install soundfile")

    if not os.path.exists(TESTHOST_BIN):
        pytest.skip(
            f"ambix_testhost not built: {TESTHOST_BIN}\n"
            f"build with: cmake -B _build -DBUILD_TESTHOST=ON && "
            f"cmake --build _build --target ambix_testhost"
        )

    plugin_path = vst3(plugin_name)
    if not os.path.exists(plugin_path):
        pytest.skip(f"Plugin not built: {plugin_path}")

    if audio.ndim == 1:
        audio = audio[None, :]
    assert audio.shape[0] == in_channels, (
        f"audio has {audio.shape[0]} channels, expected {in_channels}"
    )

    with tempfile.TemporaryDirectory() as tmp:
        in_wav  = os.path.join(tmp, "in.wav")
        out_wav = os.path.join(tmp, "out.wav")

        # soundfile expects (samples, channels) for multi-channel writes.
        sf.write(in_wav, audio.T, fs, subtype="FLOAT")

        cmd = [
            TESTHOST_BIN,
            "--plugin",       plugin_path,
            "--in-channels",  str(in_channels),
            "--out-channels", str(out_channels),
            "--input",        in_wav,
            "--output",       out_wav,
            "--samplerate",   str(fs),
        ]
        if params:
            par_json = os.path.join(tmp, "params.json")
            with open(par_json, "w") as f:
                json.dump(params, f)
            cmd += ["--params", par_json]

        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            raise RuntimeError(
                f"ambix_testhost failed (rc={result.returncode})\n"
                f"stdout: {result.stdout}\nstderr: {result.stderr}"
            )

        data, _ = sf.read(out_wav, dtype="float32")
        # soundfile returns (samples,) or (samples, channels) — normalise.
        if data.ndim == 1:
            data = data[:, None]
        return data.T  # (channels, samples)


# ---------------------------------------------------------------------------
# Golden file helpers
# ---------------------------------------------------------------------------

def load_golden(name: str) -> np.ndarray:
    return np.load(os.path.join(GOLDEN_DIR, f"{name}.npy"))


def save_golden(name: str, array: np.ndarray) -> None:
    os.makedirs(GOLDEN_DIR, exist_ok=True)
    np.save(os.path.join(GOLDEN_DIR, f"{name}.npy"), array)


def golden_exists(name: str) -> bool:
    return os.path.exists(os.path.join(GOLDEN_DIR, f"{name}.npy"))


# ---------------------------------------------------------------------------
# --update-golden pytest flag
# ---------------------------------------------------------------------------

def pytest_addoption(parser):
    parser.addoption("--update-golden", action="store_true", default=False,
                     help="Overwrite golden reference files")
