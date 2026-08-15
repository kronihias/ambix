"""
pluginval validation for every ambix VST3.

pluginval (https://github.com/Tracktion/pluginval) is Tracktion's JUCE-based
plugin validator. It exercises the things a real host does and that the DSP
tests here don't cover: repeated cold/warm instantiation, bus-layout
negotiation, sample-rate and block-size changes (incl. mid-stream), calling
processBlock without prepareToPlay, parameter thread-safety, state
save/restore round-trips, editor open/close and — at strictness 10 —
parameter fuzzing. It exits non-zero on any failure.

The validator binary is downloaded once by scripts/get_pluginval.py into
_build/tools/; scripts/run_tests.py does that automatically, so CI needs no
extra step. Without it these tests skip.

Knobs (all env vars):
    PLUGINVAL             use this validator binary instead of downloading
    PLUGINVAL_VERSION     release tag to download (see scripts/get_pluginval.py)
    PLUGINVAL_STRICTNESS  1..10 (default 10 — the suite is fast enough)
    PLUGINVAL_SKIP_GUI    0 to include editor tests (default 1: skip them)
    PLUGINVAL_TIMEOUT_MS  per-test inactivity timeout, -1 to disable
    PLUGINVAL_RANDOM_SEED fix the seed to reproduce a fuzz failure
    PLUGINVAL_REQUIRED    fail instead of skipping when the binary is missing
"""

import os
import subprocess
import sys

import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import ALL_PLUGINS, REPO_ROOT, VST3_DIR, vst3, vst3_load_path

# scripts/get_pluginval.py owns where the validator lives and how it's fetched.
sys.path.insert(0, os.path.join(REPO_ROOT, "scripts"))
from get_pluginval import find_pluginval


LOG_DIR = os.path.join(REPO_ROOT, "_build", "pluginval-logs")

STRICTNESS = os.environ.get("PLUGINVAL_STRICTNESS", "10")
# Editor tests are skipped by default: ambix_encoder's editor opens an
# OpenGL context (Source/SphereOpenGL.cpp), which hosted CI runners can't be
# relied on to provide. Set PLUGINVAL_SKIP_GUI=0 on a machine with a real
# display to include them.
SKIP_GUI = os.environ.get("PLUGINVAL_SKIP_GUI", "1") not in ("0", "false", "no")
# pluginval's own timeout fires when a single test produces no output for this
# long. 30 s (its default) is tight for a loaded CI runner.
TIMEOUT_MS = os.environ.get("PLUGINVAL_TIMEOUT_MS", "300000")
# Belt-and-braces wall-clock limit so a wedged validator fails the test
# instead of hanging the job until the workflow timeout.
PROC_TIMEOUT_S = 900


def _plugin_names():
    """Every plugin to validate: the canonical list plus whatever else was
    built (e.g. the fixed-input ambix_encoder_i2/i4/i6/i8 variants)."""
    names = set(ALL_PLUGINS)
    if os.path.isdir(VST3_DIR):
        names |= {f[:-len(".vst3")] for f in os.listdir(VST3_DIR)
                  if f.endswith(".vst3")}
    return sorted(names)


@pytest.fixture(scope="session")
def pluginval_bin():
    try:
        found = find_pluginval()
    except RuntimeError as e:
        pytest.fail(str(e))
    if not found:
        msg = "pluginval not installed — run: python scripts/get_pluginval.py"
        # CI sets PLUGINVAL_REQUIRED so a failed download surfaces as a red
        # job rather than silently dropping this whole layer of coverage.
        if os.environ.get("PLUGINVAL_REQUIRED", "") not in ("", "0", "false", "no"):
            pytest.fail(f"PLUGINVAL_REQUIRED is set but {msg}")
        pytest.skip(msg)
    return str(found)


def _run_pluginval(pluginval_bin, plugin_path, plugin_name):
    """Validate one plugin. Returns (returncode, command, combined output)."""
    os.makedirs(LOG_DIR, exist_ok=True)
    cmd = [
        pluginval_bin,
        "--strictness-level", STRICTNESS,
        "--timeout-ms", TIMEOUT_MS,
        "--output-dir", LOG_DIR,
    ]
    if SKIP_GUI:
        cmd.append("--skip-gui-tests")
    seed = os.environ.get("PLUGINVAL_RANDOM_SEED")
    if seed:
        cmd += ["--random-seed", seed]
    cmd += ["--validate", plugin_path]

    try:
        res = subprocess.run(cmd, capture_output=True, text=True,
                             timeout=PROC_TIMEOUT_S)
    except subprocess.TimeoutExpired:
        pytest.fail(f"{plugin_name}: pluginval did not finish within "
                    f"{PROC_TIMEOUT_S}s\n+ {' '.join(cmd)}")
    return res.returncode, cmd, (res.stdout or "") + (res.stderr or "")


@pytest.mark.parametrize("plugin_name", _plugin_names())
def test_pluginval(plugin_name, pluginval_bin):
    path = vst3(plugin_name)
    if not os.path.exists(path):
        pytest.skip(f"Plugin not built: {path}")

    rc, cmd, out = _run_pluginval(pluginval_bin, path, plugin_name)

    if rc != 0 and "Num plugins found: 0" in out:
        # Nothing was scanned rather than something failing. On Windows the
        # host may not accept the bundle directory (same wrinkle conftest's
        # vst3_load_path works around for pedalboard) — retry the inner DLL
        # before calling it a failure.
        inner = vst3_load_path(path)
        if inner != path and os.path.isfile(inner):
            print(f"{plugin_name}: bundle scan found no plugins, "
                  f"retrying with {inner}")
            rc, cmd, out = _run_pluginval(pluginval_bin, inner, plugin_name)

    if rc != 0:
        # pluginval logs every test it runs; only the tail is useful in a CI
        # log. The full run is in _build/pluginval-logs/ (uploaded as a CI
        # artifact on failure), and the seed printed near the top of the
        # output replays a fuzz failure via PLUGINVAL_RANDOM_SEED.
        tail = "\n".join(out.splitlines()[-60:])
        pytest.fail(
            f"{plugin_name}: pluginval exited {rc}\n"
            f"+ {' '.join(cmd)}\n"
            f"--- last 60 lines ---\n{tail}"
        )
