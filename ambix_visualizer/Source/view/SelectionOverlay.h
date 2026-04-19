#pragma once

#include <JuceHeader.h>

#include "../model/EncoderSource.h"

// Draws a small translucent box at the bottom-left of `bounds` listing each
// currently-dragged source. Multi-touch drags get one line per source so the
// user can see ID / track name / azimuth / elevation at a glance.
namespace SelectionOverlay
{
    void draw (juce::Graphics& g,
               const std::vector<const EncoderSource*>& selected,
               juce::Rectangle<int> bounds);
}
