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
#include "HammerAitov.h"
#include "Conversions.h"

//==============================================================================
/*
*/
class HammerAitovGrid : public juce::Component
{
public:
    HammerAitovGrid()
    {
        setBufferedToImage (true);

        //calculating path prototype
        outline.clear();
        boldGrid.clear();
        regularGrid.clear();

        juce::Path* workingPath;

        // vertical lines
        for (int ele = -90; ele <= 90; ele += 30)
        {
            if (ele % 45 == 0)
                workingPath = &boldGrid;
            else
                workingPath = &regularGrid;

            workingPath->startNewSubPath (anglesToPoint (-180, ele));
            for (int azi = -165; azi <= 180; azi += 15)
            {
                workingPath->lineTo (anglesToPoint (azi, ele));
            }
        }

        // horizontal lines
        for (int azi = -180; azi <= 180; azi += 30)
        {
            if (azi % 90 == 0)
                workingPath = &boldGrid;
            else
                workingPath = &regularGrid;

            workingPath->startNewSubPath (anglesToPoint (azi, -90));
            for (int ele = -85; ele <= 90; ele += 5)
            {
                workingPath->lineTo (anglesToPoint (azi, ele));
            }
        }

        outline.startNewSubPath (anglesToPoint (180, -90));
        for (int ele = -85; ele <= 90; ele += 5)
        {
            outline.lineTo (anglesToPoint (180, ele));
        }
        for (int ele = 85; ele >= -85; ele -= 5)
        {
            outline.lineTo (anglesToPoint (-180, ele));
        }
        outline.closeSubPath();
    }

    ~HammerAitovGrid() override = default;

    void paint (juce::Graphics& g) override
    {
        juce::Path path;

        // Fill only inside the globe outline with a dark background
        path = outline;
        path.applyTransform (toArea);
        g.setColour (juce::Colour (0xFF2D2D2D));
        g.fillPath (path);

        g.setColour (juce::Colours::white);

        path = boldGrid;
        path.applyTransform (toArea);
        g.strokePath (path, juce::PathStrokeType (1.0f));

        path = regularGrid;
        path.applyTransform (toArea);
        g.strokePath (path, juce::PathStrokeType (0.2f));

        // text
        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 1)));
        g.setFont (12.0f);
        juce::Point<float> textPos;

        textPos = anglesToPoint (0, 0);
        textPos.applyTransform (toArea);
        g.drawText ("FRONT", textPos.x - 22, textPos.y - 12, 44, 12, juce::Justification::centred);

        textPos = anglesToPoint (90, 0);
        textPos.applyTransform (toArea);
        g.drawText ("RIGHT", textPos.x - 40, textPos.y - 12, 44, 12, juce::Justification::centred);

        textPos = anglesToPoint (-90, 0);
        textPos.applyTransform (toArea);
        g.drawText ("LEFT", textPos.x - 4, textPos.y - 12, 44, 12, juce::Justification::centred);

        textPos = anglesToPoint (180, 0);
        textPos.applyTransform (toArea);
        g.drawText ("BACK", textPos.x - 38, textPos.y - 12, 40, 12, juce::Justification::centred);

        textPos = anglesToPoint (-180, 0);
        textPos.applyTransform (toArea);
        g.drawText ("BACK", textPos.x - 2, textPos.y - 12, 40, 12, juce::Justification::centred);

        textPos = anglesToPoint (0, -90);
        textPos.applyTransform (toArea);
        g.drawText ("TOP", textPos.x - 15, textPos.y - 12, 30, 12, juce::Justification::centred);

        textPos = anglesToPoint (0, 90);
        textPos.applyTransform (toArea);
        g.drawText ("BOTTOM", textPos.x - 25, textPos.y, 50, 12, juce::Justification::centred);

        g.setFont (getLookAndFeel().getTypefaceForFont (juce::Font (12.0f, 2)));
        g.setFont (12.0f);

        // azimuth labels
        for (int azi = -150; azi <= 150; azi += 30)
        {
            textPos = anglesToPoint (azi, 0);
            textPos.applyTransform (toArea);
            g.drawText (juce::String (azi) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")),
                        textPos.x - 20,
                        textPos.y,
                        40,
                        12,
                        juce::Justification::centred);
        }

        // elevation labels
        for (int ele = -60; ele <= 60; ele += 30)
        {
            if (ele != 0)
            {
                textPos = anglesToPoint (0, -ele);
                textPos.applyTransform (toArea);
                g.drawText (juce::String (ele) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")),
                            textPos.x + 2,
                            textPos.y - 12,
                            35,
                            12,
                            juce::Justification::centredLeft);
            }
        }
    }

    void resized() override
    {
        juce::Rectangle<int> area = getLocalBounds();

        toArea = juce::AffineTransform::fromTargetPoints (area.getCentreX(),
                                                          area.getCentreY(),
                                                          area.getRight() - 10.0f,
                                                          area.getCentreY(),
                                                          area.getCentreX(),
                                                          area.getBottom() - 20.0f);
    }

    juce::Point<float> anglesToPoint (int azimuthInDegree,
                                      int elevationInDegree) //hammer-aitov-projection
    {
        // Negate azimuth for clockwise-positive display convention
        return HammerAitov::sphericalToXY (
            Conversions<float>::degreesToRadians (-azimuthInDegree),
            Conversions<float>::degreesToRadians (elevationInDegree));
    }

private:
    juce::Path outline;
    juce::Path boldGrid;
    juce::Path regularGrid;
    juce::AffineTransform toArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HammerAitovGrid)
};
