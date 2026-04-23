#include "SourceRegistry.h"

#include "../view/LevelMapping.h"

SourceRegistry::SourceRegistry()
{
    startTimerHz (4);
}

SourceRegistry::~SourceRegistry()
{
    stopTimer();
}

void SourceRegistry::applyAmbixUpdate (const AmbixUpdate& update)
{
    const auto now = juce::Time::getMillisecondCounter();

    {
        const juce::ScopedLock sl (lock);

        // Key Ambix sources on (id, senderIp + ":" + replyPort) — identity is
        // the plugin instance, not the display name, and not `id` alone. The
        // encoder's `m_id` static counter resets per DAW process, so two
        // Reaper projects (or Reaper + Logic) can both emit sources with
        // id=1 — only the listen port distinguishes them. `replyPort` is
        // broadcast in the 9-arg /ambi_enc variant and is unique per plugin
        // instance (UDP bind uniqueness). A DAW track rename rebroadcasts
        // with a new `trackName`; we want to update the existing entry in
        // place instead of spawning a duplicate puck, so `trackName` stays
        // out of the key. Must match `EncoderSource::key()`.
        const EncoderSource::Key key {
            EncoderOrigin::Ambix, update.id, juce::String(),
            update.senderIp + ":" + juce::String (update.replyPort)
        };
        auto& src = sources[key];

        const bool isDragged = draggedKeys.count (key) > 0;

        src.origin         = EncoderOrigin::Ambix;
        src.id             = update.id;
        src.trackName      = update.trackName;
        src.originTag      = juce::String();
        if (! isDragged)
        {
            src.azimuthDeg   = update.azimuthDeg;
            src.elevationDeg = update.elevationDeg;
            src.size         = update.size;
        }

        // Mirror the plugin's own kMeterThreshold. The encoder sends OSC on
        // position OR meter change, so a frozen meter (paused DAW) paired
        // with any position movement produces packets with unchanged rms/dpk.
        // Only advance lastLevelUpdateMs when the level actually moved,
        // otherwise the meter fade would read those position-driven packets
        // as "still playing".
        constexpr float kLevelChangeThreshold = 0.002f;
        const bool levelMoved = (std::abs (src.rmsLinear  - update.rmsLinear)  > kLevelChangeThreshold
                              || std::abs (src.peakLinear - update.peakLinear) > kLevelChangeThreshold);
        if (levelMoved || src.lastLevelUpdateMs == 0)
            src.lastLevelUpdateMs = now;

        src.rmsLinear      = update.rmsLinear;
        src.peakLinear     = update.peakLinear;

        // Proper ballistic peak-hold: refresh whenever the incoming peak is
        // >= the *currently displayed* (post-decay) held level. For a steady
        // tone, block-boundary sampling makes each reported peak slightly
        // smaller than the previous max, so a naive `>= src.peakHoldLinear`
        // never refreshes the timestamp — the display decays to zero and then
        // pops back up on the next reset. Comparing against the decayed value
        // keeps the hold alive as long as the true peak hasn't dropped.
        const float effectiveHeld = LevelMapping::peakHoldEffective (
            src.peakHoldLinear, src.peakHoldTimeMs, now);
        if (update.peakLinear >= effectiveHeld)
        {
            src.peakHoldLinear = update.peakLinear;
            src.peakHoldTimeMs = now;
        }

        src.senderIp   = update.senderIp;
        // Preserve user-set override; otherwise refresh from the incoming message.
        if (! src.replyPortUserSet)
            src.replyPort = update.replyPort;
        src.lastSeenMs = now;
    }

    sendChangeMessage();
}

void SourceRegistry::applyMultiEncoderUpdate (const MultiEncoderUpdate& update)
{
    const auto now = juce::Time::getMillisecondCounter();

    const juce::String originTag = update.senderIp + ":" + juce::String (update.senderPort);

    {
        const juce::ScopedLock sl (lock);

        const EncoderSource::Key key {
            EncoderOrigin::MultiEncoder, update.sourceIndex, update.addressPrefix, originTag
        };
        auto& src = sources[key];

        src.origin    = EncoderOrigin::MultiEncoder;
        src.id        = update.sourceIndex;
        src.trackName = update.addressPrefix;
        src.originTag = originTag;
        src.senderIp  = update.senderIp;
        src.lastSeenMs = now;

        const bool isDragged = draggedKeys.count (key) > 0;

        using Param = MultiEncoderUpdate::Param;
        switch (update.param)
        {
            case Param::Azimuth:
                if (! isDragged) src.azimuthDeg   = update.value;
                break;
            case Param::Elevation:
                if (! isDragged) src.elevationDeg = update.value;
                break;
            case Param::Gain:
                src.gainDb = update.value;
                // Represent gain as pseudo-rms so the colour ramp still applies.
                // mute overrides to silence.
                src.rmsLinear = src.muted ? 0.0f
                                          : std::pow (10.0f, update.value / 20.0f);
                src.lastLevelUpdateMs = now;
                break;
            case Param::Mute:
                src.muted = update.value >= 0.5f;
                if (src.muted) src.rmsLinear = 0.0f;
                else           src.rmsLinear = std::pow (10.0f, src.gainDb / 20.0f);
                src.lastLevelUpdateMs = now;
                break;
            case Param::Solo:
                src.soloed = update.value >= 0.5f;
                break;
        }
    }

    sendChangeMessage();
}

void SourceRegistry::beginDrag (const EncoderSource::Key& key)
{
    const juce::ScopedLock sl (lock);
    ++draggedKeys[key];
    auto it = sources.find (key);
    if (it != sources.end())
        it->second.lastInteractionMs = juce::Time::getMillisecondCounter();
}

void SourceRegistry::endDrag (const EncoderSource::Key& key)
{
    const juce::ScopedLock sl (lock);
    auto it = draggedKeys.find (key);
    if (it == draggedKeys.end())
        return;
    if (--it->second <= 0)
        draggedKeys.erase (it);
}

void SourceRegistry::setReplyPort (const EncoderSource::Key& key, int port)
{
    {
        const juce::ScopedLock sl (lock);
        auto it = sources.find (key);
        if (it == sources.end())
            return;
        if (port <= 0)
        {
            it->second.replyPort = 0;
            it->second.replyPortUserSet = false;
        }
        else
        {
            it->second.replyPort = port;
            it->second.replyPortUserSet = true;
        }
    }
    sendChangeMessage();
}

void SourceRegistry::setIcon (const EncoderSource::Key& key, IconCatalogue::Icon icon)
{
    {
        const juce::ScopedLock sl (lock);
        auto it = sources.find (key);
        if (it == sources.end()) return;
        it->second.icon = icon;
    }
    sendChangeMessage();
}

void SourceRegistry::setGroup (const EncoderSource::Key& key, int groupId)
{
    {
        const juce::ScopedLock sl (lock);
        auto it = sources.find (key);
        if (it == sources.end()) return;
        it->second.groupId = groupId;
    }
    sendChangeMessage();
}

void SourceRegistry::applyLocalMove (const EncoderSource::Key& key,
                                     float azimuthDeg,
                                     float elevationDeg)
{
    {
        const juce::ScopedLock sl (lock);
        auto it = sources.find (key);
        if (it == sources.end())
            return;
        it->second.azimuthDeg = azimuthDeg;
        it->second.elevationDeg = elevationDeg;
        it->second.lastInteractionMs = juce::Time::getMillisecondCounter();
    }
    sendChangeMessage();
}

void SourceRegistry::clearAll()
{
    {
        const juce::ScopedLock sl (lock);
        sources.clear();
    }
    sendChangeMessage();
}

void SourceRegistry::remove (const EncoderSource::Key& key)
{
    bool changed = false;
    {
        const juce::ScopedLock sl (lock);
        changed = (sources.erase (key) > 0);
        // Dragged-key refcount would otherwise survive the source — clear it
        // so a later resurrection starts fresh.
        draggedKeys.erase (key);
    }
    if (changed)
        sendChangeMessage();
}

void SourceRegistry::clearStale()
{
    bool changed = false;
    const auto now = juce::Time::getMillisecondCounter();

    {
        const juce::ScopedLock sl (lock);
        for (auto it = sources.begin(); it != sources.end();)
        {
            if (now - it->second.lastSeenMs > staleTimeoutMs)
            {
                it = sources.erase (it);
                changed = true;
            }
            else
            {
                ++it;
            }
        }
    }

    if (changed)
        sendChangeMessage();
}

std::vector<EncoderSource> SourceRegistry::snapshot() const
{
    const juce::ScopedLock sl (lock);
    std::vector<EncoderSource> out;
    out.reserve (sources.size());
    for (const auto& kv : sources)
        out.push_back (kv.second);
    return out;
}

std::optional<EncoderSource> SourceRegistry::findByKey (const EncoderSource::Key& key) const
{
    const juce::ScopedLock sl (lock);
    auto it = sources.find (key);
    if (it == sources.end())
        return std::nullopt;
    return it->second;
}

void SourceRegistry::timerCallback()
{
    if (staleTimeoutMs > 0)
        clearStale();
}
