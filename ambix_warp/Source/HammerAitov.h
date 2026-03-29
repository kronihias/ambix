/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once
#include <cfloat>

class HammerAitov
{
public:
    /**
     Calculates the HammerAitov-projection for azimuth and elevation.
     The x output represents the horizontal axis, with 0 being in the middle. Negative azimuth values result in positive x values (to the right) and vice versa.
     The y output represents the vertical axis, with 0 being in the middle. Positive elevation values result in positive y values (up) and vice versa.
     */
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
        constexpr float sqrt2Half = 0.5f * 1.41421356237309504880168872420969808;
        const float z =
            std::sqrt (1.0f - juce::square (x * sqrt2Half) - juce::square (y * sqrt2Half));
        azimuthInRadians = 2.0f * std::atan (sqrt2Half * -x * z / (juce::square (z) - 0.5f));
        elevationInRadians = std::asin (std::sqrt (2) * y * z);
    }
};
