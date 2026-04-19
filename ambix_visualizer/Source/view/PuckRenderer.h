#pragma once

#include <JuceHeader.h>

#include "../app/Settings.h"
#include "../model/EncoderSource.h"

namespace PuckRenderer
{
    constexpr float kMinRadiusPx = 10.0f;
    constexpr float kMaxRadiusPx = 28.0f;

    // Scale factor applied to the base radius range based on the user's
    // puck-size preference. Small = 1.0× (the original sizing); Medium and
    // Large bump it up for distant viewing / touch targets.
    inline float sizeScale (AppSettings::PuckSize sz) noexcept
    {
        switch (sz)
        {
            case AppSettings::PuckSize::Medium: return 1.5f;
            case AppSettings::PuckSize::Large:  return 2.1f;
            case AppSettings::PuckSize::Small:
            default:                            return 1.0f;
        }
    }

    void draw (juce::Graphics& g,
               const EncoderSource& source,
               juce::Point<float> centre,
               float radiusPx,
               bool isSelected,
               bool isBackHemisphere,
               const AppSettings& settings);
}
