#pragma once

#include <JuceHeader.h>

// Catalogue of simple, readable symbols that can be assigned to a source.
// Each glyph is drawn into a bounding rect with strokes in a supplied colour,
// so the icon visibly sits on top of the puck's level-coloured fill.
namespace IconCatalogue
{
    enum class Icon
    {
        Default = 0, // plain puck with ID text
        Vocal,       // condenser mic (ball head on stick)
        Mic,         // handheld dynamic mic
        Speaker,     // speaker cone
        Amp,         // amp cab with two speakers
        Guitar,      // electric guitar silhouette (horizontal)
        Bass,        // upright / double bass silhouette (vertical)
        Drum,        // snare drum side view (cylinder with rods)
        Keys,        // piano keys
        Synth,       // sine wave
        Horn,        // brass horn
        Star,        // generic landmark
        Note,        // eighth note ♪
        Headphones,  // monitor headphones
        Circle,      // ring marker
        Square,      // square marker
        Triangle,    // triangle marker
        Diamond,     // diamond marker
        Plus         // plus/cross marker
    };

    juce::StringArray getNames();
    Icon              fromName (const juce::String& name);
    juce::String      toName (Icon);

    // Draws the glyph centred inside `bounds`. For `Icon::Default` it draws
    // nothing — the caller keeps whatever it was going to draw instead (e.g.
    // the source ID number).
    void drawGlyph (juce::Graphics& g,
                    Icon icon,
                    juce::Rectangle<float> bounds,
                    juce::Colour strokeColour);
}
