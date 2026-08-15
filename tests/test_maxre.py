"""
ambix_maxre — max-rE weighting gains per ambisonic order band.

Parameters:
    0  apply   <0.33 = inverse weighting, 0.33..0.66 = bypass, >0.66 = max-rE
    1  order   0 .. AMBI_ORDER

At apply = 0.5 (bypass) the plugin is unity.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, BLOCK, N_AMBI_CH, save_golden, load_golden, golden_exists


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(10)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


def test_bypass_is_transparent(plugin_maxre, noise_ambi):
    plugin_maxre["apply"] = 0.5          # bypass bucket (0.33..0.66)
    plugin_maxre["order"] = 1.0
    out = plugin_maxre(noise_ambi, SR)
    np.testing.assert_allclose(out, noise_ambi, atol=1e-4,
        err_msg="apply=0.5 should bypass weighting")


def test_w_channel_invariant(plugin_maxre, noise_ambi):
    """W (l=0) max-rE weight is 1 regardless of order — always unity."""
    plugin_maxre["apply"] = 1.0          # max-rE bucket
    plugin_maxre["order"] = 1.0
    out = plugin_maxre(noise_ambi, SR)
    np.testing.assert_allclose(out[0], noise_ambi[0], atol=1e-4,
        err_msg="W (l=0) must be unchanged by max-rE weighting")


def test_max_re_attenuates_higher_orders(plugin_maxre, noise_ambi):
    """
    At max-rE the l=1 channels are scaled by cos(137.9°/(N+1)) where
    N = order. For 1st-order that is cos(137.9°/2) ≈ 0.577 < 1.
    Check the l=1 channels are attenuated (amplitude < input amplitude).
    """
    plugin_maxre["apply"] = 1.0
    plugin_maxre["order"] = 1.0

    out = plugin_maxre(noise_ambi, SR)

    for ch in (1, 2, 3):   # Y, Z, X (l=1)
        in_rms  = np.sqrt(np.mean(noise_ambi[ch] ** 2))
        out_rms = np.sqrt(np.mean(out[ch] ** 2))
        assert out_rms < in_rms, (
            f"ACN ch{ch}: expected attenuation under max-rE, "
            f"got in={in_rms:.3f} out={out_rms:.3f}"
        )


def test_order_zero(plugin_maxre, noise_ambi):
    """order = 0 keeps W and mutes every l >= 1 band.

    Regression guard: order 0 asks LegendreU for a 1-element P_n, and the
    unconditional `Pn(1) = arg` used to write one double past it. That
    corrupted the heap and crashed hosts on Windows (pluginval's Automation
    test, which sweeps `order` down to its minimum, died with
    STATUS_HEAP_CORRUPTION); elsewhere the stray write landed in malloc
    slack and went unnoticed.
    """
    plugin_maxre["apply"] = 1.0          # max-rE bucket
    plugin_maxre["order"] = 0.0

    out = plugin_maxre(noise_ambi, SR)

    assert np.all(np.isfinite(out)), "order=0 produced non-finite output"
    np.testing.assert_allclose(out[0], noise_ambi[0], atol=1e-4,
        err_msg="W (l=0) must pass through unweighted at order 0")
    for ch in range(1, N_AMBI_CH):
        np.testing.assert_allclose(out[ch], 0.0, atol=1e-6,
            err_msg=f"ACN ch{ch} (l>=1) must be muted at order 0")


# ===========================================================================
# Golden regression: max-rE at 1st order
# ===========================================================================

GOLDEN_TAG = "maxre_order1_apply1_noise"


def test_golden_regression(plugin_maxre, noise_ambi, request):
    """Snapshot the per-channel max-rE weights. The l=1 gain at 1st order
    is cos(137.9°/2) ≈ 0.5774 — pin it so any silent table edit fails."""
    plugin_maxre["apply"] = 1.0
    plugin_maxre["order"] = 1.0

    out = plugin_maxre(noise_ambi, SR)

    if request.config.getoption("--update-golden", default=False):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden updated")
    elif not golden_exists(GOLDEN_TAG):
        save_golden(GOLDEN_TAG, out)
        pytest.skip("Golden created — re-run to compare")
    else:
        golden = load_golden(GOLDEN_TAG)
        np.testing.assert_allclose(out, golden, atol=1e-5)
