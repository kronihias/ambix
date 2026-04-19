#pragma once

#include <JuceHeader.h>

#include "EncoderSource.h"

class SourceRegistry : public juce::ChangeBroadcaster,
                       private juce::Timer
{
public:
    // Full update for ambix_encoder: arrives in a single multi-arg OSC message.
    struct AmbixUpdate
    {
        int id { 0 };
        juce::String trackName;
        float azimuthDeg   { 0.0f };
        float elevationDeg { 0.0f };
        float size         { 0.0f };
        float rmsLinear    { 0.0f };
        float peakLinear   { 0.0f };
        juce::String senderIp;
        int  replyPort { 0 };
    };

    // Partial update for IEM MultiEncoder: one parameter per OSC message.
    struct MultiEncoderUpdate
    {
        enum class Param { Azimuth, Elevation, Gain, Mute, Solo };

        int sourceIndex { 0 };
        Param param { Param::Azimuth };
        float value { 0.0f };
        juce::String addressPrefix;     // used as trackName (e.g. "MultiEncoder")
        juce::String senderIp;
        int senderPort { 0 };
    };

    SourceRegistry();
    ~SourceRegistry() override;

    void setStaleTimeoutMs (juce::uint32 timeoutMs) noexcept { staleTimeoutMs = timeoutMs; }
    juce::uint32 getStaleTimeoutMs() const noexcept { return staleTimeoutMs; }

    void applyAmbixUpdate (const AmbixUpdate& update);
    void applyMultiEncoderUpdate (const MultiEncoderUpdate& update);

    void applyLocalMove (const EncoderSource::Key& key,
                         float azimuthDeg,
                         float elevationDeg);

    // User-entered reply port override. If `port` is 0, clears the override
    // and future OSC-inferred ports for ambix sources take effect again.
    void setReplyPort (const EncoderSource::Key& key, int port);

    void setIcon  (const EncoderSource::Key& key, IconCatalogue::Icon icon);
    void setGroup (const EncoderSource::Key& key, int groupId);

    // While a key is flagged as being dragged, incoming OSC position updates
    // for that source are ignored (to prevent flicker from a stale round-trip
    // arriving after the user moved the puck). Levels, sender IP, and
    // lastSeen still update so meters stay live and auto-clear doesn't fire.
    // Refcounted so multi-touch (same puck grabbed by two fingers) releases
    // the lock only when the last finger lifts.
    void beginDrag (const EncoderSource::Key& key);
    void endDrag (const EncoderSource::Key& key);

    void clearAll();
    void clearStale();

    // Remove a single source by key. If the key isn't present this is a
    // no-op. Used by the Source List panel's per-row remove button. The
    // source reappears if its encoder keeps broadcasting — removal just
    // drops the current stale state.
    void remove (const EncoderSource::Key& key);

    std::vector<EncoderSource> snapshot() const;
    std::optional<EncoderSource> findByKey (const EncoderSource::Key& key) const;

private:
    void timerCallback() override;

    mutable juce::CriticalSection lock;
    std::map<EncoderSource::Key, EncoderSource> sources;
    std::map<EncoderSource::Key, int> draggedKeys; // key → refcount
    juce::uint32 staleTimeoutMs { 10000 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceRegistry)
};
