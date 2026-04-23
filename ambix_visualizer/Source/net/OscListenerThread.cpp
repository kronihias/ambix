#include "OscListenerThread.h"

OscListenerThread::OscListenerThread()
    : juce::Thread ("AmbixOscListener")
{
    fifoStorage.resize (kFifoCapacity);
}

OscListenerThread::~OscListenerThread()
{
    stopListening();
}

bool OscListenerThread::startListening (int port, Callback callback)
{
    stopListening();

    onMessage = std::move (callback);

    socket = std::make_unique<juce::DatagramSocket> (false);
    socket->setEnablePortReuse (true);

    if (! socket->bindToPort (port))
    {
        socket.reset();
        return false;
    }

    boundPort = socket->getBoundPort();
    listening = true;
    startThread (juce::Thread::Priority::normal);
    return true;
}

void OscListenerThread::stopListening()
{
    listening = false;

    if (socket != nullptr)
        socket->shutdown();

    stopThread (1000);
    cancelPendingUpdate();
    fifo.reset();
    socket.reset();
    boundPort = 0;
}

void OscListenerThread::run()
{
    constexpr int bufferSize = 8192;
    juce::HeapBlock<char> buffer (bufferSize);

    while (! threadShouldExit() && listening.load())
    {
        const auto ready = socket->waitUntilReady (true, 20);
        if (ready < 0)
            break;
        if (ready == 0)
            continue;

        juce::String senderIp;
        int senderPort = 0;
        const int bytes = socket->read (buffer.getData(), bufferSize, false, senderIp, senderPort);
        if (bytes <= 0)
            continue;

        packetsReceived.fetch_add (1);

        IncomingMessage msg;
        msg.senderIp = senderIp;
        msg.senderPort = senderPort;

        // Try ambix_encoder first (cheaper to reject by exact address match).
        if (AmbiEncParser::parse (buffer.getData(), static_cast<size_t> (bytes), msg.ambix))
        {
            msg.kind = Kind::Ambix;
        }
        else if (MultiEncoderParser::parse (buffer.getData(), static_cast<size_t> (bytes), msg.multiEncoder))
        {
            msg.kind = Kind::MultiEncoder;
        }
        else
        {
            DBG ("ambix_visualizer: ignored packet from " << senderIp << ":" << senderPort
                 << " (" << bytes << " bytes)");
            continue;
        }

        packetsParsed.fetch_add (1);

        // Write into the lock-free ring buffer and wake the message thread.
        int start1, size1, start2, size2;
        fifo.prepareToWrite (1, start1, size1, start2, size2);
        const int slot = (size1 > 0) ? start1 : (size2 > 0 ? start2 : -1);
        if (slot >= 0)
        {
            fifoStorage[slot] = std::move (msg);
            fifo.finishedWrite (1);
            triggerAsyncUpdate(); // coalescing: only posts if not already pending
        }
        // else: ring buffer full — drop packet (shouldn't happen at normal rates)
    }
}

void OscListenerThread::handleAsyncUpdate()
{
    const int numReady = fifo.getNumReady();
    if (numReady <= 0 || ! onMessage)
        return;

    int start1, size1, start2, size2;
    fifo.prepareToRead (numReady, start1, size1, start2, size2);

    for (int i = 0; i < size1; ++i)
        onMessage (fifoStorage[start1 + i]);
    for (int i = 0; i < size2; ++i)
        onMessage (fifoStorage[start2 + i]);

    fifo.finishedRead (size1 + size2);
}
