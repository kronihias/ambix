#include "OscListenerThread.h"

OscListenerThread::OscListenerThread()
    : juce::Thread ("AmbixOscListener")
{
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
    socket.reset();
    boundPort = 0;
}

void OscListenerThread::run()
{
    constexpr int bufferSize = 8192;
    juce::HeapBlock<char> buffer (bufferSize);

    while (! threadShouldExit() && listening.load())
    {
        const auto ready = socket->waitUntilReady (true, 200);
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

        auto cb = onMessage;
        juce::MessageManager::callAsync ([cb, msg]()
        {
            if (cb)
                cb (msg);
        });
    }
}
