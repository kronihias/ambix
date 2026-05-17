#pragma once

#include <JuceHeader.h>

#include "../view/IconCatalogue.h"

enum class EncoderOrigin
{
    Ambix,          // our ambix_encoder legacy /ambi_enc (one source per plugin, with rms/peak)
    AmbixSource,    // our ambix_encoder ≥0.5 per-source: /ambix_encoder/source/<n>/...
    MultiEncoder    // IEM MultiEncoder (up to 64 sources per plugin, no levels)
};

struct EncoderSource
{
    EncoderOrigin origin { EncoderOrigin::Ambix };

    int           id { 0 };      // ambix: plugin instance id; multi: source index 0..63
    juce::String  trackName;     // ambix: real DAW track name; multi: "" (unused for display)
    juce::String  originTag;     // disambiguates instances (multi: "ip:port"; ambix: empty)

    float azimuthDeg   { 0.0f };
    float elevationDeg { 0.0f };
    float size         { 0.0f };
    float gainDb       { 0.0f }; // MultiEncoder only
    bool  muted        { false };
    bool  soloed       { false };

    float rmsLinear  { 0.0f };
    float peakLinear { 0.0f };
    float peakHoldLinear { 0.0f };
    juce::uint32 peakHoldTimeMs { 0 };

    juce::String senderIp;
    int          replyPort { 0 };
    bool         replyPortUserSet { false };
    juce::uint32 lastSeenMs { 0 };
    // Distinct from lastSeenMs: advances only when the incoming level actually
    // changed. ambix_encoder sends OSC whenever EITHER position OR level
    // changed, so a paused DAW (no new audio, frozen rms/peak) still emits
    // packets as long as position moves — those carry stale level values.
    // The meter fade keys off this, not lastSeenMs, so those stale packets
    // can't keep a silent source's meter lit.
    juce::uint32 lastLevelUpdateMs { 0 };
    // Bumped whenever the user interacts with this source (touch / click /
    // drag). Newer interaction brings the puck to the foreground in both
    // views so overlapping pucks are unambiguous to grab again.
    juce::uint32 lastInteractionMs { 0 };

    IconCatalogue::Icon icon { IconCatalogue::Icon::Default };
    int                  groupId { 0 }; // 0 = no group

    // MultiEncoder OSC protocol uses 0-based source indices (azimuth0..azimuth63)
    // but the UI displays 1-based numbers. `id` always stores the 0-based index
    // for MultiEncoder sources; call this accessor for display.
    //
    // For AmbixSource, `id` is the 1-based source number from the plugin so
    // `id` is the display value directly.
    int getDisplayId() const noexcept
    {
        return origin == EncoderOrigin::MultiEncoder ? id + 1 : id;
    }

    juce::String getOriginTypeName() const noexcept
    {
        switch (origin)
        {
            case EncoderOrigin::Ambix:        return "ambix_encoder";
            case EncoderOrigin::AmbixSource:  return "ambix_encoder";
            case EncoderOrigin::MultiEncoder: return "IEM MultiEncoder";
        }
        return {};
    }

    juce::String getDisplayLabel() const
    {
        if (origin == EncoderOrigin::Ambix)
            return trackName.isNotEmpty() ? trackName : juce::String ("src ") + juce::String (id);
        if (origin == EncoderOrigin::AmbixSource)
        {
            const auto prefix = trackName.isNotEmpty() ? trackName : juce::String ("ambix");
            return prefix + " / " + juce::String (id);
        }
        const auto prefix = trackName.isNotEmpty() ? trackName : juce::String ("MultiEncoder");
        return prefix + " " + juce::String (getDisplayId());
    }

    struct Key
    {
        EncoderOrigin origin { EncoderOrigin::Ambix };
        int           id { 0 };
        juce::String  trackName;
        juce::String  originTag;

        bool operator== (const Key& o) const noexcept
        {
            return origin == o.origin && id == o.id
                && trackName == o.trackName && originTag == o.originTag;
        }

        bool operator< (const Key& o) const noexcept
        {
            if (origin    != o.origin)    return origin < o.origin;
            if (id        != o.id)        return id < o.id;
            if (originTag != o.originTag) return originTag.compare (o.originTag) < 0;
            return trackName.compare (o.trackName) < 0;
        }
    };

    Key key() const noexcept
    {
        // Ambix sources are identified by (origin, id, ip+replyPort). The
        // encoder's `m_id` is a static counter that RESETS per DAW process —
        // two Reaper projects (each in their own Reaper instance) can both
        // emit sources with id=1, so `id` alone isn't unique across hosts.
        // The encoder's OSC *listen* port (`replyPort`, carried in the 9-arg
        // /ambi_enc variant) IS unique per plugin instance — UDP bind
        // uniqueness guarantees two instances on the same host can't share
        // it. Combining IP + replyPort in `originTag` disambiguates multiple
        // encoders with the same `m_id` living on the same machine.
        // Track renames don't affect identity — `trackName` stays out of the
        // key so a DAW track rename updates the row in place.
        //
        // MultiEncoder keeps the existing scheme: `originTag` is `ip:port` (to
        // disambiguate multiple plugin instances on the same host), and `id` is
        // the source index within that plugin — `trackName` is just the OSC
        // prefix and likewise isn't identity.
        if (origin == EncoderOrigin::Ambix)
            return { origin, id, juce::String(),
                     senderIp + ":" + juce::String (replyPort) };
        if (origin == EncoderOrigin::AmbixSource)
        {
            // AmbixSource pucks are identified by (sender endpoint, source
            // index within the plugin). The sender endpoint is the outgoing
            // UDP socket bound by the plugin's OSCSender — stable for a
            // running plugin and unique across plugin instances on the same
            // host. Track name stays out of the key so a DAW rename updates
            // the row in place.
            return { origin, id, juce::String(), originTag };
        }
        // MultiEncoder: keep previous behaviour — originTag (ip:port) plus
        // sourceIndex fully disambiguates across instances.
        return { origin, id, trackName, originTag };
    }
};
