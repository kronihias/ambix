/*
 ==============================================================================
 ==============================================================================
 */

#include "HammerAitoffView.h"
#include "../../common/JuceCompat.h"
#include <cmath>

namespace
{
    constexpr float kSqrt2     = 1.41421356237f;
    constexpr float kTwoSqrt2  = 2.82842712475f;

    // Ellipse half-axes for the *normalised* projection (x ∈ [-2√2, 2√2],
    // y ∈ [-√2, √2]). We normalise them to [-1, 1] for screen mapping.
    inline float normX (float xRaw) { return xRaw / kTwoSqrt2; }
    inline float normY (float yRaw) { return yRaw / kSqrt2;    }
}

HammerAitoffView::HammerAitoffView()
{
    setOpaque (true);
    setMouseCursor (juce::MouseCursor::CrosshairCursor);

    upperOnlyToggle.setTooltip ("Show only the upper hemisphere (plus a few degrees "
                                "below the equator). Useful for loudspeaker domes.");
    upperOnlyToggle.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffd6def0));
    upperOnlyToggle.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xffe8d76c));
    upperOnlyToggle.setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (0xff5b7392));
    upperOnlyToggle.onClick = [this]
    {
        if (! processor) return;
        const bool v = upperOnlyToggle.getToggleState();
        if (processor->ha_upper_hemisphere_only == v) return;
        processor->ha_upper_hemisphere_only = v;
        lastUpperOnly = v;
        // Editor + popout both listen on the processor — push a change so the
        // sibling H-A instance (and any other UI bits) refresh in lock-step.
        processor->sendChangeMessage();
        repaint();
    };
    addChildComponent (upperOnlyToggle); // hidden until a processor is attached

    startTimerHz (30);
}

void HammerAitoffView::setProcessor (Ambix_encoderAudioProcessor* p)
{
    processor = p;
    if (p != nullptr)
    {
        lastUpperOnly = p->ha_upper_hemisphere_only;
        upperOnlyToggle.setToggleState (lastUpperOnly, juce::dontSendNotification);
        upperOnlyToggle.setVisible (true);
    }
    else
    {
        upperOnlyToggle.setVisible (false);
    }
}

bool HammerAitoffView::isUpperOnly() const
{
    return processor != nullptr && processor->ha_upper_hemisphere_only;
}

juce::Range<float> HammerAitoffView::getVisibleEleDegRange() const
{
    if (isUpperOnly())
        return { -10.0f, 90.0f };
    return { -90.0f, 90.0f };
}

void HammerAitoffView::timerCallback()
{
    // The other panner instance may have toggled upper-only. Mirror the
    // change locally so this view's toggle button and projection update.
    if (processor != nullptr && processor->ha_upper_hemisphere_only != lastUpperOnly)
    {
        lastUpperOnly = processor->ha_upper_hemisphere_only;
        upperOnlyToggle.setToggleState (lastUpperOnly, juce::dontSendNotification);
        resized(); // bounds aspect depends on the visible elevation range
    }
    repaint();
}

juce::Point<float> HammerAitoffView::project (float azRad, float elRad)
{
    // Wrap azimuth into [-π, π]. The closed lower bound matters: the
    // polyline meridian/parallel drawers iterate from exactly -π and we
    // need that endpoint to land on the left edge of the ellipse, not
    // jump across to the right edge.
    while (azRad >   juce::MathConstants<float>::pi) azRad -= juce::MathConstants<float>::twoPi;
    while (azRad <  -juce::MathConstants<float>::pi) azRad += juce::MathConstants<float>::twoPi;
    elRad = juce::jlimit (-juce::MathConstants<float>::halfPi,
                           juce::MathConstants<float>::halfPi, elRad);

    const float cosE = std::cos (elRad);
    const float halfA = azRad * 0.5f;
    const float denom = std::sqrt (1.f + cosE * std::cos (halfA));
    const float xRaw = (2.f * kSqrt2 * cosE * std::sin (halfA)) / denom;
    const float yRaw = (kSqrt2 * std::sin (elRad))              / denom;
    return { normX (xRaw), normY (yRaw) };
}

bool HammerAitoffView::unproject (float x, float y, float& azRad, float& elRad)
{
    // Convert back to raw Hammer-Aitoff coordinates.
    const float xRaw = x * kTwoSqrt2;
    const float yRaw = y * kSqrt2;

    // Auxiliary z = √(1 - (x/4)² - (y/2)²)  (Wikipedia, Hammer projection inverse)
    const float a = (xRaw * 0.25f);
    const float b = (yRaw * 0.5f);
    const float zSq = 1.f - a * a - b * b;
    if (zSq <= 0.f) return false;

    const float z = std::sqrt (zSq);
    elRad = std::asin (juce::jlimit (-1.f, 1.f, z * yRaw));
    const float arg = z * xRaw / (2.f * (2.f * z * z - 1.f));
    azRad = 2.f * std::atan (arg);
    return true;
}

juce::Rectangle<float> HammerAitoffView::getProjectionBounds() const
{
    // Inset so dots don't run off the edge.
    const float pad = 12.f;
    auto r = getLocalBounds().toFloat().reduced (pad);
    // Aspect = 2 / (visYMax - visYMin) so the visible region just fills the
    // shorter axis. Full sphere is 2:1; upper hemisphere (-10..+90) is
    // ~1.7:1.
    const auto er = getVisibleEleDegRange();
    constexpr float deg2rad = juce::MathConstants<float>::pi / 180.f;
    const float visYMin = std::sin (er.getStart() * deg2rad);
    const float visYMax = std::sin (er.getEnd()   * deg2rad);
    const float dY = juce::jmax (0.001f, visYMax - visYMin);
    const float ar = 2.f / dY;
    if (r.getWidth() / r.getHeight() > ar)
        r = r.withSizeKeepingCentre (r.getHeight() * ar, r.getHeight());
    else
        r = r.withSizeKeepingCentre (r.getWidth(), r.getWidth() / ar);
    return r;
}

float HammerAitoffView::haYToScreenY (float yH, const juce::Rectangle<float>& bounds) const
{
    const auto er = getVisibleEleDegRange();
    constexpr float deg2rad = juce::MathConstants<float>::pi / 180.f;
    const float visYMin = std::sin (er.getStart() * deg2rad);
    const float visYMax = std::sin (er.getEnd()   * deg2rad);
    const float dY = juce::jmax (0.001f, visYMax - visYMin);
    // yH = visYMax  → screen top; yH = visYMin → screen bottom
    return bounds.getY() + (visYMax - yH) / dY * bounds.getHeight();
}

float HammerAitoffView::screenYToHaY (float yS, const juce::Rectangle<float>& bounds) const
{
    const auto er = getVisibleEleDegRange();
    constexpr float deg2rad = juce::MathConstants<float>::pi / 180.f;
    const float visYMin = std::sin (er.getStart() * deg2rad);
    const float visYMax = std::sin (er.getEnd()   * deg2rad);
    const float dY = juce::jmax (0.001f, visYMax - visYMin);
    return visYMax - (yS - bounds.getY()) / bounds.getHeight() * dY;
}

juce::Point<float> HammerAitoffView::sourceScreenPos (int idx) const
{
    if (! processor) return {};
    const auto pos = processor->getSourceDisplayPos (idx);
    const float azRad = pos.azDeg * juce::MathConstants<float>::pi / 180.f;
    const float elRad = pos.elDeg * juce::MathConstants<float>::pi / 180.f;
    const auto p = project (azRad, elRad);
    const auto bounds = getProjectionBounds();
    return { bounds.getCentreX() + p.x * bounds.getWidth() * 0.5f,
             haYToScreenY (p.y, bounds) };
}

int HammerAitoffView::findSourceUnder (juce::Point<float> screenPt) const
{
    if (! processor) return -1;
    const int active = processor->getActiveSources();
    int best = -1;
    // Match the visScale used in paint() so a bigger panner = bigger hit
    // zone too (no point growing the pucks without growing the grab area).
    const auto bounds = getProjectionBounds();
    const float visScale = juce::jlimit (1.0f, 3.0f, bounds.getWidth() / 600.f);
    const float hitR = 18.f * visScale;
    float bestDistSq = hitR * hitR;
    for (int i = 0; i < active; ++i)
    {
        const auto sp = sourceScreenPos (i);
        const float dSq = sp.getDistanceSquaredFrom (screenPt);
        if (dSq < bestDistSq)
        {
            bestDistSq = dSq;
            best = i;
        }
    }
    return best;
}

void HammerAitoffView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff101418));

    if (! processor) return;
    const auto bounds = getProjectionBounds();
    const float cx = bounds.getCentreX();
    const float halfW = bounds.getWidth()  * 0.5f;
    const auto eleRange = getVisibleEleDegRange();
    const float minEleDeg = eleRange.getStart();
    const float maxEleDeg = eleRange.getEnd();
    const float minEleRad = minEleDeg * juce::MathConstants<float>::pi / 180.f;
    const float maxEleRad = maxEleDeg * juce::MathConstants<float>::pi / 180.f;

    auto toScreen = [&] (float xN, float yN)
    {
        return juce::Point<float> { cx + xN * halfW, haYToScreenY (yN, bounds) };
    };

    constexpr int meridianStep = 30;
    constexpr int parallelStep = 30;
    constexpr int meridianSegments = 64;

    // Outline path. In full-sphere mode this is the unit ellipse; in
    // upper-only mode it's the visible region's boundary — parallel at
    // minEle along the bottom, parallel at maxEle along the top (if not at
    // the pole), connected by az=±180° arcs on the sides.
    juce::Path outline;
    {
        const int seg = meridianSegments;
        // Bottom parallel: az from -π → +π at minEle
        for (int i = 0; i <= seg; ++i)
        {
            const float t = (float)i / (float)seg;
            const float azRad = -juce::MathConstants<float>::pi
                              + t * juce::MathConstants<float>::twoPi;
            const auto p = project (azRad, minEleRad);
            const auto sp = toScreen (p.x, p.y);
            if (i == 0) outline.startNewSubPath (sp);
            else        outline.lineTo (sp);
        }
        // Right edge: az = +π from minEle → maxEle
        for (int i = 1; i <= seg; ++i)
        {
            const float t = (float)i / (float)seg;
            const float elRad = minEleRad + t * (maxEleRad - minEleRad);
            const auto p = project (juce::MathConstants<float>::pi, elRad);
            outline.lineTo (toScreen (p.x, p.y));
        }
        // Top parallel (only if there's a flat top, i.e. maxEle < 90°)
        if (maxEleDeg < 89.5f)
        {
            for (int i = 1; i <= seg; ++i)
            {
                const float t = (float)i / (float)seg;
                const float azRad = juce::MathConstants<float>::pi
                                  - t * juce::MathConstants<float>::twoPi;
                const auto p = project (azRad, maxEleRad);
                outline.lineTo (toScreen (p.x, p.y));
            }
        }
        // Left edge: az = -π from maxEle → minEle
        for (int i = 1; i <= seg; ++i)
        {
            const float t = (float)i / (float)seg;
            const float elRad = maxEleRad - t * (maxEleRad - minEleRad);
            const auto p = project (-juce::MathConstants<float>::pi, elRad);
            outline.lineTo (toScreen (p.x, p.y));
        }
        outline.closeSubPath();
    }

    // Fill background
    g.setColour (juce::Colour (0xff1c2632));
    g.fillPath (outline);

    // Grid: meridians (constant azimuth) — every 30°, polylines for smooth
    // curves. Clipped to the visible region so the dome's curved boundary
    // stays clean.
    {
        juce::Graphics::ScopedSaveState clipState (g);
        g.reduceClipRegion (outline);
        g.setColour (juce::Colour (0x554d6378));

    for (int azDeg = -180; azDeg <= 180; azDeg += meridianStep)
    {
        if (azDeg == 0) continue; // drawn separately, highlighted
        juce::Path m;
        const float azRad = (float) azDeg * juce::MathConstants<float>::pi / 180.f;
        for (int i = 0; i <= meridianSegments; ++i)
        {
            const float t = (float)i / (float)meridianSegments;
            const float elRad = minEleRad + t * (maxEleRad - minEleRad);
            const auto p = project (azRad, elRad);
            const auto sp = toScreen (p.x, p.y);
            if (i == 0) m.startNewSubPath (sp);
            else        m.lineTo (sp);
        }
        g.strokePath (m, juce::PathStrokeType (0.7f));
    }

    // Parallels (constant elevation) — only those inside the visible band
    for (int elDeg = -60; elDeg <= 60; elDeg += parallelStep)
    {
        if (elDeg == 0) continue;
        if (elDeg < minEleDeg + 0.5f || elDeg > maxEleDeg - 0.5f) continue;
        juce::Path p;
        const float elRad = (float) elDeg * juce::MathConstants<float>::pi / 180.f;
        for (int i = 0; i <= meridianSegments; ++i)
        {
            const float t = (float)i / (float)meridianSegments;
            const float azRad = -juce::MathConstants<float>::pi
                              + t * 2.f * juce::MathConstants<float>::pi;
            const auto pp = project (azRad, elRad);
            const auto sp = toScreen (pp.x, pp.y);
            if (i == 0) p.startNewSubPath (sp);
            else        p.lineTo (sp);
        }
        g.strokePath (p, juce::PathStrokeType (0.7f));
    }

    // Equator + prime meridian highlighted (equator only if visible)
    g.setColour (juce::Colour (0xaa9fb4cc));
    if (0.f >= minEleDeg && 0.f <= maxEleDeg)
    {
        juce::Path eq;
        for (int i = 0; i <= meridianSegments; ++i)
        {
            const float t = (float)i / (float)meridianSegments;
            const float azRad = -juce::MathConstants<float>::pi
                              + t * 2.f * juce::MathConstants<float>::pi;
            const auto p = project (azRad, 0.f);
            const auto sp = toScreen (p.x, p.y);
            if (i == 0) eq.startNewSubPath (sp);
            else        eq.lineTo (sp);
        }
        g.strokePath (eq, juce::PathStrokeType (1.2f));
    }
    {
        juce::Path pm;
        for (int i = 0; i <= meridianSegments; ++i)
        {
            const float t = (float)i / (float)meridianSegments;
            const float elRad = minEleRad + t * (maxEleRad - minEleRad);
            const auto p = project (0.f, elRad);
            const auto sp = toScreen (p.x, p.y);
            if (i == 0) pm.startNewSubPath (sp);
            else        pm.lineTo (sp);
        }
        g.strokePath (pm, juce::PathStrokeType (1.2f));
    }

    } // end clipped grid drawing

    // Outline stroke draws on the unclipped graphics so the line itself
    // (centred on the boundary) isn't half cut away.
    g.setColour (juce::Colour (0xff5b7392));
    g.strokePath (outline, juce::PathStrokeType (1.5f));

    // Labels. In a Hammer-Aitoff projection of the sphere with the prime
    // meridian along front, the CENTRE of the ellipse is the front direction
    // (az=0, el=0); the top is the zenith and the bottom the nadir. The far
    // left and right edges both correspond to "back" (azimuth ±180°). So
    // "front" belongs at the centre, not the top.
    const float cyEquator = haYToScreenY (0.f, bounds);
    g.setColour (juce::Colour (0x88a0b3c8));
    g.setFont (juce::Font (juce::FontOptions { 10.f, juce::Font::plain }));
    if (maxEleDeg >= 89.5f)
        g.drawText ("up",   (int)cx - 20, (int)bounds.getY() - 14, 40, 12, juce::Justification::centred);
    if (minEleDeg <= -89.5f)
        g.drawText ("down", (int)cx - 20, (int)bounds.getBottom() + 2, 40, 12, juce::Justification::centred);
    if (0.f >= minEleDeg && 0.f <= maxEleDeg)
    {
        g.drawText ("back",  (int)bounds.getX() - 28, (int)cyEquator - 6, 28, 12, juce::Justification::centred);
        g.drawText ("back",  (int)bounds.getRight() + 2, (int)cyEquator - 6, 28, 12, juce::Justification::centred);
        g.drawText ("front", (int)cx - 22, (int)cyEquator + 6,  44, 12, juce::Justification::centred);
        g.drawText ("left",  (int)cx - (int)(bounds.getWidth() * 0.25f) - 14, (int)cyEquator - 6, 28, 12, juce::Justification::centred);
        g.drawText ("right", (int)cx + (int)(bounds.getWidth() * 0.25f) - 14, (int)cyEquator - 6, 28, 12, juce::Justification::centred);
    }

    // Crosshair marker at the centre so "front" is unambiguous. The "front"
    // direction projects to (az=0, el=0) — i.e. to the equator at the
    // prime meridian. In dome mode that's no longer the geometric centre
    // of the canvas, so anchor on the equator's screen y.
    if (0.f >= minEleDeg && 0.f <= maxEleDeg)
    {
        g.setColour (juce::Colour (0x60a0b3c8));
        g.drawLine (cx - 5, cyEquator, cx + 5, cyEquator, 1.0f);
        g.drawLine (cx, cyEquator - 5, cx, cyEquator + 5, 1.0f);
    }

    // Visual size scale: pucks/labels grow with the ellipse so a fullscreen
    // popout doesn't end up with tiny dots floating in a huge ellipse, but
    // never shrink below the legible inline-editor size. Reference width
    // 600 px gives the original ~7 px puck — same as inline editor. Scale
    // factor capped at 3× for very large windows.
    const float visScale = juce::jlimit (1.0f, 3.0f, bounds.getWidth() / 600.f);

    // Master azimuth/elevation marker — only meaningful in linked mode
    // with more than one source, where it shows the centre of the auto-
    // spread (same convention as the sphere view's small red dot).
    const int active = processor->getActiveSources();
    if (active > 1 && processor->isLinked())
    {
        const float masterAzDeg = (processor->getParameter (Ambix_encoderAudioProcessor::AzimuthParam)   - 0.5f) * 360.f;
        const float masterElDeg = (processor->getParameter (Ambix_encoderAudioProcessor::ElevationParam) - 0.5f) * 360.f;
        const float azRad = masterAzDeg * juce::MathConstants<float>::pi / 180.f;
        const float elRad = masterElDeg * juce::MathConstants<float>::pi / 180.f;
        const auto p = project (azRad, elRad);
        const juce::Point<float> sp { cx + p.x * bounds.getWidth() * 0.5f,
                                      haYToScreenY (p.y, bounds) };
        const float r = 4.f * visScale;
        g.setColour (juce::Colours::red.withAlpha (0.7f));
        g.fillEllipse (sp.x - r, sp.y - r, r * 2.f, r * 2.f);
    }

    // Sources
    for (int i = 0; i < active; ++i)
    {
        const auto sp = sourceScreenPos (i);
        const auto pos = processor->getSourceDisplayPos (i);

        // Size halo (visualises the size param as a translucent disc).
        if (pos.size > 0.f)
        {
            const float radius = (6.f + pos.size * 26.f) * visScale;
            g.setColour (juce::Colours::yellow.withAlpha (0.18f));
            g.fillEllipse (sp.x - radius, sp.y - radius, radius * 2.f, radius * 2.f);
        }

        // Meter ring (thickness scaled by RMS).
        if (pos.rms > 0.005f)
        {
            const float ringR = 14.f * visScale;
            const float thickness = juce::jmin (4.f * visScale, pos.rms * 12.f + 1.f);
            g.setColour (juce::Colours::limegreen.withAlpha (0.7f));
            g.drawEllipse (sp.x - ringR, sp.y - ringR, ringR * 2.f, ringR * 2.f, thickness);
        }

        // Dot
        const float r = 7.f * visScale;
        // Highlight any puck currently being dragged by any finger.
        bool isDragging = (lastGrabbed == i);
        if (! isDragging)
            for (const auto& kv : draggingByTouchIndex)
                if (kv.second == i) { isDragging = true; break; }
        g.setColour (isDragging ? juce::Colours::orange : juce::Colours::yellow);
        g.fillEllipse (sp.x - r, sp.y - r, r * 2.f, r * 2.f);
        g.setColour (juce::Colours::black);
        g.drawEllipse (sp.x - r, sp.y - r, r * 2.f, r * 2.f, 1.f);

        // Label
        const float labelSize = 11.f * visScale;
        const int halfBoxW = (int) (10.f * visScale);
        const int halfBoxH = (int) (7.f  * visScale);
        g.setColour (juce::Colours::black);
        g.setFont (juce::Font (juce::FontOptions { labelSize, juce::Font::bold }));
        g.drawText (juce::String (i + 1),
                    (int)(sp.x - halfBoxW), (int)(sp.y - halfBoxH), halfBoxW * 2, halfBoxH * 2,
                    juce::Justification::centred);
    }
}

void HammerAitoffView::resized()
{
    // Compact overlay toggle pinned to the top-left, matching the
    // visualizer's "Upper only" placement. Wide enough for the label +
    // tick at the inline-editor size; doesn't grow with the canvas
    // because the surrounding pucks shouldn't fight for space.
    upperOnlyToggle.setBounds (8, 4, 110, 22);
}

void HammerAitoffView::mouseDown (const juce::MouseEvent& e)
{
    if (! processor) return;
    int puck = findSourceUnder (e.getPosition().toFloat());
    if (puck < 0)
    {
        // Click in empty space: drag the nearest source there (simpler than
        // creating new sources). Fall through to mouseDrag for the position
        // update.
        puck = 0;
    }
    draggingByTouchIndex[e.source.getIndex()] = puck;
    lastGrabbed = puck;
    mouseDrag (e);
}

void HammerAitoffView::mouseUp (const juce::MouseEvent& e)
{
    draggingByTouchIndex.erase (e.source.getIndex());
    if (draggingByTouchIndex.empty())
        lastGrabbed = -1;
}

void HammerAitoffView::mouseDrag (const juce::MouseEvent& e)
{
    if (! processor) return;
    auto it = draggingByTouchIndex.find (e.source.getIndex());
    if (it == draggingByTouchIndex.end()) return;
    const int draggingSource = it->second;
    lastGrabbed = draggingSource;

    const auto bounds = getProjectionBounds();
    const float cx = bounds.getCentreX();
    const float halfW = bounds.getWidth()  * 0.5f;

    float xN = (e.getPosition().x - cx) / halfW;
    float yN = screenYToHaY (e.getPosition().y, bounds);

    // The Hammer-Aitoff ellipse in normalised (xN, yN) space is the unit
    // circle xN² + yN² = 1. Handle out-of-ellipse drags two ways:
    //   * Horizontal: at any given latitude, the ellipse spans
    //     xN ∈ [-maxXn, +maxXn] where maxXn = √(1 - yN²). Both edges
    //     correspond to azimuth ±180° (the back of the sphere), so dragging
    //     past one edge wraps to the other AT THE SAME LATITUDE. This must
    //     wrap modulo 2·maxXn, not modulo 2 — at high elevations the ellipse
    //     is narrower than the bounding box, and a "modulo 2" wrap would
    //     leave the cursor outside the ellipse even after wrapping.
    //   * Vertical: clamp to the visible elevation range. In dome mode
    //     that's just inside the floor parallel; the top edge already
    //     stops at the pole.
    constexpr float kBoundary = 0.999f;
    constexpr float deg2rad   = juce::MathConstants<float>::pi / 180.f;
    const auto er = getVisibleEleDegRange();
    const float visYMin = std::sin (er.getStart() * deg2rad);
    const float visYMax = std::sin (er.getEnd()   * deg2rad);
    yN = juce::jlimit (visYMin * kBoundary, visYMax * kBoundary, yN);

    const float maxXn = std::sqrt (std::max (1e-6f, 1.f - yN * yN));
    const float widthAtY = 2.f * maxXn;
    while (xN >  maxXn) xN -= widthAtY;
    while (xN < -maxXn) xN += widthAtY;

    // Stay just inside the boundary so the inverse formula's 2z²-1
    // denominator doesn't approach zero (where azimuth pops to ±180°).
    xN = juce::jlimit (-maxXn * kBoundary, maxXn * kBoundary, xN);

    float azRad = 0.f, elRad = 0.f;
    if (! unproject (xN, yN, azRad, elRad))
        return;

    // Both azimuth AND elevation are encoded as [0,1] → [-180°, +180°] in
    // the param model (azimuth uses the full circle; elevation is physically
    // restricted to ±90° but uses the same normalisation for consistency).
    const float azNorm = (azRad / juce::MathConstants<float>::twoPi) + 0.5f;
    const float elNorm = (elRad / juce::MathConstants<float>::twoPi) + 0.5f;

    if (processor->isLinked())
    {
        // In linked mode the dot drives the global azimuth/elevation.
        setParameterNotifyingHost (processor,
                                   Ambix_encoderAudioProcessor::AzimuthParam,
                                   juce::jlimit (0.f, 1.f, azNorm));
        setParameterNotifyingHost (processor,
                                   Ambix_encoderAudioProcessor::ElevationParam,
                                   juce::jlimit (0.f, 1.f, elNorm));
    }
    else
    {
        const int idx = draggingSource;
        setParameterNotifyingHost (processor,
                                   Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcAz),
                                   juce::jlimit (0.f, 1.f, azNorm));
        setParameterNotifyingHost (processor,
                                   Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcEl),
                                   juce::jlimit (0.f, 1.f, elNorm));
    }
}
