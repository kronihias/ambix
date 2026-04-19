#pragma once

#include <JuceHeader.h>

struct MultiEncoderPayload
{
    enum class Param { Azimuth, Elevation, Gain, Mute, Solo };

    Param param       { Param::Azimuth };
    int   sourceIndex { 0 };
    float value       { 0.0f };
    // The address prefix between the leading `/` and the last `/`. Captured
    // so the visualizer can handle user-customised OSC addresses in IEM's
    // footer (e.g. /MySynth/azimuth3) and use the prefix as the source label.
    juce::String addressPrefix;
};

class MultiEncoderParser
{
public:
    // Parses `/<anyprefix>/<param><N> <float>` messages. Returns true iff the
    // leaf matches a supported per-source parameter and the message decoded
    // cleanly. Accepts any prefix so the user can rename their IEM plugin's
    // OSC address and still be picked up.
    static bool parse (const char* data, size_t size, MultiEncoderPayload& out);

    // Build an outbound param address: (prefix="MultiEncoder", azimuth, 5) -> "/MultiEncoder/azimuth5"
    static juce::String buildAddress (const juce::String& prefix,
                                      MultiEncoderPayload::Param param,
                                      int index);
};
