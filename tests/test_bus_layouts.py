"""VST3 bus-layout negotiation.

Every other test in this suite hosts the plugins through JUCE (pedalboard,
ambix_testhost) or through pluginval, which is itself JUCE-based. That hides a
whole class of bug: when a plugin cannot answer `getBusArrangement()`, JUCE's
host substitutes `discreteChannels(count)` and carries on, so a plugin whose
buses it cannot describe still looks perfectly healthy. Real hosts are less
forgiving — Max/MSP left ambix_encoder stuck on its 1st-order default rather
than negotiate with a bus it could not read, which is the bug these tests were
written for.

So these go through `ambix_bustest`, which calls IComponent / IAudioProcessor
directly. Three things are asserted, for every plugin:

  * its default layout is describable — a host that inspects a plugin before
    negotiating gets an answer rather than a failure,
  * it is still describable after negotiating each width the plugin supports,
    proposed the way a host that asks by channel count would,
  * negotiating does not permute channels, which is what the applyBusLayouts
    guard in common/ambix_buses.h exists to prevent.

A VST3 SpeakerArrangement is a bit per speaker. `(1 << n) - 1` is what a host
asking by channel count sends; note that kAmbi5th/6th/7thOrderACN are defined
as exactly those masks for 36/49/64, while orders 1-4 have their ACN speakers
up in bits 20+. That asymmetry is why orders 1-4 and 5-7 used to behave
differently, and why the ambisonic widths below are all worth testing.
"""

import json
import os
import subprocess
import sys

import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import ALL_PLUGINS, REPO_ROOT, vst3

_BUSTEST_EXE = "ambix_bustest.exe" if os.name == "nt" else "ambix_bustest"
BUSTEST_BIN = os.path.join(REPO_ROOT, "_build", "testhost", _BUSTEST_EXE)

# Ambisonic widths, orders 1-7.
AMBISONIC_WIDTHS = [4, 9, 16, 25, 36, 49, 64]

# Widths for the buses that carry plain channels rather than an ambisonic
# signal: encoder sources, decoder loudspeakers, vmic virtual mics. Every one
# of these has to be describable — a discrete layout is made reportable by
# JUCE_patches/juce_VST3Common.h.patch, on top of what common/ambix_buses.h
# does for the widths it covers on its own.
PLAIN_WIDTHS = [1, 2, 3, 8, 12, 19, 20, 24, 32]

# Layouts JUCE's reorder table would shuffle if applyBusLayouts let them
# through: 7.1 music (rear pair before side pair) and 7.1.4.
K71_MUSIC = 0x00C00060F
K71_4     = 0x00C02D60F

# Which bus of each plugin carries plain channels, and how wide the other one
# has to be. Everything not listed here is ambisonic on both sides.
PLAIN_BUS = {
    "ambix_encoder": ("in",  36),   # sources in, 5th-order out
    "ambix_decoder": ("out",  4),   # 1st-order in, loudspeakers out
    "ambix_vmic":    ("out",  4),   # 1st-order in, virtual mics out
}

# Fixed output widths that cannot be negotiated away.
FIXED_OUTPUT = {
    "ambix_binaural": 2,
}


def mask(n):
    """The arrangement a host sends when it asks for n channels by count."""
    return (1 << 64) - 1 if n >= 64 else (1 << n) - 1


@pytest.fixture(scope="session")
def bustest_bin():
    if not os.path.isfile(BUSTEST_BIN):
        pytest.skip(
            f"ambix_bustest not built at {BUSTEST_BIN} — "
            "cmake -B _build -DBUILD_VST3=ON -DBUILD_TESTHOST=ON && "
            "cmake --build _build --target ambix_bustest"
        )
    return BUSTEST_BIN


def probe(binary, plugin, in_arr=None, out_arr=None, check_order=False):
    path = vst3(plugin)
    if not os.path.exists(path):
        pytest.skip(f"{plugin}.vst3 not built")

    cmd = [binary, "--plugin", path]
    if in_arr is not None:
        cmd += ["--in-arr", hex(in_arr)]
    if out_arr is not None:
        cmd += ["--out-arr", hex(out_arr)]
    if check_order:
        cmd += ["--check-order"]

    run = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    assert run.returncode == 0, f"{plugin}: ambix_bustest failed: {run.stderr.strip()}"

    # Plugins chatter on stdout while loading (preset scans); the JSON object
    # is the last thing printed.
    brace = run.stdout.rindex("{\n")
    return json.loads(run.stdout[brace:])


def buses(result, stage):
    return [b for b in result["buses"] if b["stage"] == stage]


def describe(bus):
    return f"{bus['dir']} bus, {bus['channels']} ch"


def widths_for(plugin):
    """The (in_width, out_width) pairs worth negotiating for this plugin."""
    plain = PLAIN_BUS.get(plugin)
    fixed_out = FIXED_OUTPUT.get(plugin)

    if plain is None:
        return [(n, fixed_out if fixed_out else n) for n in AMBISONIC_WIDTHS]

    side, other = plain
    widths = sorted(set(PLAIN_WIDTHS + AMBISONIC_WIDTHS))

    if side == "in":
        return [(n, other) for n in widths]
    return [(other, n) for n in widths]


@pytest.mark.parametrize("plugin", ALL_PLUGINS)
def test_default_layout_is_describable(bustest_bin, plugin):
    """A host inspecting the plugin before negotiating must get an answer.

    ambix_encoder used to default its source bus to discreteChannels(64),
    which VST3 cannot express, so this query failed and Max abandoned the
    negotiation — leaving the encoder on its 4-channel default.
    """
    result = probe(bustest_bin, plugin)

    for bus in buses(result, "default"):
        assert bus["arrangement_named"], (
            f"{plugin}: default {describe(bus)} has no VST3 arrangement — "
            "a host cannot see what is on it"
        )
        assert bus["arrangement_channels"] == bus["channels"], (
            f"{plugin}: default {describe(bus)} reports an arrangement of "
            f"{bus['arrangement_channels']} channels"
        )


@pytest.mark.parametrize("plugin", ALL_PLUGINS)
def test_negotiated_layout_is_describable(bustest_bin, plugin):
    """...and still get one after negotiating, at every width the plugin takes.

    Orders 1-4 used to fail here: the guard rewrote the host's layout to
    discreteChannels(N), which stock JUCE cannot report back. Both halves of
    that are now covered — the guard keeps an ambisonic identity where it can,
    and JUCE_patches/juce_VST3Common.h.patch gives a discrete layout an
    arrangement for every width up to 64.
    """
    for in_width, out_width in widths_for(plugin):
        result = probe(bustest_bin, plugin, mask(in_width), mask(out_width))

        assert result["negotiated"], (
            f"{plugin}: refused {in_width} in / {out_width} out"
        )

        for bus in buses(result, "negotiated"):
            width = in_width if bus["dir"] == "in" else out_width

            assert bus["channels"] == width, (
                f"{plugin}: asked for {width} channels on the {bus['dir']} bus, "
                f"got {bus['channels']}"
            )

            assert bus["arrangement_named"], (
                f"{plugin}: after negotiating {in_width} in / {out_width} out, "
                f"the {bus['dir']} bus has no VST3 arrangement"
            )


@pytest.mark.parametrize("plugin", ["ambix_mirror", "ambix_rotator", "ambix_converter"])
@pytest.mark.parametrize("arrangement,name", [
    (mask(4),   "4-channel mask"),
    (mask(16),  "16-channel mask"),
    (mask(36),  "36-channel mask"),
    (K71_MUSIC, "k71Music"),
    (K71_4,     "k71_4"),
])
def test_negotiating_does_not_reorder_channels(bustest_bin, plugin, arrangement, name):
    """Channel k in must be channel k out, whatever layout the host proposed.

    JUCE's VST3 wrapper reorders channels to match a named layout's speaker
    positions — k71Music and k71_4 are two it would shuffle. applyBusLayouts
    dodges that by storing a layout with no reorder table entry; this is the
    test that notices if that protection goes away.

    The plugins here are the ones that pass audio through unchanged at their
    default settings, so any permutation is the wrapper's doing.
    """
    result = probe(bustest_bin, plugin, arrangement, arrangement, check_order=True)

    if not result["negotiated"]:
        pytest.skip(f"{plugin} does not accept {name}")

    assert result["reordered_channels"] == 0, (
        f"{plugin}: {result['reordered_channels']} channels come out in the "
        f"wrong place after negotiating {name}"
    )
