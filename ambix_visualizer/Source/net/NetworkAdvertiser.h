#pragma once

#include <JuceHeader.h>

// Owns the JUCE NetworkServiceDiscovery (NSD) plumbing for the visualizer:
// browses for encoders so they show up in the Discover tab, and quietly
// advertises a minimal visualizer heartbeat. The advertisement isn't
// user-facing — subscribe messages carry our UUID + IP directly — but on
// macOS pure UDP *listening* on a broadcast port isn't always enough to
// convince the OS to grant Local Network permission and actually deliver
// inbound broadcasts. A tiny periodic broadcast from this process keeps us
// recognised as a local-network participant so the encoder's broadcasts reach
// us reliably.
//
// The encoder advertisement carries a `|key=value|...` description string
// (parsed here into `DiscoveredEncoder`). See the plan file for the protocol.
class NetworkAdvertiser : public juce::ChangeBroadcaster,
                          private juce::Timer
{
public:
    struct DiscoveredEncoder
    {
        // Stable per-plugin-load identifier the encoder broadcasts as
        // `euid=…`. This is the single key used for subscription matching;
        // the JUCE NSD `Service::instanceID` churns on every Advertiser
        // rebuild and is deliberately ignored.
        juce::String encoderUuid;
        juce::IPAddress ip;
        int port { 0 };            // encoder OSC listen port
        juce::String track;
        int  encoderId { 0 };      // m_id (arg0 of /ambi_enc)
        juce::String host;
        juce::String daw;
        juce::String project;
        juce::Time lastSeen;
    };

    // Broadcast port + UID MUST match the encoder.
    static constexpr int   kBroadcastPort   = 35514;
    static constexpr const char* kEncoderUID    = "ambix.encoder.v1";
    static constexpr const char* kVisualizerUID = "ambix.visualizer.v1";

    NetworkAdvertiser();
    ~NetworkAdvertiser() override;

    std::vector<DiscoveredEncoder> getEncoders() const;

    // Tear down and recreate the underlying NSD AvailableServiceList + quiet
    // Advertiser. Use this when discovery appears stuck — e.g. after a wifi
    // reassociation, sleep/wake, or when the user hits a Refresh button in the
    // Discover tab. Clears the cache and fires a change notification so the UI
    // updates immediately; discovered encoders reappear on the next broadcast
    // tick (~1.5 s) from each live encoder.
    void refresh();

    // Build a `|k=v|...` description string from a map of fields.
    static juce::String buildDescription (const juce::StringPairArray& fields);
    // Parse a `|k=v|...` description into a StringPairArray.
    static juce::StringPairArray parseDescription (const juce::String& desc);

private:
    void timerCallback() override;
    void rebuildEncoders();
    void createNsdPair();

    std::unique_ptr<juce::NetworkServiceDiscovery::AvailableServiceList> encoderList;
    std::unique_ptr<juce::NetworkServiceDiscovery::Advertiser>           heartbeatAdvertiser;

    mutable juce::CriticalSection cacheLock;
    std::vector<DiscoveredEncoder> encodersCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkAdvertiser)
};
