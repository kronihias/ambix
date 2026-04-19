#include "AmbiEncParser.h"

namespace
{
    inline size_t padTo4 (size_t n) noexcept { return (n + 3u) & ~size_t (3u); }

    // OSC uses big-endian network byte order.
    bool readInt32 (const char* data, size_t size, size_t& pos, juce::int32& out)
    {
        if (pos + 4 > size)
            return false;
        auto* p = reinterpret_cast<const juce::uint8*> (data + pos);
        out = static_cast<juce::int32> (
            (juce::uint32 (p[0]) << 24)
          | (juce::uint32 (p[1]) << 16)
          | (juce::uint32 (p[2]) << 8)
          |  juce::uint32 (p[3]));
        pos += 4;
        return true;
    }

    bool readFloat32 (const char* data, size_t size, size_t& pos, float& out)
    {
        juce::int32 raw = 0;
        if (! readInt32 (data, size, pos, raw))
            return false;
        std::memcpy (&out, &raw, sizeof (float));
        return true;
    }

    bool readOscString (const char* data, size_t size, size_t& pos, juce::String& out)
    {
        if (pos >= size)
            return false;

        const char* start = data + pos;
        size_t len = 0;
        while (pos + len < size && start[len] != '\0')
            ++len;

        if (pos + len >= size)
            return false; // no terminating null found

        out = juce::String::fromUTF8 (start, static_cast<int> (len));
        pos += padTo4 (len + 1);
        return true;
    }
}

bool AmbiEncParser::parse (const char* data, size_t size, AmbiEncPayload& out)
{
    size_t pos = 0;

    juce::String address;
    if (! readOscString (data, size, pos, address))
        return false;
    if (address != kAddress)
        return false;

    juce::String typeTag;
    if (! readOscString (data, size, pos, typeTag))
        return false;
    if (typeTag.isEmpty() || typeTag[0] != ',')
        return false;

    const juce::String tags = typeTag.substring (1);

    // Expected: isffffff (8 args = i,s,distance,az,el,size,peak,rms)
    // or        isffffffi (9 args, extra trailing i = replyPort)
    if (! (tags == "isffffff" || tags == "isffffffi"))
        return false;

    juce::int32 i32 = 0;
    float f = 0.0f;
    juce::String s;

    if (! readInt32 (data, size, pos, i32)) return false;
    out.id = static_cast<int> (i32);

    if (! readOscString (data, size, pos, s)) return false;
    out.trackName = s;

    if (! readFloat32 (data, size, pos, f)) return false;
    out.distance = f;

    if (! readFloat32 (data, size, pos, f)) return false;
    out.azimuthDeg = f;

    if (! readFloat32 (data, size, pos, f)) return false;
    out.elevationDeg = f;

    if (! readFloat32 (data, size, pos, f)) return false;
    out.size = f;

    if (! readFloat32 (data, size, pos, f)) return false;
    out.peakLinear = f;

    if (! readFloat32 (data, size, pos, f)) return false;
    out.rmsLinear = f;

    if (tags.endsWithChar ('i'))
    {
        if (! readInt32 (data, size, pos, i32)) return false;
        out.replyPort = static_cast<int> (i32);
        out.hasReplyPort = true;
    }
    else
    {
        out.hasReplyPort = false;
        out.replyPort = 0;
    }

    return true;
}
