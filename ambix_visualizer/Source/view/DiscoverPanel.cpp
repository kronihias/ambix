#include "DiscoverPanel.h"

namespace
{
    juce::StringArray splitIds (const juce::String& packed)
    {
        juce::StringArray out;
        out.addTokens (packed, "\n", {});
        out.removeEmptyStrings();
        return out;
    }
    juce::String joinIds (const juce::StringArray& ids)
    {
        return ids.joinIntoString ("\n");
    }
}

//==============================================================================
class DiscoverPanel::SubscribeCell : public juce::Component
{
public:
    explicit SubscribeCell (DiscoverPanel& p) : panel (p)
    {
        toggle.onClick = [this]()
        {
            if (encoderUuid.isNotEmpty())
                panel.setSubscribed (encoderUuid, toggle.getToggleState());
        };
        addAndMakeVisible (toggle);
    }
    void resized() override { toggle.setBounds (getLocalBounds().withSizeKeepingCentre (28, 28)); }

    void setRow (const juce::String& uuid, bool subscribed)
    {
        encoderUuid = uuid;
        toggle.setToggleState (subscribed, juce::dontSendNotification);
        // Encoders from older builds (no euid broadcast — DAW is still
        // running a cached pre-euid VST3 binary) are visible but can't be
        // subscribed to. Tooltip explains why.
        const bool canSubscribe = uuid.isNotEmpty();
        toggle.setEnabled (canSubscribe);
        toggle.setTooltip (canSubscribe
            ? juce::String()
            : juce::String ("Encoder is advertising but did not broadcast a stable ID "
                            "(euid). Restart your DAW to pick up the rebuilt encoder "
                            "plugin."));
    }

private:
    DiscoverPanel& panel;
    juce::ToggleButton toggle;
    juce::String encoderUuid;
};

//==============================================================================
DiscoverPanel::DiscoverPanel (NetworkAdvertiser& adv,
                              OscCommandSender& s,
                              AppSettings& appSettings,
                              SourceRegistry& srcReg,
                              GroupRegistry& grpReg,
                              std::function<void()> persist)
    : advertiser (adv), sender (s), settings (appSettings),
      sourceRegistry (srcReg), groupRegistry (grpReg),
      persistFn (std::move (persist))
{
    auto& header = table.getHeader();
    header.addColumn ("Connect", ColSubscribed, 70,  50, 100);
    header.addColumn ("Track",   ColTrack,     180, 80, 400);
    header.addColumn ("ID",      ColId,         50, 40, 80);
    header.addColumn ("Host",    ColHost,      130, 80, 250);
    header.addColumn ("DAW",     ColDaw,       120, 70, 220);
    header.addColumn ("Project", ColProject,   150, 80, 300);
    header.addColumn ("Endpoint",ColEndpoint,  150, 80, 300);

    table.setModel (this);
    table.setRowHeight (28);
    header.setStretchToFitActive (true);
    addAndMakeVisible (table);

    emptyLabel.setJustificationType (juce::Justification::centred);
    emptyLabel.setColour (juce::Label::textColourId,
                          juce::Colours::white.withAlpha (0.55f));
    emptyLabel.setText ("No encoders discovered yet.\n"
                        "Make sure the encoder plugin has \"Discoverable on network\" enabled "
                        "and both machines are on the same LAN.",
                        juce::dontSendNotification);
    addAndMakeVisible (emptyLabel);

    // Em-dash is spelled out as its UTF-8 bytes (\xE2\x80\x94) and the whole
    // string is wrapped in CharPointer_UTF8. On iOS, JUCE's String(const char*)
    // asserts when it gets raw non-ASCII bytes through the default char
    // constructor; the wrapper tells it the bytes are already valid UTF-8.
    refreshButton.setTooltip (juce::String (juce::CharPointer_UTF8 (
        "Restart network discovery. Use this if encoders you expect to see "
        "aren't showing up \xe2\x80\x94 typical after Wi-Fi reconnect or sleep/wake. "
        "The list will clear and repopulate within a couple of seconds.")));
    refreshButton.onClick = [this]()
    {
        // Tear down + recreate the NSD sockets. Cache clears immediately;
        // encoders repopulate as their next broadcast arrives. Re-assert any
        // persisted subscriptions so they don't drift out of sync.
        advertiser.refresh();
        reassertSubscriptions();
    };
    addAndMakeVisible (refreshButton);

    connectAllButton.setTooltip ("Subscribe to every encoder currently listed.");
    connectAllButton.onClick = [this]() { connectAll(); };
    addAndMakeVisible (connectAllButton);

    disconnectAllButton.setTooltip ("Unsubscribe from every encoder currently listed.");
    disconnectAllButton.onClick = [this]() { disconnectAll(); };
    addAndMakeVisible (disconnectAllButton);

    advertiser.addChangeListener (this);
    cache = advertiser.getEncoders();

    // Heartbeat — re-send subscribes every few seconds so each subscribed
    // encoder's `lastActivity` stays fresh, even if the encoder's NSD listener
    // didn't bind (multiple plugin instances per process on macOS).
    startTimer (3000);
}

DiscoverPanel::~DiscoverPanel()
{
    stopTimer();
    advertiser.removeChangeListener (this);
}

void DiscoverPanel::timerCallback()
{
    reassertSubscriptions();
}

void DiscoverPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1B1B1B));
}

void DiscoverPanel::resized()
{
    auto area = getLocalBounds();

    // Action bar at the bottom: Disconnect all on the far right, Connect all
    // next to it, Refresh on the far left (it acts on the whole view, not on
    // the per-row connection state — keep it visually separated).
    auto bottom = area.removeFromBottom (40).reduced (8, 6);
    disconnectAllButton.setBounds (bottom.removeFromRight (140));
    bottom.removeFromRight (8);
    connectAllButton.setBounds    (bottom.removeFromRight (120));
    refreshButton.setBounds       (bottom.removeFromLeft  (110));

    table.setBounds (area);
    emptyLabel.setBounds (area.reduced (20));
}

int DiscoverPanel::getNumRows() { return static_cast<int> (cache.size()); }

void DiscoverPanel::paintRowBackground (juce::Graphics& g, int row, int, int, bool selected)
{
    const auto base = (row % 2 == 0) ? juce::Colour (0xFF262626) : juce::Colour (0xFF202020);
    g.fillAll (selected ? base.brighter (0.15f) : base);
}

void DiscoverPanel::paintCell (juce::Graphics& g, int row, int col, int w, int h, bool)
{
    if (row < 0 || row >= static_cast<int> (cache.size())) return;
    const auto& e = cache[static_cast<size_t> (row)];

    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.setFont (juce::FontOptions (13.0f));

    auto area = juce::Rectangle<int> (4, 0, w - 8, h);

    const auto endpoint = e.ip.toString() + ":" + juce::String (e.port);

    switch (col)
    {
        case ColTrack:    g.drawText (e.track, area, juce::Justification::centredLeft, true); break;
        case ColId:       g.drawText (juce::String (e.encoderId), area, juce::Justification::centredLeft, false); break;
        case ColHost:     g.drawText (e.host, area, juce::Justification::centredLeft, true); break;
        case ColDaw:      g.drawText (e.daw,  area, juce::Justification::centredLeft, true); break;
        case ColProject:
        {
            // Project text — rendered underlined to hint it's clickable
            // (opens the "add group…" menu). Placeholder is built from the
            // em-dash codepoint (U+2014) rather than a UTF-8 source literal
            // to side-step any narrow-string encoding surprises between the
            // source file and juce::String.
            const bool hasProject = e.project.isNotEmpty();
            const auto text = hasProject
                ? e.project
                : juce::String::charToString ((juce::juce_wchar) 0x2014);
            juce::AttributedString as;
            as.append (text,
                       juce::FontOptions (13.0f).withStyle (hasProject ? "underline" : juce::String()),
                       juce::Colours::white.withAlpha (hasProject ? 0.9f : 0.4f));
            as.setJustification (juce::Justification::centredLeft);
            as.draw (g, area.toFloat());
            break;
        }
        case ColEndpoint: g.drawText (endpoint, area, juce::Justification::centredLeft, true); break;
        case ColSubscribed: break; // rendered by SubscribeCell
        default: break;
    }
}

juce::Component* DiscoverPanel::refreshComponentForCell (int row, int col, bool,
                                                        juce::Component* existing)
{
    if (col != ColSubscribed)
    {
        delete existing;
        return nullptr;
    }
    if (row < 0 || row >= static_cast<int> (cache.size()))
    {
        delete existing;
        return nullptr;
    }
    auto* cell = dynamic_cast<SubscribeCell*> (existing);
    if (cell == nullptr) { delete existing; cell = new SubscribeCell (*this); }
    const auto& e = cache[static_cast<size_t> (row)];
    cell->setRow (e.encoderUuid, isSubscribed (e.encoderUuid));
    return cell;
}

bool DiscoverPanel::isSubscribed (const juce::String& encoderUuid) const
{
    if (encoderUuid.isEmpty()) return false;
    const auto ids = splitIds (settings.subscribedEncoderIds);
    return ids.contains (encoderUuid);
}

void DiscoverPanel::setSubscribed (const juce::String& encoderUuid, bool on)
{
    // Defensive — SubscribeCell already gates the toggle, but any other caller
    // (ConnectAll, a future keyboard shortcut, etc.) must not push an empty
    // UUID into the persisted list: it would falsely match every other
    // encoder-without-euid row on the next startup.
    if (encoderUuid.isEmpty()) return;

    // Find the matching encoder in the cache for address / port info.
    const NetworkAdvertiser::DiscoveredEncoder* match = nullptr;
    for (const auto& e : cache) if (e.encoderUuid == encoderUuid) { match = &e; break; }

    auto ids = splitIds (settings.subscribedEncoderIds);
    const bool already = ids.contains (encoderUuid);

    if (on && ! already) ids.add (encoderUuid);
    if (! on && already) ids.removeString (encoderUuid);

    settings.subscribedEncoderIds = joinIds (ids);
    if (persistFn) persistFn();

    if (match != nullptr)
    {
        if (on)
        {
            // Pick the best local IP to include in the subscribe message so
            // the encoder doesn't have to wait for NSD to resolve our UUID.
            juce::String ownIp;
            for (const auto& a : juce::IPAddress::getAllAddresses (false))
            {
                const auto s = a.toString();
                if (s.startsWith ("127.") || s.startsWith ("169.254.") || s == "0.0.0.0")
                    continue;
                ownIp = s; break;
            }
            sender.sendSubscribe (match->ip.toString(), match->port,
                                  settings.visualizerUuid,
                                  settings.listenPort,
                                  juce::SystemStats::getComputerName(),
                                  ownIp);
        }
        else
            sender.sendUnsubscribe (match->ip.toString(), match->port,
                                    settings.visualizerUuid);
    }

    table.updateContent();
    table.repaint();
}

void DiscoverPanel::connectAll()
{
    if (cache.empty())
        return;

    // Pick our best local IP so subscribes carry it (avoids the encoder having
    // to resolve us via NSD). Matches the logic in setSubscribed().
    juce::String ownIp;
    for (const auto& a : juce::IPAddress::getAllAddresses (false))
    {
        const auto s = a.toString();
        if (s.startsWith ("127.") || s.startsWith ("169.254.") || s == "0.0.0.0")
            continue;
        ownIp = s; break;
    }

    auto ids = splitIds (settings.subscribedEncoderIds);
    bool anyAdded = false;

    for (const auto& e : cache)
    {
        // Skip encoders advertising without euid — see SubscribeCell::setRow.
        if (e.encoderUuid.isEmpty())
            continue;

        if (! ids.contains (e.encoderUuid))
        {
            ids.add (e.encoderUuid);
            anyAdded = true;
        }
        // Always (re)send subscribe — cheap, and covers the case where we
        // were already in the list but the encoder has since forgotten us.
        sender.sendSubscribe (e.ip.toString(), e.port,
                              settings.visualizerUuid,
                              settings.listenPort,
                              juce::SystemStats::getComputerName(),
                              ownIp);
    }

    if (anyAdded)
    {
        settings.subscribedEncoderIds = joinIds (ids);
        if (persistFn) persistFn();
    }

    table.updateContent();
    table.repaint();
}

void DiscoverPanel::disconnectAll()
{
    const auto subscribedIds = splitIds (settings.subscribedEncoderIds);
    if (subscribedIds.isEmpty())
        return;

    // Send /ambi_enc_unsubscribe to every encoder we can still see on the net.
    // Any entry that's no longer discovered will time out on the encoder side
    // once heartbeats stop.
    for (const auto& e : cache)
    {
        if (subscribedIds.contains (e.encoderUuid))
            sender.sendUnsubscribe (e.ip.toString(), e.port, settings.visualizerUuid);
    }

    settings.subscribedEncoderIds.clear();
    if (persistFn) persistFn();

    table.updateContent();
    table.repaint();
}

void DiscoverPanel::reassertSubscriptions()
{
    const auto ids = splitIds (settings.subscribedEncoderIds);

    juce::String ownIp;
    for (const auto& a : juce::IPAddress::getAllAddresses (false))
    {
        const auto s = a.toString();
        if (s.startsWith ("127.") || s.startsWith ("169.254.") || s == "0.0.0.0")
            continue;
        ownIp = s; break;
    }

    for (const auto& e : cache)
    {
        if (! ids.contains (e.encoderUuid))
            continue;
        sender.sendSubscribe (e.ip.toString(), e.port,
                              settings.visualizerUuid,
                              settings.listenPort,
                              juce::SystemStats::getComputerName(),
                              ownIp);
    }
}

void DiscoverPanel::changeListenerCallback (juce::ChangeBroadcaster*)
{
    cache = advertiser.getEncoders();
    emptyLabel.setVisible (cache.empty());
    table.setVisible      (! cache.empty());
    table.updateContent();
    table.repaint();

    reassertSubscriptions();
}

juce::String DiscoverPanel::getCellTooltip (int /*row*/, int col)
{
    if (col == ColProject)
        return "Click to create a group from this project or just this encoder.";
    return {};
}

void DiscoverPanel::cellClicked (int row, int col, const juce::MouseEvent&)
{
    // Any click (including a touch tap on iPad, which can't easily distinguish
    // left/right) on the Project column opens the group-creation menu. Other
    // columns are passive.
    if (col == ColProject)
        showProjectMenu (row);
}

void DiscoverPanel::showProjectMenu (int row)
{
    if (row < 0 || row >= static_cast<int> (cache.size()))
        return;

    const auto& e = cache[static_cast<size_t> (row)];
    const auto projectName = e.project;

    juce::PopupMenu menu;
    if (projectName.isNotEmpty())
    {
        menu.addSectionHeader ("Project: " + projectName);
        menu.addItem (1, "Add Group and add all sources from the same Project");
    }
    else
    {
        menu.addSectionHeader ("(No project name advertised)");
    }
    menu.addItem (2, "Add Group and add this source only");

    // Capture the row + project by value — the cache may shift between now and
    // when the user picks an item.
    const int capturedRow = row;
    const auto capturedProject = projectName;
    menu.showMenuAsync (juce::PopupMenu::Options{},
                        [this, capturedRow, capturedProject] (int result)
    {
        if (result == 1)
            addGroupForProject (capturedProject, /*projectWide*/ true,  -1);
        else if (result == 2)
            addGroupForProject (capturedProject, /*projectWide*/ false, capturedRow);
    });
}

void DiscoverPanel::addGroupForProject (const juce::String& projectName,
                                        bool projectWide,
                                        int singleRow)
{
    // Collect the encoders we want to pull into the new group.
    std::vector<NetworkAdvertiser::DiscoveredEncoder> targets;
    if (projectWide)
    {
        for (const auto& e : cache)
            if (e.project == projectName && projectName.isNotEmpty())
                targets.push_back (e);
    }
    else
    {
        if (singleRow < 0 || singleRow >= static_cast<int> (cache.size()))
            return;
        targets.push_back (cache[static_cast<size_t> (singleRow)]);
    }

    if (targets.empty())
        return;

    // Group name: project name if we have one; otherwise fall back to the
    // single track's name, and finally to the auto-numbered default.
    juce::String groupName;
    if (projectWide && projectName.isNotEmpty())
        groupName = projectName;
    else if (! projectWide && ! targets.front().track.isEmpty())
        groupName = targets.front().track;

    const int newGroupId = groupRegistry.add (groupName);

    // Map each target encoder to a registry key and assign the group. Only
    // sources already seen in the registry (i.e. /ambi_enc has arrived) can
    // be assigned — unreceived sources simply aren't in the map yet.
    const auto sources = sourceRegistry.snapshot();
    for (const auto& enc : targets)
    {
        for (const auto& src : sources)
        {
            if (src.origin == EncoderOrigin::Ambix
                && src.id == enc.encoderId
                && src.trackName == enc.track)
            {
                sourceRegistry.setGroup (src.key(), newGroupId);
                break;
            }
        }
    }
}
