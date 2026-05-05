/*
 ==============================================================================
 Local subset of ambix_visualizer's LevelMapping — colormap sampling and dB
 helpers used for the directional energy heatmap.
 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"

namespace Colormap
{
    enum class Map
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

    juce::StringArray getNames();
    Map               fromName (const juce::String& name);
    juce::String      toName (Map m);

    // Sample colormap at normalized t in [0,1].
    juce::Colour sample (Map m, float t) noexcept;

    inline float linearToDb (float linear) noexcept
    {
        return linear > 0.00001f ? 20.0f * std::log10 (linear) : -100.0f;
    }

    // Map a linear level to t in [0,1] using a configurable dB floor (negative).
    inline float linearToLevelT (float linear, float minDb) noexcept
    {
        const float db    = linearToDb (linear);
        const float range = juce::jmax (1.0f, -minDb);
        return juce::jlimit (0.0f, 1.0f, (db - minDb) / range);
    }
}
