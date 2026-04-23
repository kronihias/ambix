#pragma once

#include <JuceHeader.h>

#include "AmbiEncParser.h"
#include "MultiEncoderParser.h"

class OscListenerThread : private juce::Thread,
                          private juce::AsyncUpdater
{
public:
    enum class Kind { Ambix, MultiEncoder };

    struct IncomingMessage
    {
        juce::String senderIp;
        int senderPort { 0 };
        Kind kind { Kind::Ambix };
        AmbiEncPayload      ambix;
        MultiEncoderPayload multiEncoder;
    };

    using Callback = std::function<void (const IncomingMessage&)>;

    OscListenerThread();
    ~OscListenerThread() override;

    bool startListening (int port, Callback callback);
    void stopListening();
    bool isListening() const noexcept { return listening.load(); }
    int  getListenPort() const noexcept { return boundPort; }

    int getPacketsReceived() const noexcept { return packetsReceived.load(); }
    int getPacketsParsed()   const noexcept { return packetsParsed.load(); }

private:
    void run() override;
    void handleAsyncUpdate() override;

    Callback onMessage;
    std::unique_ptr<juce::DatagramSocket> socket;
    std::atomic<bool> listening      { false };
    std::atomic<int>  packetsReceived { 0 };
    std::atomic<int>  packetsParsed   { 0 };
    int boundPort { 0 };

    // Lock-free SPSC ring buffer: listener thread writes, message thread drains.
    static constexpr int kFifoCapacity = 256;
    juce::AbstractFifo fifo { kFifoCapacity };
    std::vector<IncomingMessage> fifoStorage; // allocated once in ctor, size kFifoCapacity

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscListenerThread)
};
