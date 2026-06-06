"""
ambix_vmic — virtual-microphone array. Extracts M directional mic signals
from an N-channel ambisonic input.

Like ambix_directional_loudness this is config-heavy (per-filter azimuth /
elevation / shape / width / height / gain / window). Without an external
configuration step we assert only structural invariants here.

The main bus is asymmetric (4-channel ambisonic in → NUM_FILTERS_VMIC discrete
virtual-mic outs), so it can't be hosted by pedalboard; we drive it through the
ambix_testhost CLI like the other asymmetric plugins.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, N_AMBI_CH, run_testhost


BLOCK = 4096
N_OUT = 8   # NUM_FILTERS_VMIC (default 8 in the root CMakeLists.txt)


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(13)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32) * 0.3


def test_output_shape_4in_8out(noise_ambi):
    """The plugin must accept (4, N) input and produce (8, N) output."""
    out = run_testhost("ambix_vmic", noise_ambi, N_AMBI_CH, N_OUT)
    assert out.shape == (N_OUT, BLOCK), f"Expected ({N_OUT}, {BLOCK}), got {out.shape}"


def test_output_finite(noise_ambi):
    out = run_testhost("ambix_vmic", noise_ambi, N_AMBI_CH, N_OUT)
    assert np.all(np.isfinite(out))
    assert out.shape[1] == noise_ambi.shape[1]


def test_silence_in_silence_out():
    silence = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
    out = run_testhost("ambix_vmic", silence, N_AMBI_CH, N_OUT)
    assert np.all(np.isfinite(out))
    assert np.max(np.abs(out)) < 1e-5
