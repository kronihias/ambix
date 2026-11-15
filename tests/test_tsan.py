"""ambix_binaural under ThreadSanitizer.

The binaural loads presets on a background thread while the host keeps
calling in; TSan catches races there that a normal run rarely hits.

Needs a separate build tree (AMBIX_TSAN_BUILD_DIR, default _build_tsan):

    cmake -B _build_tsan -DSANITIZE_THREAD=ON -DBUILD_VST=OFF -DBUILD_VST3=ON \\
          -DBUILD_AU=OFF -DBUILD_STANDALONE=OFF -DBUILD_VISUALIZER=OFF -DBUILD_TESTHOST=ON
    cmake --build _build_tsan --target ambix_binaural_VST3 ambix_bustest
"""

import os
import subprocess
import sys
import tempfile

import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import REPO_ROOT
from test_parameter_scenarios import PRESETS_DIR, xml_state

TSAN_BUILD_DIR = os.environ.get("AMBIX_TSAN_BUILD_DIR", os.path.join(REPO_ROOT, "_build_tsan"))
BUSTEST = os.path.join(TSAN_BUILD_DIR, "testhost",
                       "ambix_bustest.exe" if os.name == "nt" else "ambix_bustest")
BINAURAL = os.path.join(TSAN_BUILD_DIR, "vst3", "ambix_binaural.vst3")

# Block-size changes make the plug-in reload on the loader thread, while the
# host keeps processing and reading the state.
SCENARIOS = {
    "after_load": ["wait=3000", "block=512", "wait=3000", "block=256", "wait=3000"],
    "during_load": ["block=512", "block=128", "wait=200", "block=1024", "wait=3000"],
}


@pytest.mark.parametrize("steps", SCENARIOS.values(), ids=SCENARIOS.keys())
def test_reload_without_races(steps):
    if not (os.path.isfile(BUSTEST) and os.path.exists(BINAURAL)):
        pytest.skip(f"no ThreadSanitizer build at {TSAN_BUILD_DIR}")

    state = {"activePreset": "birectangleA-1h1v.config", "presetDir": PRESETS_DIR,
             "Gain": 0.5, "storeConfigDataInProject": 0}

    with tempfile.TemporaryDirectory() as tmp:
        state_path = os.path.join(tmp, "state.bin")
        with open(state_path, "wb") as f:
            f.write(xml_state(state))

        cmd = [BUSTEST, "--plugin", BINAURAL, "--state-in", state_path]
        for step in steps:
            cmd += ["--step", step]

        run = subprocess.run(cmd, capture_output=True, text=True, timeout=600)

    reports = [line for line in run.stderr.splitlines() if line.startswith("SUMMARY: ThreadSanitizer")]
    assert not reports, "\n".join(reports) + "\n\n" + run.stderr[-6000:]
    assert run.returncode == 0, run.stderr[-2000:]
