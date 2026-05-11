#include "AmbixSourceParser.h"

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

    using Param = AmbixSourcePayload::Param;

    bool matchGlobal (const juce::String& tail, Param& outParam)
    {
        if (tail == "linked")          { outParam = Param::Linked;          return true; }
        if (tail == "active_sources")  { outParam = Param::ActiveSources;   return true; }
        if (tail == "azimuth")         { outParam = Param::GlobalAzimuth;   return true; }
        if (tail == "elevation")       { outParam = Param::GlobalElevation; return true; }
        if (tail == "width")           { outParam = Param::GlobalWidth;     return true; }
        if (tail == "size")            { outParam = Param::GlobalSize;      return true; }
        return false;
    }

    bool matchPerSource (const juce::String& leaf, Param& outParam)
    {
        if (leaf == "azimuth")   { outParam = Param::Azimuth;   return true; }
        if (leaf == "elevation") { outParam = Param::Elevation; return true; }
        if (leaf == "size")      { outParam = Param::Size;      return true; }
        if (leaf == "meter")     { outParam = Param::Meter;     return true; }
        return false;
    }
}

bool AmbixSourceParser::parse (const char* data, size_t size, AmbixSourcePayload& out)
{
    size_t pos = 0;

    juce::String address;
    if (! readOscString (data, size, pos, address))
        return false;
    if (! address.startsWith (kPrefix))
        return false;

    const auto tail = address.substring ((int) std::strlen (kPrefix)); // "source/<n>/azimuth" or "linked" etc.

    // Per-source: "source/<n>/<leaf>"
    if (tail.startsWith ("source/"))
    {
        const auto rest = tail.substring ((int) std::strlen ("source/"));
        const int slash = rest.indexOfChar ('/');
        if (slash <= 0) return false;
        const auto idxStr = rest.substring (0, slash);
        const auto leaf   = rest.substring (slash + 1);
        if (! idxStr.containsOnly ("0123456789")) return false;
        const int idx = idxStr.getIntValue();
        if (idx < 1 || idx > 64) return false;

        Param p;
        if (! matchPerSource (leaf, p)) return false;
        out.param = p;
        out.sourceIndex = idx;
    }
    else
    {
        Param p;
        if (! matchGlobal (tail, p)) return false;
        out.param = p;
        out.sourceIndex = 0;
    }

    juce::String typeTag;
    if (! readOscString (data, size, pos, typeTag)) return false;
    if (typeTag.length() < 2 || typeTag[0] != ',')  return false;

    if (typeTag[1] == 'f')
    {
        float v = 0.f;
        if (! readFloat32 (data, size, pos, v)) return false;
        out.value = v;
    }
    else if (typeTag[1] == 'i')
    {
        juce::int32 v = 0;
        if (! readInt32 (data, size, pos, v)) return false;
        out.value = (float) v;
    }
    else
    {
        return false;
    }

    return true;
}
