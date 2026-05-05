/*
 ==============================================================================
 Hammer-Aitoff projection helper.
 Originally from the IEM plug-in suite (Daniel Rudrich, 2017),
 https://iem.at — GPLv3.
 Copied verbatim into this plugin for self-contained builds.
 ==============================================================================
 */

#pragma once
#include <cfloat>
#include "JuceHeader.h"

class HammerAitov
{
public:
    static void sphericalToXY (float azimuthInRadians, float elevationInRadians, float& x, float& y)
    {
        while (azimuthInRadians > juce::MathConstants<float>::pi + FLT_EPSILON)
            azimuthInRadians -= 2.0f * juce::MathConstants<float>::pi;
        while (azimuthInRadians < -juce::MathConstants<float>::pi - FLT_EPSILON)
            azimuthInRadians += 2.0f * juce::MathConstants<float>::pi;

        const float cosEle = std::cos (elevationInRadians);
        const float factor = 1.0f / std::sqrt (1.0f + cosEle * std::cos (0.5f * azimuthInRadians));
        x = factor * -cosEle * std::sin (0.5f * azimuthInRadians);
        y = factor * std::sin (elevationInRadians);
    }

    static juce::Point<float> sphericalToXY (float azimuthInRadians, float elevationInRadians)
    {
        juce::Point<float> ret;
        sphericalToXY (azimuthInRadians, elevationInRadians, ret.x, ret.y);
        return ret;
    }

    static void XYToSpherical (float x, float y, float& azimuthInRadians, float& elevationInRadians)
    {
        constexpr float sqrt2Half = 0.5f * 1.41421356237309504880168872420969808f;
        const float arg = 1.0f - juce::square (x * sqrt2Half) - juce::square (y * sqrt2Half);
        if (arg <= 0.0f)
        {
            azimuthInRadians = 0.0f;
            elevationInRadians = 0.0f;
            return;
        }
        const float z = std::sqrt (arg);
        azimuthInRadians = 2.0f * std::atan (sqrt2Half * -x * z / (juce::square (z) - 0.5f));
        elevationInRadians = std::asin (std::sqrt (2.0f) * y * z);
    }

    // Returns true if (x,y) lies inside the projection ellipse (the half-extents
    // are sqrt(2) horizontally and 1/sqrt(2) vertically). Avoids feeding NaN
    // into XYToSpherical.
    static bool isInside (float x, float y) noexcept
    {
        constexpr float sqrt2Half = 0.5f * 1.41421356237309504880168872420969808f;
        return (1.0f - juce::square (x * sqrt2Half) - juce::square (y * sqrt2Half)) > 0.0f;
    }
};
