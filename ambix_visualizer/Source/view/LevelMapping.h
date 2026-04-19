#pragma once

#include <JuceHeader.h>

namespace LevelMapping
{
    enum class ColorMap
    {
        Jet,     // default
        Parula,
        Turbo,
        Viridis,
        Hot,
        Cool,
        Gray,
        Hsv
    };

    juce::StringArray getColorMapNames();
    ColorMap          colorMapFromName (const juce::String& name);
    juce::String      colorMapToName (ColorMap map);

    // Sample the colormap at normalized t in [0,1].
    juce::Colour sampleColorMap (ColorMap map, float t) noexcept;

    inline float linearToDb (float linear) noexcept
    {
        return linear > 0.00001f ? 20.0f * std::log10 (linear) : -100.0f;
    }

    // Map a linear level to t in [0,1] using a configurable dB floor.
    inline float linearToLevelT (float linear, float minDb) noexcept
    {
        const float db = linearToDb (linear);
        const float range = juce::jmax (1.0f, -minDb); // protect against minDb >= 0
        return juce::jlimit (0.0f, 1.0f, (db - minDb) / range);
    }

    // Peak ring colour: transparent white below -20 dB, ramping to solid red at 0 dB.
    juce::Colour peakRingColour (float peakHoldDb) noexcept;

    // Classic peak-hold meter with decay. Hold long enough that block-boundary
    // jitter on steady tones doesn't visibly dip the indicator; release slowly
    // so a real level drop still reads as a decay rather than an instant snap.
    inline float peakHoldEffective (float heldLinear,
                                    juce::uint32 heldAtMs,
                                    juce::uint32 nowMs) noexcept
    {
        constexpr float holdMs    = 1500.0f;
        constexpr float releaseMs = 1200.0f;
        const float elapsed = static_cast<float> (nowMs - heldAtMs);
        if (elapsed <= holdMs)
            return heldLinear;
        const float t = juce::jlimit (0.0f, 1.0f, (elapsed - holdMs) / releaseMs);
        return heldLinear * (1.0f - t);
    }
}
