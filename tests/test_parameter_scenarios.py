"""Order- and history-dependent parameter behaviour, per plugin.

The sweep (test_parameter_sweep.py) changes one parameter on a fresh
instance. Much of what the plugins do with their parameters only shows up in
sequences:

  * which of two parameters set in the same block wins (rotator and move pick
    Euler or quaternion mode by whichever was touched last; mirror's preset
    rewrites the other parameters),
  * history (encoder's Set/SetRel against the move gate, linked -> unlinked
    snapshot, warp's curve switch at exactly 0.5, converter's 0.66 dead zone),
  * parameters that only matter in combination (maxre's order once weighting
    is on, widening's Mod T once depth is non-zero, its latency),
  * layouts above first order (move/warp/maxre orders),
  * state that a project restores (binaural/decoder with a configuration
    loaded, vmic recomputing after a restore, legacy states missing an
    attribute).

Scenarios marked "offline" process the way a host renders, for plugins
whose real-time output legitimately depends on timing (binaural's convolver
drops late partitions rather than wait for them). "late_state" restores the
state after activating instead of before.

Each scenario runs through `ambix_bustest --step ...` on one instance. After
every step it records latency, per-channel output RMS and correlation, and
every parameter's value and text. Goldens in tests/golden/scenarios/, recorded
from the legacy build; regenerate only for a reviewed change:
    pytest tests/test_parameter_scenarios.py --update-golden
"""

import json
import os
import struct
import subprocess
import sys
import tempfile

import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import GOLDEN_DIR, REPO_ROOT, vst3
from test_parameters import BUSTEST_BIN

SCENARIO_GOLDEN_DIR = os.path.join(GOLDEN_DIR, "scenarios")
PRESETS_DIR = os.path.join(REPO_ROOT, "ambix_binaural", "Presets")

AUDIO_RTOL = 1e-4
AUDIO_ATOL = 1e-6
VALUE_TOL  = 1e-6


def hash_id(text):
    """JUCE's VST3 ID for a parameter ID string: Java-style String hash,
    masked to 31 bits (see juce_VST3ClientExtensions.cpp)."""
    h = 0
    for ch in text:
        h = (31 * h + ord(ch)) & 0xFFFFFFFF
    return h & 0x7FFFFFFF


# Plugins whose parameter count collides with a parameter's hashed index
# (vmic and dirloud: 56 = hash("8") = the wrapper bypass ID).
NUM_PARAMS = {"ambix_vmic": 56, "ambix_directional_loudness": 56}


def vst3_id(index, plugin=None):
    """The VST3 ID of parameter `index`, as ambix::Parameter::idFor() picks
    it: the hashed index, or the hashed "<index>_vst3" where the plain one
    would land on the bypass."""
    plain = hash_id(str(index))
    if plugin is not None and plain == NUM_PARAMS.get(plugin):
        return hash_id(f"{index}_vst3")
    return plain


def mask(n):
    return (1 << 64) - 1 if n >= 64 else (1 << n) - 1


def xml_state(attributes, tag="MYPLUGINSETTINGS"):
    """A state blob as a host would hand it back: the processor's
    copyXmlToBinary() chunk ("VC2!", size, UTF-8 XML, NUL)."""
    attrs = " ".join(f'{k}="{v}"' for k, v in attributes.items())
    xml = f'<?xml version="1.0" encoding="UTF-8"?>\n\n<{tag} {attrs}/>\n'.encode() + b"\0"
    return b"VC2!" + struct.pack("<I", len(xml)) + xml


# ---------------------------------------------------------------------------
# Scenarios. A step is a list of (parameter index, value) pairs, sent in
# that order in one block; ("wait", ms) pauses first. Values are normalised.
# ---------------------------------------------------------------------------

def W(ms):
    return ("wait", ms)


class RawID(int):
    """A VST3 parameter ID used as is, rather than a parameter index."""


ENC_AZ, ENC_EL, ENC_SIZE, ENC_WIDTH = 0, 1, 2, 3
ENC_AZSET, ENC_AZREL, ENC_AZMV, ENC_ELSET, ENC_ELREL, ENC_ELMV = 4, 5, 6, 7, 8, 9
ENC_SPEED, ENC_LINKED, ENC_ACTIVE = 10, 11, 12


def enc_src(src, slot):
    """slot: 0 az, 1 el, 2 size, 3 mute, 4 solo."""
    return 13 + src * 5 + slot


def enc_active(n, max_sources=64):
    """ActiveSources value for n sources (normToCount = round(v*(max-1))+1)."""
    return (n - 1) / (max_sources - 1)


SCENARIOS = {
    "ambix_encoder": {
        "set_gate": {"steps": [
            [(ENC_AZSET, 0.7)],                     # gate closed (Mv 0.5): Az follows
            [(ENC_AZMV, 0.6)],                      # outside the gate
            [(ENC_AZSET, 0.3)],                     # ... so no effect
            [(ENC_AZMV, 0.5)],
            [(ENC_AZREL, 0.6)],                     # relative: Az += 0.1
            [(ENC_AZREL, 0.1)],                     # wraps below 0
            [(ENC_ELSET, 0.8)], [(ENC_ELREL, 0.3)],
        ]},
        "rotate_linked": {"steps": [
            [(ENC_SPEED, 0.5), (ENC_AZMV, 0.8)], [], [], [(ENC_ELMV, 0.2)], [], [(ENC_AZMV, 0.5)],
        ]},
        "unlink_snapshot": {"steps": [
            [(ENC_ACTIVE, enc_active(3)), (ENC_WIDTH, 0.3), (ENC_AZ, 0.6)],
            [(ENC_LINKED, 0.0)],                    # snapshot of the spread
            [(enc_src(1, 0), 0.7)],
            [(ENC_AZMV, 0.8)], [],                  # unlinked: sources rotate
            [(ENC_LINKED, 1.0)],                    # no snapshot back
        ]},
        "same_block_order": {"steps": [
            [(ENC_ACTIVE, enc_active(4)), (ENC_LINKED, 0.0)],
            [(ENC_LINKED, 1.0)],
            [(ENC_LINKED, 0.0), (ENC_ACTIVE, enc_active(2))],
        ]},
        "mute_solo": {"steps": [
            [(ENC_ACTIVE, enc_active(4)), (ENC_WIDTH, 0.4)],
            [(enc_src(1, 3), 1.0)], [(enc_src(2, 4), 1.0)], [(enc_src(5, 4), 1.0)],
            [(enc_src(2, 4), 0.0)], [(enc_src(1, 3), 0.4)],
        ]},
    },

    "ambix_rotator": {
        "euler_then_quaternion": {"steps": [[(0, 0.75)], [(5, 1.0)], [(1, 0.6)]]},
        "same_block_euler_last": {"steps": [[(5, 1.0), (0, 0.6)]]},
        "same_block_quaternion_last": {"steps": [[(0, 0.6), (5, 1.0)]]},
        "rotation_order": {"steps": [
            [(0, 0.6), (1, 0.4), (2, 0.7)], [(3, 1.0)], [(3, 0.5)], [(3, 0.0)],
        ]},
        "quaternion_invert": {"steps": [
            [(4, 0.9), (5, 0.7), (6, 0.6)], [(8, 1.0)], [(8, 0.0)],
            [(4, 0.5), (5, 0.5), (6, 0.5), (7, 0.5)],   # all zero -> identity
        ]},
        "legacy_state_without_mode": {
            "state": {str(i): v for i, v in enumerate([0.75, 0.5, 0.5, 0, 0.5, 1, 0.5, 0.5, 0])},
            "steps": [[], [(1, 0.6)]],
        },
    },

    "ambix_rotator_z": {
        "sequence": {"steps": [[(0, 0.6)], [(0, 0.6)], [(0, 0.1)], [(0, 1.0)]]},
    },

    "ambix_mirror": {
        "preset_then_tweak": {"steps": [
            [(14, 3 / 8)], [(0, 0.5)],              # flip, then a host tweak: preset stays
            [(14, 3.5 / 8)],                        # same preset again: tweak undone
            [(14, 6 / 8)], [(14, 1.0)], [(14, 0.1)],
        ]},
        "same_block_preset_first": {"steps": [[(14, 4 / 8), (3, 1.0)]]},
        "same_block_preset_last": {"steps": [[(3, 1.0), (14, 4 / 8)]]},
        "state_preset_overrides": {
            "state": {**{str(i): (0.75 if i % 2 == 0 else 0) for i in range(14)}, "2": 0.3, "14": 5 / 8},
            "steps": [[]],
        },
    },

    "ambix_move": {
        "orders_third": {"arr": (16, 16), "steps": [
            [(13, 3 / 7), (14, 3 / 7)], [(0, 0.7), (1, 0.4)], [(13, 1 / 7)], [(14, 2 / 7)], [(13, 1.0), (14, 1.0)],
        ]},
        "quaternion_flag": {"steps": [[(4, 0.75)], [(9, 1.0)], [(5, 0.6)], [(9, 0.8)]]},
        "translation_switch": {"steps": [
            [(0, 0.8), (3, 0.2)], [(15, 0.0)], [(15, 1.0)], [(16, 0.0), (4, 0.7)], [(16, 1.0)],
        ]},
        "legacy_state_missing_switches": {
            "state": {str(i): v for i, v in enumerate([0.6, 0.5, 0.5, 0, 0.7, 0.5, 0.5, 0,
                                                       0.5, 0.5, 0.5, 0.5, 0, 1, 1])},
            "steps": [[]],
        },
    },

    "ambix_warp": {
        "orders_and_curves": {"arr": (16, 16), "steps": [
            [(4, 3 / 7), (5, 3 / 7)], [(0, 0.8)], [(1, 1.0)], [(1, 0.5)], [(1, 0.0)],
            [(2, 0.3), (3, 1.0)], [(6, 0.0)], [(6, 0.5)], [(6, 1.0)], [(5, 1 / 7)],
        ]},
    },

    "ambix_widening": {
        "depth_period_latency": {"steps": [
            [(0, 0.3)], [(1, 0.5)], [(3, 0.0)], [(1, 0.2)], [(2, 0.7)], [(3, 1.0)], [(3, 0.5)],
        ]},
    },

    "ambix_maxre": {
        "weighting_and_order": {"arr": (16, 16), "steps": [
            [(0, 1.0)], [(1, 0.2)], [(1, 0.6)], [(0, 0.0)], [(1, 0.3)],
            [(0, 0.33)], [(0, 0.329)], [(0, 0.66)], [(0, 0.659)],
        ]},
    },

    "ambix_converter": {
        "norm_matrix": {"steps": [
            [(2, a), (3, b)] for a in (0.0, 0.5, 1.0) for b in (0.0, 0.5, 1.0)]},
        "sequence_matrix": {"steps": [
            [(0, a), (1, b)] for a in (0.0, 0.5, 1.0) for b in (0.0, 0.5, 1.0)]},
        "flags": {"steps": [
            [(4, 1.0)], [(5, 1.0)], [(6, 1.0)], [(7, 1.0)], [(8, 1.0)], [(9, 1.0)],
            [(8, 0.0), (1, 0.5)], [(4, 0.0), (5, 0.0), (6, 0.0), (7, 0.0), (9, 0.0)],
        ]},
        "dead_zone": {"steps": [[(2, 1.0)], [(2, 0.66)], [(2, 0.0)], [(2, 0.66)], [(2, 0.5)], [(2, 0.66)]]},
    },

    "ambix_vmic": {
        "filter_edits": {"steps": [
            [(5, 0.7)], [(0, 0.7)], [(2, 1.0), (3, 0.3), (4, 0.2)], [(2, 0.5)], [(12, 0.2)],
            [(RawID(vst3_id(8)), 1.0)],             # ID 56: the bypass, not parameter 8
            [(RawID(vst3_id(8)), 0.0)],
        ]},
        "restore_defaults_recomputes": {
            "state": {str(i): v for f in range(8) for i, v in
                      zip(range(7 * f, 7 * f + 7), [0.5, 0.5, 0, 0.1, 0.1, 0.5, 0])},
            "steps": [[]],
        },
        "elevation_2_reaches_dsp": {"steps": [
            [(12, 0.2)], [(8, 0.8)], [(8, 0.3)],   # filter 2 attenuated, then moved
        ]},
        "restore_reaches_parameter_8": {
            "state": {**{str(i): v for f in range(8) for i, v in
                         zip(range(7 * f, 7 * f + 7), [0.5, 0.5, 0, 0.1, 0.1, 0.5, 0])}, "8": 0.8},
            "steps": [[]],
        },
    },

    "ambix_directional_loudness": {
        "elevation_2_reaches_dsp": {"steps": [
            [(12, 0.2)], [(8, 0.8)], [(8, 0.3)],   # filter 2 attenuated, then moved
        ]},
        "filter_edits_and_solo": {"steps": [
            [(5, 0.3)], [(2, 1.0), (3, 0.4), (4, 0.3)], [(6, 1.0)], [(12, 0.8)], [(13, 1.0)],
            [(6, 0.0), (13, 0.0)], [(2, 0.5)], [(RawID(vst3_id(8)), 1.0)], [(RawID(vst3_id(8)), 0.0)],
        ]},
        "restore_defaults_recomputes": {
            "state": {str(i): v for f in range(8) for i, v in
                      zip(range(7 * f, 7 * f + 7), [0.5, 0.5, 0, 0.1, 0.1, 0.5, 0])},
            "steps": [[]],
        },
    },

    "ambix_binaural": {
        "gain_with_config": {
            "state": {"activePreset": "birectangleA-1h1v.config", "presetDir": PRESETS_DIR,
                      "Gain": 0.5, "storeConfigDataInProject": 0},
            "steps": [[W(4000)], [(0, 0.3)], [(0, 0.8)], [(0, 0.0)], [(0, 0.5)]],
            "sound_from_step": 0,
            "offline": True,
        },
    },

    "ambix_decoder": {
        "gain_with_config": {
            "arr": (16, 10),
            "state": {"activePreset": "9.1-AllRad_o3.config", "presetDir": PRESETS_DIR,
                      "Gain": 0.6, "storeConfigDataInProject": 0},
            "steps": [[W(3000)], [(0, 0.3)], [(0, 0.8)], [(0, 0.0)]],
            "sound_from_step": 0,
            "offline": True,
        },
    },
}


# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def bustest_bin():
    if not os.path.isfile(BUSTEST_BIN):
        pytest.skip(f"ambix_bustest not built at {BUSTEST_BIN}")
    return BUSTEST_BIN


def step_arg(step, plugin):
    parts = []
    for key, value in step:
        if key == "wait":
            parts.append(f"wait={value}")
        else:
            pid = int(key) if isinstance(key, RawID) else vst3_id(key, plugin)
            parts.append(f"{pid}={value!r}")
    return ",".join(parts)


def run_scenario(binary, plugin, scenario):
    path = vst3(plugin)
    if not os.path.exists(path):
        pytest.skip(f"{plugin}.vst3 not built")

    cmd = [binary, "--plugin", path]
    if scenario.get("offline"):
        cmd += ["--offline"]
    if scenario.get("late_state"):
        cmd += ["--late-state"]
    if "arr" in scenario:
        n_in, n_out = scenario["arr"]
        cmd += ["--in-arr", hex(mask(n_in)), "--out-arr", hex(mask(n_out))]

    with tempfile.TemporaryDirectory() as tmp:
        if "state" in scenario:
            state = os.path.join(tmp, "state.bin")
            with open(state, "wb") as f:
                f.write(xml_state(scenario["state"]))
            cmd += ["--state-in", state]
        for step in scenario["steps"]:
            cmd += ["--step", step_arg(step, plugin)]

        run = subprocess.run(cmd, capture_output=True, text=True, timeout=600)

    assert run.returncode == 0, f"{plugin}: ambix_bustest failed: {run.stderr.strip()[-2000:]}"
    lines = run.stdout.splitlines()
    starts = [i for i, line in enumerate(lines) if line.strip() == "{"]
    report = json.loads("\n".join(lines[starts[-1]:]))
    assert report["steps"] is not None, f"{plugin}: could not set up the instance"
    return {"channels": report["channels"], "steps": report["steps"]}


CASES = [(plugin, name) for plugin, scenarios in SCENARIOS.items() for name in scenarios]


@pytest.fixture(params=CASES, ids=[f"{p}-{n}" for p, n in CASES])
def scenario(request, bustest_bin):
    plugin, name = request.param
    spec = SCENARIOS[plugin][name]
    current = run_scenario(bustest_bin, plugin, spec)

    if "sound_from_step" in spec:
        # the configuration loads in the background; don't pin silence
        first = spec["sound_from_step"]
        assert any(r > 1e-4 for r in current["steps"][first]["rms"]), (
            f"{plugin}/{name}: still silent at step {first} — configuration not loaded?")

    path = os.path.join(SCENARIO_GOLDEN_DIR, f"{plugin}.json")
    goldens = {}
    if os.path.exists(path):
        with open(path) as f:
            goldens = json.load(f)

    if request.config.getoption("--update-golden") or name not in goldens:
        goldens[name] = current
        os.makedirs(SCENARIO_GOLDEN_DIR, exist_ok=True)
        with open(path, "w") as f:
            json.dump(goldens, f, indent=0, sort_keys=True)
            f.write("\n")
        pytest.skip(f"{plugin}/{name}: golden written — re-run to compare")

    return plugin, name, current, goldens[name]


def close(a, b):
    return len(a) == len(b) and all(
        abs(x - y) <= AUDIO_ATOL + AUDIO_RTOL * abs(y) for x, y in zip(a, b))


def test_scenario(scenario):
    plugin, name, current, golden = scenario
    where = f"{plugin}/{name}"

    assert current["channels"] == golden["channels"], f"{where}: bus widths"
    assert len(current["steps"]) == len(golden["steps"])

    for i, (got, want) in enumerate(zip(current["steps"], golden["steps"])):
        at = f"{where} step {i}"
        assert got["ok"] == want["ok"], f"{at}: process() result"
        assert got["latency"] == want["latency"], f"{at}: latency {got['latency']}, was {want['latency']}"

        assert got["values"].keys() == want["values"].keys(), f"{at}: parameter set"
        for pid, (value, text) in want["values"].items():
            gv, gt = got["values"][pid]
            assert gv == pytest.approx(value, abs=VALUE_TOL), f"{at}: parameter {pid} is {gv}, was {value}"
            assert gt == text, f"{at}: parameter {pid} shows {gt!r}, was {text!r}"

        assert close(got["rms"], want["rms"]) and close(got["proj"], want["proj"]), (
            f"{at}: output differs")


# ---------------------------------------------------------------------------
# A project restores the same, whenever the host restores it
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("plugin", ["ambix_binaural", "ambix_decoder"])
def test_restore_before_or_after_activation(plugin, bustest_bin):
    """Hosts restore a project before or after activating the plugin. Both
    must end up with the configuration loaded at the running sample rate."""
    spec = SCENARIOS[plugin]["gain_with_config"]
    before = run_scenario(bustest_bin, plugin, spec)
    after = run_scenario(bustest_bin, plugin, {**spec, "late_state": True})

    for i, (a, b) in enumerate(zip(before["steps"], after["steps"])):
        assert close(a["rms"], b["rms"]) and close(a["proj"], b["proj"]), (
            f"{plugin} step {i}: restoring before activation sounds different "
            f"from restoring after ({a['rms']} vs {b['rms']})")
