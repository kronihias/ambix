"""
ambix_vmic — virtual-microphone array. Extracts M directional mic signals
from an N-channel ambisonic input.

Like ambix_directional_loudness this is config-heavy (per-filter azimuth /
elevation / shape / width / height / gain / window). Without an external
configuration step we assert only structural invariants here.
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
    rng = np.random.default_rng(13)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32) * 0.3


def test_silence_in_silence_out(plugin_vmic):
    silence = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
    out = plugin_vmic(silence, SR)
    assert np.all(np.isfinite(out))
    assert np.max(np.abs(out)) < 1e-5


def test_output_finite(plugin_vmic, noise_ambi):
    out = plugin_vmic(noise_ambi, SR)
    assert np.all(np.isfinite(out))
    assert out.shape[1] == noise_ambi.shape[1]
