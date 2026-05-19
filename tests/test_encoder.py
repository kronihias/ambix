"""
ambix_encoder — mono → 1st-order ambisonic point-source encoder.

Hosted via `ambix_testhost` because pedalboard cannot handle the 1-in /
4-out main bus. The testhost subprocess sets discrete bus widths explicitly.

Parameter mapping (verified against ambix_encoder/Source/PluginProcessor.cpp):
    Azimuth     0..1 -> -180°..+180°,  0.5 = front (0°)
                       0.75 = +90° (left),  0.25 = -90° (right)
    Elevation   0..1 -> -180°..+180°,  0.5 = horizon (0°)
                       0.75 = +90° (up),    0.25 = -90° (down)
    Size        0..1  — source width / spread (0 = point source)

At a point source on the unit sphere, 1st-order SN3D produces:
    W = s,    Y = sin(az)·cos(el)·s,
    Z = sin(el)·s,
    X = cos(az)·cos(el)·s
(channels are in ACN order: W=0, Y=1, Z=2, X=3.)

The plugin's parameter smoothing settles within the first few thousand
samples — assertions use the trailing portion of the output buffer.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, run_testhost


N_OUT = 4   # 1st-order ambisonic
TAIL_START = 8000   # discard the smoothing-ramp prefix


@pytest.fixture(scope="module")
def mono_noise():
    rng = np.random.default_rng(11)
    return (rng.standard_normal(16384).astype(np.float32) * 0.3)[None, :]


def _encode(mono, az_norm, el_norm):
    return run_testhost("ambix_encoder", mono, 1, N_OUT,
                        params={"Azimuth": az_norm,
                                "Elevation": el_norm,
                                "Size": 0.0})


def _channel_rms_ratio(out, mono):
    """RMS per output channel / RMS of mono input (post-settle window)."""
    tail   = out[:, TAIL_START:]
    in_rms = float(np.sqrt(np.mean(mono[0, TAIL_START:] ** 2)))
    return np.sqrt(np.mean(tail ** 2, axis=1)) / in_rms


# ---------------------------------------------------------------------------
# Directional encoding: per-channel gains at the cardinal points
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("label, az, el, expected", [
    #                            W    Y    Z    X
    ("front  (0°, 0°)",   0.5,  0.5,  (1.0, 0.0, 0.0, 1.0)),
    ("left   (+90°, 0°)", 0.75, 0.5,  (1.0, 1.0, 0.0, 0.0)),
    ("right  (-90°, 0°)", 0.25, 0.5,  (1.0, 1.0, 0.0, 0.0)),  # |Y| only, sign lost via RMS
    ("up     (0°, +90°)", 0.5,  0.75, (1.0, 0.0, 1.0, 0.0)),
    ("down   (0°, -90°)", 0.5,  0.25, (1.0, 0.0, 1.0, 0.0)),
])
def test_directional_gain_magnitude(mono_noise, label, az, el, expected):
    """At each cardinal direction the per-channel |gain| matches the
    spherical-harmonic expansion (W=1 always; |Y|=|sin(az)·cos(el)| etc.).
    RMS is sign-blind — see test_signed_pair for the L/R sign check."""
    out = _encode(mono_noise, az, el)
    rms = _channel_rms_ratio(out, mono_noise)
    np.testing.assert_allclose(rms, expected, atol=0.02,
        err_msg=f"{label}: got per-channel RMS {rms.tolist()}")


# ---------------------------------------------------------------------------
# Sign check: Y at +90° vs -90° must be opposite
# ---------------------------------------------------------------------------

def test_left_and_right_y_opposite_sign(mono_noise):
    """sin(+90°)=+1 and sin(-90°)=-1, so Y must flip sign between left
    and right. Cross-correlation captures the sign that RMS hides."""
    left  = _encode(mono_noise, 0.75, 0.5)
    right = _encode(mono_noise, 0.25, 0.5)
    # Inner product (post-settle) is positive when in phase, negative when inverted.
    corr = float(np.sum(left[1, TAIL_START:] * right[1, TAIL_START:]))
    norm = float(np.sum(left[1, TAIL_START:] ** 2))
    assert norm > 1e-3, "Y channel must carry energy at ±90°"
    assert corr < -0.5 * norm, (
        f"Y at +90° and -90° must be in anti-phase (corr/norm = {corr/norm:.3f})"
    )


# ---------------------------------------------------------------------------
# W is direction-invariant
# ---------------------------------------------------------------------------

def test_w_invariant_across_directions(mono_noise):
    """The omnidirectional W component must be the same regardless of
    the source direction (it's the SH coefficient for l=0)."""
    az_vals = [0.0, 0.25, 0.5, 0.75]
    el_vals = [0.25, 0.5, 0.75]
    ws = []
    for az in az_vals:
        for el in el_vals:
            out = _encode(mono_noise, az, el)
            ws.append(np.sqrt(np.mean(out[0, TAIL_START:] ** 2)))
    ws = np.array(ws)
    # Spread should be tiny (only float32 noise + parameter ramp residuals).
    rel = (ws.max() - ws.min()) / ws.mean()
    assert rel < 0.02, (
        f"W energy varies across directions by {rel*100:.2f}% — should be invariant"
    )


# ---------------------------------------------------------------------------
# Silence in → silence out
# ---------------------------------------------------------------------------

def test_silence_in_silence_out():
    silence = np.zeros((1, 4096), dtype=np.float32)
    out = run_testhost("ambix_encoder", silence, 1, N_OUT,
                       params={"Azimuth": 0.5, "Elevation": 0.5})
    assert np.all(np.isfinite(out))
    assert np.max(np.abs(out)) < 1e-5
