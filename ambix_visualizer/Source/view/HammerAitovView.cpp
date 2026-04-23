#include "HammerAitovView.h"
#include "PuckRenderer.h"
#include "SelectionOverlay.h"
#include "third_party/HammerAitov.h"
#include "third_party/Conversions.h"

HammerAitovView::HammerAitovView (SourceRegistry& r,
                                  OscCommandSender& s,
                                  const AppSettings& st,
                                  GroupRegistry& gr)
    : registry (r), sender (s), settings (st), groups (gr), groupPanel (gr)
{
    // Clicks on the view must not propagate up to MainComponent where
    // traversal would land on portEditor (and pop up the iOS keyboard).
    setMouseClickGrabsKeyboardFocus (false);

    addAndMakeVisible (grid);
    grid.setInterceptsMouseClicks (false, false); // let drags reach this view
    grid.setAmbixElevationRange (settings.minVisibleEleDeg(), settings.maxVisibleEleDeg());
    grid.setFlipLeftRight (settings.flipLeftRight);

    // Groups panel goes on top of the grid (JUCE paints children in addition
    // order; later-added wins). Add after the grid so the panel is visible.
    addAndMakeVisible (groupPanel);

    groups.addChangeListener (this);

    upperOnlyToggle.setToggleState (settings.upperHemisphereOnly, juce::dontSendNotification);
    upperOnlyToggle.setTooltip (juce::String (juce::CharPointer_UTF8 (
        "Show only the upper hemisphere (plus ~10\xc2\xb0 below the equator). "
        "Useful for loudspeaker domes where nothing lives below the floor.")));
    // Redundant onClick + onStateChange so the callback fires even when iOS's
    // first-tap lands on a pre-focus transition and onClick is swallowed.
    auto notifyChange = [this]()
    {
        if (onUpperOnlyChanged)
            onUpperOnlyChanged (upperOnlyToggle.getToggleState());
    };
    upperOnlyToggle.onClick       = notifyChange;
    upperOnlyToggle.onStateChange = notifyChange;
    addAndMakeVisible (upperOnlyToggle);

    flipLRToggle.setToggleState (settings.flipLeftRight, juce::dontSendNotification);
    flipLRToggle.setTooltip ("Mirror left/right on both the 2D and 3D views. "
                             "Useful when the visualizer is facing the audience instead of the listener.");
    auto notifyFlip = [this]()
    {
        if (onFlipLRChanged)
            onFlipLRChanged (flipLRToggle.getToggleState());
    };
    flipLRToggle.onClick       = notifyFlip;
    flipLRToggle.onStateChange = notifyFlip;
    addAndMakeVisible (flipLRToggle);

    puckSizeLabel.setText ("Pucks:", juce::dontSendNotification);
    puckSizeLabel.setJustificationType (juce::Justification::centredRight);
    puckSizeLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (puckSizeLabel);

    // ComboBox IDs are 1-based; keep them aligned with PuckSize enum + 1.
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

    showMetersToggle.setToggleState (settings.showPuckLevelMeter, juce::dontSendNotification);
    showMetersToggle.setTooltip ("Show a vertical level meter beside each ambix_encoder source. "
                                 "Applies to both 2D and 3D views.");
    auto notifyMeters = [this]()
    {
        if (onShowMetersChanged)
            onShowMetersChanged (showMetersToggle.getToggleState());
    };
    showMetersToggle.onClick       = notifyMeters;
    showMetersToggle.onStateChange = notifyMeters;
    addAndMakeVisible (showMetersToggle);

    registry.addChangeListener (this);
    cache = registry.snapshot();
    startTimerHz (60); // for smooth peak-ring animation
}

HammerAitovView::~HammerAitovView()
{
    registry.removeChangeListener (this);
    groups.removeChangeListener (this);
    stopTimer();
}

void HammerAitovView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1B1B1B));
}

void HammerAitovView::paintOverChildren (juce::Graphics& g)
{
    const float minEle = settings.minVisibleEleDeg();
    const float maxEle = settings.maxVisibleEleDeg();

    std::vector<const EncoderSource*> selected;
    for (const auto& src : cache)
    {
        if (src.elevationDeg < minEle - 1.0f || src.elevationDeg > maxEle + 1.0f)
            continue;
        if (! groups.isVisible (src.groupId))
            continue;
        const auto p = sphericalToLocal (src.azimuthDeg, src.elevationDeg);
        const float r = puckRadius (src);
        const bool isSel = isKeyBeingDragged (src.key());
        PuckRenderer::draw (g, src, p, r, isSel, false, settings);
        if (isSel)
            selected.push_back (&src);
    }

    SelectionOverlay::draw (g, selected, getLocalBounds());
}

bool HammerAitovView::isKeyBeingDragged (const EncoderSource::Key& k) const
{
    for (const auto& kv : puckTouches)
        if (kv.second == k)
            return true;
    return false;
}

void HammerAitovView::resized()
{
    grid.setBounds (getLocalBounds());
    // Bigger touch target for the "Upper only" + "Flip L/R" toggles.
    upperOnlyToggle.setBounds (10, 8, 130, 32);
    flipLRToggle   .setBounds (146, 8, 110, 32);
    puckSizeLabel  .setBounds (262, 8, 52, 32);
    puckSizeCombo  .setBounds (316, 8, 96, 32);
    showMetersToggle.setBounds (420, 8, 96, 32);

    // Group visibility panel floats at the top-right.
    const int gpH = groupPanel.getPreferredHeight();
    const int gpW = groupPanel.getPreferredWidth();
    if (gpH > 0)
        groupPanel.setBounds (getWidth() - gpW - 10, 8, gpW, gpH);
    else
        groupPanel.setBounds (getWidth() - gpW - 10, 8, gpW, 0);
    rebuildTransform();
}

void HammerAitovView::rebuildTransform()
{
    const auto area = getLocalBounds().toFloat();
    projectionBounds = area;

    // Same math as HammerAitovGrid::computeToArea so pucks align exactly.
    toArea = HammerAitovGrid::computeToArea (area,
                                             settings.minVisibleEleDeg(),
                                             settings.maxVisibleEleDeg(),
                                             settings.flipLeftRight);
    fromArea = toArea.inverted();
}

void HammerAitovView::settingsChanged()
{
    grid.setAmbixElevationRange (settings.minVisibleEleDeg(), settings.maxVisibleEleDeg());
    grid.setFlipLeftRight (settings.flipLeftRight);
    upperOnlyToggle.setToggleState (settings.upperHemisphereOnly, juce::dontSendNotification);
    flipLRToggle   .setToggleState (settings.flipLeftRight,       juce::dontSendNotification);
    puckSizeCombo  .setSelectedId  (static_cast<int> (settings.puckSize) + 1,
                                    juce::dontSendNotification);
    showMetersToggle.setToggleState (settings.showPuckLevelMeter, juce::dontSendNotification);
    rebuildTransform();
    repaint();
}

void HammerAitovView::setUpperOnlyCallback (std::function<void (bool)> callback)
{
    onUpperOnlyChanged = std::move (callback);
}

void HammerAitovView::setFlipLRCallback (std::function<void (bool)> callback)
{
    onFlipLRChanged = std::move (callback);
}

void HammerAitovView::setPuckSizeCallback (std::function<void (AppSettings::PuckSize)> callback)
{
    onPuckSizeChanged = std::move (callback);
}

void HammerAitovView::setShowMetersCallback (std::function<void (bool)> callback)
{
    onShowMetersChanged = std::move (callback);
}

juce::Point<float> HammerAitovView::sphericalToLocal (float azimuthDeg, float elevationDeg) const
{
    // ambix uses clockwise-positive azimuth (+90° = right); HammerAitov uses
    // math convention (+90° = left). Also, HammerAitovGrid labels +ele near
    // the top of the screen via `anglesToPoint(0, -ele)`. Negate both so that
    // ambix (+az = right, +el = up) maps to the expected screen positions.
    const float azRad = Conversions<float>::degreesToRadians (-azimuthDeg);
    const float elRad = Conversions<float>::degreesToRadians (-elevationDeg);
    auto p = HammerAitov::sphericalToXY (azRad, elRad);
    p.applyTransform (toArea);
    return p;
}

bool HammerAitovView::localToSpherical (juce::Point<float> localPt,
                                        float& azimuthDeg,
                                        float& elevationDeg) const
{
    auto norm = localPt;
    norm.applyTransform (fromArea);

    // Clamp to the HA disk (|x/√2|² + |y/√2|² ≤ 1) to avoid NaN in inverse.
    constexpr float sqrt2Half = 0.7071067811865476f;
    const float xS = norm.x * sqrt2Half;
    const float yS = norm.y * sqrt2Half;
    const float r2 = xS * xS + yS * yS;
    if (r2 > 0.999f)
    {
        const float scale = std::sqrt (0.999f / r2);
        norm.x *= scale;
        norm.y *= scale;
    }

    float az = 0.0f, el = 0.0f;
    HammerAitov::XYToSpherical (norm.x, norm.y, az, el);
    if (std::isnan (az) || std::isnan (el))
        return false;

    azimuthDeg   = -Conversions<float>::radiansToDegrees (az);
    elevationDeg = -Conversions<float>::radiansToDegrees (el);
    return true;
}

float HammerAitovView::puckRadius (const EncoderSource& s) const
{
    const float scale = PuckRenderer::sizeScale (settings.puckSize);
    const float base = juce::jmap (juce::jlimit (0.0f, 1.0f, s.size),
                                   PuckRenderer::kMinRadiusPx * scale,
                                   PuckRenderer::kMaxRadiusPx * scale);
    return base;
}

const EncoderSource* HammerAitovView::hitTest (juce::Point<float> localPt) const
{
    // Generous hit area: visual radius + 18 px ring, minimum 36 px (72 px
    // diameter) so a finger-tip can grab even the smallest puck reliably
    // during multi-touch. Among pucks within range, the most recently
    // interacted one wins — so the visual "foreground" puck grabs first.
    const EncoderSource* best = nullptr;
    juce::uint32 bestTime = 0;
    float bestDist = std::numeric_limits<float>::max();
    for (const auto& src : cache)
    {
        if (! groups.isVisible (src.groupId))
            continue;
        const auto p = sphericalToLocal (src.azimuthDeg, src.elevationDeg);
        const float r = juce::jmax (puckRadius (src) + 18.0f, 36.0f);
        const float d = p.getDistanceFrom (localPt);
        if (d > r) continue;

        const bool newer = src.lastInteractionMs > bestTime;
        const bool sameTimeCloser = (src.lastInteractionMs == bestTime && d < bestDist);
        if (best == nullptr || newer || sameTimeCloser)
        {
            best = &src;
            bestTime = src.lastInteractionMs;
            bestDist = d;
        }
    }
    return best;
}

void HammerAitovView::mouseDown (const juce::MouseEvent& e)
{
    if (auto* src = hitTest (e.position))
    {
        const auto key = src->key();
        puckTouches[e.source.getIndex()] = key;
        registry.beginDrag (key);
    }
    repaint();
}

void HammerAitovView::mouseDrag (const juce::MouseEvent& e)
{
    auto it = puckTouches.find (e.source.getIndex());
    if (it == puckTouches.end())
        return;

    float az = 0.0f, el = 0.0f;
    if (! localToSpherical (e.position, az, el))
        return;

    el = juce::jlimit (settings.minVisibleEleDeg(), settings.maxVisibleEleDeg(), el);

    const auto& key = it->second;
    registry.applyLocalMove (key, az, el);

    const auto src = registry.findByKey (key);
    if (src.has_value())
        sendPositionUpdate (*src, az, el);
}

void HammerAitovView::sendPositionUpdate (const EncoderSource& src, float azDeg, float elDeg)
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

void HammerAitovView::mouseUp (const juce::MouseEvent& e)
{
    auto it = puckTouches.find (e.source.getIndex());
    if (it != puckTouches.end())
    {
        registry.endDrag (it->second);
        puckTouches.erase (it);
    }
    repaint();
}

void HammerAitovView::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // Always defer to the 60 Hz timer rather than rebuilding inline. This
    // decouples the OSC packet rate from the render rate: the message thread
    // sets a flag here and the timer does one snapshot+sort+repaint per frame,
    // regardless of how many packets arrived in between.
    cacheDirty = true;
}

void HammerAitovView::timerCallback()
{
    // Drive peak-ring relax + mild animation even when no messages arrive.
    // Skip on hidden tabs so we don't waste message-thread ticks compositing
    // a view the user can't see.
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
