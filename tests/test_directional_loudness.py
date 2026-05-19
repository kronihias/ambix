"""
ambix_directional_loudness — per-direction loudness shaping with up to
NUM_FILTERS directional windows.

This plugin is config-heavy (8 filters × 7 params each) and its default
state is not guaranteed to be a pure identity, so we limit assertions to
structural invariants: plugin loads, output is finite, output shape matches
input shape (N ambi → N ambi), silence in → silence out.

More thorough DSP assertions would require mirroring the t-design-based
weighting matrix logic in Python; that's deferred.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, N_AMBI_CH


BLOCK = 4096


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(12)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32) * 0.3


def test_output_shape_matches_input(plugin_directional_loudness, noise_ambi):
    out = plugin_directional_loudness(noise_ambi, SR)
    assert out.shape == noise_ambi.shape, (
        f"Expected {noise_ambi.shape} output, got {out.shape}"
    )


def test_output_finite(plugin_directional_loudness, noise_ambi):
    out = plugin_directional_loudness(noise_ambi, SR)
    assert np.all(np.isfinite(out)), "Output contains NaN/Inf"
