#include "IconCatalogue.h"

namespace IconCatalogue
{
    juce::StringArray getNames()
    {
        return { "default", "vocal", "mic", "speaker", "amp", "guitar", "bass",
                 "drum", "keys", "synth", "horn", "star",
                 "note", "headphones", "circle", "square", "triangle", "diamond", "plus" };
    }

    Icon fromName (const juce::String& name)
    {
        const auto n = name.trim().toLowerCase();
        if (n == "vocal")      return Icon::Vocal;
        if (n == "mic")        return Icon::Mic;
        if (n == "speaker")    return Icon::Speaker;
        if (n == "amp")        return Icon::Amp;
        if (n == "guitar")     return Icon::Guitar;
        if (n == "bass")       return Icon::Bass;
        if (n == "drum")       return Icon::Drum;
        if (n == "keys")       return Icon::Keys;
        if (n == "synth")      return Icon::Synth;
        if (n == "horn")       return Icon::Horn;
        if (n == "star")       return Icon::Star;
        if (n == "note")       return Icon::Note;
        if (n == "headphones") return Icon::Headphones;
        if (n == "circle")     return Icon::Circle;
        if (n == "square")     return Icon::Square;
        if (n == "triangle")   return Icon::Triangle;
        if (n == "diamond")    return Icon::Diamond;
        if (n == "plus")       return Icon::Plus;
        return Icon::Default;
    }

    juce::String toName (Icon i)
    {
        switch (i)
        {
            case Icon::Vocal:      return "vocal";
            case Icon::Mic:        return "mic";
            case Icon::Speaker:    return "speaker";
            case Icon::Amp:        return "amp";
            case Icon::Guitar:     return "guitar";
            case Icon::Bass:       return "bass";
            case Icon::Drum:       return "drum";
            case Icon::Keys:       return "keys";
            case Icon::Synth:      return "synth";
            case Icon::Horn:       return "horn";
            case Icon::Star:       return "star";
            case Icon::Note:       return "note";
            case Icon::Headphones: return "headphones";
            case Icon::Circle:     return "circle";
            case Icon::Square:     return "square";
            case Icon::Triangle:   return "triangle";
            case Icon::Diamond:    return "diamond";
            case Icon::Plus:       return "plus";
            case Icon::Default:    break;
        }
        return "default";
    }

    // Helpers: each path is built for the unit square (0,0)..(1,1) then
    // transformed to fit `bounds`.
    namespace
    {
        juce::Path unitVocal()
        {
            juce::Path p;
            // Ball head (ellipse) and stand
            const float cx = 0.5f, cyHead = 0.30f;
            const float rx = 0.18f, ry = 0.22f;
            p.addEllipse (cx - rx, cyHead - ry, rx * 2, ry * 2);
            // Stem from bottom of head down
            p.startNewSubPath (cx, cyHead + ry);
            p.lineTo (cx, 0.80f);
            // Base
            p.startNewSubPath (cx - 0.20f, 0.80f);
            p.lineTo (cx + 0.20f, 0.80f);
            return p;
        }

        juce::Path unitMic()
        {
            juce::Path p;
            // Handheld mic: rounded rect body + grille + stand
            p.addRoundedRectangle (0.30f, 0.10f, 0.40f, 0.45f, 0.18f);
            // Grille lines (horizontal)
            for (int i = 1; i <= 3; ++i)
            {
                const float y = 0.18f + i * 0.07f;
                p.startNewSubPath (0.35f, y);
                p.lineTo (0.65f, y);
            }
            // Stand
            p.startNewSubPath (0.50f, 0.55f);
            p.lineTo (0.50f, 0.85f);
            p.startNewSubPath (0.35f, 0.85f);
            p.lineTo (0.65f, 0.85f);
            return p;
        }

        juce::Path unitSpeaker()
        {
            juce::Path p;
            // Cone pointing right. Trapezoid + inner voice coil.
            p.startNewSubPath (0.15f, 0.35f);
            p.lineTo (0.40f, 0.35f);
            p.lineTo (0.75f, 0.15f);
            p.lineTo (0.75f, 0.85f);
            p.lineTo (0.40f, 0.65f);
            p.lineTo (0.15f, 0.65f);
            p.closeSubPath();
            // sound-wave arc
            p.startNewSubPath (0.80f, 0.30f);
            p.quadraticTo   (0.95f, 0.50f, 0.80f, 0.70f);
            return p;
        }

        juce::Path unitAmp()
        {
            juce::Path p;
            // Outer cabinet
            p.addRoundedRectangle (0.15f, 0.20f, 0.70f, 0.60f, 0.06f);
            // Two speaker circles
            p.addEllipse (0.23f, 0.35f, 0.22f, 0.22f);
            p.addEllipse (0.55f, 0.35f, 0.22f, 0.22f);
            // Control strip
            p.startNewSubPath (0.25f, 0.72f);
            p.lineTo (0.75f, 0.72f);
            return p;
        }

        juce::Path unitGuitar()
        {
            juce::Path p;
            // Acoustic-guitar silhouette, body on the left, neck extending
            // diagonally to the upper right with a small headstock.
            // Body figure-8 — two overlapping ellipses make the classic shape.
            p.addEllipse (0.08f, 0.40f, 0.42f, 0.52f); // lower bout
            p.addEllipse (0.14f, 0.20f, 0.30f, 0.32f); // upper bout
            // Sound hole
            p.addEllipse (0.22f, 0.54f, 0.14f, 0.14f);
            // Neck: thick diagonal slab running up to the right
            p.startNewSubPath (0.42f, 0.22f);
            p.lineTo (0.82f, 0.08f);
            p.lineTo (0.86f, 0.18f);
            p.lineTo (0.46f, 0.32f);
            p.closeSubPath();
            // Headstock: small perpendicular cap at the neck end
            p.addRectangle (0.82f, 0.02f, 0.14f, 0.10f);
            return p;
        }

        juce::Path unitBass()
        {
            juce::Path p;
            // Upright / double bass — clearly distinct from the guitar by
            // orientation. Tall violin-shaped body, vertical neck, scroll on top.
            // Body (vertical ellipse)
            p.addEllipse (0.28f, 0.38f, 0.44f, 0.52f);
            // f-holes — two short slashes either side of the bridge
            p.startNewSubPath (0.38f, 0.58f);
            p.lineTo (0.38f, 0.70f);
            p.startNewSubPath (0.62f, 0.58f);
            p.lineTo (0.62f, 0.70f);
            // Bridge
            p.startNewSubPath (0.40f, 0.76f);
            p.lineTo (0.60f, 0.76f);
            // Neck: vertical rectangle from body up
            p.addRectangle (0.44f, 0.12f, 0.12f, 0.30f);
            // Scroll at top (little curl)
            p.addEllipse (0.40f, 0.02f, 0.20f, 0.14f);
            return p;
        }

        juce::Path unitDrum()
        {
            juce::Path p;
            // Snare drum, side view. Cylinder body with tension rods — clearly
            // reads as "drum" even at puck scale, unlike a plain circle.
            // Top head (shallow ellipse)
            p.addEllipse (0.12f, 0.24f, 0.76f, 0.14f);
            // Shell sides
            p.startNewSubPath (0.12f, 0.31f);
            p.lineTo (0.12f, 0.69f);
            p.startNewSubPath (0.88f, 0.31f);
            p.lineTo (0.88f, 0.69f);
            // Bottom rim (shallow arc, just the front half)
            p.startNewSubPath (0.12f, 0.69f);
            p.quadraticTo (0.50f, 0.80f, 0.88f, 0.69f);
            // Tension rods — four vertical lines around the shell
            for (int i = 0; i < 4; ++i)
            {
                const float x = 0.24f + static_cast<float> (i) * 0.17f;
                p.startNewSubPath (x, 0.34f);
                p.lineTo (x, 0.70f);
            }
            return p;
        }

        juce::Path unitNote()
        {
            juce::Path p;
            // Eighth note: slanted notehead + stem + flag.
            // Notehead (tilted ellipse-ish — just use an ellipse for clarity)
            p.addEllipse (0.18f, 0.62f, 0.30f, 0.22f);
            // Stem (from right edge of notehead up)
            p.startNewSubPath (0.46f, 0.72f);
            p.lineTo (0.46f, 0.12f);
            // Flag
            p.quadraticTo (0.72f, 0.20f, 0.66f, 0.46f);
            return p;
        }

        juce::Path unitHeadphones()
        {
            juce::Path p;
            // Headband (top arc)
            p.startNewSubPath (0.15f, 0.55f);
            p.quadraticTo (0.50f, 0.05f, 0.85f, 0.55f);
            // Left ear cup
            p.addRoundedRectangle (0.10f, 0.52f, 0.18f, 0.30f, 0.05f);
            // Right ear cup
            p.addRoundedRectangle (0.72f, 0.52f, 0.18f, 0.30f, 0.05f);
            return p;
        }

        juce::Path unitCircle()
        {
            juce::Path p;
            p.addEllipse (0.15f, 0.15f, 0.70f, 0.70f);
            return p;
        }

        juce::Path unitSquare()
        {
            juce::Path p;
            p.addRectangle (0.18f, 0.18f, 0.64f, 0.64f);
            return p;
        }

        juce::Path unitTriangle()
        {
            juce::Path p;
            // Equilateral, point up
            p.startNewSubPath (0.50f, 0.12f);
            p.lineTo (0.88f, 0.82f);
            p.lineTo (0.12f, 0.82f);
            p.closeSubPath();
            return p;
        }

        juce::Path unitDiamond()
        {
            juce::Path p;
            p.startNewSubPath (0.50f, 0.10f);
            p.lineTo (0.88f, 0.50f);
            p.lineTo (0.50f, 0.90f);
            p.lineTo (0.12f, 0.50f);
            p.closeSubPath();
            return p;
        }

        juce::Path unitPlus()
        {
            juce::Path p;
            // Two thick bars meeting in the middle (drawn as a single stroked
            // cross so the stroke width gives it presence).
            p.startNewSubPath (0.50f, 0.15f);
            p.lineTo (0.50f, 0.85f);
            p.startNewSubPath (0.15f, 0.50f);
            p.lineTo (0.85f, 0.50f);
            return p;
        }

        juce::Path unitKeys()
        {
            juce::Path p;
            // 7 white-key verticals + 3 black-key verticals on top
            for (int i = 0; i < 7; ++i)
            {
                const float x = 0.15f + i * 0.10f;
                p.startNewSubPath (x, 0.25f);
                p.lineTo (x, 0.85f);
            }
            // Top horizontal
            p.startNewSubPath (0.15f, 0.25f);
            p.lineTo (0.75f, 0.25f);
            // Bottom horizontal
            p.startNewSubPath (0.15f, 0.85f);
            p.lineTo (0.75f, 0.85f);
            // Black keys (shortened)
            const float xs[] = { 0.22f, 0.32f, 0.52f, 0.62f, 0.72f };
            for (float x : xs)
            {
                p.addRectangle (x - 0.02f, 0.28f, 0.04f, 0.30f);
            }
            return p;
        }

        juce::Path unitSynth()
        {
            juce::Path p;
            // Sine wave
            constexpr int samples = 40;
            for (int i = 0; i <= samples; ++i)
            {
                const float t = static_cast<float> (i) / samples;
                const float x = 0.10f + t * 0.80f;
                const float y = 0.50f + 0.30f * std::sin (t * juce::MathConstants<float>::twoPi);
                if (i == 0) p.startNewSubPath (x, y);
                else        p.lineTo (x, y);
            }
            return p;
        }

        juce::Path unitHorn()
        {
            juce::Path p;
            // Horn bell (right side) + tube
            p.startNewSubPath (0.15f, 0.45f);
            p.lineTo (0.55f, 0.45f);
            p.lineTo (0.55f, 0.30f);
            p.lineTo (0.90f, 0.15f);
            p.lineTo (0.90f, 0.85f);
            p.lineTo (0.55f, 0.70f);
            p.lineTo (0.55f, 0.55f);
            p.lineTo (0.15f, 0.55f);
            p.closeSubPath();
            return p;
        }

        juce::Path unitStar()
        {
            juce::Path p;
            constexpr int points = 5;
            const float cx = 0.5f, cy = 0.5f;
            const float outer = 0.40f, inner = 0.18f;
            for (int i = 0; i <= points * 2; ++i)
            {
                const float angle = (float) i * juce::MathConstants<float>::pi / points
                                    - juce::MathConstants<float>::halfPi;
                const float r = (i % 2 == 0) ? outer : inner;
                const float x = cx + r * std::cos (angle);
                const float y = cy + r * std::sin (angle);
                if (i == 0) p.startNewSubPath (x, y);
                else        p.lineTo (x, y);
            }
            p.closeSubPath();
            return p;
        }

        juce::Path unitPath (Icon icon)
        {
            switch (icon)
            {
                case Icon::Vocal:      return unitVocal();
                case Icon::Mic:        return unitMic();
                case Icon::Speaker:    return unitSpeaker();
                case Icon::Amp:        return unitAmp();
                case Icon::Guitar:     return unitGuitar();
                case Icon::Bass:       return unitBass();
                case Icon::Drum:       return unitDrum();
                case Icon::Keys:       return unitKeys();
                case Icon::Synth:      return unitSynth();
                case Icon::Horn:       return unitHorn();
                case Icon::Star:       return unitStar();
                case Icon::Note:       return unitNote();
                case Icon::Headphones: return unitHeadphones();
                case Icon::Circle:     return unitCircle();
                case Icon::Square:     return unitSquare();
                case Icon::Triangle:   return unitTriangle();
                case Icon::Diamond:    return unitDiamond();
                case Icon::Plus:       return unitPlus();
                case Icon::Default:    break;
            }
            return {};
        }
    }

    void drawGlyph (juce::Graphics& g,
                    Icon icon,
                    juce::Rectangle<float> bounds,
                    juce::Colour strokeColour)
    {
        if (icon == Icon::Default)
            return;

        auto p = unitPath (icon);
        if (p.isEmpty())
            return;

        p.applyTransform (juce::AffineTransform::scale (bounds.getWidth(), bounds.getHeight())
                              .translated (bounds.getX(), bounds.getY()));

        g.setColour (strokeColour);
        const float strokeW = juce::jmax (1.2f, bounds.getWidth() * 0.09f);
        g.strokePath (p, juce::PathStrokeType (strokeW,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }
}
