"""
ambix_converter at 2nd-order Ambisonics (9 channels).

Hosted via `ambix_testhost` so the converter's main bus can be configured
to AudioChannelSet::ambisonic(2). pedalboard pins the bus width at load
time and can't address anything past 1st order.

Order-2 testing matters because several converter features are *no-op at
1st order* and only have an effect at order >= 2:

  * Condon-Shortley phase flip — at 1st order the CS sign table is all +1
    in channels 0..3 (only ±1 alternation is at |m|=1, which is already
    sign-symmetric there); at order 2 it inverts ACN 1, 3, 5, 7.
  * `InputIs2D` / `OutputIs2D` — drops 3D-only channels (those with
    |m| < l), which only exist from l >= 2.

Reference: common/SphericalHarmonic/ch_cs.h holds the per-ACN sign table
the plugin uses.
"""

import os
import sys
import numpy as np
import pytest

sys.path.insert(0, os.path.dirname(__file__))

from conftest import SR, run_testhost


N_O2 = 9   # (order + 1)^2 = 9 for order 2


@pytest.fixture(scope="module")
def noise_9ch():
    rng = np.random.default_rng(20)
    return rng.standard_normal((N_O2, 4096)).astype(np.float32)


# Plugin parameter helpers (cross-referenced with test_converter.py)
DEFAULTS = {
    "InChannelSeq":  0.0,   # ACN
    "OutChannelSeq": 0.0,   # ACN
    "InNormalization":  0.0,   # SN3D
    "OutNormalization": 0.0,   # SN3D
    "MirrorLeftRight": 0.0,
    "MirrorFrontBack": 0.0,
    "MirrorTopBottom": 0.0,
    "FlipCondonShortleyPhase": 0.0,
    "InputIs2D":  0.0,
    "OutputIs2D": 0.0,
}


def _convert(audio, **overrides):
    params = dict(DEFAULTS)
    params.update(overrides)
    return run_testhost("ambix_converter", audio,
                        in_channels=N_O2, out_channels=N_O2,
                        params=params)


# ---------------------------------------------------------------------------
# Identity at order 2
# ---------------------------------------------------------------------------

def test_identity_passthrough_order2(noise_9ch):
    """ACN/SN3D → ACN/SN3D, no mirrors — bit-exact pass-through across all
    9 channels (was only exercised at 4 channels before)."""
    out = _convert(noise_9ch)
    np.testing.assert_allclose(out, noise_9ch, atol=1e-6)


# ---------------------------------------------------------------------------
# SN3D → N3D scaling at order 2
# ---------------------------------------------------------------------------

def test_sn3d_to_n3d_gains_order2(noise_9ch):
    """SN3D → N3D multiplies each ACN channel by sqrt(2l+1).
    Order 0..2:  W ×1,  l=1 (3 ch) ×√3,  l=2 (5 ch) ×√5."""
    out = _convert(noise_9ch,
                   InNormalization=0.0,   # SN3D
                   OutNormalization=1.0)  # N3D

    expected = np.array(
        [1.0]
        + [np.sqrt(3.0)] * 3   # ACN 1..3 (l=1)
        + [np.sqrt(5.0)] * 5,  # ACN 4..8 (l=2)
        dtype=np.float32,
    )
    for ch, g in enumerate(expected):
        np.testing.assert_allclose(
            out[ch], noise_9ch[ch] * g, atol=1e-5,
            err_msg=f"ACN ch{ch}: expected SN3D→N3D gain {g:.4f}"
        )


def test_n3d_to_sn3d_round_trip(noise_9ch):
    """A SN3D→N3D→SN3D round trip is identity."""
    n3d  = _convert(noise_9ch,
                    InNormalization=0.0, OutNormalization=1.0)
    back = _convert(n3d,
                    InNormalization=1.0, OutNormalization=0.0)
    # 2× float32 multiplies of magnitude √5 — atol scales accordingly.
    np.testing.assert_allclose(back, noise_9ch, atol=2e-5)


# ---------------------------------------------------------------------------
# Condon-Shortley flip (only meaningful at order >= 1, but the order-2
# channels expose the pattern most cleanly)
# ---------------------------------------------------------------------------

# acn_cs_phase from common/SphericalHarmonic/ch_cs.h
# (alternates +1,-1 for the first 9 ACN indices)
ACN_CS_SIGNS = np.array(
    [+1, -1, +1, -1, +1, -1, +1, -1, +1], dtype=np.float32)


def test_condon_shortley_flip_signs(noise_9ch):
    """The CS-phase toggle multiplies each ACN channel by the entry in
    common/SphericalHarmonic/ch_cs.h::acn_cs_phase. Validate the full
    9-channel sign pattern (1st-order alone hides indices 4..8)."""
    out = _convert(noise_9ch, FlipCondonShortleyPhase=1.0)
    for ch in range(N_O2):
        np.testing.assert_allclose(
            out[ch], ACN_CS_SIGNS[ch] * noise_9ch[ch], atol=1e-6,
            err_msg=f"ACN ch{ch}: CS sign expected {ACN_CS_SIGNS[ch]:+.0f}"
        )


# ---------------------------------------------------------------------------
# 2D input/output flags: channel packing, not just "drop 3D"
# ---------------------------------------------------------------------------
#
# In the converter, the 2D layout packs only the |m|==l channels (the
# horizontal "ring") into a contiguous prefix, ordered by ACN-of-3D.
# At order 2 the mapping is:
#
#     2D pos 0 ← 3D ACN 0 (W,    l=0)
#     2D pos 1 ← 3D ACN 1 (Y,    l=1, m=-1)
#     2D pos 2 ← 3D ACN 3 (X,    l=1, m=+1)
#     2D pos 3 ← 3D ACN 4 (V,    l=2, m=-2)
#     2D pos 4 ← 3D ACN 8 (U,    l=2, m=+2)
#
# (Derived from common/SphericalHarmonic/tools.h::ACN3DtoACN2D.)

TWO_D_RING_3D_ACN = [0, 1, 3, 4, 8]   # 3D ACN indices of the 2D ring
THREE_D_ONLY_ACN  = [2, 5, 6, 7]      # |m| < l


def test_output_2d_packs_ring_channels(noise_9ch):
    """`OutputIs2D=1` re-packs the horizontal-ring inputs into consecutive
    output positions 0..4; output positions 5..8 are zeroed."""
    out = _convert(noise_9ch, OutputIs2D=1.0)
    for two_d_pos, three_d_acn in enumerate(TWO_D_RING_3D_ACN):
        np.testing.assert_allclose(
            out[two_d_pos], noise_9ch[three_d_acn], atol=1e-5,
            err_msg=(f"2D output pos {two_d_pos} should carry 3D ACN "
                     f"{three_d_acn}")
        )
    for ch in range(len(TWO_D_RING_3D_ACN), N_O2):
        np.testing.assert_allclose(
            out[ch], 0.0, atol=1e-5,
            err_msg=f"Output ch{ch} should be silent under OutputIs2D=1"
        )


def test_input_2d_unpacks_to_ring_channels():
    """`InputIs2D=1` reads input channels 0..4 as the packed 2D ring and
    routes them to the corresponding 3D ACN positions; the 3D-only ACN
    positions on the output stay silent."""
    rng = np.random.default_rng(21)
    packed = np.zeros((N_O2, 4096), dtype=np.float32)
    packed[:5] = rng.standard_normal((5, 4096)).astype(np.float32)

    out = _convert(packed, InputIs2D=1.0)

    for two_d_pos, three_d_acn in enumerate(TWO_D_RING_3D_ACN):
        np.testing.assert_allclose(
            out[three_d_acn], packed[two_d_pos], atol=1e-5,
            err_msg=(f"3D ACN {three_d_acn} should carry 2D input ch "
                     f"{two_d_pos}")
        )
    for ch in THREE_D_ONLY_ACN:
        np.testing.assert_allclose(
            out[ch], 0.0, atol=1e-5,
            err_msg=f"3D-only ACN ch{ch} should be silent under InputIs2D=1"
        )


def test_input_and_output_2d_round_trip():
    """Packing the 2D ring to output and reading it back as 2D input is
    an identity for the ring channels."""
    rng = np.random.default_rng(22)
    packed = np.zeros((N_O2, 4096), dtype=np.float32)
    packed[:5] = rng.standard_normal((5, 4096)).astype(np.float32)

    # 2D-pack input back into the 3D ACN positions, then re-pack 2D→2D.
    unpacked = _convert(packed,    InputIs2D=1.0)
    repacked = _convert(unpacked,  OutputIs2D=1.0)

    np.testing.assert_allclose(repacked[:5], packed[:5], atol=1e-5)
    np.testing.assert_allclose(repacked[5:], 0.0, atol=1e-5)
