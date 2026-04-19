#pragma once

#include <JuceHeader.h>

#include "../app/Settings.h"
#include "../model/GroupRegistry.h"
#include "../model/SourceRegistry.h"
#include "../net/OscCommandSender.h"
#include "GroupVisibilityPanel.h"
#include "third_party/HammerAitovGrid.h"

class HammerAitovView : public juce::Component,
                        public juce::ChangeListener,
                        private juce::Timer
{
public:
    HammerAitovView (SourceRegistry& registry,
                     OscCommandSender& sender,
                     const AppSettings& settings,
                     GroupRegistry& groups);
    ~HammerAitovView() override;

    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    // Call when AppSettings changes (e.g. upper-hemisphere toggle) so the
    // grid + transform rebuild.
    void settingsChanged();

    // Called from the 2D view's own "Upper only" toggle so MainComponent can
    // persist the setting and notify the 3D view too.
    void setUpperOnlyCallback (std::function<void (bool)> callback);

    // Called from the 2D view's own "Flip L/R" toggle so MainComponent can
    // persist the setting and notify the 3D view too.
    void setFlipLRCallback (std::function<void (bool)> callback);

    // Called from the 2D view's own "Puck size" dropdown so MainComponent
    // can persist the setting and notify the 3D view too.
    void setPuckSizeCallback (std::function<void (AppSettings::PuckSize)> callback);

private:
    juce::Point<float> sphericalToLocal (float azimuthDeg, float elevationDeg) const;
    bool localToSpherical (juce::Point<float> localPt, float& azimuthDeg, float& elevationDeg) const;

    float puckRadius (const EncoderSource& s) const;

    void rebuildTransform();

    const EncoderSource* hitTest (juce::Point<float> localPt) const;

    void sendPositionUpdate (const EncoderSource& src, float azDeg, float elDeg);

    SourceRegistry& registry;
    OscCommandSender& sender;
    const AppSettings& settings;
    GroupRegistry& groups;
    HammerAitovGrid grid;
    GroupVisibilityPanel groupPanel;

    juce::AffineTransform toArea;     // normalized → local
    juce::AffineTransform fromArea;   // local → normalized
    juce::Rectangle<float> projectionBounds;

    std::vector<EncoderSource> cache;
    // Set when a change fires while the tab is hidden. Next show tick
    // (timerCallback) refreshes the cache lazily.
    bool cacheDirty { false };

    juce::ToggleButton upperOnlyToggle { "Upper only" };
    std::function<void (bool)> onUpperOnlyChanged;

    juce::ToggleButton flipLRToggle { "Flip L/R" };
    std::function<void (bool)> onFlipLRChanged;

    juce::Label    puckSizeLabel;
    juce::ComboBox puckSizeCombo;
    std::function<void (AppSettings::PuckSize)> onPuckSizeChanged;

    // Per-touch drag map (MouseSource index → puck key) so multi-touch lets
    // the user drag multiple pucks simultaneously on iPad.
    std::map<int, EncoderSource::Key> puckTouches;

    bool isKeyBeingDragged (const EncoderSource::Key& k) const;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HammerAitovView)
};
