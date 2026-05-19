"""
ambix_decoder — N-channel ambisonic → M-loudspeaker matrix decoder.

Hosted via `ambix_testhost` so the 4-in / N_OUT-out main bus is addressable
(pedalboard refuses asymmetric layouts).

The decode matrix is loaded from a runtime configuration file describing
loudspeaker positions. Without a preset there is no useful decode matrix,
so assertions here are limited to: silence-in-silence-out, output shape,
and finiteness for a non-silent input.

`N_OUT` matches the plugin's build-time `NUM_OUTPUTS_DECODER` CMake option
(default 64). Adjust here if the plugin is rebuilt with a different value.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, N_AMBI_CH, run_testhost


BLOCK = 4096
N_OUT = 64   # matches NUM_OUTPUTS_DECODER default in root CMakeLists.txt


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(15)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32) * 0.3


def test_silence_in_silence_out():
    silence = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
    out = run_testhost("ambix_decoder", silence, N_AMBI_CH, N_OUT)
    assert out.shape == (N_OUT, BLOCK)
    assert np.all(np.isfinite(out))
    assert np.max(np.abs(out)) < 1e-5


def test_output_finite(noise_ambi):
    out = run_testhost("ambix_decoder", noise_ambi, N_AMBI_CH, N_OUT)
    assert out.shape == (N_OUT, BLOCK)
    assert np.all(np.isfinite(out))
