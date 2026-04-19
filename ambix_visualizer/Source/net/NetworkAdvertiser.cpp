#include "NetworkAdvertiser.h"

NetworkAdvertiser::NetworkAdvertiser()
{
    createNsdPair();

    // Also tick periodically so lastSeen refreshes in the UI and timed-out
    // services get pruned in our cache.
    startTimerHz (2);
}

NetworkAdvertiser::~NetworkAdvertiser()
{
    stopTimer();
    heartbeatAdvertiser.reset();
    if (encoderList != nullptr)
        encoderList->onChange = nullptr;
    encoderList.reset();
}

void NetworkAdvertiser::createNsdPair()
{
    encoderList = std::make_unique<juce::NetworkServiceDiscovery::AvailableServiceList> (
        kEncoderUID, kBroadcastPort);
    encoderList->onChange = [this]() { rebuildEncoders(); };

    // Quiet, always-on heartbeat broadcast. Not shown in any UI. Exists only
    // so macOS recognises this process as an active local-network participant
    // — without sending anything, inbound broadcasts from encoders may be
    // suppressed by the system's Local Network privacy gate. Payload contents
    // don't matter to the encoder (it doesn't rely on visualizer NSD for
    // correctness), so we keep the description minimal.
    juce::StringPairArray fields;
    fields.set ("host", juce::SystemStats::getComputerName());
    const auto description = buildDescription (fields);

    heartbeatAdvertiser = std::make_unique<juce::NetworkServiceDiscovery::Advertiser> (
        kVisualizerUID, description, kBroadcastPort, /*connectionPort*/ 0);
}

void NetworkAdvertiser::refresh()
{
    // Must run on the message thread: the NSD objects own timers/sockets that
    // assert on MM ownership during teardown. DiscoverPanel's Refresh button
    // calls us from the MM already; guard anyway.
    JUCE_ASSERT_MESSAGE_THREAD;

    // Tear down current NSD objects. Drop the onChange callback first so any
    // in-flight scan doesn't re-enter us mid-destruction.
    if (encoderList != nullptr)
        encoderList->onChange = nullptr;
    heartbeatAdvertiser.reset();
    encoderList.reset();

    // Clear the cache so the UI goes into "empty" state immediately — gives
    // the user clear feedback that the refresh happened. Genuine encoders
    // reappear on their next broadcast (~1.5 s per the encoder's NSD tick).
    {
        const juce::ScopedLock sl (cacheLock);
        encodersCache.clear();
    }
    sendChangeMessage();

    // Rebuild. The new AvailableServiceList binds a fresh UDP socket — this
    // is what actually fixes the "stuck" state.
    createNsdPair();
}

void NetworkAdvertiser::timerCallback()
{
    rebuildEncoders();
}

void NetworkAdvertiser::rebuildEncoders()
{
    if (encoderList == nullptr)
        return;

    const auto live = encoderList->getServices();

    std::vector<DiscoveredEncoder> out;
    out.reserve (live.size());
    for (const auto& svc : live)
    {
        const auto fields = parseDescription (svc.description);

        DiscoveredEncoder e;
        // `euid` is the subscribe key. It's empty only when the DAW is still
        // running an old pre-euid build of the encoder (cached VST3 binary,
        // needs a DAW restart). We still surface the row so the user can see
        // the encoder is advertising; the Discover panel disables the
        // Subscribe toggle when this is empty.
        e.encoderUuid = fields.getValue ("euid",  {});
        e.ip          = svc.address;
        e.port        = svc.port;
        e.lastSeen    = svc.lastSeen;
        e.track       = fields.getValue ("track", {});
        e.encoderId   = fields.getValue ("id",     "0").getIntValue();
        e.host        = fields.getValue ("host",  {});
        e.daw         = fields.getValue ("daw",   {});
        e.project     = fields.getValue ("proj",  {});
        out.push_back (std::move (e));
    }

    bool changed = false;
    {
        const juce::ScopedLock sl (cacheLock);
        if (out.size() != encodersCache.size())
            changed = true;
        else
        {
            for (size_t i = 0; i < out.size(); ++i)
            {
                const auto& a = out[i];
                const auto& b = encodersCache[i];
                if (a.encoderUuid != b.encoderUuid
                    || a.ip.toString() != b.ip.toString()
                    || a.port != b.port
                    || a.track != b.track
                    || a.encoderId != b.encoderId
                    || a.host != b.host
                    || a.daw != b.daw
                    || a.project != b.project)
                {
                    changed = true; break;
                }
            }
        }
        encodersCache = std::move (out);
    }

    if (changed)
        sendChangeMessage();
}

std::vector<NetworkAdvertiser::DiscoveredEncoder> NetworkAdvertiser::getEncoders() const
{
    const juce::ScopedLock sl (cacheLock);
    return encodersCache;
}

juce::String NetworkAdvertiser::buildDescription (const juce::StringPairArray& fields)
{
    // Format: key=value|key=value|...  Values with '|' or '=' are encoded by
    // replacing them with unicode Private Use chars — no dependency and easy
    // to reverse. Long values (>120 chars) are clamped to keep packets small.
    juce::String s;
    const auto& allKeys = fields.getAllKeys();
    for (int i = 0; i < allKeys.size(); ++i)
    {
        const auto& k = allKeys[i];
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

juce::StringPairArray NetworkAdvertiser::parseDescription (const juce::String& desc)
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
