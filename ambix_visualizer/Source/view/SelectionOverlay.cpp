#include "SelectionOverlay.h"

namespace SelectionOverlay
{
    void draw (juce::Graphics& g,
               const std::vector<const EncoderSource*>& selected_in,
               juce::Rectangle<int> bounds)
    {
        if (selected_in.empty())
            return;

        // Stable display order so lines don't swap positions mid-drag (which
        // would cause visible flicker when lastInteractionMs changes). Sort
        // by source identity — the order stays fixed as long as the same
        // pucks are held.
        auto selected = selected_in;
        std::sort (selected.begin(), selected.end(),
                   [] (const EncoderSource* a, const EncoderSource* b)
                   { return a->key() < b->key(); });

        constexpr int lineHeight = 18;
        constexpr int padX       = 10;
        constexpr int padY       = 6;
        constexpr int boxWidth   = 360;
        constexpr int margin     = 8;

        const int boxHeight = lineHeight * static_cast<int> (selected.size()) + 2 * padY;

        juce::Rectangle<int> box (bounds.getX() + margin,
                                  bounds.getBottom() - boxHeight - margin,
                                  boxWidth,
                                  boxHeight);

        g.setColour (juce::Colours::black.withAlpha (0.72f));
        g.fillRoundedRectangle (box.toFloat(), 6.0f);
        g.setColour (juce::Colours::white.withAlpha (0.2f));
        g.drawRoundedRectangle (box.toFloat(), 6.0f, 1.0f);

        g.setFont (juce::FontOptions (13.0f));
        g.setColour (juce::Colours::white.withAlpha (0.92f));

        const juce::String degSym (juce::CharPointer_UTF8 ("\xc2\xb0"));

        for (size_t i = 0; i < selected.size(); ++i)
        {
            const auto& s = *selected[i];
            const int y = box.getY() + padY + static_cast<int> (i) * lineHeight;

            juce::String line;
            line << "#" << s.getDisplayId() << "  ";
            if (s.trackName.isNotEmpty())
                line << s.trackName << "   ";
            line << "az " << juce::String (s.azimuthDeg, 1) << degSym
                 << "  el " << juce::String (s.elevationDeg, 1) << degSym;

            g.drawText (line,
                        box.getX() + padX,
                        y,
                        box.getWidth() - 2 * padX,
                        lineHeight,
                        juce::Justification::centredLeft,
                        true);
        }
    }
}
