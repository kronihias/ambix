#pragma once
#include "JuceHeader.h"
#include <cmath>
#include <vector>

#ifndef BESSEL_APPR
#define BESSEL_APPR 8
#endif

class WideningVisualizer : public juce::Component
{
public:
    WideningVisualizer() {}

    void setParams (float modDepth, float modT, float rotOffset, bool singleSided)
    {
        if (modDepth == lastModDepth && modT == lastModT
            && rotOffset == lastRotOffset && singleSided == lastSingleSided)
            return;

        lastModDepth = modDepth;
        lastModT = modT;
        lastRotOffset = rotOffset;
        lastSingleSided = singleSided;

        recompute();
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced (2.0f);

        // Background
        g.setColour (juce::Colour (0xFF2D2D2D));
        g.fillRoundedRectangle (area, 4.0f);

        // Plot area with margins
        float plotLeft = area.getX() + 40.0f;
        float plotRight = area.getRight() - 15.0f;
        float plotTop = area.getY() + 15.0f;
        float plotBottom = area.getBottom() - 25.0f;
        float plotW = plotRight - plotLeft;
        float plotH = plotBottom - plotTop;

        if (plotW < 10.0f || plotH < 10.0f) return;

        // Grid lines — azimuth
        g.setFont (10.0f);
        for (int azDeg = -180; azDeg <= 180; azDeg += 45)
        {
            float x = plotLeft + ((float) (azDeg + 180) / 360.0f) * plotW;

            g.setColour (azDeg == 0
                         ? juce::Colours::white.withAlpha (0.25f)
                         : juce::Colours::grey.withAlpha (0.15f));
            g.drawLine (x, plotTop, x, plotBottom, 0.5f);

            // Label
            if (azDeg % 90 == 0)
            {
                g.setColour (juce::Colours::white.withAlpha (0.6f));
                g.drawText (juce::String (azDeg) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")),
                            (int) (x - 20), (int) plotBottom + 2, 40, 14,
                            juce::Justification::centred);
            }
        }

        // X-axis label
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (10.0f);
        g.drawText ("azimuth", (int) plotLeft, (int) plotBottom + 13, (int) plotW, 12,
                     juce::Justification::centred);

        // Grid lines — frequency (horizontal)
        // Show normalized frequency labels (0 = DC, 1 = Nyquist)
        for (int fi = 0; fi <= 4; ++fi)
        {
            float frac = (float) fi / 4.0f;
            float y = plotBottom - frac * plotH;

            g.setColour (juce::Colours::grey.withAlpha (0.12f));
            g.drawLine (plotLeft, y, plotRight, y, 0.5f);
        }

        // Y-axis label
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (10.0f);

        // Rotated "frequency" label on left
        g.saveState();
        g.addTransform (juce::AffineTransform::rotation (
            -juce::MathConstants<float>::halfPi,
            area.getX() + 12.0f, plotTop + plotH * 0.5f));
        g.drawText ("frequency", (int) (area.getX() + 12.0f - plotH * 0.5f),
                     (int) (plotTop + plotH * 0.5f - 6), (int) plotH, 12,
                     juce::Justification::centred);
        g.restoreState();

        // Y-axis tick labels (left side)
        g.setFont (9.0f);
        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.drawText ("0", (int) (plotLeft - 25), (int) (plotBottom - 6), 22, 12,
                     juce::Justification::centredRight);
        g.drawText ("fs/2", (int) (plotLeft - 32), (int) (plotTop - 6), 30, 12,
                     juce::Justification::centredRight);

        // Draw the point cloud
        if (points.empty()) return;

        for (const auto& pt : points)
        {
            // Map azimuth angle (-pi..pi) to x
            float azDeg = pt.azimuthDeg;
            float x = plotLeft + ((azDeg + 180.0f) / 360.0f) * plotW;

            // Map normalized frequency (0..1) to y (bottom=0, top=1)
            float y = plotBottom - pt.freqNorm * plotH;

            // Size and alpha based on magnitude; smaller dots for single-sided
            float mag = pt.magnitude;
            float alpha = juce::jlimit (0.08f, 0.95f, mag * 1.5f);
            float radius = pt.singleSided ? (1.0f + 2.0f * mag) : (1.5f + 3.5f * mag);

            g.setColour (juce::Colour (0xFF4488FF).withAlpha (alpha));
            g.fillEllipse (x - radius, y - radius, radius * 2.0f, radius * 2.0f);
        }
    }

    void resized() override
    {
        recompute();
    }

private:
    struct PointData
    {
        float azimuthDeg;
        float freqNorm;   // 0..1 (DC to Nyquist)
        float magnitude;  // 0..1
        bool singleSided; // smaller dots when single-sided
    };

    std::vector<PointData> points;

    float lastModDepth = -1.0f;
    float lastModT = -1.0f;
    float lastRotOffset = -1.0f;
    bool lastSingleSided = false;

    void recompute()
    {
        points.clear();

        double phi_hat = (double) lastModDepth * 2.0 * juce::MathConstants<double>::pi;
        double rot_offset = juce::MathConstants<double>::pi
                            - ((double) lastRotOffset + 0.002) * 2.0 * juce::MathConstants<double>::pi;

        // Compute Bessel coefficients for m=1
        float cosCoeff[BESSEL_APPR + 1];
        float sinCoeff[BESSEL_APPR + 1];

        for (int j = 0; j <= BESSEL_APPR; ++j)
        {
            double bessel = jn (j, phi_hat);
            cosCoeff[j] = (float) (std::cos (juce::MathConstants<double>::halfPi * j + rot_offset) * bessel);
            sinCoeff[j] = (float) (std::sin (juce::MathConstants<double>::halfPi * j + rot_offset) * bessel);
        }

        // Q in samples — use a representative value for visualization
        // We use modT to derive Q; actual Q = (modT * 0.0698 + 0.0002) * sampleRate
        // For visualization we just need Q*omega products, so we parameterize by normalized freq
        // omega = 2*pi*f/fs, and Q*omega = 2*pi*f*Q/fs
        // We sample normalized frequency f/fs from 0 to 0.5 (DC to Nyquist)
        // At each freq, compute the effective transfer function H_cos and H_sin

        // Use Q = (modT * 0.0698 + 0.0002) * 44100 as representative
        double Q = (lastModT * 0.0698 + 0.0002) * 44100.0;

        int numFreqPoints = 400;
        bool singleSided = lastSingleSided;

        for (int fi = 0; fi < numFreqPoints; ++fi)
        {
            // Normalized frequency: 0 to 0.5 (DC to Nyquist)
            double fNorm = (double) fi / (double) (numFreqPoints - 1) * 0.5;
            double omega = 2.0 * juce::MathConstants<double>::pi * fNorm; // omega = 2*pi*f/fs

            // Always use symmetric (zero-phase) formula for azimuth visualization
            // H_cos(omega) = cosCoeff[0] + 2 * sum_{j=1}^{8} cosCoeff[j] * cos(j*Q*omega)
            double Hcos = cosCoeff[0];
            double Hsin = sinCoeff[0];
            for (int j = 1; j <= BESSEL_APPR; ++j)
            {
                double cosOmega = std::cos (j * Q * omega);
                Hcos += 2.0 * cosCoeff[j] * cosOmega;
                Hsin += 2.0 * sinCoeff[j] * cosOmega;
            }

            double angle = std::atan2 (Hsin, Hcos);
            double mag = std::sqrt (Hcos * Hcos + Hsin * Hsin);

            points.push_back ({
                (float) (-angle * 180.0 / juce::MathConstants<double>::pi),
                (float) (fNorm * 2.0), // 0..1
                juce::jlimit (0.0f, 1.0f, (float) mag),
                singleSided
            });
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WideningVisualizer)
};
