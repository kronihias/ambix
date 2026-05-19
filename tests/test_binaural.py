"""
ambix_binaural — 1st-order ambisonic → stereo binaural renderer.

Hosted via `ambix_testhost` so the 4-in / 2-out main bus is addressable
(pedalboard refuses asymmetric layouts).

The plugin loads HRTF impulse responses from a preset file at startup; the
default preset directory is `~/Library/Application Support/ambix/binaural_presets/`
on macOS. Without an installed preset the plugin runs but produces silent
output for a non-silent input — assertions here are limited to the
structural invariants that hold either way: output shape, no NaN/Inf,
silence-in-silence-out.

Real impulse-response / directional-cue tests would need a committed HRTF
fixture in the test data, which is out of scope for this suite.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, N_AMBI_CH, run_testhost


BLOCK = 4096
N_OUT = 2


@pytest.fixture(scope="module")
def noise_ambi():
    rng = np.random.default_rng(14)
    return rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32) * 0.3


def test_output_shape_4in_2out(noise_ambi):
    """The plugin must accept (4, N) input and produce (2, N) output."""
    out = run_testhost("ambix_binaural", noise_ambi, N_AMBI_CH, N_OUT)
    assert out.shape == (N_OUT, BLOCK), f"Expected (2, {BLOCK}), got {out.shape}"


def test_output_finite(noise_ambi):
    out = run_testhost("ambix_binaural", noise_ambi, N_AMBI_CH, N_OUT)
    assert np.all(np.isfinite(out))


def test_silence_in_silence_out():
    silence = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
    out = run_testhost("ambix_binaural", silence, N_AMBI_CH, N_OUT)
    assert np.all(np.isfinite(out))
    assert np.max(np.abs(out)) < 1e-5, (
        f"Silent input produced non-silent output (max abs {np.max(np.abs(out)):.3e})"
    )
