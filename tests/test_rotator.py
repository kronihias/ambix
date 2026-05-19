"""
ambix_rotator — full 3D sound-field rotation via Euler / quaternion.

Parameters (index order):
    0  Yaw                   0..1 -> -180..+180°, 0.5 = 0°
    1  Pitch                 0..1 -> -180..+180°, 0.5 = 0°
    2  Roll                  0..1 -> -180..+180°, 0.5 = 0°
    3  Rotation order        0 = ypr, 1 = rpy
    4-7 Quaternion q0..q3    0..1 -> -1..+1, 0.5 = 0
    8  Inverse Quaternion    0/1

Default is the identity rotation.

At yaw = +90° around Z the 1st-order invariants are the same as in
test_rotator_z.py: Y' = -X, X' = Y, W and Z untouched.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, BLOCK, N_AMBI_CH, save_golden, load_golden, golden_exists


SETTLE_SAMPLES = 16384


def run_settled(plugin, audio: np.ndarray) -> np.ndarray:
    silence = np.zeros((audio.shape[0], SETTLE_SAMPLES), dtype=np.float32)
    full    = np.concatenate([silence, audio], axis=1)
    out     = plugin(full, SR)
    return out[:, SETTLE_SAMPLES:]


def set_defaults(plugin):
    plugin["Yaw"]   = 0.5
    plugin["Pitch"] = 0.5
    plugin["Roll"]  = 0.5


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(6)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


def test_identity_at_all_zero(plugin_rotator, noise_ambi):
    """All Euler angles 0.5 (= 0°) → identity."""
    set_defaults(plugin_rotator)
    out = run_settled(plugin_rotator, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-4,
        err_msg="0/0/0 rotation should be transparent")


def test_energy_preserved_under_rotation(plugin_rotator, noise_ambi):
    """Rotation matrices are orthogonal → total channel energy is preserved."""
    set_defaults(plugin_rotator)
    plugin_rotator["Yaw"]   = 0.6   # ~+36°
    plugin_rotator["Pitch"] = 0.4   # ~-36°

    out = run_settled(plugin_rotator, noise_ambi)

    in_energy  = float(np.sum(noise_ambi ** 2))
    out_energy = float(np.sum(out ** 2))
    rel = abs(out_energy - in_energy) / in_energy
    assert rel < 1e-2, (
        f"Rotation is not energy-preserving (rel error {rel:.4f})"
    )


def test_yaw_90_swaps_x_y(plugin_rotator, noise_ambi):
    """
    Yaw = 0.75 → +90° rotation around Z:
        Y' = Y·cos(90°) - X·sin(90°) = -X
        X' = Y·sin(90°) + X·cos(90°) =  Y
        W and Z unchanged.
    """
    set_defaults(plugin_rotator)
    plugin_rotator["Yaw"] = 0.75

    out = run_settled(plugin_rotator, noise_ambi)

    np.testing.assert_allclose(out[0],  noise_ambi[0], atol=1e-3)
    np.testing.assert_allclose(out[2],  noise_ambi[2], atol=1e-3)
    np.testing.assert_allclose(out[1], -noise_ambi[3], atol=1e-3)
    np.testing.assert_allclose(out[3],  noise_ambi[1], atol=1e-3)


# ===========================================================================
# Golden regression: combined yaw + pitch + roll
# ===========================================================================

GOLDEN_TAG = "rotator_yaw60_pitch40_roll55_noise"


def test_golden_regression(plugin_rotator, noise_ambi, request):
    """Snapshot a non-trivial 3-axis rotation. Catches subtle DSP drift
    (e.g., quaternion sign flips) that the algebraic invariants miss."""
    set_defaults(plugin_rotator)
    plugin_rotator["Yaw"]   = 0.6   # ~+36°
    plugin_rotator["Pitch"] = 0.4   # ~-36°
    plugin_rotator["Roll"]  = 0.55  # ~+18°

    out = run_settled(plugin_rotator, noise_ambi)

    if request.config.getoption("--update-golden", default=False):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden updated")
    elif not golden_exists(GOLDEN_TAG):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden created — re-run to compare")
    else:
        golden = load_golden(GOLDEN_TAG)
        np.testing.assert_allclose(out, golden, atol=1e-5)
