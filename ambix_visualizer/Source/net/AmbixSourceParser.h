#pragma once

#include <JuceHeader.h>

// Parses the new per-source OSC vocabulary emitted by ambix_encoder ≥ 0.5
// when "extended OSC" is enabled. Addresses look like:
//   /ambix_encoder/source/<n>/azimuth   <float deg>
//   /ambix_encoder/source/<n>/elevation <float deg>
//   /ambix_encoder/source/<n>/size      <float 0..1>
//   /ambix_encoder/source/<n>/meter     <float 0..1 RMS>
// And the global counterparts:
//   /ambix_encoder/linked         <int>
//   /ambix_encoder/active_sources <int>
//   /ambix_encoder/azimuth /elevation /width /size <float>
//
// One OSC message = one parsed payload (the encoder splits its broadcast
// across multiple datagrams to keep the format simple).

struct AmbixSourcePayload
{
    enum class Param
    {
        Azimuth,
        Elevation,
        Size,
        Meter,
        // Global (per-plugin) scalars. sourceIndex is unused for these.
        Linked,
        ActiveSources,
        GlobalAzimuth,
        GlobalElevation,
        GlobalWidth,
        GlobalSize,
    };

    Param param       { Param::Azimuth };
    int   sourceIndex { 0 };   // 1-based; 0 means global
    float value       { 0.0f };
};

class AmbixSourceParser
{
public:
    static constexpr const char* kPrefix = "/ambix_encoder/";

    static bool parse (const char* data, size_t size, AmbixSourcePayload& out);
};
