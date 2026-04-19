#pragma once

#include <JuceHeader.h>

#include "../app/Settings.h"
#include "../model/GroupRegistry.h"
#include "../model/SourceRegistry.h"
#include "../net/OscCommandSender.h"
#include "GroupVisibilityPanel.h"

// 3D globe of the ambix sphere. Base projection keeps ambix FRONT at screen
// top, RIGHT at screen right, UP at camera depth (centre). Orbit drag rotates
// the world *before* this fixed projection, so spinning the globe shows
// different faces without ever mirroring — FRONT and LEFT stay 90° apart and
// move together, like labels painted on a real globe.
class SphereView : public juce::Component,
                   public juce::ChangeListener,
                   private juce::Timer
{
public:
    SphereView (SourceRegistry& registry,
                OscCommandSender& sender,
                const AppSettings& settings,
                GroupRegistry& groups);
    ~SphereView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void settingsChanged();

    // Initial persisted state + change callback so MainComponent can save to
    // the settings file whenever the view's orbit / lock state changes.
    void setInitialViewState (float yawDeg, float pitchDeg, bool locked);
    std::function<void (float yawDeg, float pitchDeg, bool locked)> onViewStateChanged;

    // Called from the 3D view's own "Flip L/R" toggle so MainComponent can
    // persist the setting and notify the 2D view too.
    void setFlipLRCallback (std::function<void (bool)> callback);

    // Called from the 3D view's own "Puck size" dropdown so MainComponent
    // can persist the setting and notify the 2D view too.
    void setPuckSizeCallback (std::function<void (AppSettings::PuckSize)> callback);

private:
    struct Vec3 { float x, y, z; };

    void timerCallback() override;

    Vec3 sphericalToCart (float azDeg, float elDeg) const noexcept;
    Vec3 rotateByCamera (const Vec3& world) const noexcept;
    Vec3 unrotateByCamera (const Vec3& cam) const noexcept;
    juce::Point<float> project (const Vec3& cam) const noexcept;
    bool unproject (juce::Point<float> screen, bool preferBack, Vec3& worldOut) const noexcept;

    float puckRadius (const EncoderSource&) const noexcept;

    void updateLayout();

    void sendPositionUpdate (const EncoderSource& src, float azDeg, float elDeg);

    SourceRegistry&   registry;
    OscCommandSender& sender;
    const AppSettings& settings;
    GroupRegistry&    groups;
    GroupVisibilityPanel groupPanel;
    std::vector<EncoderSource> cache;
    // Set when a change fires while the tab is hidden. Next show tick
    // (timerCallback) refreshes the cache lazily.
    bool cacheDirty { false };

    juce::Rectangle<int> sphereBounds;
    juce::Point<float> centre;
    float radiusPx { 150.0f };

    // Orbit state. Default (0, 0) = top plan view: FRONT at screen top,
    // RIGHT at screen right, UP at camera depth.
    float yawDeg   { 0.0f };   // rotation around world Z (spins the globe)
    float pitchDeg { 0.0f };   // rotation around world Y (tilts the globe)

    juce::TextButton btnTop   { "TOP"   };
    juce::TextButton btnFront { "FRONT" };
    juce::TextButton btnBack  { "BACK"  };
    juce::TextButton btnLeft  { "LEFT"  };
    juce::TextButton btnRight { "RIGHT" };
    juce::ToggleButton btnLock { "Lock view" };
    juce::ToggleButton btnFlipLR { "Flip L/R" };
    std::function<void (bool)> onFlipLRChanged;

    juce::Label    puckSizeLabel;
    juce::ComboBox puckSizeCombo;
    std::function<void (AppSettings::PuckSize)> onPuckSizeChanged;

    struct PuckTouch
    {
        EncoderSource::Key key;
        bool onBack { false };
    };
    std::map<int, PuckTouch> puckTouches; // source idx → puck info

    // Exactly one touch at a time may orbit; extra empty-area touches are ignored.
    int   orbitTouchIdx   { -1 };
    juce::Point<float> orbitStart;
    float orbitStartYaw   { 0.0f };
    float orbitStartPitch { 0.0f };

    bool isKeyBeingDragged (const EncoderSource::Key& k) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SphereView)
};
