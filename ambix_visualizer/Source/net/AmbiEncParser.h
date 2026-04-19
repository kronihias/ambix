#pragma once

#include <JuceHeader.h>

struct AmbiEncPayload
{
    int id { 0 };
    juce::String trackName;
    float distance { 2.0f };
    float azimuthDeg { 0.0f };
    float elevationDeg { 0.0f };
    float size { 0.0f };
    float peakLinear { 0.0f };
    float rmsLinear { 0.0f };
    bool hasReplyPort { false };
    int replyPort { 0 };
};

class AmbiEncParser
{
public:
    static constexpr const char* kAddress = "/ambi_enc";

    static bool parse (const char* data, size_t size, AmbiEncPayload& out);
};
