"""
ambix_mirror — per-channel gain + invert keyed by (l, m) symmetry buckets.

Each ACN channel's gain is the product of:
    - z_even (acn==0 or l+m even)     applies to W, Y, X      (NOT Z)
    - z_odd  (acn>0 and l+m odd)      applies to Z            (only)
    - y_even (m >= 0)                 applies to W, Z, X
    - y_odd  (m <  0)                 applies to Y            (only)
    - x_even ((m<0 odd) or (m>=0 even))   applies to W, Y, Z  (NOT X)
    - x_odd  ((m<0 even) or (m>=0 odd))   applies to X        (only)
    - circular (|m| == l)             applies to W, Y, X      (NOT Z)
plus the corresponding invert flag for each enabled bucket.

ParamToRMS(0.75) == 1.0, so 0.75 is the unity setting (constructor default).
The gain is ramped over one block via applyGainRamp, so we pre-roll silence
to let the ramp settle before measuring.

Buckets cross-referenced against ambix_mirror/Source/PluginProcessor.cpp::
calcParams (Source/PluginProcessor.cpp around line 436).
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, BLOCK, N_AMBI_CH


UNITY_GAIN  = 0.75   # ParamToRMS(0.75) == 1.0
HALF_GAIN   = 0.5    # ParamToRMS(0.5)  == (0.5/0.75)^2 ≈ 0.4444
INVERT_OFF  = 0.0
INVERT_ON   = 1.0

# 1st-order ACN: W=0, Y=1, Z=2, X=3
W, Y, Z, X = 0, 1, 2, 3

SETTLE_SAMPLES = 16384


def run_settled(plugin, audio: np.ndarray) -> np.ndarray:
    silence = np.zeros((audio.shape[0], SETTLE_SAMPLES), dtype=np.float32)
    full    = np.concatenate([silence, audio], axis=1)
    out     = plugin(full, SR)
    return out[:, SETTLE_SAMPLES:]


def set_defaults(plugin) -> None:
    """All buckets unity gain, all inverts off → transparent pass-through."""
    for axis in ("X", "Y", "Z"):
        for parity in ("even", "odd"):
            plugin[f"{axis} {parity} gain"]   = UNITY_GAIN
            plugin[f"{axis} {parity} invert"] = INVERT_OFF
    plugin["Circular gain"]   = UNITY_GAIN
    plugin["Circular invert"] = INVERT_OFF


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(1)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


def _assert_signs(out, ref, expected_signs, tol=1e-4):
    """expected_signs is a (4,) array of +1/-1 per channel."""
    for ch, sign in enumerate(expected_signs):
        np.testing.assert_allclose(
            out[ch], sign * ref[ch], atol=tol,
            err_msg=f"ACN ch{ch}: expected sign {sign:+d}, got mismatch"
        )


# ---------------------------------------------------------------------------
# Default pass-through (sanity)
# ---------------------------------------------------------------------------

def test_no_change_is_transparent(plugin_mirror, noise_ambi):
    set_defaults(plugin_mirror)
    out = run_settled(plugin_mirror, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-4,
        err_msg="Default config should be a transparent pass-through")


# ---------------------------------------------------------------------------
# Invert flags — one bucket at a time
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("bucket, expected_signs", [
    # (W, Y, Z, X)
    ("Y odd invert",    (+1, -1, +1, +1)),  # Y only
    ("Y even invert",   (-1, +1, -1, -1)),  # W, Z, X
    ("X odd invert",    (+1, +1, +1, -1)),  # X only
    ("X even invert",   (-1, -1, -1, +1)),  # W, Y, Z
    ("Z odd invert",    (+1, +1, -1, +1)),  # Z only
    ("Z even invert",   (-1, -1, +1, -1)),  # W, Y, X
    ("Circular invert", (-1, -1, +1, -1)),  # W, Y, X  (Z is not on the |m|==l ring)
])
def test_single_invert(plugin_mirror, noise_ambi, bucket, expected_signs):
    """Each invert flag flips exactly the channels in its (l,m) bucket."""
    set_defaults(plugin_mirror)
    plugin_mirror[bucket] = INVERT_ON
    out = run_settled(plugin_mirror, noise_ambi)
    _assert_signs(out, noise_ambi, expected_signs)


# ---------------------------------------------------------------------------
# Gain attenuation — non-invert, non-unity setting
# ---------------------------------------------------------------------------

def test_z_odd_gain_only_attenuates_z(plugin_mirror, noise_ambi):
    """
    Z-odd is the only bucket that touches Z alone. ParamToRMS(0.5) ≈ 0.444,
    so output[Z] should be ~0.444 * input[Z]; other channels unchanged.
    """
    set_defaults(plugin_mirror)
    plugin_mirror["Z odd gain"] = HALF_GAIN
    out = run_settled(plugin_mirror, noise_ambi)

    expected_gain = (0.5 / 0.75) ** 2  # ParamToRMS
    np.testing.assert_allclose(out[Z], expected_gain * noise_ambi[Z], atol=2e-4)
    for ch in (W, Y, X):
        np.testing.assert_allclose(out[ch], noise_ambi[ch], atol=1e-4,
            err_msg=f"ACN ch{ch} should be untouched by Z-odd gain")


def test_circular_gain_attenuates_ring_only(plugin_mirror, noise_ambi):
    """Circular bucket = |m|==l → W, Y, X. Z must be unaffected."""
    set_defaults(plugin_mirror)
    plugin_mirror["Circular gain"] = HALF_GAIN
    out = run_settled(plugin_mirror, noise_ambi)

    expected_gain = (0.5 / 0.75) ** 2
    np.testing.assert_allclose(out[Z], noise_ambi[Z], atol=1e-4,
        err_msg="Z (l=1,m=0) is not on the |m|==l ring — must be unchanged")
    for ch in (W, Y, X):
        np.testing.assert_allclose(out[ch], expected_gain * noise_ambi[ch],
            atol=2e-4, err_msg=f"Ring channel ACN ch{ch} should be attenuated")


# ---------------------------------------------------------------------------
# Combined: two non-overlapping inverts compose linearly
# ---------------------------------------------------------------------------

def test_y_odd_and_x_odd_invert_compose(plugin_mirror, noise_ambi):
    """Y-odd flips Y, X-odd flips X; together they flip both, nothing else."""
    set_defaults(plugin_mirror)
    plugin_mirror["Y odd invert"] = INVERT_ON
    plugin_mirror["X odd invert"] = INVERT_ON
    out = run_settled(plugin_mirror, noise_ambi)
    _assert_signs(out, noise_ambi, (+1, -1, +1, -1))
