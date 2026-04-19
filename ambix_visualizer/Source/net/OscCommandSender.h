#pragma once

#include <JuceHeader.h>

#include "MultiEncoderParser.h"

class OscCommandSender
{
public:
    OscCommandSender() = default;
    ~OscCommandSender() = default;

    // Send /ambi_enc_set to an ambix_encoder instance.
    bool sendAmbiEncSet (const juce::String& ip,
                         int port,
                         int id,
                         float azimuthDeg,
                         float elevationDeg,
                         float size);

    // Send one /<prefix>/<param><idx> message to a MultiEncoder instance.
    bool sendMultiEncoderParam (const juce::String& ip,
                                int port,
                                const juce::String& prefix,
                                MultiEncoderPayload::Param param,
                                int sourceIndex,
                                float value);

    // Discovery subscribe/unsubscribe — sent to an ambix_encoder that's
    // announcing itself via NetworkServiceDiscovery. The visualizer's own IP
    // is included in the subscribe message so the encoder doesn't have to
    // wait for NSD to correlate the UUID back to an address.
    bool sendSubscribe   (const juce::String& ip, int port,
                          const juce::String& visualizerUuid,
                          int visualizerListenPort,
                          const juce::String& visualizerFriendlyName,
                          const juce::String& visualizerIp);
    bool sendUnsubscribe (const juce::String& ip, int port,
                          const juce::String& visualizerUuid);

    void reset();

private:
    struct Endpoint
    {
        juce::String ip;
        int port { 0 };
        bool operator< (const Endpoint& o) const noexcept
        {
            if (port != o.port) return port < o.port;
            return ip.compare (o.ip) < 0;
        }
    };

    juce::OSCSender* getOrConnect (const Endpoint& ep);

    juce::CriticalSection lock;
    std::map<Endpoint, std::unique_ptr<juce::OSCSender>> senders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscCommandSender)
};
