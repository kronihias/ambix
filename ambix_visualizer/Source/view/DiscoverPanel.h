#pragma once

#include <JuceHeader.h>

#include "../app/Settings.h"
#include "../model/GroupRegistry.h"
#include "../model/SourceRegistry.h"
#include "../net/NetworkAdvertiser.h"
#include "../net/OscCommandSender.h"

// "Discover" tab content. Lists every encoder advertising itself via
// NetworkServiceDiscovery, with a per-row Subscribe toggle that drives the
// /ambi_enc_subscribe OSC message.
class DiscoverPanel : public juce::Component,
                     public juce::ChangeListener,
                     public juce::TableListBoxModel,
                     private juce::Timer
{
public:
    DiscoverPanel (NetworkAdvertiser& advertiser,
                   OscCommandSender& sender,
                   AppSettings& settings,
                   SourceRegistry& sourceRegistry,
                   GroupRegistry& groupRegistry,
                   std::function<void()> persistSettings);
    ~DiscoverPanel() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    // TableListBoxModel
    int  getNumRows() override;
    void paintRowBackground (juce::Graphics&, int row, int w, int h, bool selected) override;
    void paintCell (juce::Graphics&, int row, int col, int w, int h, bool selected) override;
    juce::Component* refreshComponentForCell (int row, int col, bool selected,
                                              juce::Component* existing) override;
    void cellClicked (int row, int col, const juce::MouseEvent& e) override;
    juce::String getCellTooltip (int row, int col) override;

    // If an auto-subscribed encoder appears, re-send the subscribe message.
    // Called on startup + on every advertiser change. Also fired periodically
    // as a heartbeat so encoders don't evict us because their NSD listener
    // failed to bind (common when multiple encoder plugin instances live in
    // one DAW process).
    void reassertSubscriptions();

private:
    // `encoderUuid` is the stable per-plugin-load identifier the encoder
    // broadcasts as `euid=…`. Stored verbatim in settings.subscribedEncoderIds.
    bool isSubscribed (const juce::String& encoderUuid) const;
    void setSubscribed (const juce::String& encoderUuid, bool);
    void connectAll();
    void disconnectAll();

    // Right-click / tap on the Project column offers to create a group from
    // everything in that project, or from just this encoder's source.
    void showProjectMenu (int row);
    void addGroupForProject (const juce::String& projectName, bool projectWide, int singleRow);

    void timerCallback() override;

    NetworkAdvertiser& advertiser;
    OscCommandSender&  sender;
    AppSettings&       settings;
    SourceRegistry&    sourceRegistry;
    GroupRegistry&     groupRegistry;
    std::function<void()> persistFn;

    juce::TableListBox table;
    juce::Label        emptyLabel;
    juce::TextButton   refreshButton       { "Refresh" };
    juce::TextButton   connectAllButton    { "Connect all" };
    juce::TextButton   disconnectAllButton { "Disconnect all" };

    std::vector<NetworkAdvertiser::DiscoveredEncoder> cache;

    enum ColumnId { ColSubscribed = 1, ColTrack, ColId, ColHost, ColDaw, ColProject,
                    ColEndpoint };

    class SubscribeCell;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscoverPanel)
};
