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
        // Ambix sources stream RMS continuously; when audio stops the stored
        // value would otherwise read as "still playing" forever. Decay it
        // towards silence past a short grace, keyed off lastLevelUpdateMs
        // (so position-only OSC packets carrying frozen levels don't keep
        // it lit). MultiEncoder's "rms" is really a gain setting that only
        // updates on user interaction, so leave it frozen there.
        const auto nowMsRms = juce::Time::getMillisecondCounter();
        const float effectiveRms = (source.origin == EncoderOrigin::Ambix)
            ? LevelMapping::rmsEffective (source.rmsLinear, source.lastLevelUpdateMs, nowMsRms)
            : source.rmsLinear;
        const float rmsT   = LevelMapping::linearToLevelT (effectiveRms, settings.minDb);
        const auto  fill   = LevelMapping::sampleColorMap (settings.colorMap, rmsT)
                                 .withMultipliedAlpha (isBackHemisphere ? 0.4f : 1.0f);

        const bool showPeakRing = (source.origin == EncoderOrigin::Ambix);
        juce::Colour ring = juce::Colours::transparentWhite;
        if (showPeakRing)
        {
            const auto nowMs = juce::Time::getMillisecondCounter();
            // Same stale-OSC fade applied to the RMS — keeps the peak ring in
            // sync with the meter so both fall silent together.
            const float fade = LevelMapping::staleFadeFactor (source.lastLevelUpdateMs, nowMs);
            const float effPeak = LevelMapping::peakHoldEffective (source.peakHoldLinear,
                                                                   source.peakHoldTimeMs,
                                                                   nowMs) * fade;
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

        // Vertical level meter beside the puck (ambix_encoder only — other
        // origins don't carry RMS/peak). Segmented green→yellow→orange→red
        // ramp, Max/spat-style. Height = puck diameter so the meter scales
        // with the user's puck-size preference; width ~30% of the puck radius
        // with a 4 px minimum so it stays legible on small pucks.
        if (settings.showPuckLevelMeter && source.origin == EncoderOrigin::Ambix)
        {
            constexpr int   kNumSegments = 10;
            constexpr float kFloorDb     = -48.0f;
            constexpr float kCeilingDb   =  +3.0f;

            const float meterW = juce::jmax (4.0f, radiusPx * 0.30f);
            const float meterH = radiusPx * 2.0f;
            const float meterX = centre.x + radiusPx + 4.0f;
            const float meterY = centre.y - radiusPx;

            const juce::Rectangle<float> meterBounds (meterX, meterY, meterW, meterH);

            // Dim backdrop so the unlit meter is still visible against the
            // dark background.
            g.setColour (juce::Colours::black.withAlpha (isBackHemisphere ? 0.25f : 0.45f));
            g.fillRect (meterBounds);
            g.setColour (juce::Colours::white.withAlpha (isBackHemisphere ? 0.12f : 0.25f));
            g.drawRect (meterBounds, 1.0f);

            auto thresholdDb = [] (int seg) noexcept
            {
                const float t = static_cast<float> (seg + 1)
                              / static_cast<float> (kNumSegments);
                return juce::jmap (t, kFloorDb, kCeilingDb);
            };
            auto segmentColour = [] (float thrDb) noexcept
            {
                if (thrDb >=  -1.0f) return juce::Colour (0xFFE04030); // red
                if (thrDb >=  -6.0f) return juce::Colour (0xFFE08A30); // orange
                if (thrDb >= -18.0f) return juce::Colour (0xFFE0C94B); // yellow
                return juce::Colour (0xFF52B03B);                      // green
            };

            const auto nowMs = juce::Time::getMillisecondCounter();
            const float fade  = LevelMapping::staleFadeFactor (source.lastLevelUpdateMs, nowMs);
            const float rmsDb = LevelMapping::linearToDb (source.rmsLinear * fade);
            const float peakL = LevelMapping::peakHoldEffective (source.peakHoldLinear,
                                                                 source.peakHoldTimeMs,
                                                                 nowMs) * fade;
            const float peakDb = LevelMapping::linearToDb (peakL);

            const float gap  = 1.0f;
            const float segH = (meterH - gap * (kNumSegments - 1)) / kNumSegments;
            const int peakIdx = juce::jlimit (0, kNumSegments - 1,
                                              static_cast<int> (std::floor (
                                                  (peakDb - kFloorDb)
                                                  / ((kCeilingDb - kFloorDb) / kNumSegments))));
            const float backAlpha = isBackHemisphere ? 0.4f : 1.0f;

            for (int i = 0; i < kNumSegments; ++i)
            {
                const float thr = thresholdDb (i);
                const auto  col = segmentColour (thr).withMultipliedAlpha (backAlpha);
                // i = 0 at bottom, top segment is (kNumSegments - 1)
                const float y = meterY + meterH - (i + 1) * segH - i * gap;
                const juce::Rectangle<float> r (meterX, y, meterW, segH);

                const bool lit      = rmsDb >= thr - 0.001f;
                const bool peakHere = (i == peakIdx) && peakL > 0.0005f;

                if (peakHere)
                {
                    g.setColour (col.brighter (0.6f));
                    g.fillRect (r);
                }
                else if (lit)
                {
                    g.setColour (col);
                    g.fillRect (r);
                }
                else
                {
                    g.setColour (col.withAlpha (0.18f * backAlpha));
                    g.fillRect (r);
                }
            }
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
