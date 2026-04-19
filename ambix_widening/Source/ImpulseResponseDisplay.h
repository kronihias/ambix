#pragma once
#ifdef _MSC_VER
#define jn _jn
#endif
#include "JuceHeader.h"
#include <cmath>

#ifndef BESSEL_APPR
#define BESSEL_APPR 8
#endif

class ImpulseResponseDisplay : public juce::Component
{
public:
    ImpulseResponseDisplay() {}

    void setParams (float modDepth, float modT, bool singleSided)
    {
        if (modDepth == lastModDepth && modT == lastModT && singleSided == lastSingleSided)
            return;

        lastModDepth = modDepth;
        lastModT = modT;
        lastSingleSided = singleSided;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced (2.0f);

        // Background
        g.setColour (juce::Colour (0xFF000000));
        g.fillRoundedRectangle (area, 4.0f);

        float tMs = lastModT * 69.8f + 0.2f; // tap spacing in ms
        double phi_hat = (double) lastModDepth * 2.0 * juce::MathConstants<double>::pi;

        // Always show full symmetric range (17 taps), regardless of single-sided
        int fullFirLength = 2 * BESSEL_APPR + 1;
        int center = BESSEL_APPR;

        // Fixed axis range
        float maxTimeMs = BESSEL_APPR * tMs;
        float minTimeMs = -maxTimeMs;
        float timeRange = maxTimeMs - minTimeMs;
        if (timeRange < 0.01f) timeRange = 1.0f;

        // Plot area with margins for labels
        float plotLeft = area.getX() + 35.0f;
        float plotRight = area.getRight() - 10.0f;
        float plotTop = area.getY() + 10.0f;
        float plotBottom = area.getBottom() - 24.0f;
        float plotW = plotRight - plotLeft;
        float plotH = plotBottom - plotTop;
        float zeroY = plotTop + plotH * 0.5f;

        // Zero line
        g.setColour (juce::Colours::grey.withAlpha (0.4f));
        g.drawLine (plotLeft, zeroY, plotRight, zeroY, 0.5f);

        // Vertical center line (t=0)
        float centerX = plotLeft + 0.5f * plotW;
        g.setColour (juce::Colours::grey.withAlpha (0.2f));
        g.drawLine (centerX, plotTop, centerX, plotBottom, 0.5f);

        // Time axis ticks and labels
        g.setFont (11.0f);
        for (int i = 0; i < fullFirLength; ++i)
        {
            float timeMs = (float) (i - center) * tMs;
            float x = plotLeft + ((timeMs - minTimeMs) / timeRange) * plotW;

            // Tick mark
            g.setColour (juce::Colours::grey.withAlpha (0.3f));
            g.drawLine (x, plotBottom, x, plotBottom + 3.0f, 0.5f);

            // Label every other tap, or all if <=9 visible
            if (fullFirLength <= 9 || i % 2 == 0)
            {
                g.setColour (juce::Colours::white.withAlpha (0.6f));
                juce::String label;
                if (std::abs (timeMs) < 0.01f)
                    label = "0";
                else
                    label = juce::String (timeMs, 1);
                g.drawText (label, (int) (x - 22), (int) plotBottom + 3, 44, 14,
                            juce::Justification::centred);
            }
        }

        // "time (ms)" axis label centered below
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.setFont (11.0f);
        g.drawText ("time (ms)", (int) plotLeft, (int) plotBottom + 10, (int) plotW, 14,
                     juce::Justification::centred);

        // Active FIR taps
        int activeFirLength = lastSingleSided ? (BESSEL_APPR + 1) : fullFirLength;
        int activeCenter = lastSingleSided ? 0 : BESSEL_APPR;

        // Draw stems for ALL positions (full range), but grey out inactive ones
        for (int i = 0; i < fullFirLength; ++i)
        {
            int j = std::abs (i - BESSEL_APPR);
            double bessel = jn (j, phi_hat);
            float amp = (float) bessel;

            float timeMs = (float) (i - center) * tMs;
            float x = plotLeft + ((timeMs - minTimeMs) / timeRange) * plotW;
            float stemTop = zeroY - amp * (plotH * 0.45f);

            // Check if this tap is active in current mode
            bool isActive;
            if (lastSingleSided)
                isActive = (i >= BESSEL_APPR); // only taps at t >= 0
            else
                isActive = true;

            if (isActive)
            {
                float alpha = juce::jlimit (0.4f, 1.0f, std::abs (amp) * 3.0f);
                g.setColour (juce::Colours::white.withAlpha (alpha));
            }
            else
            {
                g.setColour (juce::Colours::grey.withAlpha (0.25f));
            }

            // Stem line
            g.drawLine (x, zeroY, x, stemTop, isActive ? 2.0f : 1.0f);

            // Circle at top
            float r = isActive ? 3.5f : 2.5f;
            g.fillEllipse (x - r, stemTop - r, r * 2.0f, r * 2.0f);
        }

        // Y-axis labels
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.setFont (11.0f);
        g.drawText ("1", (int) area.getX() + 2, (int) plotTop - 2, 28, 14,
                     juce::Justification::centredRight);
        g.drawText ("-1", (int) area.getX() + 2, (int) plotBottom - 12, 28, 14,
                     juce::Justification::centredRight);
    }

private:
    float lastModDepth = -1.0f;
    float lastModT = -1.0f;
    bool lastSingleSided = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImpulseResponseDisplay)
};
