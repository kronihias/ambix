"""
Reference DSP for ambix_converter — channel-sequence and normalisation
conversion between ACN / FuMa / SID layouts (SN3D / FuMa / N3D norms).

Mirrors the tables in common/SphericalHarmonic/{ch_sequence,normalization}.h
for first-order (4-channel) Ambisonics, which is enough to validate the
plugin DSP; higher orders extend the same logic channel-by-channel.

ACN channel indices (1st order): 0=W, 1=Y, 2=Z, 3=X
FuMa channel order             : 0=W, 1=X, 2=Y, 3=Z

Channel map semantics
---------------------
`<scheme>_ch_map[acn_i] = index_of_acn_i_in_scheme`.
Example: fuma_ch_map[3] = 1 means the ACN-index-3 signal (X) lives at
position 1 in the FuMa layout.

In the plugin (PluginProcessor.cpp), for every ACN index i:
    output[out_ch_seq[i]] = input[in_ch_seq[i]] * in_ch_norm[i] * mirror_sign

Parameter mapping (param2scheme, from getParameter):
    p < 0.33        -> ACN  / SN3D
    0.33 <= p < 0.66 -> FuMa / FuMa
    p >= 0.66       -> SID  / N3D
"""

from __future__ import annotations

import numpy as np


# ---------------------------------------------------------------------------
# Channel maps (1st order only; 4 entries are sufficient here)
# ---------------------------------------------------------------------------

#  acn_ch_map[i]  = i   (trivial)
#  fuma_ch_map[i] = FuMa slot where ACN-index-i component lives
#  sid_ch_map[i]  = SID slot where ACN-index-i component lives
ACN_CH_MAP  = np.array([0, 1, 2, 3], dtype=np.int32)
FUMA_CH_MAP = np.array([0, 2, 3, 1], dtype=np.int32)
SID_CH_MAP  = np.array([0, 2, 3, 1], dtype=np.int32)  # same as FuMa for 1st order


# ---------------------------------------------------------------------------
# Normalisation gain tables (first-order entries, indexed by ACN)
# ---------------------------------------------------------------------------

SQRT2 = float(np.sqrt(2.0))
SQRT3 = float(np.sqrt(3.0))

# Per-ACN multiplier applied in-place (scheme_in -> scheme_out).
# Table of size 4, index by ACN channel number.
CONV_NORM = {
    ("SN3D", "SN3D"): np.array([1.0, 1.0, 1.0, 1.0], dtype=np.float32),
    ("FuMa", "FuMa"): np.array([1.0, 1.0, 1.0, 1.0], dtype=np.float32),
    ("N3D",  "N3D"):  np.array([1.0, 1.0, 1.0, 1.0], dtype=np.float32),

    # SN3D <-> FuMa:  W gets *sqrt(2); l>=1 unchanged in SN3D/FuMa ratio (both unit-max for l=1)
    ("SN3D", "FuMa"): np.array([1.0 / SQRT2, 1.0, 1.0, 1.0], dtype=np.float32),
    ("FuMa", "SN3D"): np.array([SQRT2,       1.0, 1.0, 1.0], dtype=np.float32),

    # SN3D <-> N3D: multiply by sqrt(2l+1)
    ("SN3D", "N3D"):  np.array([1.0, SQRT3, SQRT3, SQRT3], dtype=np.float32),
    ("N3D",  "SN3D"): np.array([1.0, 1.0 / SQRT3, 1.0 / SQRT3, 1.0 / SQRT3], dtype=np.float32),

    # FuMa <-> N3D: product of the two hops
    ("FuMa", "N3D"):  np.array([SQRT2, SQRT3, SQRT3, SQRT3], dtype=np.float32),
    ("N3D",  "FuMa"): np.array([1.0 / SQRT2, 1.0 / SQRT3, 1.0 / SQRT3, 1.0 / SQRT3],
                               dtype=np.float32),
}


# ---------------------------------------------------------------------------
# (l, m) tables for mirror signs
# ---------------------------------------------------------------------------

# ACN -> (l, m) for first order
LM = {
    0: (0,  0),
    1: (1, -1),
    2: (1,  0),
    3: (1,  1),
}


def mirror_signs(flip: bool, flop: bool, flap: bool,
                 flip_cs: bool = False) -> np.ndarray:
    """
    Sign multiplier per ACN channel for mirror flags.

    flip : L/R mirror (m < 0 -> invert)
    flop : F/B mirror (m<0 and even, OR m>=0 and odd -> invert)
    flap : T/B mirror (l+m odd -> invert)
    """
    sign = np.ones(4, dtype=np.float32)
    for i, (l, m) in LM.items():
        f1 = -1 if (flip and m < 0) else 1
        f2 = -1 if (flop and (((m < 0) and (m % 2 == 0)) or
                              ((m >= 0) and (m % 2 != 0)))) else 1
        f3 = -1 if (flap and (l + m) % 2 != 0) else 1
        s  = f1 * f2 * f3
        if flip_cs:
            # Condon-Shortley: acn_cs[i] for first order is all +1
            # (non-trivial only from l>=2). Leave unchanged at 1st order.
            pass
        sign[i] = s
    return sign


# ---------------------------------------------------------------------------
# Parameter helpers
# ---------------------------------------------------------------------------

SCHEME_PARAM = {
    "ACN": 0.0,   "SN3D": 0.0,
    "FuMa": 0.5,
    "SID":  1.0,  "N3D":  1.0,
}


def scheme_param(name: str) -> float:
    """Normalised param value that selects `name` on the plugin."""
    return SCHEME_PARAM[name]


# ---------------------------------------------------------------------------
# Reference implementation
# ---------------------------------------------------------------------------

_CH_MAP = {
    "ACN":  ACN_CH_MAP,
    "SID":  SID_CH_MAP,
    "FuMa": FUMA_CH_MAP,
}


def convert(audio: np.ndarray,
            in_seq: str, out_seq: str,
            in_norm: str, out_norm: str,
            flip: bool = False, flop: bool = False, flap: bool = False
            ) -> np.ndarray:
    """
    Reference DSP matching Ambix_converterAudioProcessor::processBlock.

    audio   : (4, N) float32, ordered in the input channel sequence
    in_seq  : "ACN" | "FuMa" | "SID"
    out_seq : "ACN" | "FuMa" | "SID"
    in_norm : "SN3D" | "FuMa" | "N3D"
    out_norm: "SN3D" | "FuMa" | "N3D"
    """
    assert audio.shape[0] == 4, "1st-order reference expects 4 channels"
    in_map   = _CH_MAP[in_seq]
    out_map  = _CH_MAP[out_seq]
    gains    = CONV_NORM[(in_norm, out_norm)]
    signs    = mirror_signs(flip, flop, flap)

    out = np.zeros_like(audio)
    # iterate over ACN indices as the plugin does
    for acn_i in range(4):
        src = int(in_map[acn_i])
        dst = int(out_map[acn_i])
        out[dst] = audio[src] * gains[acn_i] * signs[acn_i]
    return out
