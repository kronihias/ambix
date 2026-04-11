/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"
#include <array>
#include <cmath>
#include <functional>

//==============================================================================
/**
 MoveView

 Two orthographic 2D views of the ambix_move geometry, drawn with plain JUCE
 graphics:

   - Top view (left)   : looking down the +Z axis. Horizontal axis = -Y (left
                         on screen is +Y "left ear"); vertical axis = +X
                         (front of the listener at top of the panel).
                         Dragging in this view updates X and Y.

   - Front view (right): looking from +X (front) toward the listener. Horizontal
                         axis = -Y (left on screen is +Y "left ear"); vertical
                         axis = +Z (up). Dragging in this view updates Y and Z.

 Each view shows the reference sphere as a circle of radius `refRadius`, a
 fixed set of sampling points (Fibonacci spiral on the unit sphere, scaled by
 the reference radius), the listener as a highlighted dot, and lines from the
 listener to each sample with thickness/alpha proportional to the gain factor
 R_ref / distance.

 Coordinate convention: ambix — x = front, y = left, z = up.
*/
class MoveView : public juce::Component
{
public:
    MoveView()
    {
        // Generate ~32 evenly-distributed display points via a Fibonacci spiral.
        const int N = (int) displayPoints.size();
        const float ga = juce::MathConstants<float>::pi * (3.f - std::sqrt (5.f));
        for (int i = 0; i < N; ++i)
        {
            float z   = 1.f - 2.f * (float) i / (float) (N - 1);
            float r   = std::sqrt (juce::jmax (0.f, 1.f - z * z));
            float phi = (float) i * ga;
            displayPoints[(size_t) i] = { r * std::cos (phi), r * std::sin (phi), z };
        }
    }

    ~MoveView() override = default;

    //========================================================================
    // State setters (called from the editor's timerCallback)

    void setListener (float xMeters, float yMeters, float zMeters)
    {
        if (listener.x == xMeters && listener.y == yMeters && listener.z == zMeters)
            return;
        listener = { xMeters, yMeters, zMeters };
        repaint();
    }

    void setReferenceRadius (float rMeters)
    {
        if (refRadius == rMeters) return;
        refRadius = rMeters;
        repaint();
    }

    void setLookForward (float fx, float fy, float fz)
    {
        if (lookFwd.x == fx && lookFwd.y == fy && lookFwd.z == fz)
            return;
        lookFwd = { fx, fy, fz };
        repaint();
    }

    //========================================================================
    // Editor callbacks
    std::function<void (float xMeters, float yMeters)> onListenerXYDragged;
    std::function<void (float yMeters, float zMeters)> onListenerYZDragged;
    std::function<void (float radiusMetersDelta)>      onRadiusWheel;
    std::function<void()>                              onResetListener;
    std::function<void()>                              onGestureEnded;

    //========================================================================
    void resized() override
    {
        const auto b = getLocalBounds().reduced (4);
        const int w = b.getWidth();
        const int gap = 8;
        const int paneW = (w - gap) / 2;
        const int paneH = b.getHeight();
        const int side = juce::jmin (paneW, paneH);

        topPaneBounds   = juce::Rectangle<int> (b.getX(),                    b.getY() + (paneH - side) / 2, side, side);
        frontPaneBounds = juce::Rectangle<int> (b.getX() + paneW + gap,      b.getY() + (paneH - side) / 2, side, side);
    }

    void paint (juce::Graphics& g) override
    {
        // Background
        g.setColour (juce::Colour (0xFF1E1E1E));
        g.fillRect (getLocalBounds());

        drawPane (g, topPaneBounds,   PaneKind::Top);
        drawPane (g, frontPaneBounds, PaneKind::Front);
    }

    //========================================================================
    void mouseDown (const juce::MouseEvent& e) override
    {
        activeDrag = DragKind::None;

        if (topPaneBounds.contains (e.getPosition()))
            activeDrag = DragKind::Top;
        else if (frontPaneBounds.contains (e.getPosition()))
            activeDrag = DragKind::Front;

        handleDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        handleDrag (e);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (activeDrag != DragKind::None && onGestureEnded)
            onGestureEnded();
        activeDrag = DragKind::None;
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        if (! onResetListener) return;
        if (! topPaneBounds.contains (e.getPosition())
            && ! frontPaneBounds.contains (e.getPosition())) return;

        activeDrag = DragKind::None; // cancel any in-flight drag
        onResetListener();
        if (onGestureEnded) onGestureEnded();
    }

    void mouseWheelMove (const juce::MouseEvent& e,
                         const juce::MouseWheelDetails& wheel) override
    {
        if (! onRadiusWheel) return;
        if (! topPaneBounds.contains (e.getPosition())
            && ! frontPaneBounds.contains (e.getPosition())) return;

        // Step in metres per wheel notch (≈ 1 notch ≈ 0.25 m). The host
        // clamps the resulting parameter to the legal range.
        const float step = 0.25f;
        const float dy   = wheel.deltaY * (wheel.isReversed ? -1.f : 1.f);
        onRadiusWheel (dy * step);

        if (onGestureEnded) onGestureEnded();
    }

private:
    enum class PaneKind { Top, Front };
    enum class DragKind { None, Top, Front };

    juce::Vector3D<float> listener { 0.f, 0.f, 0.f };
    juce::Vector3D<float> lookFwd  { 1.f, 0.f, 0.f };
    float refRadius = 1.f;

    std::array<juce::Vector3D<float>, 32> displayPoints;

    juce::Rectangle<int> topPaneBounds;
    juce::Rectangle<int> frontPaneBounds;

    DragKind activeDrag = DragKind::None;

    // The reference circle is always drawn at the SAME on-screen size,
    // independent of the radius value. Axis labels show the radius in metres,
    // so the user still sees the absolute scale.
    //
    // `circleFill` is the fraction of the pane's half-size occupied by the
    // circle radius. The remaining margin leaves room for cardinal labels.
    static constexpr float circleFill = 0.78f;

    float circleScreenRadius (juce::Rectangle<int> pane) const
    {
        const float halfPx = 0.5f * (float) juce::jmin (pane.getWidth(), pane.getHeight());
        return halfPx * circleFill;
    }

    // metres -> pixels.  refRadius metres == circleScreenRadius(pane) pixels.
    float metresToPixels (juce::Rectangle<int> pane) const
    {
        const float r = juce::jmax (0.001f, refRadius);
        return circleScreenRadius (pane) / r;
    }

    //========================================================================
    // World <-> screen mapping for an orthographic pane.
    //
    // For the top pane (looking down +Z), the visible plane is X (front) /
    // Y (left). For the front pane (looking from +X), the visible plane is
    // Y (left) / Z (up).
    //
    // Convention: in both views the horizontal screen axis grows to the LEFT
    // with positive Y (so "left" on screen really is "to the listener's left")
    // and the vertical screen axis grows UP with positive X (top view) or
    // positive Z (front view).

    juce::Point<float> worldToScreen (juce::Rectangle<int> pane,
                                      PaneKind kind,
                                      juce::Vector3D<float> p) const
    {
        const float cx = (float) pane.getCentreX();
        const float cy = (float) pane.getCentreY();
        const float scale = metresToPixels (pane);

        // horizontal: -Y (so positive Y goes to the screen-left)
        // vertical:   top view -> +X up   ;   front view -> +Z up
        const float hWorld = -p.y;
        const float vWorld = (kind == PaneKind::Top ? p.x : p.z);

        return { cx + hWorld * scale, cy - vWorld * scale };
    }

    juce::Point<float> screenToWorld (juce::Rectangle<int> pane,
                                      PaneKind kind,
                                      juce::Point<float> screen) const
    {
        const float cx = (float) pane.getCentreX();
        const float cy = (float) pane.getCentreY();
        const float scale = metresToPixels (pane);

        const float hWorld = (screen.x - cx) / scale;
        const float vWorld = (cy - screen.y) / scale;

        // hWorld = -y  -> y = -hWorld
        // vWorld = x (top) or z (front)
        return { vWorld, -hWorld };
    }

    //========================================================================
    void drawPane (juce::Graphics& g, juce::Rectangle<int> pane, PaneKind kind)
    {
        if (pane.isEmpty()) return;

        // Pane background + frame
        g.setColour (juce::Colour (0xFF101010));
        g.fillRect (pane);
        g.setColour (juce::Colour (0xFF3A3A3A));
        g.drawRect (pane, 1);

        // Faint axis cross at the world origin
        const auto origin = worldToScreen (pane, kind, { 0.f, 0.f, 0.f });
        g.setColour (juce::Colour (0xFF2E2E2E));
        g.drawHorizontalLine ((int) origin.y, (float) pane.getX(), (float) pane.getRight());
        g.drawVerticalLine   ((int) origin.x, (float) pane.getY(), (float) pane.getBottom());

        // Reference sphere -> circle of fixed on-screen radius
        const float circleR = circleScreenRadius (pane);
        {
            juce::Path circle;
            circle.addCentredArc (origin.x, origin.y, circleR, circleR, 0.f,
                                  0.f, juce::MathConstants<float>::twoPi, true);
            g.setColour (juce::Colour (0x66B0B0FF));
            g.strokePath (circle, juce::PathStrokeType (1.2f));
        }

        // Radius tick labels at the four cardinal points on the circle.
        // The numeric value is the same on all four sides (= refRadius), with
        // the appropriate sign for the negative direction.
        {
            const juce::String posStr = "+" + juce::String (refRadius, 2) + " m";
            const juce::String negStr = "-" + juce::String (refRadius, 2) + " m";

            g.setColour (juce::Colour (0xFF7E8AAA));
            g.setFont (juce::Font (juce::FontOptions { 10.0f, juce::Font::plain }));

            const int tickW = 56;
            const int tickH = 12;

            // Vertical-axis labels (top and bottom of circle)
            g.drawText (posStr,
                        juce::Rectangle<float> (origin.x - tickW * 0.5f,
                                                origin.y - circleR - tickH - 1.f,
                                                (float) tickW, (float) tickH),
                        juce::Justification::centred);
            g.drawText (negStr,
                        juce::Rectangle<float> (origin.x - tickW * 0.5f,
                                                origin.y + circleR + 1.f,
                                                (float) tickW, (float) tickH),
                        juce::Justification::centred);

            // Horizontal-axis labels (left = +Y, right = -Y)
            g.drawText (posStr,
                        juce::Rectangle<float> (origin.x - circleR - tickW - 2.f,
                                                origin.y - tickH * 0.5f,
                                                (float) tickW, (float) tickH),
                        juce::Justification::centredRight);
            g.drawText (negStr,
                        juce::Rectangle<float> (origin.x + circleR + 2.f,
                                                origin.y - tickH * 0.5f,
                                                (float) tickW, (float) tickH),
                        juce::Justification::centredLeft);
        }

        // Sampling points + listener-to-sample lines (gain modulated)
        const auto listenerScreen = worldToScreen (pane, kind, listener);

        for (auto& dp : displayPoints)
        {
            const juce::Vector3D<float> sample { refRadius * dp.x,
                                                 refRadius * dp.y,
                                                 refRadius * dp.z };
            const auto sampleScreen = worldToScreen (pane, kind, sample);

            const float dx = sample.x - listener.x;
            const float dy = sample.y - listener.y;
            const float dz = sample.z - listener.z;
            const float dist = std::sqrt (dx * dx + dy * dy + dz * dz);
            const float gain = (dist > 1.0e-3f) ? (refRadius / dist) : 1.f;

            const float gNorm = juce::jlimit (0.f, 1.f, (gain - 0.3f) / 1.7f);
            const float alpha = 0.18f + 0.55f * gNorm;
            const float thick = 0.6f + 1.6f * gNorm;

            g.setColour (juce::Colour::fromFloatRGBA (1.0f, 0.85f, 0.4f, alpha));
            g.drawLine (listenerScreen.x, listenerScreen.y,
                        sampleScreen.x,   sampleScreen.y, thick);

            g.setColour (juce::Colour (0xFFCCCCCC));
            g.fillEllipse (sampleScreen.x - 2.0f, sampleScreen.y - 2.0f, 4.0f, 4.0f);
        }

        // Listener "nose" — projected look-forward vector. Drawn before the
        // head dot so the dot overlays the base of the nose cleanly.
        {
            const float noseLenPx = 22.f;
            const float noseLenM  = noseLenPx / metresToPixels (pane);

            const juce::Vector3D<float> noseTip {
                listener.x + lookFwd.x * noseLenM,
                listener.y + lookFwd.y * noseLenM,
                listener.z + lookFwd.z * noseLenM };

            const auto noseScreen = worldToScreen (pane, kind, noseTip);

            // Length of the projection into *this* pane's plane. If the
            // listener is looking straight along the view axis, the nose
            // collapses to the centre — skip drawing it then.
            const float dxPx = noseScreen.x - listenerScreen.x;
            const float dyPx = noseScreen.y - listenerScreen.y;
            const float projLen = std::sqrt (dxPx * dxPx + dyPx * dyPx);

            if (projLen > 2.5f)
            {
                g.setColour (juce::Colour (0xFFFFC060));
                g.drawLine (listenerScreen.x, listenerScreen.y,
                            noseScreen.x,     noseScreen.y, 2.4f);

                // Arrow-head: small filled triangle at the nose tip
                const float ux = dxPx / projLen;
                const float uy = dyPx / projLen;
                const float nx = -uy;
                const float ny =  ux;
                const float tipLen = 6.0f;
                const float tipHalf = 3.0f;

                juce::Path tri;
                tri.startNewSubPath (noseScreen.x, noseScreen.y);
                tri.lineTo (noseScreen.x - ux * tipLen + nx * tipHalf,
                            noseScreen.y - uy * tipLen + ny * tipHalf);
                tri.lineTo (noseScreen.x - ux * tipLen - nx * tipHalf,
                            noseScreen.y - uy * tipLen - ny * tipHalf);
                tri.closeSubPath();
                g.fillPath (tri);
            }
        }

        // Listener dot
        g.setColour (juce::Colour (0xFFFF6A00));
        g.fillEllipse (listenerScreen.x - 5.0f, listenerScreen.y - 5.0f, 10.0f, 10.0f);
        g.setColour (juce::Colours::white);
        g.drawEllipse (listenerScreen.x - 5.0f, listenerScreen.y - 5.0f, 10.0f, 10.0f, 1.2f);

        // Axis labels
        g.setColour (juce::Colour (0xFFAAAAAA));
        g.setFont (juce::Font (juce::FontOptions { 11.0f, juce::Font::plain }));
        const int pad = 4;
        if (kind == PaneKind::Top)
        {
            g.drawText ("FRONT (+X)", pane.reduced (pad).removeFromTop (14),    juce::Justification::centredTop);
            g.drawText ("BACK",       pane.reduced (pad).removeFromBottom (14), juce::Justification::centredBottom);
            {
                // Sit the LEFT/RIGHT labels just above the horizontal centerline
                // so they don't collide with the radius tick labels.
                auto leftRightStrip = pane.reduced (pad);
                const int stripH = 14;
                leftRightStrip = leftRightStrip.withHeight (stripH)
                                               .withY (pane.getCentreY() - stripH - 2);
                g.drawText ("LEFT (+Y)", leftRightStrip, juce::Justification::centredLeft);
                g.drawText ("RIGHT",     leftRightStrip, juce::Justification::centredRight);
            }

            g.setColour (juce::Colour (0xFF888888));
            g.setFont (juce::Font (juce::FontOptions { 10.0f, juce::Font::italic }));
            g.drawText ("top view",  pane.reduced (pad + 2).removeFromTop (12),
                        juce::Justification::topRight);
        }
        else
        {
            g.drawText ("UP (+Z)",    pane.reduced (pad).removeFromTop (14),    juce::Justification::centredTop);
            g.drawText ("DOWN",       pane.reduced (pad).removeFromBottom (14), juce::Justification::centredBottom);
            {
                // Sit the LEFT/RIGHT labels just above the horizontal centerline
                // so they don't collide with the radius tick labels.
                auto leftRightStrip = pane.reduced (pad);
                const int stripH = 14;
                leftRightStrip = leftRightStrip.withHeight (stripH)
                                               .withY (pane.getCentreY() - stripH - 2);
                g.drawText ("LEFT (+Y)", leftRightStrip, juce::Justification::centredLeft);
                g.drawText ("RIGHT",     leftRightStrip, juce::Justification::centredRight);
            }

            g.setColour (juce::Colour (0xFF888888));
            g.setFont (juce::Font (juce::FontOptions { 10.0f, juce::Font::italic }));
            g.drawText ("front view", pane.reduced (pad + 2).removeFromTop (12),
                        juce::Justification::topRight);
        }
    }

    //========================================================================
    void handleDrag (const juce::MouseEvent& e)
    {
        if (activeDrag == DragKind::None) return;

        const auto pane = (activeDrag == DragKind::Top) ? topPaneBounds : frontPaneBounds;
        const auto kind = (activeDrag == DragKind::Top) ? PaneKind::Top : PaneKind::Front;

        const auto world = screenToWorld (pane, kind, e.position);

        // world.x = vertical axis value, world.y = -horizontal => Y in world coords
        const float clampedY = juce::jlimit (-5.f, 5.f, world.y);

        if (kind == PaneKind::Top)
        {
            const float clampedX = juce::jlimit (-5.f, 5.f, world.x);
            if (onListenerXYDragged)
                onListenerXYDragged (clampedX, clampedY);
        }
        else
        {
            const float clampedZ = juce::jlimit (-5.f, 5.f, world.x);
            if (onListenerYZDragged)
                onListenerYZDragged (clampedY, clampedZ);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MoveView)
};
