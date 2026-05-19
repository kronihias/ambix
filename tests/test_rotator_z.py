"""
ambix_rotator_z — single-axis (Z) sound-field rotation.

Parameter: "Z-axis rot" (index 0), 0..1 -> -180°..+180°, so 0.5 = 0° (identity).

1st-order ACN invariants under rotation around Z by angle φ:
    W (ACN 0)  : unchanged
    Y (ACN 1)  : Y·cos(φ) - X·sin(φ)      (m = -1)
    Z (ACN 2)  : unchanged                  (m = 0)
    X (ACN 3)  : Y·sin(φ) + X·cos(φ)      (m = +1)

At φ = +90° this becomes:
    W' = W,  Y' = -X,  Z' = Z,  X' = Y.
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


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(5)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


def test_identity_at_zero_rotation(plugin_rotator_z, noise_ambi):
    """Z Rotation = 0.5 (0°) → output == input."""
    plugin_rotator_z["Z-axis rot"] = 0.5
    out = run_settled(plugin_rotator_z, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-4,
        err_msg="0° rotation should be transparent")


def test_w_and_z_invariant(plugin_rotator_z, noise_ambi):
    """W (ACN 0) and Z (ACN 2) are m=0 channels; unaffected by any Z rotation."""
    plugin_rotator_z["Z-axis rot"] = 0.25   # -90°
    out = run_settled(plugin_rotator_z, noise_ambi)
    np.testing.assert_allclose(out[0], noise_ambi[0], atol=1e-4)
    np.testing.assert_allclose(out[2], noise_ambi[2], atol=1e-4)


def test_90deg_rotation_swaps_x_y(plugin_rotator_z, noise_ambi):
    """
    Z Rotation = 0.75 -> +90°.
    Y' = Y·cos(90°) - X·sin(90°) = -X
    X' = Y·sin(90°) + X·cos(90°) =  Y
    """
    plugin_rotator_z["Z-axis rot"] = 0.75
    out = run_settled(plugin_rotator_z, noise_ambi)

    np.testing.assert_allclose(out[1], -noise_ambi[3], atol=1e-3,
        err_msg="Y' should equal -X at +90°")
    np.testing.assert_allclose(out[3],  noise_ambi[1], atol=1e-3,
        err_msg="X' should equal  Y at +90°")


# ===========================================================================
# Golden regression: non-trivial rotation angle
# ===========================================================================

GOLDEN_TAG = "rotator_z_55deg_noise"


def test_golden_regression(plugin_rotator_z, noise_ambi, request):
    """Snapshot a non-axis-aligned Z rotation (~+18°) so the cos/sin table
    is pinned. Algebraic invariants only cover 0° and ±90°."""
    plugin_rotator_z["Z-axis rot"] = 0.55

    out = run_settled(plugin_rotator_z, noise_ambi)

    if request.config.getoption("--update-golden", default=False):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden updated")
    elif not golden_exists(GOLDEN_TAG):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden created — re-run to compare")
    else:
        golden = load_golden(GOLDEN_TAG)
        np.testing.assert_allclose(out, golden, atol=1e-5)
