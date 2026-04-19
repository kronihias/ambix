/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 Modified for ambix_visualizer: parameterised elevation range, centred text
 labels, ambix clockwise-positive azimuth convention.

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 ==============================================================================
 */

#pragma once
#include "HammerAitov.h"
#include "Conversions.h"

class HammerAitovGrid : public juce::Component
{
public:
    HammerAitovGrid()
    {
        setBufferedToImage (true);
        rebuildPaths();
    }

    ~HammerAitovGrid() override = default;

    // Sets the visible elevation range in ambix convention (+ele = up).
    // The grid internally maps these to HA math convention.
    void setAmbixElevationRange (float minDeg, float maxDeg)
    {
        minDeg = juce::jlimit (-90.0f, 90.0f, minDeg);
        maxDeg = juce::jlimit (minDeg + 5.0f, 90.0f, maxDeg);
        if (std::abs (minDeg - minAmbixEleDeg) < 0.01f && std::abs (maxDeg - maxAmbixEleDeg) < 0.01f)
            return;
        minAmbixEleDeg = minDeg;
        maxAmbixEleDeg = maxDeg;
        rebuildPaths();
        rebuildTransform();
        repaint();
    }

    // Mirror the x-axis: ambix RIGHT ends up on screen LEFT and vice versa.
    void setFlipLeftRight (bool flip)
    {
        if (flip == flipLeftRight)
            return;
        flipLeftRight = flip;
        rebuildTransform();
        repaint();
    }

    float getMinAmbixEleDeg() const noexcept { return minAmbixEleDeg; }
    float getMaxAmbixEleDeg() const noexcept { return maxAmbixEleDeg; }

    juce::AffineTransform getProjectionTransform() const noexcept { return toArea; }

    // Static so HammerAitovView can compute the same transform.
    static juce::AffineTransform computeToArea (juce::Rectangle<float> area,
                                                float minAmbixEleDeg,
                                                float maxAmbixEleDeg,
                                                bool flipLeftRight = false)
    {
        const auto degToRad = [] (float d) { return Conversions<float>::degreesToRadians (d); };
        // In the view we negate ambix elevation before feeding to HammerAitov,
        // so HA_y(ambix_el) = -sin(ambix_el).
        const float haYMin = -std::sin (degToRad (maxAmbixEleDeg)); // top
        const float haYMax = -std::sin (degToRad (minAmbixEleDeg)); // bottom

        const float xMargin = 10.0f, yMargin = 20.0f;
        const float centreX = area.getCentreX();
        const float centreY = area.getCentreY();
        const float xAbsScale = (area.getWidth()  - 2.0f * xMargin) * 0.5f;
        const float xScale  = flipLeftRight ? -xAbsScale : xAbsScale;
        const float yRange  = juce::jmax (0.01f, haYMax - haYMin);
        const float yScale  = (area.getHeight() - 2.0f * yMargin) / yRange;
        const float yCentreHa = (haYMax + haYMin) * 0.5f;

        return juce::AffineTransform::scale (xScale, yScale)
                   .translated (centreX, centreY - yCentreHa * yScale);
    }

    void paint (juce::Graphics& g) override
    {
        juce::Path path;
        g.setColour (juce::Colour (0xFF2D2D2D));
        juce::Path rectangle;
        rectangle.addRectangle (getLocalBounds());
        rectangle.setUsingNonZeroWinding (false);

        path = outline;
        path.applyTransform (toArea);

        rectangle.addPath (path);
        g.fillPath (rectangle);

        g.setColour (juce::Colours::white);

        path = boldGrid;
        path.applyTransform (toArea);
        g.strokePath (path, juce::PathStrokeType (1.0f));

        path = regularGrid;
        path.applyTransform (toArea);
        g.strokePath (path, juce::PathStrokeType (0.2f));

        auto labelFont =
            juce::FontOptions (getLookAndFeel().getTypefaceForFont (juce::FontOptions (12.0f, 1)))
                .withHeight (12.0f);
        g.setFont (labelFont);

        // Cardinal labels are tinted so they pop out against the white grid
        // lines instead of blending in. Matches the IP-label gold used in the
        // top bar for visual consistency.
        const juce::Colour cardinalColour (0xFFE8D76C);
        g.setColour (cardinalColour);

        // Cardinal labels — callers use ambix convention; we convert to HA
        // (negating both az and el) before projecting. The 60×12 text
        // rectangle is centered on the projected point; if that rectangle
        // would spill past the view edges (typical for the BACK labels,
        // which land at the far left/right of the Hammer-Aitov ellipse),
        // we nudge it inward so it stays fully on-screen.
        const float viewW = static_cast<float> (getWidth());
        auto drawLabel = [&] (const char* text,
                              int ambixAziDeg, int ambixEleDeg, float yOff)
        {
            if (ambixEleDeg < minAmbixEleDeg - 1.0f
                || ambixEleDeg > maxAmbixEleDeg + 1.0f)
                return;
            auto p = anglesToPoint (-ambixAziDeg, -ambixEleDeg);
            p.applyTransform (toArea);

            constexpr float labelW = 60.0f;
            constexpr float edgePad = 2.0f;
            juce::Rectangle<float> rect (p.x - labelW * 0.5f, p.y + yOff, labelW, 12.0f);
            if (rect.getX() < edgePad)
                rect = rect.withX (edgePad);
            else if (rect.getRight() > viewW - edgePad)
                rect = rect.withX (viewW - edgePad - rect.getWidth());
            g.drawText (text, rect, juce::Justification::centred, false);
        };

        drawLabel ("FRONT",    0,    0, -12.0f);
        drawLabel ("LEFT",   -90,    0, -12.0f);   // ambix left = -90°
        drawLabel ("RIGHT",   90,    0, -12.0f);   // ambix right = +90°
        drawLabel ("BACK",   180,    0, -12.0f);
        drawLabel ("BACK",  -180,    0, -12.0f);
        drawLabel ("TOP",      0,   90, -12.0f);   // ambix top = +90°
        drawLabel ("BOTTOM",   0,  -90,   0.0f);   // ambix bottom = -90°

        g.setColour (juce::Colours::white);

        auto numericFont =
            juce::FontOptions (getLookAndFeel().getTypefaceForFont (juce::FontOptions (12.0f, 2)))
                .withHeight (12.0f);
        g.setFont (numericFont);

        // Azimuth numeric labels (shown on the equator; skip if equator not visible)
        if (0 >= minAmbixEleDeg && 0 <= maxAmbixEleDeg)
        {
            for (int azi = -150; azi <= 150; azi += 30)
            {
                auto p = anglesToPoint (azi, 0);
                p.applyTransform (toArea);
                g.drawText (juce::String (-azi) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")),
                            juce::Rectangle<float> (p.x - 15.0f, p.y, 30.0f, 12.0f),
                            juce::Justification::centred, false);
            }
        }

        // Elevation numeric labels
        for (int ele = -60; ele <= 60; ele += 30)
        {
            if (ele == 0) continue;
            if (ele < minAmbixEleDeg || ele > maxAmbixEleDeg) continue;
            auto p = anglesToPoint (0, -ele);
            p.applyTransform (toArea);
            g.drawText (juce::String (ele) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")),
                        juce::Rectangle<float> (p.x - 12.0f, p.y - 12.0f, 24.0f, 12.0f),
                        juce::Justification::centred, false);
        }
    }

    void resized() override
    {
        rebuildTransform();
    }

    juce::Point<float> anglesToPoint (int azimuthInDegree,
                                      int elevationInDegree)
    {
        return HammerAitov::sphericalToXY (
            Conversions<float>::degreesToRadians (azimuthInDegree),
            Conversions<float>::degreesToRadians (elevationInDegree));
    }

private:
    void rebuildTransform()
    {
        toArea = computeToArea (getLocalBounds().toFloat(),
                                minAmbixEleDeg, maxAmbixEleDeg,
                                flipLeftRight);
    }

    void rebuildPaths()
    {
        outline.clear();
        boldGrid.clear();
        regularGrid.clear();

        // Grid drawing uses the HA-math convention (+ele = down on screen) —
        // the view flips signs when projecting pucks, so here we iterate
        // HA elevation range = [-maxAmbixEle, -minAmbixEle].
        const int minHaEle = static_cast<int> (std::floor (-maxAmbixEleDeg));
        const int maxHaEle = static_cast<int> (std::ceil  (-minAmbixEleDeg));

        juce::Path* workingPath = nullptr;

        // Parallels (constant HA elevation, varying azimuth)
        for (int ele = -90; ele <= 90; ele += 30)
        {
            if (ele < minHaEle || ele > maxHaEle)
                continue;
            workingPath = (ele % 45 == 0) ? &boldGrid : &regularGrid;
            workingPath->startNewSubPath (anglesToPoint (-180, ele));
            for (int azi = -165; azi <= 180; azi += 15)
                workingPath->lineTo (anglesToPoint (azi, ele));
        }

        // Meridians (constant azimuth, varying HA elevation)
        for (int azi = -180; azi <= 180; azi += 30)
        {
            workingPath = (azi % 90 == 0) ? &boldGrid : &regularGrid;
            workingPath->startNewSubPath (anglesToPoint (azi, minHaEle));
            for (int ele = minHaEle + 5; ele <= maxHaEle; ele += 5)
                workingPath->lineTo (anglesToPoint (azi, ele));
        }

        // Outline: trace the visible region boundary. For a full sphere
        // this is the HA ellipse; for a dome it's the outer arcs plus a
        // parallel cap at the floor.
        const bool hasFloor = minHaEle > -90;
        const bool hasCeil  = maxHaEle <  90;

        outline.startNewSubPath (anglesToPoint (180, minHaEle));
        // left outer arc: azim=+180 from min to max
        for (int ele = minHaEle + 5; ele <= maxHaEle; ele += 5)
            outline.lineTo (anglesToPoint (180, ele));
        // top parallel (only if ceiling not at north pole)
        if (hasCeil)
        {
            for (int azi = 180 - 5; azi >= -180; azi -= 5)
                outline.lineTo (anglesToPoint (azi, maxHaEle));
        }
        // right outer arc: azim=-180 from max back to min
        for (int ele = maxHaEle - 5; ele >= minHaEle; ele -= 5)
            outline.lineTo (anglesToPoint (-180, ele));
        // bottom parallel (only if floor not at south pole)
        if (hasFloor)
        {
            for (int azi = -180 + 5; azi <= 180; azi += 5)
                outline.lineTo (anglesToPoint (azi, minHaEle));
        }
        outline.closeSubPath();
    }

    juce::Path outline;
    juce::Path boldGrid;
    juce::Path regularGrid;
    juce::AffineTransform toArea;

    float minAmbixEleDeg = -90.0f;
    float maxAmbixEleDeg =  90.0f;
    bool  flipLeftRight  = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HammerAitovGrid)
};
