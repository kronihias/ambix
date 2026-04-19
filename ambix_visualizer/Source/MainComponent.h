#pragma once

#include <JuceHeader.h>

#include "app/Settings.h"
#include "model/GroupRegistry.h"
#include "model/SourceRegistry.h"
#include "net/NetworkAdvertiser.h"
#include "net/OscCommandSender.h"
#include "net/OscListenerThread.h"
#include "view/DiscoverPanel.h"
#include "view/GroupPanel.h"
#include "view/HammerAitovView.h"
#include "view/SourceListPanel.h"
#include "view/SphereView.h"

class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    bool restartListener();
    void timerCallback() override;

    void handleIncoming (const OscListenerThread::IncomingMessage& msg);

    void persistSettings();
    void openSettingsPopup();

    SettingsStore settingsStore;
    AppSettings settings;

    SourceRegistry registry;
    GroupRegistry  groupRegistry;
    OscCommandSender sender;
    OscListenerThread listener;
    NetworkAdvertiser advertiser;

    HammerAitovView hammerView { registry, sender, settings, groupRegistry };
    SphereView      sphereView { registry, sender, settings, groupRegistry };
    SourceListPanel listPanel  { registry, groupRegistry, advertiser };
    GroupPanel      groupPanel { groupRegistry };
    // DiscoverPanel is constructed in the ctor body (needs advertiser + sender refs + a persistSettings lambda)
    std::unique_ptr<DiscoverPanel> discoverPanel;

    // Tab change must not cause any TextEditor to grab focus — on iOS that
    // would pop up the software keyboard even when the user just wanted to
    // switch views. Override `currentTabChanged` to unfocus everything, and
    // notify a callback so the main component can persist the tab index.
    struct NoAutoFocusTabbedComponent : public juce::TabbedComponent
    {
        using juce::TabbedComponent::TabbedComponent;
        std::function<void (int)> onTabChanged;
        void currentTabChanged (int newIndex, const juce::String&) override
        {
            juce::Component::unfocusAllComponents();
            if (onTabChanged)
                onTabChanged (newIndex);
        }
    };

    NoAutoFocusTabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };

    // Custom L&F: bigger font for the gear glyph on the settings button, and
    // a bit of extra horizontal padding on tab buttons so the labels aren't
    // cramped against the dividers.
    struct TopBarLookAndFeel : public juce::LookAndFeel_V4
    {
        juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
        int getTabButtonBestWidth (juce::TabBarButton&, int tabDepth) override;
    };
    TopBarLookAndFeel topBarLnf;

    // Compact top bar: gear button → settings popup, Clear all, IP, status.
    juce::TextButton settingsButton { juce::String (juce::CharPointer_UTF8 ("\xe2\x9a\x99")) }; // ⚙
    juce::TextButton clearButton    { "Clear source list" };
    juce::Label      ipLabel;
    juce::Label      statusLabel;

    juce::TooltipWindow tooltipWindow { this, 600 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
