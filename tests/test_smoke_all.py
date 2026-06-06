"""
Smoke tests for every ambix VST3 plugin.

Minimal safety net that catches total plugin breakage: each plugin must
- load without error
- produce finite (no NaN / Inf) output for white-noise input
- produce silence for silent input (plugins declare silenceInProducesSilenceOut=true)

Square-bus plugins go through pedalboard (in-process, fast). Asymmetric-bus
plugins (encoder mono→ambi, binaural 4→2, decoder 4→N) go through the
ambix_testhost CLI subprocess — see conftest.run_testhost.

More thorough per-plugin DSP assertions live in test_<plugin>.py modules.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, N_AMBI_CH, ALL_PLUGINS, vst3, vst3_load_path, PluginWrapper, run_testhost


BLOCK = 4096   # short buffer — smoke tests don't need a full second of audio


# Plugins whose main bus is asymmetric and therefore unhostable by
# pedalboard. Map: plugin_name -> (num_input_channels, num_output_channels).
# The output count for ambix_decoder tracks NUM_OUTPUTS_DECODER (default 64
# in the root CMakeLists.txt).
ASYMMETRIC_BUSES = {
    "ambix_encoder":  (1, 4),
    "ambix_binaural": (4, 2),
    "ambix_decoder":  (4, 64),
    # vmic outputs NUM_FILTERS_VMIC discrete virtual-mic channels (default 8
    # in the root CMakeLists.txt); input is 1st-order ambisonic (4).
    "ambix_vmic":     (4, 8),
}


def _load_or_skip_pedalboard(name: str):
    try:
        import pedalboard
    except ImportError:
        pytest.skip("pedalboard not installed")
    path = vst3(name)
    if not os.path.exists(path):
        pytest.skip(f"Plugin not built: {path}")
    return PluginWrapper(pedalboard.load_plugin(vst3_load_path(path)))


# ---------------------------------------------------------------------------
# test_plugin_loads
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("plugin_name", ALL_PLUGINS)
def test_plugin_loads(plugin_name):
    """Plugin binary exists and can be instantiated by the appropriate host."""
    if plugin_name in ASYMMETRIC_BUSES:
        # The testhost exits non-zero on load failure; a successful 1-block
        # process is a strict superset of "loads".
        in_ch, out_ch = ASYMMETRIC_BUSES[plugin_name]
        silence = np.zeros((in_ch, 256), dtype=np.float32)
        out = run_testhost(plugin_name, silence,
                           in_channels=in_ch, out_channels=out_ch)
        assert out.shape[0] == out_ch
        return

    plugin = _load_or_skip_pedalboard(plugin_name)
    assert plugin is not None
    assert plugin.parameters is not None, (
        f"{plugin_name}: parameters dict is None"
    )


# ---------------------------------------------------------------------------
# test_silence_in_silence_out
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("plugin_name", ALL_PLUGINS)
def test_silence_in_silence_out(plugin_name):
    """Silent input must produce silent output."""
    if plugin_name in ASYMMETRIC_BUSES:
        in_ch, out_ch = ASYMMETRIC_BUSES[plugin_name]
        silence = np.zeros((in_ch, BLOCK), dtype=np.float32)
        out = run_testhost(plugin_name, silence,
                           in_channels=in_ch, out_channels=out_ch)
    else:
        plugin  = _load_or_skip_pedalboard(plugin_name)
        silence = np.zeros((N_AMBI_CH, BLOCK), dtype=np.float32)
        out = plugin(silence, SR)

    assert np.all(np.isfinite(out)), f"{plugin_name}: non-finite sample in output"
    assert np.max(np.abs(out)) < 1e-5, (
        f"{plugin_name}: silent input produced output with "
        f"max abs {np.max(np.abs(out)):.3e}"
    )


# ---------------------------------------------------------------------------
# test_noise_output_is_finite
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("plugin_name", ALL_PLUGINS)
def test_noise_output_is_finite(plugin_name):
    """Plugin produces finite output and output shape is sensible."""
    rng = np.random.default_rng(0)

    if plugin_name in ASYMMETRIC_BUSES:
        in_ch, out_ch = ASYMMETRIC_BUSES[plugin_name]
        noise = rng.standard_normal((in_ch, BLOCK)).astype(np.float32) * 0.3
        out = run_testhost(plugin_name, noise,
                           in_channels=in_ch, out_channels=out_ch)
        expected_out_ch = out_ch
    else:
        plugin = _load_or_skip_pedalboard(plugin_name)
        noise  = rng.standard_normal((N_AMBI_CH, BLOCK)).astype(np.float32) * 0.3
        out    = plugin(noise, SR)
        expected_out_ch = None   # pedalboard hosts these square, so just ≥1

    assert np.all(np.isfinite(out)), (
        f"{plugin_name}: produced NaN/Inf for noise input"
    )
    assert out.shape[1] == noise.shape[1], (
        f"{plugin_name}: output sample count "
        f"{out.shape[1]} != input {noise.shape[1]}"
    )
    if expected_out_ch is not None:
        assert out.shape[0] == expected_out_ch, (
            f"{plugin_name}: expected {expected_out_ch} output channels, "
            f"got {out.shape[0]}"
        )
    else:
        assert out.shape[0] >= 1, f"{plugin_name}: output has zero channels"
