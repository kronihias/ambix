"""
ambix_converter unit tests.

The converter reorders channels between ACN / FuMa / SID sequences,
applies SN3D / FuMa / N3D normalisation conversion gains, and optionally
mirrors the sound field (L/R, F/B, T/B).

There is no parameter smoothing — the DSP is a straight per-channel copy
with a gain multiplier, so output values are deterministic within float32
precision.

Parameter names (getParameterName):
    "InChannelSeq"          ACN (<0.33) / FuMa (<0.66) / SID (>=0.66)
    "OutChannelSeq"         same
    "InNormalization"       SN3D / FuMa / N3D  (same buckets)
    "OutNormalization"      SN3D / FuMa / N3D
    "MirrorLeftRight"       bool (>0.5)    — FlipParam
    "MirrorFrontBack"       bool (>0.5)    — FlopParam
    "MirrorTopBottom"       bool (>0.5)    — FlapParam
    "FlipCondonShortleyPhase"  bool
    "InputIs2D"             bool
    "OutputIs2D"            bool

Tests run at 1st order (4 channels: W, Y, Z, X in ACN).  The plugin's
VST3 bus layout negotiates to the channel count provided by the host;
pedalboard passes our (4, N) numpy arrays through directly.
"""

import sys
import os
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from reference.converter_ref import (
    convert, scheme_param, CONV_NORM, SQRT2, SQRT3,
)
from conftest import SR, BLOCK, N_AMBI_CH, save_golden, load_golden, golden_exists


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def reset(plugin) -> None:
    """All-off: ACN/SN3D in, ACN/SN3D out, no mirrors."""
    plugin["InChannelSeq"]    = 0.0   # ACN
    plugin["OutChannelSeq"]   = 0.0   # ACN
    plugin["InNormalization"] = 0.0   # SN3D
    plugin["OutNormalization"] = 0.0  # SN3D
    plugin["MirrorLeftRight"] = 0.0
    plugin["MirrorFrontBack"] = 0.0
    plugin["MirrorTopBottom"] = 0.0
    plugin["FlipCondonShortleyPhase"] = 0.0
    plugin["InputIs2D"]  = 0.0
    plugin["OutputIs2D"] = 0.0


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(scope="module")
def noise_ambi():
    """(4, BLOCK) distinct white noise per channel, reproducible."""
    rng = np.random.default_rng(0)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


@pytest.fixture(scope="module")
def impulse_ambi():
    """(4, BLOCK): impulse at sample 0 on every channel."""
    sig = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
    sig[:, 0] = 1.0
    return sig


# ===========================================================================
# Identity passthrough
# ===========================================================================

def test_identity_passthrough(plugin_converter, noise_ambi):
    """ACN/SN3D in → ACN/SN3D out, no mirrors → output == input."""
    reset(plugin_converter)
    out = plugin_converter(noise_ambi, SR)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-6,
        err_msg="Identity configuration should be bit-exact")


# ===========================================================================
# Channel sequence reorder (ACN <-> FuMa)
# ===========================================================================

def test_acn_to_fuma_reorder(plugin_converter, noise_ambi):
    """
    Input ACN/SN3D, output FuMa/SN3D — channels are permuted but values
    untouched. Reference: ref[fuma_ch_map[i]] = in[i].
    """
    reset(plugin_converter)
    plugin_converter["OutChannelSeq"] = scheme_param("FuMa")

    out = plugin_converter(noise_ambi, SR)
    ref = convert(noise_ambi, "ACN", "FuMa", "SN3D", "SN3D")

    np.testing.assert_allclose(out, ref, atol=1e-6,
        err_msg="ACN → FuMa reorder mismatch")


def test_fuma_to_acn_reorder(plugin_converter, noise_ambi):
    """Round-trip: FuMa in, ACN out recovers original order if source was ACN."""
    reset(plugin_converter)
    plugin_converter["InChannelSeq"]  = scheme_param("FuMa")
    plugin_converter["OutChannelSeq"] = scheme_param("ACN")

    # Treat noise_ambi as if it were already in FuMa order
    out = plugin_converter(noise_ambi, SR)
    ref = convert(noise_ambi, "FuMa", "ACN", "SN3D", "SN3D")

    np.testing.assert_allclose(out, ref, atol=1e-6,
        err_msg="FuMa → ACN reorder mismatch")


# ===========================================================================
# Normalisation conversion
# ===========================================================================

@pytest.mark.parametrize("in_norm, out_norm, expected", [
    ("SN3D", "N3D",  [1.0, SQRT3, SQRT3, SQRT3]),
    ("N3D",  "SN3D", [1.0, 1 / SQRT3, 1 / SQRT3, 1 / SQRT3]),
    ("SN3D", "FuMa", [1 / SQRT2, 1.0, 1.0, 1.0]),
    ("FuMa", "SN3D", [SQRT2, 1.0, 1.0, 1.0]),
])
def test_norm_conversion_gains(plugin_converter, noise_ambi,
                                in_norm, out_norm, expected):
    """Per-channel gain after normalisation conversion matches the scalar table."""
    reset(plugin_converter)
    plugin_converter["InNormalization"]  = scheme_param(in_norm)
    plugin_converter["OutNormalization"] = scheme_param(out_norm)

    out = plugin_converter(noise_ambi, SR)

    for ch, g in enumerate(expected):
        np.testing.assert_allclose(
            out[ch], noise_ambi[ch] * np.float32(g), atol=1e-5,
            err_msg=f"{in_norm}->{out_norm}: ch{ch} expected gain {g:.4f}"
        )


# ===========================================================================
# Mirror (left/right, front/back, top/bottom)
# ===========================================================================

def test_mirror_left_right(plugin_converter, noise_ambi):
    """L/R mirror inverts ACN ch 1 (Y, m=-1); other channels unchanged."""
    reset(plugin_converter)
    plugin_converter["MirrorLeftRight"] = 1.0

    out = plugin_converter(noise_ambi, SR)
    ref = convert(noise_ambi, "ACN", "ACN", "SN3D", "SN3D", flip=True)

    np.testing.assert_allclose(out, ref, atol=1e-6)
    # Explicit per-channel assertion for clarity
    np.testing.assert_allclose(out[0],  noise_ambi[0], atol=1e-6)   # W unchanged
    np.testing.assert_allclose(out[1], -noise_ambi[1], atol=1e-6)   # Y inverted
    np.testing.assert_allclose(out[2],  noise_ambi[2], atol=1e-6)   # Z unchanged
    np.testing.assert_allclose(out[3],  noise_ambi[3], atol=1e-6)   # X unchanged


def test_mirror_front_back(plugin_converter, noise_ambi):
    """F/B mirror inverts ACN ch 3 (X, m=+1 odd)."""
    reset(plugin_converter)
    plugin_converter["MirrorFrontBack"] = 1.0

    out = plugin_converter(noise_ambi, SR)
    np.testing.assert_allclose(out[0],  noise_ambi[0], atol=1e-6)
    np.testing.assert_allclose(out[1],  noise_ambi[1], atol=1e-6)
    np.testing.assert_allclose(out[2],  noise_ambi[2], atol=1e-6)
    np.testing.assert_allclose(out[3], -noise_ambi[3], atol=1e-6)


def test_mirror_top_bottom(plugin_converter, noise_ambi):
    """T/B mirror inverts ACN ch 2 (Z, l+m=1 odd)."""
    reset(plugin_converter)
    plugin_converter["MirrorTopBottom"] = 1.0

    out = plugin_converter(noise_ambi, SR)
    np.testing.assert_allclose(out[0],  noise_ambi[0], atol=1e-6)
    np.testing.assert_allclose(out[1],  noise_ambi[1], atol=1e-6)
    np.testing.assert_allclose(out[2], -noise_ambi[2], atol=1e-6)
    np.testing.assert_allclose(out[3],  noise_ambi[3], atol=1e-6)


def test_all_three_mirrors(plugin_converter, noise_ambi):
    """flip ∘ flop ∘ flap: ch0 unchanged, ch1/2/3 all inverted."""
    reset(plugin_converter)
    plugin_converter["MirrorLeftRight"] = 1.0
    plugin_converter["MirrorFrontBack"] = 1.0
    plugin_converter["MirrorTopBottom"] = 1.0

    out = plugin_converter(noise_ambi, SR)
    ref = convert(noise_ambi, "ACN", "ACN", "SN3D", "SN3D",
                  flip=True, flop=True, flap=True)
    np.testing.assert_allclose(out, ref, atol=1e-6)


# ===========================================================================
# Silence in → silence out
# ===========================================================================

def test_silence_in_silence_out(plugin_converter):
    reset(plugin_converter)
    silence = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
    out = plugin_converter(silence, SR)
    np.testing.assert_allclose(out, 0.0, atol=1e-8)


# ===========================================================================
# Combined: ACN/SN3D → FuMa/N3D with L/R mirror
# ===========================================================================

def test_combined_transform(plugin_converter, noise_ambi):
    """Exercise seq + norm + mirror simultaneously."""
    reset(plugin_converter)
    plugin_converter["OutChannelSeq"]    = scheme_param("FuMa")
    plugin_converter["OutNormalization"] = scheme_param("N3D")
    plugin_converter["MirrorLeftRight"]  = 1.0

    out = plugin_converter(noise_ambi, SR)
    ref = convert(noise_ambi, "ACN", "FuMa", "SN3D", "N3D", flip=True)

    np.testing.assert_allclose(out, ref, atol=1e-5)


# ===========================================================================
# Golden regression
# ===========================================================================

GOLDEN_TAG = "converter_fuma_n3d_flip_noise"


def test_golden_regression(plugin_converter, noise_ambi, request):
    reset(plugin_converter)
    plugin_converter["OutChannelSeq"]    = scheme_param("FuMa")
    plugin_converter["OutNormalization"] = scheme_param("N3D")
    plugin_converter["MirrorLeftRight"]  = 1.0

    out = plugin_converter(noise_ambi, SR)

    if request.config.getoption("--update-golden", default=False):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden updated")
    elif not golden_exists(GOLDEN_TAG):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden created — re-run to compare")
    else:
        golden = load_golden(GOLDEN_TAG)
        np.testing.assert_allclose(out, golden, atol=1e-5)
