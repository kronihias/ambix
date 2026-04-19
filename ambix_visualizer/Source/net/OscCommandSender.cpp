#include "OscCommandSender.h"

juce::OSCSender* OscCommandSender::getOrConnect (const Endpoint& ep)
{
    auto it = senders.find (ep);
    if (it != senders.end())
        return it->second.get();

    auto s = std::make_unique<juce::OSCSender>();
    if (! s->connect (ep.ip, ep.port))
        return nullptr;

    auto* raw = s.get();
    senders.emplace (ep, std::move (s));
    return raw;
}

bool OscCommandSender::sendAmbiEncSet (const juce::String& ip,
                                       int port,
                                       int id,
                                       float azimuthDeg,
                                       float elevationDeg,
                                       float size)
{
    if (ip.isEmpty() || port <= 0)
        return false;

    const juce::ScopedLock sl (lock);
    auto* sender = getOrConnect ({ ip, port });
    if (sender == nullptr)
        return false;

    juce::OSCMessage msg ("/ambi_enc_set");
    msg.addInt32 (id);
    msg.addFloat32 (2.0f);                 // distance (ignored by encoder)
    msg.addFloat32 (azimuthDeg);
    msg.addFloat32 (elevationDeg);
    msg.addFloat32 (size);

    return sender->send (msg);
}

bool OscCommandSender::sendMultiEncoderParam (const juce::String& ip,
                                              int port,
                                              const juce::String& prefix,
                                              MultiEncoderPayload::Param param,
                                              int sourceIndex,
                                              float value)
{
    if (ip.isEmpty() || port <= 0)
        return false;

    const juce::ScopedLock sl (lock);
    auto* sender = getOrConnect ({ ip, port });
    if (sender == nullptr)
        return false;

    // Inverse of the convention flip in MultiEncoderParser: IEM MultiEncoder
    // uses math-convention azimuth, so negate on the way out.
    if (param == MultiEncoderPayload::Param::Azimuth)
        value = -value;

    const auto address = MultiEncoderParser::buildAddress (prefix, param, sourceIndex);
    juce::OSCMessage msg (address);
    msg.addFloat32 (value);
    return sender->send (msg);
}

bool OscCommandSender::sendSubscribe (const juce::String& ip,
                                      int port,
                                      const juce::String& visualizerUuid,
                                      int visualizerListenPort,
                                      const juce::String& visualizerFriendlyName,
                                      const juce::String& visualizerIp)
{
    if (ip.isEmpty() || port <= 0)
        return false;
    const juce::ScopedLock sl (lock);
    auto* sender = getOrConnect ({ ip, port });
    if (sender == nullptr) return false;

    juce::OSCMessage msg ("/ambi_enc_subscribe");
    msg.addString (visualizerUuid);
    msg.addInt32  (visualizerListenPort);
    msg.addString (visualizerFriendlyName);
    msg.addString (visualizerIp);                       // 4th arg: our IP (may be empty)
    return sender->send (msg);
}

bool OscCommandSender::sendUnsubscribe (const juce::String& ip, int port,
                                        const juce::String& visualizerUuid)
{
    if (ip.isEmpty() || port <= 0)
        return false;
    const juce::ScopedLock sl (lock);
    auto* sender = getOrConnect ({ ip, port });
    if (sender == nullptr) return false;

    juce::OSCMessage msg ("/ambi_enc_unsubscribe");
    msg.addString (visualizerUuid);
    return sender->send (msg);
}

void OscCommandSender::reset()
{
    const juce::ScopedLock sl (lock);
    senders.clear();
}
