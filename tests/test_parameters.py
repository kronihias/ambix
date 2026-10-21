"""Parameter and state contract, as a VST3 host sees it.

Moving a plugin from JUCE's legacy setParameter() API to parameter objects
changes what JUCE's wrapper exports, and saved sessions depend on all of it:

  * automation lanes are keyed on the VST3 parameter ID,
  * hosts show the title and the display text,
  * a project stores the component state blob, which the new build has to
    restore, and which an older build should still be able to read back.

None of that shows up in the audio tests, and JUCE hosting JUCE (pedalboard,
pluginval) smooths over several of these differences. So this module goes
through `ambix_bustest --params`, which talks to IComponent / IEditController
directly, and compares against goldens recorded from the legacy build:

  tests/golden/params/<plugin>.json   identity, fresh values, displayed text
  tests/golden/state/<plugin>.bin     state saved with every parameter at 0.55

Values are set the way a host plays automation (IParameterChanges handed to
process()), so the audio-thread path is the one exercised.

Plugins listed in MIGRATED are additionally held to what the legacy API could
not provide: a default that matches the value a fresh instance starts at, and
display text for a value other than the current one.

Regenerate goldens (only when a change is intended and reviewed) with
    pytest tests/test_parameters.py --update-golden
"""

import json
import os
import re
import struct
import subprocess
import sys
import tempfile

import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import ALL_PLUGINS, BUILD_DIR, GOLDEN_DIR, vst3

_BUSTEST_EXE = "ambix_bustest.exe" if os.name == "nt" else "ambix_bustest"
BUSTEST_BIN = os.path.join(BUILD_DIR, "testhost", _BUSTEST_EXE)

PARAMS_GOLDEN_DIR = os.path.join(GOLDEN_DIR, "params")
STATE_GOLDEN_DIR  = os.path.join(GOLDEN_DIR, "state")

# Normalised values every parameter is set to in turn. The ends, the middle
# (0° for the rotators) and two values off any axis.
PROBES = ["0", "0.25", "0.5", "0.55", "0.75", "1"]
STATE_PROBE = "0.55"

# What a host must never see change: an automation lane or a saved mapping
# is keyed on these.
IDENTITY_FIELDS = ["index", "id", "title", "short_title", "units", "step_count",
                   "can_automate", "read_only", "is_bypass", "is_list"]

# Plugins already on parameter objects.
MIGRATED = {"ambix_rotator_z"}

# Values are floats sent through process() and read back; allow for the
# float -> double round trip, nothing more.
VALUE_TOL = 1e-6


# ---------------------------------------------------------------------------
# ambix_bustest plumbing
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def bustest_bin():
    if not os.path.isfile(BUSTEST_BIN):
        pytest.skip(f"ambix_bustest not built at {BUSTEST_BIN} — "
                    "cmake --build _build --target ambix_bustest")
    return BUSTEST_BIN


def run_bustest(binary, plugin, sets=None, state_in=None, state_out=None):
    path = vst3(plugin)
    if not os.path.exists(path):
        pytest.skip(f"{plugin}.vst3 not built")

    cmd = [binary, "--plugin", path, "--params"]
    for pid, value in (sets or {}).items():
        cmd += ["--set", f"{pid}={value}"]
    if state_in:
        cmd += ["--state-in", state_in]
    if state_out:
        cmd += ["--state-out", state_out]

    run = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    assert run.returncode == 0, f"{plugin}: ambix_bustest failed: {run.stderr.strip()}"

    # Plugins may print while loading; the report is the last bare "{" on.
    lines = run.stdout.splitlines()
    starts = [i for i, line in enumerate(lines) if line.strip() == "{"]
    assert starts, f"{plugin}: no report from ambix_bustest:\n{run.stdout[-2000:]}"
    report = json.loads("\n".join(lines[starts[-1]:]))

    if sets:
        assert report.get("automation_processed"), f"{plugin}: process() with automation failed"
    if state_in:
        assert report.get("state_restored"), f"{plugin}: setState() refused the blob"
    if state_out:
        assert report.get("state_saved"), f"{plugin}: getState() failed"

    return report


def settable(params):
    """Every parameter a host would automate. Bypass changes the processing
    path rather than a value, so it stays out of the sweep."""
    return [p for p in params if p["can_automate"] and not p["is_bypass"]]


def processor_chunk(blob):
    """The part of a JUCE VST3 state blob that the processor wrote.

    JUCE's wrapper appends its own "JUCEPrivateData" block (bypass, program)
    after whatever getStateInformation() produced. For the copyXmlToBinary
    format ("VC2!", uint32 size, UTF-8 XML) return the XML text; otherwise
    the raw bytes before JUCE's block."""
    if blob[:4] == b"VC2!":
        (size,) = struct.unpack("<I", blob[4:8])
        xml = blob[8:8 + size].rstrip(b"\0").decode("utf-8")
        # binaural/decoder save their preset folder, which lives under the
        # user's home: not part of the format, and different on every machine
        return re.sub(r'(presetDir|last_config_dir)="[^"]*"', r'\1="<dir>"', xml)
    marker = blob.find(b"JUCEPrivateData")
    return blob if marker < 0 else blob[:marker]


# ---------------------------------------------------------------------------
# One sweep per plugin, shared by every test below
# ---------------------------------------------------------------------------

_sweeps = {}


def sweep(binary, plugin):
    """Fresh instance, then one instance per probe with every settable
    parameter set to it; the STATE_PROBE run also saves its state."""
    if plugin in _sweeps:
        return _sweeps[plugin]

    fresh = run_bustest(binary, plugin)
    ids = [p["id"] for p in settable(fresh["parameters"])]

    displayed = {}
    state = None

    with tempfile.TemporaryDirectory() as tmp:
        for probe in PROBES:
            out = os.path.join(tmp, "state.bin") if probe == STATE_PROBE else None
            report = run_bustest(binary, plugin, {pid: probe for pid in ids}, state_out=out)

            # After setting a value, ask the plugin to display that same
            # value: the legacy and the new API have to agree on this one.
            displayed[probe] = {
                str(p["id"]): {"value": p["value"], "text": p["text"][probe]}
                for p in settable(report["parameters"])
            }

            if out:
                with open(out, "rb") as f:
                    state = f.read()

        # What a reopened project comes back with. Not always exactly what
        # was automated: values pass through the plugin's own state format
        # (the encoder stores rounded floats, maxre derives its order).
        report = run_bustest(binary, plugin, state_in=os.path.join(tmp, "state.bin"))
        restored = {str(p["id"]): p["value"] for p in settable(report["parameters"])}

    _sweeps[plugin] = {
        "parameters": [{k: p[k] for k in IDENTITY_FIELDS} for p in fresh["parameters"]],
        "fresh": {str(p["id"]): p for p in fresh["parameters"]},
        "displayed": displayed,
        "state": state,
        "restored": restored,
    }
    return _sweeps[plugin]


def golden_paths(plugin):
    return (os.path.join(PARAMS_GOLDEN_DIR, f"{plugin}.json"),
            os.path.join(STATE_GOLDEN_DIR, f"{plugin}.bin"))


@pytest.fixture(params=ALL_PLUGINS)
def contract(request, bustest_bin):
    """(current sweep, golden) for one plugin, writing the golden first if
    asked to or if it does not exist yet."""
    plugin = request.param
    current = sweep(bustest_bin, plugin)
    params_path, state_path = golden_paths(plugin)

    if request.config.getoption("--update-golden") or not os.path.exists(params_path):
        os.makedirs(PARAMS_GOLDEN_DIR, exist_ok=True)
        os.makedirs(STATE_GOLDEN_DIR, exist_ok=True)
        with open(params_path, "w") as f:
            json.dump({
                "parameters": current["parameters"],
                "fresh_values": {pid: p["value"] for pid, p in current["fresh"].items()},
                "displayed": current["displayed"],
                "restored": current["restored"],
            }, f, indent=1, sort_keys=True)
            f.write("\n")
        with open(state_path, "wb") as f:
            f.write(current["state"])
        pytest.skip(f"{plugin}: golden written — re-run to compare")

    with open(params_path) as f:
        golden = json.load(f)
    with open(state_path, "rb") as f:
        golden["state"] = f.read()

    return plugin, current, golden


# ---------------------------------------------------------------------------
# What must not change
# ---------------------------------------------------------------------------

def test_parameter_identity(contract):
    """IDs, titles, units and flags: what automation and saved mappings
    are keyed on."""
    plugin, current, golden = contract
    assert current["parameters"] == golden["parameters"], (
        f"{plugin}: parameter list as a host sees it changed")


def test_fresh_instance_values(contract):
    """A new instance starts where it always did."""
    plugin, current, golden = contract
    for pid, value in golden["fresh_values"].items():
        assert current["fresh"][pid]["value"] == pytest.approx(value, abs=VALUE_TOL), (
            f"{plugin}: parameter {pid} starts at a different value")


@pytest.mark.parametrize("probe", PROBES)
def test_automated_value_and_text(contract, probe):
    """Automating a parameter to a value lands on the same value and shows
    the same text."""
    plugin, current, golden = contract
    assert current["displayed"][probe].keys() == golden["displayed"][probe].keys()

    for pid, want in golden["displayed"][probe].items():
        got = current["displayed"][probe][pid]
        assert got["value"] == pytest.approx(want["value"], abs=VALUE_TOL), (
            f"{plugin}: parameter {pid} automated to {probe} reads back differently")
        assert got["text"] == want["text"], (
            f"{plugin}: parameter {pid} at {probe} displays differently")


def test_legacy_state_restores(contract, bustest_bin, tmp_path):
    """A project saved with the legacy build comes back the way the legacy
    build itself restores it."""
    plugin, current, golden = contract
    blob = tmp_path / "legacy.bin"
    blob.write_bytes(golden["state"])

    report = run_bustest(bustest_bin, plugin, state_in=str(blob))
    values = {str(p["id"]): p["value"] for p in report["parameters"]}

    for pid, want in golden["restored"].items():
        assert values[pid] == pytest.approx(want, abs=VALUE_TOL), (
            f"{plugin}: parameter {pid} not restored from a legacy project")


def test_state_format_unchanged(contract):
    """What the processor writes into a project is what the legacy build
    wrote, so a project saved now still opens in an older version."""
    plugin, current, golden = contract
    assert processor_chunk(current["state"]) == processor_chunk(golden["state"]), (
        f"{plugin}: state format changed")


# ---------------------------------------------------------------------------
# What only parameter objects can do
# ---------------------------------------------------------------------------

@pytest.fixture(params=sorted(MIGRATED))
def migrated(request, bustest_bin):
    return request.param, sweep(bustest_bin, request.param)


def test_default_is_the_starting_value(migrated):
    """A host's "reset to default" goes where a fresh instance starts. The
    legacy API reports 0 for every parameter."""
    plugin, current = migrated
    for pid, p in current["fresh"].items():
        if p["is_bypass"]:
            continue
        assert p["default"] == pytest.approx(p["value"], abs=VALUE_TOL), (
            f"{plugin}: parameter {pid} ({p['title']}) default {p['default']} "
            f"but starts at {p['value']}")


def test_text_for_any_value(migrated):
    """Hosts format values the parameter is not at (automation lanes,
    tooltips). The legacy API could only describe the current value."""
    plugin, current = migrated
    for pid, p in current["fresh"].items():
        if p["is_bypass"] or not p["can_automate"]:
            continue
        for probe in PROBES:
            assert p["text"][probe] == current["displayed"][probe][pid]["text"], (
                f"{plugin}: parameter {pid} displays {probe} as {p['text'][probe]!r} "
                f"while not at it, {current['displayed'][probe][pid]['text']!r} when at it")
