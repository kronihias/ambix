#include "MainComponent.h"

#include "view/SettingsPopup.h"

juce::Font MainComponent::TopBarLookAndFeel::getTextButtonFont (juce::TextButton& b, int buttonHeight)
{
    // The gear glyph rendered at the default ~15pt cap looks tiny in a 38x?
    // button — scale it up for any button that opts in via a property.
    if ((bool) b.getProperties().getWithDefault ("largeGlyph", false))
        return juce::Font (juce::FontOptions ((float) buttonHeight * 0.7f));

    return juce::LookAndFeel_V4::getTextButtonFont (b, buttonHeight);
}

int MainComponent::TopBarLookAndFeel::getTabButtonBestWidth (juce::TabBarButton& b, int tabDepth)
{
    // Default packs the label flush against the tab dividers. Add a bit of
    // breathing room either side.
    constexpr int kExtraHorizontalPadding = 28; // px, total (14 each side)
    return juce::LookAndFeel_V4::getTabButtonBestWidth (b, tabDepth) + kExtraHorizontalPadding;
}

MainComponent::MainComponent()
{
    settings = settingsStore.load();

    // Views are constructed *before* the body runs (member-initializer order),
    // so their ctors saw the default-initialised `settings`. Now that the
    // persisted settings are in, push them through — this re-syncs the 2D
    // grid's elevation range + flip state to the loaded upper-hemisphere
    // toggle, so the drawing matches the panning logic.
    hammerView.settingsChanged();
    sphereView.settingsChanged();

    setSize (1280, 800);

    // DiscoverPanel needs refs + a persistence callback — build it before tabs
    // reference it.
    discoverPanel = std::make_unique<DiscoverPanel> (
        advertiser, sender, settings, registry, groupRegistry,
        [this]() { persistSettings(); });

    const auto tabBg = juce::Colour (0xFF1B1B1B);
    tabs.addTab ("2D View",     tabBg, &hammerView,        false);
    tabs.addTab ("3D View",     tabBg, &sphereView,        false);
    tabs.addTab ("Source List", tabBg, &listPanel,         false);
    tabs.addTab ("Groups",      tabBg, &groupPanel,        false);
    tabs.addTab ("Discover",    tabBg, discoverPanel.get(), false);
    tabs.setOutline (0);
    tabs.setTabBarDepth (44);
    tabs.getTabbedButtonBar().setMinimumTabScaleFactor (1.25); // wider tabs
    tabs.getTabbedButtonBar().setLookAndFeel (&topBarLnf);     // extra tab padding

    const int loadedTab = juce::jlimit (0, tabs.getNumTabs() - 1, settings.currentTabIndex);
    tabs.setCurrentTabIndex (loadedTab, juce::dontSendNotification);
    tabs.onTabChanged = [this] (int idx)
    {
        if (idx != settings.currentTabIndex)
        {
            settings.currentTabIndex = idx;
            persistSettings();
        }
    };
    addAndMakeVisible (tabs);

    sphereView.setInitialViewState (settings.sphereYawDeg,
                                    settings.spherePitchDeg,
                                    settings.sphereLocked);
    sphereView.onViewStateChanged = [this] (float yaw, float pitch, bool locked)
    {
        if (juce::approximatelyEqual (settings.sphereYawDeg, yaw)
            && juce::approximatelyEqual (settings.spherePitchDeg, pitch)
            && settings.sphereLocked == locked)
            return;
        settings.sphereYawDeg   = yaw;
        settings.spherePitchDeg = pitch;
        settings.sphereLocked   = locked;
        persistSettings();
    };

    hammerView.setUpperOnlyCallback ([this] (bool v)
    {
        if (v == settings.upperHemisphereOnly) return;
        settings.upperHemisphereOnly = v;
        persistSettings();
        hammerView.settingsChanged();
    });

    // Flip L/R lives on both views' top bars; either toggle mutates the shared
    // setting and syncs the sibling view.
    auto onFlipLR = [this] (bool v)
    {
        if (v == settings.flipLeftRight) return;
        settings.flipLeftRight = v;
        persistSettings();
        hammerView.settingsChanged();
        sphereView.settingsChanged();
        repaint();
    };
    hammerView.setFlipLRCallback (onFlipLR);
    sphereView.setFlipLRCallback (onFlipLR);

    // Puck size lives on both views' top bars; either combo mutates the shared
    // setting and syncs the sibling view so the selection stays in lock-step.
    auto onPuckSize = [this] (AppSettings::PuckSize v)
    {
        if (v == settings.puckSize) return;
        settings.puckSize = v;
        persistSettings();
        hammerView.settingsChanged();
        sphereView.settingsChanged();
        repaint();
    };
    hammerView.setPuckSizeCallback (onPuckSize);
    sphereView.setPuckSizeCallback (onPuckSize);

    // --- Top bar -------------------------------------------------------------
    settingsButton.setTooltip ("Settings");
    settingsButton.onClick = [this]() { openSettingsPopup(); };
    settingsButton.getProperties().set ("largeGlyph", true); // bump gear size
    settingsButton.setLookAndFeel (&topBarLnf);
    addAndMakeVisible (settingsButton);

    clearButton.setTooltip ("Remove every puck. Sources return as new OSC arrives.");
    clearButton.onClick = [this]() { registry.clearAll(); };
    addAndMakeVisible (clearButton);

    ipLabel.setJustificationType (juce::Justification::centredLeft);
    ipLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    ipLabel.setColour (juce::Label::textColourId,
                       juce::Colour (0xFFE8D76C));
    ipLabel.setTooltip (
        "This device's local IP address(es). Configure your DAW/plugin to send "
        "OSC to <this ip>:<listen port>.");
    addAndMakeVisible (ipLabel);

    statusLabel.setJustificationType (juce::Justification::centredRight);
    statusLabel.setTooltip ("Live counters: UDP packets received vs. successfully parsed.");
    addAndMakeVisible (statusLabel);

    // -------------------------------------------------------------------------
    // Apply auto-clear state to the registry (it was loaded from settings).
    registry.setStaleTimeoutMs (settings.autoClearEnabled
                                ? static_cast<juce::uint32> (settings.autoClearTimeoutSec * 1000)
                                : 0u);

    restartListener();

    startTimerHz (4);
}

MainComponent::~MainComponent()
{
    stopTimer();
    listener.stopListening();

    // Detach our custom L&F before topBarLnf is destroyed.
    settingsButton.setLookAndFeel (nullptr);
    tabs.getTabbedButtonBar().setLookAndFeel (nullptr);
}

void MainComponent::openSettingsPopup()
{
    SettingsPopup::Callbacks cbs;
    cbs.onPersist             = [this]() { persistSettings(); };
    cbs.onListenPortChanged   = [this] (int)  { restartListener(); };
    cbs.onAutoClearChanged    = [this]()
    {
        registry.setStaleTimeoutMs (settings.autoClearEnabled
                                    ? static_cast<juce::uint32> (settings.autoClearTimeoutSec * 1000)
                                    : 0u);
    };
    cbs.onColorOrRangeChanged = [this]() { repaint(); };

    auto content = std::make_unique<SettingsPopup> (settings, std::move (cbs),
                                                    settingsStore.getFile().getFullPathName(),
                                                    settingsStore.fileExists());

    const auto btnBounds = settingsButton.getScreenBounds();
    juce::CallOutBox::launchAsynchronously (std::move (content),
                                            btnBounds,
                                            nullptr);
}

void MainComponent::persistSettings()
{
    settingsStore.save (settings);
}

void MainComponent::timerCallback()
{
    // Refresh IP label periodically.
    {
        const auto addresses = juce::IPAddress::getAllAddresses (false);
        juce::StringArray parts;
        for (const auto& a : addresses)
        {
            const auto s = a.toString();
            if (s.startsWith ("127.") || s.startsWith ("169.254.") || s == "0.0.0.0")
                continue;
            parts.add (s);
        }
        const auto ipText = parts.isEmpty() ? juce::String ("no network")
                                            : parts.joinIntoString (" / ");
        ipLabel.setText ("IP: " + ipText, juce::dontSendNotification);
    }

    if (! listener.isListening())
        return;

    juce::String text;
    text << "UDP " << listener.getListenPort()
         << "   rx: " << listener.getPacketsReceived()
         << "   parsed: " << listener.getPacketsParsed();
    statusLabel.setText (text, juce::dontSendNotification);
}

void MainComponent::handleIncoming (const OscListenerThread::IncomingMessage& msg)
{
    if (msg.kind == OscListenerThread::Kind::Ambix)
    {
        SourceRegistry::AmbixUpdate up;
        up.id           = msg.ambix.id;
        up.trackName    = msg.ambix.trackName;
        up.azimuthDeg   = msg.ambix.azimuthDeg;
        up.elevationDeg = msg.ambix.elevationDeg;
        up.size         = msg.ambix.size;
        up.rmsLinear    = msg.ambix.rmsLinear;
        up.peakLinear   = msg.ambix.peakLinear;
        up.senderIp     = msg.senderIp;
        up.replyPort    = msg.ambix.hasReplyPort ? msg.ambix.replyPort
                                                 : (7200 + msg.ambix.id);
        registry.applyAmbixUpdate (up);
    }
    else
    {
        SourceRegistry::MultiEncoderUpdate up;
        up.sourceIndex   = msg.multiEncoder.sourceIndex;
        up.value         = msg.multiEncoder.value;
        up.addressPrefix = msg.multiEncoder.addressPrefix;
        up.senderIp      = msg.senderIp;
        up.senderPort    = msg.senderPort;

        using SrcParam = SourceRegistry::MultiEncoderUpdate::Param;
        using MsgParam = MultiEncoderPayload::Param;
        switch (msg.multiEncoder.param)
        {
            case MsgParam::Azimuth:   up.param = SrcParam::Azimuth;   break;
            case MsgParam::Elevation: up.param = SrcParam::Elevation; break;
            case MsgParam::Gain:      up.param = SrcParam::Gain;      break;
            case MsgParam::Mute:      up.param = SrcParam::Mute;      break;
            case MsgParam::Solo:      up.param = SrcParam::Solo;      break;
        }
        registry.applyMultiEncoderUpdate (up);
    }
}

bool MainComponent::restartListener()
{
    listener.stopListening();

    const bool ok = listener.startListening (settings.listenPort,
        [this] (const OscListenerThread::IncomingMessage& msg) { handleIncoming (msg); });

    if (ok)
        statusLabel.setText ("Listening on UDP " + juce::String (listener.getListenPort()),
                             juce::dontSendNotification);
    else
        statusLabel.setText ("Failed to bind port " + juce::String (settings.listenPort),
                             juce::dontSendNotification);

    return ok;
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF141414));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Respect the iOS status-bar / home-indicator safe areas (no-op on desktop
    // where insets are zero).
    if (auto* d = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
        area = d->safeAreaInsets.subtractedFrom (area);

    auto row = area.removeFromTop (44).reduced (8, 6);
    // 32 px tall (row height minus reduce) * a bit wider than square so the
    // gear glyph isn't horizontally squeezed. 48 reads visually balanced.
    settingsButton.setBounds (row.removeFromLeft (48));
    row.removeFromLeft (6);
    clearButton.setBounds (row.removeFromLeft (90));
    row.removeFromLeft (10);
    statusLabel.setBounds (row.removeFromRight (260));
    ipLabel.setBounds (row);

    tabs.setBounds (area.reduced (4));
}
