"""
ambix_move — translation + rotation of the sound-field origin.

Parameters (index order):
    0  X                  0..1 -> -5..+5 m, 0.5 = origin
    1  Y                  0..1 -> -5..+5 m, 0.5 = origin
    2  Z                  0..1 -> -5..+5 m, 0.5 = origin
    3  Reference radius   0..1 -> 0..N m (matters only when translating)
    4  Yaw / 5 Pitch / 6 Roll      0..1 -> -180..+180°, 0.5 = 0°
    7  Rotation order
    8-11 Quaternion q0..q3
    12  Inverse Quaternion Rotation
    13  Ambi In order
    14  Ambi Out order
    15  Translation enabled   1 = active, 0 = bypass (listener at origin)
    16  Rotation enabled      1 = active, 0 = bypass (identity rotation)

Defaults (constructor initial values) place the listener at the origin with
no rotation — the plugin is then a pure pass-through for ambisonic input at
the declared order.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, BLOCK, N_AMBI_CH


SETTLE_SAMPLES = 16384


def run_settled(plugin, audio: np.ndarray) -> np.ndarray:
    silence = np.zeros((audio.shape[0], SETTLE_SAMPLES), dtype=np.float32)
    full    = np.concatenate([silence, audio], axis=1)
    out     = plugin(full, SR)
    return out[:, SETTLE_SAMPLES:]


def set_defaults(plugin):
    plugin["X"]     = 0.5
    plugin["Y"]     = 0.5
    plugin["Z"]     = 0.5
    plugin["Yaw"]   = 0.5
    plugin["Pitch"] = 0.5
    plugin["Roll"]  = 0.5


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(7)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


def test_identity_at_origin_no_rotation(plugin_move, noise_ambi):
    set_defaults(plugin_move)
    out = run_settled(plugin_move, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-3,
        err_msg="Origin + 0° rotation should be transparent")


def test_rotation_only_is_energy_preserving(plugin_move, noise_ambi):
    """Pure rotation (translation = origin, non-zero Yaw) preserves energy.

    Rotation is an orthogonal transform — energy should be preserved to
    float32 precision. Measured error is ~1e-7; the bound is set well
    above that so float noise doesn't flake the test but a real DSP
    regression (e.g., a missing normalisation) will fail it.
    """
    set_defaults(plugin_move)
    plugin_move["Yaw"] = 0.7

    out = run_settled(plugin_move, noise_ambi)
    in_energy  = float(np.sum(noise_ambi ** 2))
    out_energy = float(np.sum(out ** 2))
    rel = abs(out_energy - in_energy) / in_energy
    assert rel < 1e-4, (
        f"Pure rotation should preserve energy (rel error {rel:.6f})"
    )


def test_translation_bypass_is_transparent(plugin_move, noise_ambi):
    """Bypassing translation while at a non-origin position should produce
    the same output as being at the origin."""
    set_defaults(plugin_move)
    plugin_move["X"] = 0.8          # off-origin
    plugin_move["Translation enabled"] = 0.0   # bypass

    out = run_settled(plugin_move, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-3,
        err_msg="Translation bypass should be transparent regardless of X/Y/Z values")


def test_rotation_bypass_is_transparent(plugin_move, noise_ambi):
    """Bypassing rotation while yaw is non-zero should produce pass-through."""
    set_defaults(plugin_move)
    plugin_move["Yaw"] = 0.7        # non-zero rotation
    plugin_move["Rotation enabled"] = 0.0      # bypass

    out = run_settled(plugin_move, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-3,
        err_msg="Rotation bypass should be transparent regardless of Yaw/Pitch/Roll values")
