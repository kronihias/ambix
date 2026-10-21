"""One parameter at a time: what automating it does, to the audio included.

test_parameters.py pins what a host sees of each parameter. This module pins
what each parameter *does*, so moving a plugin to parameter objects cannot
quietly wire a parameter to the wrong place, drop a side effect, or change the
processing. `ambix_bustest --sweep` gives every run a fresh instance with a
single parameter automated through process(), then sends deterministic noise
through every input, and reports:

  * per output channel, the RMS and the correlation with a fixed +-1 mix of
    the inputs (the second one is sign-sensitive: +90 and -90 deg differ),
  * every parameter whose value ended up different from an untouched
    instance — the automated one, plus any side effects or coupling,
  * the text the automated parameter displays,
  * whether the saved state changed.

Goldens in tests/golden/sweep/<plugin>.json, recorded from the legacy build.
Regenerate only for a reviewed, intended change:
    pytest tests/test_parameter_sweep.py --update-golden
"""

import json
import os
import subprocess
import sys

import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import ALL_PLUGINS, GOLDEN_DIR, vst3
from test_parameters import BUSTEST_BIN

SWEEP_GOLDEN_DIR = os.path.join(GOLDEN_DIR, "sweep")

# Below / at / above the thresholds the plugins use for their switches
# (0.33 and 0.66 for three-way selectors, 0.5 for on/off).
SWEEP_PROBES = "0.2,0.5,0.8"

# Same build, same input: only floating-point differences between platforms
# (libm, vectorisation) should remain.
AUDIO_RTOL = 1e-4
AUDIO_ATOL = 1e-6
VALUE_TOL  = 1e-6


@pytest.fixture(scope="session")
def bustest_bin():
    if not os.path.isfile(BUSTEST_BIN):
        pytest.skip(f"ambix_bustest not built at {BUSTEST_BIN}")
    return BUSTEST_BIN


def run_sweep(binary, plugin):
    path = vst3(plugin)
    if not os.path.exists(path):
        pytest.skip(f"{plugin}.vst3 not built")

    run = subprocess.run([binary, "--plugin", path, "--sweep", SWEEP_PROBES],
                         capture_output=True, text=True, timeout=900)
    assert run.returncode == 0, f"{plugin}: ambix_bustest failed: {run.stderr.strip()[-2000:]}"

    lines = run.stdout.splitlines()
    starts = [i for i, line in enumerate(lines) if line.strip() == "{"]
    assert starts, f"{plugin}: no report from ambix_bustest"
    sweep = json.loads("\n".join(lines[starts[-1]:]))["sweep"]
    assert sweep is not None, f"{plugin}: could not run an untouched instance"

    # The state hash covers everything the plugin saves, preset paths under
    # the user's home included, so only "did it change" is portable.
    base = sweep.pop("baseline_state")
    for r in sweep["runs"]:
        if r["ok"]:
            r["state_changed"] = r.pop("state") != base
    return sweep


@pytest.fixture(params=ALL_PLUGINS, scope="module")
def sweep(request, bustest_bin):
    plugin = request.param
    current = run_sweep(bustest_bin, plugin)
    path = os.path.join(SWEEP_GOLDEN_DIR, f"{plugin}.json")

    if request.config.getoption("--update-golden") or not os.path.exists(path):
        os.makedirs(SWEEP_GOLDEN_DIR, exist_ok=True)
        with open(path, "w") as f:
            json.dump(current, f, indent=0, sort_keys=True)
            f.write("\n")
        pytest.skip(f"{plugin}: sweep golden written — re-run to compare")

    with open(path) as f:
        golden = json.load(f)
    return plugin, current, golden


def close(a, b):
    return len(a) == len(b) and all(
        abs(x - y) <= AUDIO_ATOL + AUDIO_RTOL * abs(y) for x, y in zip(a, b))


def describe(run):
    return f"parameter {run['id']} automated to {run['set']}"


def test_untouched_instance(sweep):
    """With nothing automated, the output is what it always was."""
    plugin, current, golden = sweep
    assert close(current["baseline_rms"], golden["baseline_rms"]), f"{plugin}: RMS"
    assert close(current["baseline_proj"], golden["baseline_proj"]), f"{plugin}: correlation"


def test_same_runs(sweep):
    """The same parameters are swept, and every run still works."""
    plugin, current, golden = sweep
    key = lambda r: (r["id"], r["set"], r["ok"])
    assert [key(r) for r in current["runs"]] == [key(r) for r in golden["runs"]]


def test_side_effects(sweep):
    """Automating one parameter changes the same set of parameters, to the
    same values, as before."""
    plugin, current, golden = sweep
    for got, want in zip(current["runs"], golden["runs"]):
        if not want["ok"]:
            continue
        assert got["changed"].keys() == want["changed"].keys(), (
            f"{plugin}: {describe(want)} changes {sorted(got['changed'])}, "
            f"used to change {sorted(want['changed'])}")
        for pid, value in want["changed"].items():
            assert got["changed"][pid] == pytest.approx(value, abs=VALUE_TOL), (
                f"{plugin}: {describe(want)} leaves parameter {pid} at {got['changed'][pid]}, "
                f"used to be {value}")


def test_displayed_text(sweep):
    plugin, current, golden = sweep
    for got, want in zip(current["runs"], golden["runs"]):
        if want["ok"]:
            assert got["text"] == want["text"], f"{plugin}: {describe(want)} displays differently"


def test_saved_state_follows(sweep):
    """A value that used to end up in the saved state still does."""
    plugin, current, golden = sweep
    for got, want in zip(current["runs"], golden["runs"]):
        if want["ok"]:
            assert got["state_changed"] == want["state_changed"], (
                f"{plugin}: {describe(want)}: saved state "
                f"{'now' if got['state_changed'] else 'no longer'} changes")


def test_audio(sweep):
    """What each parameter does to the sound."""
    plugin, current, golden = sweep
    failures = [describe(want) for got, want in zip(current["runs"], golden["runs"])
                if want["ok"] and not (close(got["rms"], want["rms"])
                                       and close(got["proj"], want["proj"]))]
    assert not failures, f"{plugin}: output differs for: " + "; ".join(failures[:20])
