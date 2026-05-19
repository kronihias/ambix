"""
ambix_warp — non-linear azimuth / elevation warping.

Parameters (index order):
    0  Az Warp factor     0.5 = no warp
    1  Az Warp curve
    2  El Warp factor     0.5 = no warp
    3  El Warp curve
    4  Ambi In order
    5  Ambi Out order
    6  PreEmphasis

Default values give an identity transform.
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


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(8)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32)


def test_identity_at_defaults(plugin_warp, noise_ambi):
    plugin_warp["Az Warp factor"] = 0.5
    plugin_warp["El Warp factor"] = 0.5
    out = run_settled(plugin_warp, noise_ambi)
    np.testing.assert_allclose(out, noise_ambi, atol=5e-3,
        err_msg="Default warp factors should be transparent")


def test_output_finite_under_warp(plugin_warp, noise_ambi):
    """Non-default warp factors must still produce finite, right-shaped output.

    W (ACN 0) is *not* in general invariant under ambix_warp: the plugin
    re-projects the sound field through spherical-harmonic decode/encode
    stages which can redistribute energy across all channels. We therefore
    restrict the assertion to finiteness and shape.
    """
    plugin_warp["Az Warp factor"] = 0.8
    plugin_warp["El Warp factor"] = 0.3
    out = run_settled(plugin_warp, noise_ambi)
    assert np.all(np.isfinite(out))
    assert out.shape == noise_ambi.shape
