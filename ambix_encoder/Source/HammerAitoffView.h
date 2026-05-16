/*
 ==============================================================================

 Hammer-Aitoff equal-area projection panner for ambix_encoder.

 Pure JUCE 2D drawing — no OpenGL dependency. Maps the full sphere to an
 ellipse via the Hammer projection:
     x = (2√2 cos φ sin λ/2) / √(1 + cos φ cos λ/2)
     y = (√2 sin φ)          / √(1 + cos φ cos λ/2)
 where φ = elevation (-π/2 .. π/2), λ = azimuth (-π .. π).

 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"

class HammerAitoffView : public juce::Component,
                         private juce::Timer
{
public:
    HammerAitoffView();
    ~HammerAitoffView() override = default;

    void setProcessor (Ambix_encoderAudioProcessor* p) { processor = p; }

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown  (const juce::MouseEvent& e) override;
    void mouseDrag  (const juce::MouseEvent& e) override;
    void mouseUp    (const juce::MouseEvent& e) override;

private:
    void timerCallback() override { repaint(); }

    // Forward Hammer-Aitoff: az/el in radians → normalised x/y in [-1, 1].
    static juce::Point<float> project (float azRad, float elRad);

    // Inverse Hammer-Aitoff: normalised x/y in [-1, 1] → az/el in radians.
    // Returns false if (x, y) is outside the ellipse.
    static bool unproject (float x, float y, float& azRad, float& elRad);

    juce::Rectangle<float> getProjectionBounds() const;

    int findSourceUnder (juce::Point<float> screenPt) const;
    juce::Point<float> sourceScreenPos (int idx) const;

    Ambix_encoderAudioProcessor* processor = nullptr;

    // Per-touch-source drag state keyed by MouseInputSource::getIndex().
    // JUCE delivers each finger as an independent event stream with its own
    // index, so storing one puck index per source means multiple fingers
    // can drag separate pucks at the same time on touch displays.
    std::map<int, int> draggingByTouchIndex;

    // Index of the puck currently rendered in "dragging" colour (orange).
    // Tracks the most recently grabbed puck across all touches.
    int lastGrabbed = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HammerAitoffView)
};
