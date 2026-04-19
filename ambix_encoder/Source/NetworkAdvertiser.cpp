/*
 ==============================================================================
 NetworkAdvertiser.cpp — zeroconf discovery for ambix_encoder
 Part of the ambix Ambisonic plug-in suite (GPLv2+).
 ==============================================================================
 */

#include "NetworkAdvertiser.h"

namespace
{
    juce::String buildDescription (const juce::StringPairArray& fields)
    {
        juce::String s;
        const auto& keys = fields.getAllKeys();
        for (int i = 0; i < keys.size(); ++i)
        {
            const auto& k = keys[i];
            juce::String v = fields[k];
            if (v.length() > 120)
                v = v.substring (0, 120);
            v = v.replaceCharacter ('|', (juce::juce_wchar) 0xE001);
            v = v.replaceCharacter ('=', (juce::juce_wchar) 0xE002);
            if (i > 0) s << '|';
            s << k << '=' << v;
        }
        return s;
    }

    juce::StringPairArray parseDescription (const juce::String& desc)
    {
        juce::StringPairArray result;
        juce::StringArray parts;
        parts.addTokens (desc, "|", {});
        for (const auto& part : parts)
        {
            const int eq = part.indexOfChar ('=');
            if (eq <= 0) continue;
            const auto key = part.substring (0, eq).trim();
            auto val       = part.substring (eq + 1);
            val = val.replaceCharacter ((juce::juce_wchar) 0xE001, '|');
            val = val.replaceCharacter ((juce::juce_wchar) 0xE002, '=');
            result.set (key, val);
        }
        return result;
    }
}

NetworkAdvertiser::NetworkAdvertiser()
{
    // Delay the first advertiser broadcast by a few seconds: hosts destroy
    // plugins very quickly during scan (usually <1 s), so anything this short
    // never hits the wire. Real user sessions always outlive this window.
    advertiseAfter = juce::Time::getCurrentTime() + juce::RelativeTime::seconds (2.0);

    visualizerList = std::make_unique<juce::NetworkServiceDiscovery::AvailableServiceList> (
        kVisualizerUID, kBroadcastPort);
    visualizerList->onChange = [this]()
    {
        if (reconcileWithVisualizers())
            sendChangeMessage();
    };

    startTimerHz (2); // drives reconcile + lastSeen refresh in UI
}

NetworkAdvertiser::~NetworkAdvertiser()
{
    stopTimer();
    if (visualizerList != nullptr)
        visualizerList->onChange = nullptr;
    visualizerList.reset();
    advertiser.reset();
}

void NetworkAdvertiser::setAdvertising (bool enabled,
                                        int connectionPort,
                                        const juce::String& encoderUuid,
                                        const juce::String& trackName,
                                        int encoderId,
                                        const juce::String& host,
                                        const juce::String& daw,
                                        const juce::String& projectName)
{
    const bool changed = (enabled != wantsAdvertising
                        || connectionPort != currentConnectionPort
                        || encoderUuid != currentEuid
                        || trackName   != currentTrack
                        || encoderId   != currentId
                        || host        != currentHost
                        || daw         != currentDaw
                        || projectName != currentProject);
    if (! changed)
        return;

    wantsAdvertising      = enabled;
    currentConnectionPort = connectionPort;
    currentEuid           = encoderUuid;
    currentTrack          = trackName;
    currentId             = encoderId;
    currentHost           = host;
    currentDaw            = daw;
    currentProject        = projectName;

    rebuildAdvertiser();
}

void NetworkAdvertiser::rebuildAdvertiser()
{
    advertiser.reset();

    if (! wantsAdvertising || currentConnectionPort <= 0)
        return;

    // Suppress broadcast until the startup-delay window has elapsed — the
    // timer below keeps trying every 500 ms, so we'll come back through here
    // automatically once the gate opens.
    if (juce::Time::getCurrentTime() < advertiseAfter)
        return;

    juce::StringPairArray fields;
    // Stable per-plugin-load UUID. The visualizer's Discover tab keys the
    // "Subscribed?" state on this — without it, every description-triggered
    // rebuild of the underlying juce NSD Advertiser would mint a new random
    // instanceID and force the user to re-subscribe. The visualizer drops
    // any encoder advertisement that arrives without it.
    fields.set ("euid",  currentEuid);
    fields.set ("track", currentTrack);
    fields.set ("id",    juce::String (currentId));
    fields.set ("host",  currentHost);
    fields.set ("daw",   currentDaw);
    if (currentProject.isNotEmpty())
        fields.set ("proj", currentProject);

    advertiser = std::make_unique<juce::NetworkServiceDiscovery::Advertiser> (
        kEncoderUID, buildDescription (fields), kBroadcastPort, currentConnectionPort);
}

bool NetworkAdvertiser::addSubscriber (const juce::String& uuid,
                                       const juce::String& ip, int port,
                                       const juce::String& friendlyName)
{
    if (uuid.isEmpty() || port <= 0)
        return false;

    // Prefer the IP the visualizer told us about; if it's empty, try to fill
    // it in from our NSD list. Either may still be empty on first receipt —
    // we keep the entry anyway and let `reconcileWithVisualizers` finish the
    // resolution once the visualizer's NSD packet arrives.
    juce::String resolvedIp = ip;
    if (resolvedIp.isEmpty())
        resolvedIp = lookupVisualizerIpByUuid (uuid);

    {
        const juce::ScopedLock sl (lock);
        auto& sub = subscribers[uuid];
        sub.uuid = uuid;
        sub.ip = resolvedIp;      // may be empty — will be filled on NSD arrival
        sub.port = port;
        sub.friendlyName = friendlyName;
        sub.lastActivity = juce::Time::getCurrentTime();
        sub.stillAdvertising = resolvedIp.isNotEmpty();
    }
    sendChangeMessage();
    return true;
}

juce::String NetworkAdvertiser::lookupVisualizerIpByUuid (const juce::String& uuid) const
{
    if (visualizerList == nullptr)
        return {};
    const auto services = visualizerList->getServices();
    for (const auto& svc : services)
    {
        const auto fields = parseDescription (svc.description);
        if (fields.getValue ("uuid", {}) == uuid)
            return svc.address.toString();
    }
    return {};
}

void NetworkAdvertiser::removeSubscriber (const juce::String& uuid)
{
    bool changed = false;
    {
        const juce::ScopedLock sl (lock);
        changed = (subscribers.erase (uuid) > 0);
    }
    if (changed)
        sendChangeMessage();
}

std::vector<NetworkAdvertiser::Subscriber> NetworkAdvertiser::getSubscribers() const
{
    const juce::ScopedLock sl (lock);
    std::vector<Subscriber> out;
    out.reserve (subscribers.size());
    for (const auto& kv : subscribers)
        out.push_back (kv.second);
    return out;
}

bool NetworkAdvertiser::reconcileWithVisualizers()
{
    if (visualizerList == nullptr)
        return false;

    const auto services = visualizerList->getServices();

    // Build (uuid → ip) for currently advertised visualizers.
    std::map<juce::String, juce::String> uuidToIp;
    for (const auto& svc : services)
    {
        const auto fields = parseDescription (svc.description);
        const auto uuid = fields.getValue ("uuid", {});
        if (uuid.isNotEmpty())
            uuidToIp[uuid] = svc.address.toString();
    }

    bool changed = false;
    {
        const juce::ScopedLock sl (lock);
        for (auto it = subscribers.begin(); it != subscribers.end(); )
        {
            auto adv = uuidToIp.find (it->first);
            const bool stillAdvertising = (adv != uuidToIp.end());

            // Backfill IP if we didn't have one yet.
            if (stillAdvertising && it->second.ip.isEmpty())
            {
                it->second.ip = adv->second;
                changed = true;
            }

            const bool hasUsableIp = it->second.ip.isNotEmpty();

            // IMPORTANT: do NOT use NSD absence as an eviction signal.
            // When multiple encoder plugin instances share one process (e.g.
            // several inserts in a DAW), only one AvailableServiceList bound
            // to UDP 35514 actually receives visualizer broadcasts on macOS
            // (SO_REUSEADDR alone doesn't share UDP broadcasts across sockets
            // in a process). The other instances' NSD list stays empty and
            // `stillAdvertising` would falsely become false for every
            // subscriber — causing all but one encoder to drop the sub.
            //
            // So eviction is purely activity-based: the visualizer
            // heartbeats /ambi_enc_subscribe every few seconds, which bumps
            // lastActivity. If we haven't heard from it in `staleSeconds`
            // *and* NSD has lost sight of it, drop the subscriber. Pure
            // silence without NSD data (which is the normal case for the
            // non-binding instances) is not enough.
            constexpr double staleSeconds = 20.0;
            const bool silentTooLong =
                (juce::Time::getCurrentTime() - it->second.lastActivity).inSeconds() > staleSeconds;

            if (! stillAdvertising && silentTooLong && ! hasUsableIp)
            {
                it = subscribers.erase (it);
                changed = true;
                continue;
            }
            if (! stillAdvertising && silentTooLong && hasUsableIp)
            {
                // NSD gone AND silent for 20s → visualizer is almost certainly
                // gone. Safe to evict.
                it = subscribers.erase (it);
                changed = true;
                continue;
            }

            if (it->second.stillAdvertising != stillAdvertising)
            {
                it->second.stillAdvertising = stillAdvertising;
                changed = true;
            }
            ++it;
        }
    }
    return changed;
}

void NetworkAdvertiser::timerCallback()
{
    // Late-start the advertiser once the scan-avoidance window has elapsed.
    if (advertiser == nullptr && wantsAdvertising && currentConnectionPort > 0
        && juce::Time::getCurrentTime() >= advertiseAfter)
    {
        rebuildAdvertiser();
    }

    if (reconcileWithVisualizers())
        sendChangeMessage();
}
