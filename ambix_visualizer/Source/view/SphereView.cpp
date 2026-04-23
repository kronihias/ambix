#include "SphereView.h"

#include "PuckRenderer.h"
#include "SelectionOverlay.h"

namespace
{
    constexpr float kDegToRad = juce::MathConstants<float>::pi / 180.0f;
    constexpr float kRadToDeg = 180.0f / juce::MathConstants<float>::pi;
}

SphereView::SphereView (SourceRegistry& r,
                        OscCommandSender& s,
                        const AppSettings& st,
                        GroupRegistry& gr)
    : registry (r), sender (s), settings (st), groups (gr), groupPanel (gr)
{
    // Clicks on the view must not propagate up to MainComponent where
    // traversal would land on portEditor (and pop up the iOS keyboard).
    setMouseClickGrabsKeyboardFocus (false);

    addAndMakeVisible (groupPanel);

    registry.addChangeListener (this);
    groups.addChangeListener (this);
    cache = registry.snapshot();

    // Preset yaw/pitch values applied before the fixed top-view axis swap.
    // BACK/LEFT/RIGHT: no L/R mirror. FRONT is the exception — a pure rotation
    // from the top projection can't land at the non-mirror front view, so
    // ambix RIGHT shows on screen-left in the FRONT preset.
    struct P { juce::TextButton& btn; float yaw; float pitch; const char* tip; };
    const P presets[] = {
        { btnTop,     0.0f,    0.0f,
          "Top view (default): FRONT at top, RIGHT at right, UP at centre." },
        { btnFront, 180.0f,   90.0f,
          "Front view: listener's face toward camera, UP at top. "
          "Note: ambix RIGHT shows on screen-left in this view." },
        { btnBack,    0.0f,   90.0f,
          "Back view: listener's back toward camera, UP at top, RIGHT at right." },
        { btnLeft,   90.0f,   90.0f,
          "Left side view: listener's left ear at centre, UP at top. "
          "FRONT appears on screen-left, BACK on screen-right." },
        { btnRight, -90.0f,   90.0f,
          "Right side view: listener's right ear at centre, UP at top. "
          "FRONT appears on screen-right, BACK on screen-left." },
    };
    for (const auto& p : presets)
    {
        auto& btn = p.btn;
        const float y = p.yaw;
        const float pp = p.pitch;
        btn.onClick = [this, y, pp]()
        {
            yawDeg = y; pitchDeg = pp; repaint();
            if (onViewStateChanged)
                onViewStateChanged (yawDeg, pitchDeg, btnLock.getToggleState());
        };
        btn.setTooltip (p.tip);
        addAndMakeVisible (btn);
    }

    btnLock.setTooltip (
        "Lock sphere rotation. When on, dragging the background does nothing; "
        "puck drags still work. Handy to avoid bumping the view by accident.");
    btnLock.onClick = [this]()
    {
        if (onViewStateChanged)
            onViewStateChanged (yawDeg, pitchDeg, btnLock.getToggleState());
    };
    addAndMakeVisible (btnLock);

    btnFlipLR.setToggleState (settings.flipLeftRight, juce::dontSendNotification);
    btnFlipLR.setTooltip ("Mirror left/right on both the 2D and 3D views. "
                          "Useful when the visualizer is facing the audience instead of the listener.");
    auto notifyFlip = [this]()
    {
        if (onFlipLRChanged)
            onFlipLRChanged (btnFlipLR.getToggleState());
    };
    btnFlipLR.onClick       = notifyFlip;
    btnFlipLR.onStateChange = notifyFlip;
    addAndMakeVisible (btnFlipLR);

    puckSizeLabel.setText ("Pucks:", juce::dontSendNotification);
    puckSizeLabel.setJustificationType (juce::Justification::centredRight);
    puckSizeLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (puckSizeLabel);

    puckSizeCombo.addItem ("Small",  static_cast<int> (AppSettings::PuckSize::Small)  + 1);
    puckSizeCombo.addItem ("Medium", static_cast<int> (AppSettings::PuckSize::Medium) + 1);
    puckSizeCombo.addItem ("Large",  static_cast<int> (AppSettings::PuckSize::Large)  + 1);
    puckSizeCombo.setSelectedId (static_cast<int> (settings.puckSize) + 1,
                                 juce::dontSendNotification);
    puckSizeCombo.setTooltip ("Visual size of source pucks. Applies to both 2D and 3D views.");
    puckSizeCombo.onChange = [this]()
    {
        if (! onPuckSizeChanged) return;
        const int id = puckSizeCombo.getSelectedId();
        if (id <= 0) return;
        onPuckSizeChanged (static_cast<AppSettings::PuckSize> (id - 1));
    };
    addAndMakeVisible (puckSizeCombo);

    startTimerHz (60);
}

void SphereView::setFlipLRCallback (std::function<void (bool)> callback)
{
    onFlipLRChanged = std::move (callback);
}

void SphereView::setPuckSizeCallback (std::function<void (AppSettings::PuckSize)> callback)
{
    onPuckSizeChanged = std::move (callback);
}

void SphereView::setInitialViewState (float yaw, float pitch, bool locked)
{
    yawDeg = yaw;
    pitchDeg = pitch;
    btnLock.setToggleState (locked, juce::dontSendNotification);
    repaint();
}

SphereView::~SphereView()
{
    registry.removeChangeListener (this);
    groups.removeChangeListener (this);
    stopTimer();
}

// Spherical (ambix: +az clockwise from front, +el up) → world Cartesian.
SphereView::Vec3 SphereView::sphericalToCart (float azDeg, float elDeg) const noexcept
{
    const float az = azDeg * kDegToRad;
    const float el = elDeg * kDegToRad;
    const float cosEl = std::cos (el);
    return { cosEl * std::cos (az),   // +X forward
             cosEl * std::sin (az),   // +Y right  (clockwise-positive az → +y)
             std::sin (el) };         // +Z up
}

// Globe rotation: yaw (around world Z) + pitch (around world Y) applied to
// the world, followed by a fixed axis swap so the base orientation is the
// top plan view (FRONT at screen top, RIGHT at screen right, UP at centre).
// The swap is the same for every orbit position, so FRONT and LEFT keep
// their 90° relationship — the labels move with the globe, never mirroring.
SphereView::Vec3 SphereView::rotateByCamera (const Vec3& w) const noexcept
{
    const float yaw = yawDeg   * kDegToRad;
    const float pit = pitchDeg * kDegToRad;
    const float cy = std::cos (yaw), sy = std::sin (yaw);
    const float cp = std::cos (pit), sp = std::sin (pit);

    // R_z(yaw): spin the globe around its vertical (UP) axis
    const float x1 =  w.x * cy + w.y * sy;
    const float y1 = -w.x * sy + w.y * cy;
    const float z1 =  w.z;

    // R_y(pitch): tilt the globe forward/backward
    const float x2 =  x1 * cp + z1 * sp;
    const float y2 =  y1;
    const float z2 = -x1 * sp + z1 * cp;

    // Fixed top plan-view projection: (x, y, z) → (z, y, x)
    return { z2, y2, x2 };
}

SphereView::Vec3 SphereView::unrotateByCamera (const Vec3& c) const noexcept
{
    // Inverse of the axis swap (self-inverse).
    const float x2 = c.z;
    const float y2 = c.y;
    const float z2 = c.x;

    const float yaw = yawDeg   * kDegToRad;
    const float pit = pitchDeg * kDegToRad;
    const float cy = std::cos (yaw), sy = std::sin (yaw);
    const float cp = std::cos (pit), sp = std::sin (pit);

    // Inverse pitch: R_y(-pitch)
    const float x1 =  x2 * cp - z2 * sp;
    const float y1 =  y2;
    const float z1 =  x2 * sp + z2 * cp;

    // Inverse yaw: R_z(-yaw)
    const float X =  x1 * cy - y1 * sy;
    const float Y =  x1 * sy + y1 * cy;
    const float Z =  z1;

    return { X, Y, Z };
}

juce::Point<float> SphereView::project (const Vec3& cam) const noexcept
{
    const float xSign = settings.flipLeftRight ? -1.0f : 1.0f;
    return { centre.x + xSign * cam.y * radiusPx,
             centre.y - cam.z * radiusPx };
}

bool SphereView::unproject (juce::Point<float> screen, bool preferBack, Vec3& worldOut) const noexcept
{
    const float xSign = settings.flipLeftRight ? -1.0f : 1.0f;
    const float y = xSign * (screen.x - centre.x) / radiusPx;
    const float z = (centre.y - screen.y) / radiusPx;
    const float d = std::sqrt (y * y + z * z);

    Vec3 cam;

    if (d <= 1.0f)
    {
        // Inside the visible disc: exact inverse of the sphere projection, so
        // the puck tracks the cursor pixel-for-pixel.
        float depth = std::sqrt (juce::jmax (0.0f, 1.0f - d * d));
        if (preferBack)
            depth = -depth;
        cam = { depth, y, z };
    }
    else
    {
        // Outside the disc: wrap around to the opposite hemisphere so the user
        // can drag past the silhouette into the back (or, when holding a back
        // puck, into the front). Polar-angle mapping:
        //   d = 1 → equator (matches the inside branch at the seam)
        //   d = 2 → opposite pole (antipode of the grabbed hemisphere)
        const float tClamped = juce::jlimit (1.0f, 2.0f, d);
        const float theta = tClamped * juce::MathConstants<float>::halfPi;
        float depth  = std::cos (theta);
        float planar = std::sin (theta);
        if (preferBack)
            depth = -depth;
        cam = { depth, planar * (y / d), planar * (z / d) };
    }

    worldOut = unrotateByCamera (cam);
    return true;
}

float SphereView::puckRadius (const EncoderSource& s) const noexcept
{
    const float scale = PuckRenderer::sizeScale (settings.puckSize);
    return juce::jmap (juce::jlimit (0.0f, 1.0f, s.size),
                       PuckRenderer::kMinRadiusPx * scale,
                       PuckRenderer::kMaxRadiusPx * scale);
}

void SphereView::updateLayout()
{
    const auto b = sphereBounds.toFloat();
    centre = { b.getCentreX(), b.getCentreY() };
    radiusPx = juce::jmax (40.0f, juce::jmin (b.getWidth(), b.getHeight()) * 0.45f);
}

void SphereView::resized()
{
    auto area = getLocalBounds();
    // Bar is sized for touch on iPad: 44 px iOS HIG minimum target, reduced
    // to ~36 px inner height. The view-preset buttons (Top/Front/Back/…)
    // were hard to hit at the old 24 px height.
    auto bar = area.removeFromTop (44).reduced (6, 4);
    sphereBounds = area;

    // Wider than the old 56 so labels like "Front"/"Right" fit comfortably at
    // the larger button height without tight horizontal padding.
    constexpr int btnWidth = 72;
    constexpr int gap = 4;
    btnTop  .setBounds (bar.removeFromLeft (btnWidth));  bar.removeFromLeft (gap);
    btnFront.setBounds (bar.removeFromLeft (btnWidth));  bar.removeFromLeft (gap);
    btnBack .setBounds (bar.removeFromLeft (btnWidth));  bar.removeFromLeft (gap);
    btnLeft .setBounds (bar.removeFromLeft (btnWidth));  bar.removeFromLeft (gap);
    btnRight.setBounds (bar.removeFromLeft (btnWidth));
    bar.removeFromLeft (16);
    btnLock  .setBounds (bar.removeFromLeft (90));
    bar.removeFromLeft (8);
    btnFlipLR.setBounds (bar.removeFromLeft (90));
    bar.removeFromLeft (16);
    puckSizeLabel.setBounds (bar.removeFromLeft (52));
    puckSizeCombo.setBounds (bar.removeFromLeft (96));

    const int gpH = groupPanel.getPreferredHeight();
    const int gpW = groupPanel.getPreferredWidth();
    if (gpH > 0)
        groupPanel.setBounds (getWidth() - gpW - 10, 40, gpW, gpH);
    else
        groupPanel.setBounds (getWidth() - gpW - 10, 40, gpW, 0);

    updateLayout();
}

void SphereView::settingsChanged()
{
    btnFlipLR.setToggleState (settings.flipLeftRight, juce::dontSendNotification);
    puckSizeCombo.setSelectedId (static_cast<int> (settings.puckSize) + 1,
                                 juce::dontSendNotification);
    updateLayout();
    repaint();
}

void SphereView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF141414));

    // Sphere fill with soft radial shading
    const juce::Rectangle<float> sphereRect (centre.x - radiusPx,
                                             centre.y - radiusPx,
                                             radiusPx * 2.0f,
                                             radiusPx * 2.0f);

    juce::ColourGradient bg (juce::Colour (0xFF2A2A2A),
                             centre.x - radiusPx * 0.3f, centre.y - radiusPx * 0.3f,
                             juce::Colour (0xFF0E0E0E),
                             centre.x + radiusPx, centre.y + radiusPx, true);
    g.setGradientFill (bg);
    g.fillEllipse (sphereRect);

    // Meridians & parallels (two passes: back first, then front on top)
    auto drawArc = [&] (auto sampler, int samples, bool frontPass)
    {
        const float bright = frontPass ? 0.45f : 0.15f;
        g.setColour (juce::Colours::white.withAlpha (bright));

        Vec3 prevC = rotateByCamera (sampler (0));
        auto prevP = project (prevC);

        for (int i = 1; i <= samples; ++i)
        {
            Vec3 c = rotateByCamera (sampler (i));
            auto p = project (c);

            const bool segFront = (prevC.x + c.x) * 0.5f > 0.0f;
            if (segFront == frontPass)
                g.drawLine (prevP.x, prevP.y, p.x, p.y, 1.0f);

            prevC = c;
            prevP = p;
        }
    };

    for (int pass = 0; pass < 2; ++pass)
    {
        const bool frontPass = (pass == 1);

        // Parallels (fixed elevation)
        for (int elDeg = -60; elDeg <= 60; elDeg += 30)
        {
            drawArc ([this, elDeg] (int i)
            {
                const float azDeg = -180.0f + static_cast<float> (i) * (360.0f / 72.0f);
                return sphericalToCart (azDeg, static_cast<float> (elDeg));
            }, 72, frontPass);
        }

        // Equator a bit brighter
        g.setColour (juce::Colours::white.withAlpha (frontPass ? 0.65f : 0.2f));
        juce::Path eq;
        bool first = true;
        for (int i = 0; i <= 144; ++i)
        {
            const float azDeg = -180.0f + i * (360.0f / 144.0f);
            Vec3 c = rotateByCamera (sphericalToCart (azDeg, 0.0f));
            if ((c.x > 0.0f) == frontPass)
            {
                auto p = project (c);
                if (first) { eq.startNewSubPath (p); first = false; }
                else       eq.lineTo (p);
            }
            else
            {
                first = true;
            }
        }
        g.strokePath (eq, juce::PathStrokeType (1.2f));

        // Meridians (fixed azimuth)
        for (int azDeg = -180; azDeg < 180; azDeg += 30)
        {
            drawArc ([this, azDeg] (int i)
            {
                const float elDeg = -90.0f + static_cast<float> (i) * (180.0f / 60.0f);
                return sphericalToCart (static_cast<float> (azDeg), elDeg);
            }, 60, frontPass);
        }
    }

    // Outline
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawEllipse (sphereRect, 1.0f);

    // Cardinal letters around the equator — all are at cam.x = 0 in the plan
    // view (on the silhouette) so we draw them unconditionally with a mid
    // alpha.
    auto drawCardinal = [&] (const char* s, float azDeg, float elDeg)
    {
        const Vec3 c = rotateByCamera (sphericalToCart (azDeg, elDeg));
        if (c.x < -0.02f) return;
        const auto p = project (c);
        g.setColour (juce::Colours::white.withAlpha (c.x < 0.1f ? 0.7f : 0.85f));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (s,
                    juce::Rectangle<float> (p.x - 30, p.y - 8, 60, 16),
                    juce::Justification::centred, false);
    };
    drawCardinal ("FRONT",    0.0f, 0.0f);
    drawCardinal ("RIGHT",   90.0f, 0.0f);
    drawCardinal ("BACK",   180.0f, 0.0f);
    drawCardinal ("LEFT",   -90.0f, 0.0f);

    // Pucks — back first, then front, so front occludes back
    std::vector<const EncoderSource*> selected;
    for (int pass = 0; pass < 2; ++pass)
    {
        const bool frontPass = (pass == 1);
        for (const auto& s : cache)
        {
            if (! groups.isVisible (s.groupId)) continue;
            const Vec3 c = rotateByCamera (sphericalToCart (s.azimuthDeg, s.elevationDeg));
            const bool isFront = c.x >= 0.0f;
            if (isFront != frontPass) continue;

            const auto p = project (c);
            const float r = puckRadius (s);
            const bool isSel = isKeyBeingDragged (s.key());
            PuckRenderer::draw (g, s, p, r, isSel, ! isFront, settings);
            if (isSel)
                selected.push_back (&s);
        }
    }

    SelectionOverlay::draw (g, selected, getLocalBounds());
}

bool SphereView::isKeyBeingDragged (const EncoderSource::Key& k) const
{
    for (const auto& kv : puckTouches)
        if (kv.second.key == k)
            return true;
    return false;
}

void SphereView::mouseDown (const juce::MouseEvent& e)
{
    // Hit-test pucks — front pucks first (since they're on top visually), and
    // among pucks within range prefer the one with the most recent interaction
    // (matches visual foreground). Generous hit radius for multi-touch.
    const EncoderSource* hit = nullptr;
    bool hitOnBack = false;
    juce::uint32 bestTime = 0;
    float bestDist = std::numeric_limits<float>::max();
    for (int pass = 0; pass < 2 && hit == nullptr; ++pass)
    {
        const bool wantFront = (pass == 0);
        for (const auto& s : cache)
        {
            if (! groups.isVisible (s.groupId)) continue;
            const Vec3 c = rotateByCamera (sphericalToCart (s.azimuthDeg, s.elevationDeg));
            const bool isFront = c.x >= 0.0f;
            if (isFront != wantFront) continue;
            const auto p = project (c);
            const float r = juce::jmax (puckRadius (s) + 18.0f, 36.0f);
            const float d = p.getDistanceFrom (e.position);
            if (d > r) continue;

            const bool newer = s.lastInteractionMs > bestTime;
            const bool sameTimeCloser = (s.lastInteractionMs == bestTime && d < bestDist);
            if (hit == nullptr || newer || sameTimeCloser)
            {
                hit = &s;
                hitOnBack = ! isFront;
                bestTime = s.lastInteractionMs;
                bestDist = d;
            }
        }
    }

    const int srcIdx = e.source.getIndex();
    if (hit != nullptr)
    {
        puckTouches[srcIdx] = { hit->key(), hitOnBack };
        registry.beginDrag (hit->key());
    }
    else if (orbitTouchIdx < 0 && ! btnLock.getToggleState())
    {
        // First empty-area touch takes the orbit slot; extras are ignored.
        // When rotation is locked, empty-area drags are no-ops.
        orbitTouchIdx    = srcIdx;
        orbitStart       = e.position;
        orbitStartYaw    = yawDeg;
        orbitStartPitch  = pitchDeg;
    }
    repaint();
}

void SphereView::mouseDrag (const juce::MouseEvent& e)
{
    const int srcIdx = e.source.getIndex();

    if (srcIdx == orbitTouchIdx)
    {
        const auto d = e.position - orbitStart;
        const float xSign = settings.flipLeftRight ? 1.0f : -1.0f; // flip feels natural
        yawDeg   = orbitStartYaw   + xSign * d.x * 0.4f;
        pitchDeg = juce::jlimit (-89.0f, 89.0f, orbitStartPitch + d.y * 0.4f);
        repaint();
        return;
    }

    auto it = puckTouches.find (srcIdx);
    if (it == puckTouches.end())
        return;

    Vec3 world;
    if (! unproject (e.position, it->second.onBack, world))
        return;

    const float az = std::atan2 (world.y, world.x) * kRadToDeg;
    const float el = std::asin (juce::jlimit (-1.0f, 1.0f, world.z)) * kRadToDeg;

    const auto& key = it->second.key;
    registry.applyLocalMove (key, az, el);
    const auto src = registry.findByKey (key);
    if (src.has_value())
        sendPositionUpdate (*src, az, el);
}

void SphereView::sendPositionUpdate (const EncoderSource& src, float azDeg, float elDeg)
{
    if (src.senderIp.isEmpty())
        return;

    if (src.origin == EncoderOrigin::Ambix)
    {
        if (src.replyPort > 0)
            sender.sendAmbiEncSet (src.senderIp, src.replyPort, src.id,
                                   azDeg, elDeg, src.size);
    }
    else
    {
        if (src.replyPort > 0)
        {
            sender.sendMultiEncoderParam (src.senderIp, src.replyPort, src.trackName,
                                          MultiEncoderPayload::Param::Azimuth,
                                          src.id, azDeg);
            sender.sendMultiEncoderParam (src.senderIp, src.replyPort, src.trackName,
                                          MultiEncoderPayload::Param::Elevation,
                                          src.id, elDeg);
        }
    }
}

void SphereView::mouseUp (const juce::MouseEvent& e)
{
    const int srcIdx = e.source.getIndex();

    const bool wasOrbiting = (srcIdx == orbitTouchIdx);
    if (wasOrbiting)
        orbitTouchIdx = -1;

    auto it = puckTouches.find (srcIdx);
    if (it != puckTouches.end())
    {
        registry.endDrag (it->second.key);
        puckTouches.erase (it);
    }

    if (wasOrbiting && onViewStateChanged)
        onViewStateChanged (yawDeg, pitchDeg, btnLock.getToggleState());

    repaint();
}

void SphereView::mouseDoubleClick (const juce::MouseEvent&)
{
    yawDeg = 0.0f;
    pitchDeg = 0.0f;
    repaint();
}

void SphereView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // Defer to the 30 Hz timer — see matching comment in
    // HammerAitovView::changeListenerCallback.
    cacheDirty = true;
}

void SphereView::timerCallback()
{
    if (! isShowing())
        return;

    if (cacheDirty)
    {
        cache = registry.snapshot();
        std::sort (cache.begin(), cache.end(),
                   [] (const EncoderSource& a, const EncoderSource& b)
                   { return a.lastInteractionMs < b.lastInteractionMs; });
        cacheDirty = false;
    }
    repaint();
}
