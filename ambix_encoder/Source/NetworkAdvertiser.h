/*
 ==============================================================================
 NetworkAdvertiser.h — zeroconf discovery for ambix_encoder

 Wraps JUCE's NetworkServiceDiscovery:
   • Advertises this plugin instance as "ambix.encoder.v1" (while enabled).
   • Browses for "ambix.visualizer.v1" advertisers so we can auto-expire
     subscribers whose visualizer went away.
   • Tracks explicit /ambi_enc_subscribe subscribers.

 Part of the ambix Ambisonic plug-in suite (GPLv2+).
 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"

class NetworkAdvertiser : public juce::ChangeBroadcaster,
                          private juce::Timer
{
public:
    static constexpr int   kBroadcastPort    = 35514;
    static constexpr const char* kEncoderUID    = "ambix.encoder.v1";
    static constexpr const char* kVisualizerUID = "ambix.visualizer.v1";

    struct Subscriber
    {
        juce::String uuid;          // visualizer-provided UUID
        juce::String ip;
        int port { 0 };             // visualizer OSC listen port
        juce::String friendlyName;
        juce::Time lastActivity;    // last subscribe msg / NSD seen time
        bool        stillAdvertising { true };
    };

    NetworkAdvertiser();
    ~NetworkAdvertiser() override;

    // Advertising control — the plugin's "Discoverable on network" toggle.
    // `connectionPort` is the encoder's OSC listen port (osc_in_port); this
    // is what visualizers will send /ambi_enc_subscribe to.
    //
    // `encoderUuid` is a stable per-plugin-load identifier that travels in
    // the NSD description (`euid=…`). The visualizer uses it to key its
    // subscription list — without it, every rebuild of the underlying
    // juce NSD Advertiser would mint a new random instanceID and orphan the
    // visualizer's "subscribed" state, forcing the user to re-subscribe on
    // every description change (e.g. when the REAPER project name finally
    // resolves or the track is renamed).
    void setAdvertising (bool enabled,
                         int connectionPort,
                         const juce::String& encoderUuid,
                         const juce::String& trackName,
                         int encoderId,
                         const juce::String& host,
                         const juce::String& daw,
                         const juce::String& projectName);

    // Subscriber bookkeeping — called from PluginProcessor's OSC handler.
    // The IP can be empty; if so we auto-fill it from the NSD visualizer list
    // using the uuid. Returns true if the subscriber was added / updated.
    bool addSubscriber    (const juce::String& uuid,
                           const juce::String& ip, int port,
                           const juce::String& friendlyName);
    void removeSubscriber (const juce::String& uuid);

    // Returns current IP for a visualizer UUID found in the NSD list, or empty.
    juce::String lookupVisualizerIpByUuid (const juce::String& uuid) const;

    std::vector<Subscriber> getSubscribers() const;

    // Keep only subscribers whose NSD visualizer is still in the AvailableServiceList.
    // Returns true if anything changed.
    bool reconcileWithVisualizers();

private:
    void timerCallback() override;
    void rebuildAdvertiser();

    std::unique_ptr<juce::NetworkServiceDiscovery::Advertiser>           advertiser;
    std::unique_ptr<juce::NetworkServiceDiscovery::AvailableServiceList> visualizerList;

    mutable juce::CriticalSection lock;
    std::map<juce::String, Subscriber> subscribers;

    bool         wantsAdvertising { false };
    juce::String currentEuid;
    juce::String currentTrack, currentHost, currentDaw, currentProject;
    int          currentId { 0 };
    int          currentConnectionPort { 0 };

    // Don't advertise until this wall-clock time. Hosts instantiate and
    // destroy plugins during scan — delaying NSD broadcasts keeps the
    // plugin invisible during those short-lived lifecycles.
    juce::Time   advertiseAfter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkAdvertiser)
};
