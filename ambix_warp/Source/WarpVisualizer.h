/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ambix is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 ==============================================================================
 */

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include "HammerAitov.h"
#include "Conversions.h"
#include "JuceHeader.h"

class WarpVisualizer : public juce::Component
{
public:
    WarpVisualizer()
    {
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        rebuildPaths();
    }

    ~WarpVisualizer() override = default;

    // Callback: (phiNormalized, thetaNormalized) in 0..1 parameter range
    std::function<void (float, float)> onWarpDragged;
    std::function<void()> onGestureEnded;

    void setWarpParams (float phi, float phiCurve, float theta, float thetaCurve)
    {
        if (phi != phi_param || phiCurve != phi_curve_param
            || theta != theta_param || thetaCurve != theta_curve_param)
        {
            phi_param = phi;
            phi_curve_param = phiCurve;
            theta_param = theta;
            theta_curve_param = thetaCurve;
            rebuildPaths();
            repaint();
        }
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        dragStartPhiParam = phi_param;
        dragStartThetaParam = theta_param;
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (onGestureEnded) onGestureEnded();
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        // Reset both warp factors to 0 (0.5 normalized = center of -0.9..0.9 range)
        if (onWarpDragged)
            onWarpDragged (0.5f, 0.5f);
        if (onGestureEnded) onGestureEnded();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        // Horizontal drag → az warp, vertical drag → el warp
        // Scale: full width = full parameter range (0..1.8 mapped to -0.9..0.9)
        float dx = (float) e.getDistanceFromDragStartX() / (float) getWidth();
        float dy = (float) e.getDistanceFromDragStartY() / (float) getHeight();

        float newPhi = juce::jlimit (0.0f, 1.0f, dragStartPhiParam + dx * 1.5f);
        float newTheta = juce::jlimit (0.0f, 1.0f, dragStartThetaParam - dy * 1.5f);

        if (onWarpDragged)
            onWarpDragged (newPhi, newTheta);
    }

    void paint (juce::Graphics& g) override
    {
        juce::Path unwarpedTransformed = unwarpedCircles;
        juce::Path warpedTransformed = warpedCircles;

        unwarpedTransformed.applyTransform (toArea);
        warpedTransformed.applyTransform (toArea);

        // Draw unwarped circles: thin red lines
        g.setColour (juce::Colour (0xFFCC4444));
        g.strokePath (unwarpedTransformed, juce::PathStrokeType (1.0f));

        // Draw warped circles: bold blue lines
        g.setColour (juce::Colour (0xFF3333DD));
        g.strokePath (warpedTransformed, juce::PathStrokeType (3.0f));
    }

    void resized() override
    {
        juce::Rectangle<int> area = getLocalBounds();

        toArea = juce::AffineTransform::fromTargetPoints (
            (float) area.getCentreX(),
            (float) area.getCentreY(),
            (float) area.getRight() - 10.0f,
            (float) area.getCentreY(),
            (float) area.getCentreX(),
            (float) area.getBottom() - 20.0f);
    }

private:
    // --- Warp parameters (0.0 - 1.0 normalized) ---
    float phi_param = 0.5f;
    float phi_curve_param = 0.0f;
    float theta_param = 0.5f;
    float theta_curve_param = 0.0f;

    // Drag state
    float dragStartPhiParam = 0.5f;
    float dragStartThetaParam = 0.5f;

    juce::Path unwarpedCircles;
    juce::Path warpedCircles;
    juce::AffineTransform toArea;

    static constexpr float circleAngularRadius = 13.0f * (float) M_PI / 180.0f; // ~13 degrees
    static constexpr int circleNumPoints = 36;

    // --- Vector math helpers ---
    struct Vec3
    {
        float x, y, z;
    };

    static Vec3 cross (Vec3 a, Vec3 b)
    {
        return { a.y * b.z - a.z * b.y,
                 a.z * b.x - a.x * b.z,
                 a.x * b.y - a.y * b.x };
    }

    static float dot (Vec3 a, Vec3 b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static float length (Vec3 v)
    {
        return std::sqrt (dot (v, v));
    }

    static Vec3 normalize (Vec3 v)
    {
        float len = length (v);
        if (len < 1e-10f)
            return { 1.0f, 0.0f, 0.0f };
        return { v.x / len, v.y / len, v.z / len };
    }

    static Vec3 scale (Vec3 v, float s)
    {
        return { v.x * s, v.y * s, v.z * s };
    }

    static Vec3 add (Vec3 a, Vec3 b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    // --- Sphere circle sampling ---
    static Vec3 sphericalToCart (float az, float el)
    {
        float cosEl = std::cos (el);
        return { cosEl * std::cos (az), cosEl * std::sin (az), std::sin (el) };
    }

    static void cartToSpherical (Vec3 v, float& az, float& el)
    {
        az = std::atan2 (v.y, v.x);
        el = std::atan2 (v.z, std::sqrt (v.x * v.x + v.y * v.y));
    }

    void addCirclePath (juce::Path& path, float az0Rad, float el0Rad, float angRad)
    {
        Vec3 center = sphericalToCart (az0Rad, el0Rad);

        // Build tangent frame
        Vec3 up = { 0.0f, 0.0f, 1.0f };
        if (std::abs (el0Rad) > 1.4f) // near poles
            up = { 1.0f, 0.0f, 0.0f };

        Vec3 tangent = normalize (cross (up, center));
        Vec3 bitangent = cross (center, tangent); // already unit

        float cosR = std::cos (angRad);
        float sinR = std::sin (angRad);

        float prevAz = 0.0f;
        bool started = false;

        for (int i = 0; i <= circleNumPoints; ++i)
        {
            float t = (float) i / (float) circleNumPoints * juce::MathConstants<float>::twoPi;

            Vec3 pt = add (scale (center, cosR),
                           add (scale (tangent, sinR * std::cos (t)),
                                scale (bitangent, sinR * std::sin (t))));

            float az, el;
            cartToSpherical (pt, az, el);

            // Detect seam crossing (azimuth jump > pi)
            if (started && std::abs (az - prevAz) > juce::MathConstants<float>::pi)
            {
                started = false; // will start a new sub-path
            }

            // Negate azimuth for clockwise-positive display convention
            auto xy = HammerAitov::sphericalToXY (-az, el);

            if (! started)
            {
                path.startNewSubPath (xy);
                started = true;
            }
            else
            {
                path.lineTo (xy);
            }

            prevAz = az;
        }
    }

    // --- Warp math (replicates PluginProcessor.cpp lines 407-510) ---
    static void warpPoint (float az, float el,
                           float phiParam, float phiCurveParam,
                           float thetaParam, float thetaCurveParam,
                           float& outAz, float& outEl)
    {
        double phi_alpha = (double) phiParam * 1.8 - 0.9;
        double theta_alpha = (double) thetaParam * 1.8 - 0.9;

        // Elevation pole mode needs negation (forward = -inverse for Moebius on sin(el)).
        // Azimuth does NOT need negation — the half-angle domain (az/2) changes the sign.
        if (thetaCurveParam <= 0.5f)
            theta_alpha = -theta_alpha;

        outAz = az;
        outEl = el;

        // Elevation warp
        if (theta_alpha != 0.0)
        {
            double mu = std::sin ((double) el);

            if (thetaCurveParam <= 0.5f)
            {
                // Warp toward a pole
                double denom = 1.0 + theta_alpha * mu;
                if (std::abs (denom) > 1e-12)
                    outEl = (float) std::asin (juce::jlimit (-1.0, 1.0, (mu + theta_alpha) / denom));
            }
            else
            {
                // Warp toward/away from equator
                if (theta_alpha > 0.0)
                {
                    double denom = 2.0 * theta_alpha * mu;
                    if (std::abs (denom) > 1e-12)
                    {
                        double val = (theta_alpha - 1.0 + std::sqrt (std::pow (theta_alpha - 1.0, 2) + 4.0 * theta_alpha * mu * mu)) / denom;
                        outEl = (float) std::asin (juce::jlimit (-1.0, 1.0, val));
                    }
                }
                else
                {
                    double denom = 1.0 + theta_alpha * mu * mu;
                    if (std::abs (denom) > 1e-12)
                        outEl = (float) std::asin (juce::jlimit (-1.0, 1.0, ((1.0 + theta_alpha) * mu) / denom));
                }
            }
        }

        // Azimuth warp
        if (phi_alpha != 0.0)
        {
            double phi_temp = (double) az * 0.5;

            if (phiCurveParam <= 0.5f)
            {
                // Warp toward poles
                double sinPhi = std::sin (phi_temp);
                double denom = 1.0 + phi_alpha * sinPhi;
                if (std::abs (denom) > 1e-12)
                    outAz = (float) (std::asin (juce::jlimit (-1.0, 1.0, (sinPhi + phi_alpha) / denom)));
                else
                    outAz = (float) phi_temp;
            }
            else
            {
                // Warp toward equator
                if (phi_temp > 0.0)
                {
                    double cosPhi2 = std::cos (phi_temp * 2.0);
                    double denom = 1.0 + phi_alpha * cosPhi2;
                    if (std::abs (denom) > 1e-12)
                        outAz = (float) (std::acos (juce::jlimit (-1.0, 1.0, (cosPhi2 + phi_alpha) / denom)) / 2.0);
                    else
                        outAz = (float) phi_temp;
                }
                else
                {
                    double arg = M_PI - (phi_temp - M_PI / 2.0) * 2.0;
                    double cosArg = std::cos (arg);
                    double denom = 1.0 + phi_alpha * cosArg;
                    if (std::abs (denom) > 1e-12)
                        outAz = (float) (-std::acos (juce::jlimit (-1.0, 1.0, (cosArg + phi_alpha) / denom)) / 2.0);
                    else
                        outAz = (float) phi_temp;
                }
            }

            outAz *= 2.0f;
        }
    }

    void addWarpedCirclePath (juce::Path& path, float az0Rad, float el0Rad, float angRad)
    {
        Vec3 center = sphericalToCart (az0Rad, el0Rad);

        // Build tangent frame
        Vec3 up = { 0.0f, 0.0f, 1.0f };
        if (std::abs (el0Rad) > 1.4f)
            up = { 1.0f, 0.0f, 0.0f };

        Vec3 tangent = normalize (cross (up, center));
        Vec3 bitangent = cross (center, tangent);

        float cosR = std::cos (angRad);
        float sinR = std::sin (angRad);

        float prevAz = 0.0f;
        bool started = false;

        for (int i = 0; i <= circleNumPoints; ++i)
        {
            float t = (float) i / (float) circleNumPoints * juce::MathConstants<float>::twoPi;

            Vec3 pt = add (scale (center, cosR),
                           add (scale (tangent, sinR * std::cos (t)),
                                scale (bitangent, sinR * std::sin (t))));

            float az, el;
            cartToSpherical (pt, az, el);

            // Apply warp
            float warpedAz, warpedEl;
            warpPoint (az, el, phi_param, phi_curve_param, theta_param, theta_curve_param, warpedAz, warpedEl);

            // Detect seam crossing
            if (started && std::abs (warpedAz - prevAz) > juce::MathConstants<float>::pi)
                started = false;

            // Negate azimuth for clockwise-positive display convention
            auto xy = HammerAitov::sphericalToXY (-warpedAz, warpedEl);

            if (! started)
            {
                path.startNewSubPath (xy);
                started = true;
            }
            else
            {
                path.lineTo (xy);
            }

            prevAz = warpedAz;
        }
    }

    void rebuildPaths()
    {
        unwarpedCircles.clear();
        warpedCircles.clear();

        // Staggered circle grid matching the reference paper figures:
        // Odd rows (el = -60, 0, 60): az = -90, 0, 90
        // Even rows (el = -30, 30): az = -135, -45, 45, 135
        const float rowA_az[] = { -90.0f, 0.0f, 90.0f };
        const float rowB_az[] = { -135.0f, -45.0f, 45.0f, 135.0f };

        // Row at el = 60 (rowA)
        for (float az : rowA_az)
            addCirclePair (az, 60.0f);

        // Row at el = 30 (rowB)
        for (float az : rowB_az)
            addCirclePair (az, 30.0f);

        // Row at el = 0 (rowA)
        for (float az : rowA_az)
            addCirclePair (az, 0.0f);

        // Row at el = -30 (rowB)
        for (float az : rowB_az)
            addCirclePair (az, -30.0f);

        // Row at el = -60 (rowA)
        for (float az : rowA_az)
            addCirclePair (az, -60.0f);
    }

    void addCirclePair (float azDeg, float elDeg)
    {
        float azRad = Conversions<float>::degreesToRadians (azDeg);
        float elRad = Conversions<float>::degreesToRadians (elDeg);
        addCirclePath (unwarpedCircles, azRad, elRad, circleAngularRadius);
        addWarpedCirclePath (warpedCircles, azRad, elRad, circleAngularRadius);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WarpVisualizer)
};
