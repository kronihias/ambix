#include "MultiEncoderParser.h"

namespace
{
    inline size_t padTo4 (size_t n) noexcept { return (n + 3u) & ~size_t (3u); }

    bool readInt32 (const char* data, size_t size, size_t& pos, juce::int32& out)
    {
        if (pos + 4 > size) return false;
        auto* p = reinterpret_cast<const juce::uint8*> (data + pos);
        out = static_cast<juce::int32> (
            (juce::uint32 (p[0]) << 24) | (juce::uint32 (p[1]) << 16)
          | (juce::uint32 (p[2]) << 8)  |  juce::uint32 (p[3]));
        pos += 4;
        return true;
    }

    bool readFloat32 (const char* data, size_t size, size_t& pos, float& out)
    {
        juce::int32 raw = 0;
        if (! readInt32 (data, size, pos, raw)) return false;
        std::memcpy (&out, &raw, sizeof (float));
        return true;
    }

    bool readOscString (const char* data, size_t size, size_t& pos, juce::String& out)
    {
        if (pos >= size) return false;
        const char* start = data + pos;
        size_t len = 0;
        while (pos + len < size && start[len] != '\0') ++len;
        if (pos + len >= size) return false;
        out = juce::String::fromUTF8 (start, static_cast<int> (len));
        pos += padTo4 (len + 1);
        return true;
    }

    using Param = MultiEncoderPayload::Param;

    struct ParamName { const char* name; Param tag; };
    constexpr ParamName kParamNames[] = {
        { "azimuth",   Param::Azimuth   },
        { "elevation", Param::Elevation },
        { "gain",      Param::Gain      },
        { "mute",      Param::Mute      },
        { "solo",      Param::Solo      },
    };

    // Parse "<paramName><integer>" (e.g. "azimuth5") into (param, index).
    bool parseParamNameWithIndex (const juce::String& s, Param& param, int& index)
    {
        for (const auto& entry : kParamNames)
        {
            const auto name = juce::String (entry.name);
            if (! s.startsWith (name))
                continue;
            const auto rest = s.substring (name.length());
            if (rest.isEmpty() || ! rest.containsOnly ("0123456789"))
                continue;
            param = entry.tag;
            index = rest.getIntValue();
            return true;
        }
        return false;
    }
}

bool MultiEncoderParser::parse (const char* data, size_t size, MultiEncoderPayload& out)
{
    size_t pos = 0;

    juce::String address;
    if (! readOscString (data, size, pos, address))
        return false;
    if (! address.startsWithChar ('/'))
        return false;

    // Split address into `<prefix>/<leaf>` on the last '/' (ignoring the leading one).
    const auto noLeading  = address.substring (1);
    const int lastSlash   = noLeading.lastIndexOfChar ('/');
    if (lastSlash <= 0)
        return false; // need at least /<something>/<leaf>

    const auto prefix = noLeading.substring (0, lastSlash);
    const auto leaf   = noLeading.substring (lastSlash + 1);

    Param param;
    int index = 0;
    if (! parseParamNameWithIndex (leaf, param, index))
        return false;
    if (index < 0 || index > 63)
        return false;

    juce::String typeTag;
    if (! readOscString (data, size, pos, typeTag))
        return false;
    if (typeTag.length() < 2 || typeTag[0] != ',')
        return false;

    // Accept ",f" (float) or ",i" (int). Both occur for normalized/raw values.
    float value = 0.0f;
    if (typeTag[1] == 'f')
    {
        if (! readFloat32 (data, size, pos, value))
            return false;
    }
    else if (typeTag[1] == 'i')
    {
        juce::int32 i = 0;
        if (! readInt32 (data, size, pos, i))
            return false;
        value = static_cast<float> (i);
    }
    else
    {
        return false;
    }

    // IEM uses math-convention azimuth (counter-clockwise-positive), while
    // the rest of the app (ambix convention) is clockwise-positive. Flip the
    // sign on the boundary so the registry stores a consistent convention.
    if (param == Param::Azimuth)
        value = -value;

    out.addressPrefix = prefix;
    out.param         = param;
    out.sourceIndex   = index;
    out.value         = value;
    return true;
}

juce::String MultiEncoderParser::buildAddress (const juce::String& prefix,
                                               MultiEncoderPayload::Param param,
                                               int index)
{
    const char* name = "azimuth";
    switch (param)
    {
        case Param::Azimuth:   name = "azimuth";   break;
        case Param::Elevation: name = "elevation"; break;
        case Param::Gain:      name = "gain";      break;
        case Param::Mute:      name = "mute";      break;
        case Param::Solo:      name = "solo";      break;
    }
    const auto cleanPrefix = prefix.trim().isNotEmpty() ? prefix : juce::String ("MultiEncoder");
    return "/" + cleanPrefix + "/" + name + juce::String (index);
}
