#include "PuckRenderer.h"
#include "IconCatalogue.h"
#include "LevelMapping.h"

namespace PuckRenderer
{
    void draw (juce::Graphics& g,
               const EncoderSource& source,
               juce::Point<float> centre,
               float radiusPx,
               bool isSelected,
               bool isBackHemisphere,
               const AppSettings& settings)
    {
        const float rmsT   = LevelMapping::linearToLevelT (source.rmsLinear, settings.minDb);
        const auto  fill   = LevelMapping::sampleColorMap (settings.colorMap, rmsT)
                                 .withMultipliedAlpha (isBackHemisphere ? 0.4f : 1.0f);

        const bool showPeakRing = (source.origin == EncoderOrigin::Ambix);
        juce::Colour ring = juce::Colours::transparentWhite;
        if (showPeakRing)
        {
            const auto nowMs = juce::Time::getMillisecondCounter();
            const float effPeak = LevelMapping::peakHoldEffective (source.peakHoldLinear,
                                                                   source.peakHoldTimeMs,
                                                                   nowMs);
            const float peakDb = LevelMapping::linearToDb (effPeak);
            ring = LevelMapping::peakRingColour (peakDb);
        }

        const juce::Rectangle<float> puckRect (centre.x - radiusPx,
                                               centre.y - radiusPx,
                                               radiusPx * 2.0f,
                                               radiusPx * 2.0f);

        // Selection halo
        if (isSelected)
        {
            g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.fillEllipse (puckRect.expanded (5.0f));
        }

        // Peak ring (ambix only)
        const float ringExpand = 4.0f;
        if (showPeakRing && ring.getAlpha() > 0)
        {
            g.setColour (ring);
            g.drawEllipse (puckRect.expanded (ringExpand), 2.5f);
        }

        // Muted indicator: dashed outer ring
        if (source.muted)
        {
            g.setColour (juce::Colours::red.withAlpha (isBackHemisphere ? 0.4f : 0.85f));
            const auto mRect = puckRect.expanded (ringExpand);
            float dashes[] = { 4.0f, 4.0f };
            juce::Path circ;
            circ.addEllipse (mRect);
            juce::PathStrokeType (1.8f).createDashedStroke (circ, circ, dashes, 2);
            g.strokePath (circ, juce::PathStrokeType (1.8f));
        }

        // Puck body
        g.setColour (fill);
        g.fillEllipse (puckRect);

        // Puck border
        g.setColour (juce::Colours::black.withAlpha (isBackHemisphere ? 0.3f : 0.6f));
        g.drawEllipse (puckRect, 1.0f);

        // Icon glyph replaces the ID-inside-puck when the user picks one;
        // when it's Default, we fall back to drawing the ID number inside.
        const auto glyphColour = fill.contrasting (0.7f);
        if (source.icon != IconCatalogue::Icon::Default)
        {
            IconCatalogue::drawGlyph (g, source.icon, puckRect.reduced (radiusPx * 0.2f), glyphColour);
        }
        else
        {
            g.setColour (glyphColour);
            const float fontSize = juce::jmax (9.0f, radiusPx * 0.9f);
            g.setFont (juce::FontOptions (fontSize, juce::Font::bold));
            g.drawText (juce::String (source.getDisplayId()),
                        puckRect,
                        juce::Justification::centred,
                        false);
        }

        // Track / source name below puck. When an icon replaced the ID inside
        // the puck, prepend "#id  " so the id is still visible.
        const auto label = source.getDisplayLabel();
        const juce::String finalLabel = (source.icon != IconCatalogue::Icon::Default
                                          ? juce::String ("#") + juce::String (source.getDisplayId()) + "  " + label
                                          : label);
        if (finalLabel.isNotEmpty())
        {
            const auto nameRect = juce::Rectangle<float> (centre.x - 70.0f,
                                                          centre.y + radiusPx + ringExpand + 2.0f,
                                                          140.0f,
                                                          14.0f);
            g.setColour (juce::Colours::white.withAlpha (isBackHemisphere ? 0.3f : 0.85f));
            g.setFont (juce::FontOptions (11.0f));
            g.drawText (finalLabel, nameRect, juce::Justification::centredTop, true);
        }
    }
}
