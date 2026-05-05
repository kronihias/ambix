#include "Colormap.h"

namespace Colormap
{
    namespace
    {
        using Stop = std::pair<float, juce::Colour>;

        const std::vector<Stop> kJet = {
            { 0.000f, juce::Colour::fromRGB (  0,   0, 128) },
            { 0.125f, juce::Colour::fromRGB (  0,   0, 255) },
            { 0.375f, juce::Colour::fromRGB (  0, 255, 255) },
            { 0.500f, juce::Colour::fromRGB (  0, 255,   0) },
            { 0.625f, juce::Colour::fromRGB (255, 255,   0) },
            { 0.875f, juce::Colour::fromRGB (255,   0,   0) },
            { 1.000f, juce::Colour::fromRGB (128,   0,   0) },
        };

        const std::vector<Stop> kParula = {
            { 0.000f, juce::Colour::fromRGB ( 53,  42, 135) },
            { 0.200f, juce::Colour::fromRGB ( 18,  99, 223) },
            { 0.400f, juce::Colour::fromRGB (  4, 163, 187) },
            { 0.600f, juce::Colour::fromRGB ( 81, 188, 120) },
            { 0.800f, juce::Colour::fromRGB (229, 184,  65) },
            { 1.000f, juce::Colour::fromRGB (249, 251,  14) },
        };

        const std::vector<Stop> kTurbo = {
            { 0.000f, juce::Colour::fromRGB ( 48,  18,  59) },
            { 0.125f, juce::Colour::fromRGB ( 70, 107, 227) },
            { 0.250f, juce::Colour::fromRGB ( 41, 191, 236) },
            { 0.375f, juce::Colour::fromRGB ( 40, 229, 152) },
            { 0.500f, juce::Colour::fromRGB (145, 246,  66) },
            { 0.625f, juce::Colour::fromRGB (226, 222,  53) },
            { 0.750f, juce::Colour::fromRGB (249, 150,  39) },
            { 0.875f, juce::Colour::fromRGB (219,  66,  26) },
            { 1.000f, juce::Colour::fromRGB (122,  11,   4) },
        };

        const std::vector<Stop> kViridis = {
            { 0.000f, juce::Colour::fromRGB ( 68,   1,  84) },
            { 0.250f, juce::Colour::fromRGB ( 59,  82, 139) },
            { 0.500f, juce::Colour::fromRGB ( 33, 145, 140) },
            { 0.750f, juce::Colour::fromRGB ( 94, 201,  97) },
            { 1.000f, juce::Colour::fromRGB (253, 231,  37) },
        };

        const std::vector<Stop> kHot = {
            { 0.000f, juce::Colour::fromRGB (  0,   0,   0) },
            { 0.333f, juce::Colour::fromRGB (230,   0,   0) },
            { 0.666f, juce::Colour::fromRGB (255, 230,   0) },
            { 1.000f, juce::Colour::fromRGB (255, 255, 255) },
        };

        const std::vector<Stop> kCool = {
            { 0.000f, juce::Colour::fromRGB (  0, 255, 255) },
            { 1.000f, juce::Colour::fromRGB (255,   0, 255) },
        };

        const std::vector<Stop> kGray = {
            { 0.000f, juce::Colour::fromRGB (  0,   0,   0) },
            { 1.000f, juce::Colour::fromRGB (255, 255, 255) },
        };

        const std::vector<Stop>& stopsFor (Map m) noexcept
        {
            switch (m)
            {
                case Map::Jet:     return kJet;
                case Map::Parula:  return kParula;
                case Map::Turbo:   return kTurbo;
                case Map::Viridis: return kViridis;
                case Map::Hot:     return kHot;
                case Map::Cool:    return kCool;
                case Map::Gray:    return kGray;
                case Map::Hsv:     return kJet;
            }
            return kJet;
        }
    }

    juce::StringArray getNames()
    {
        return { "jet", "parula", "turbo", "viridis", "hot", "cool", "gray", "hsv" };
    }

    Map fromName (const juce::String& name)
    {
        const auto n = name.trim().toLowerCase();
        if (n == "jet")     return Map::Jet;
        if (n == "parula")  return Map::Parula;
        if (n == "turbo")   return Map::Turbo;
        if (n == "viridis") return Map::Viridis;
        if (n == "hot")     return Map::Hot;
        if (n == "cool")    return Map::Cool;
        if (n == "gray" || n == "grey") return Map::Gray;
        if (n == "hsv")     return Map::Hsv;
        return Map::Jet;
    }

    juce::String toName (Map m)
    {
        switch (m)
        {
            case Map::Jet:     return "jet";
            case Map::Parula:  return "parula";
            case Map::Turbo:   return "turbo";
            case Map::Viridis: return "viridis";
            case Map::Hot:     return "hot";
            case Map::Cool:    return "cool";
            case Map::Gray:    return "gray";
            case Map::Hsv:     return "hsv";
        }
        return "jet";
    }

    juce::Colour sample (Map m, float t) noexcept
    {
        t = juce::jlimit (0.0f, 1.0f, t);

        if (m == Map::Hsv)
            return juce::Colour::fromHSV (t, 1.0f, 1.0f, 1.0f);

        const auto& stops = stopsFor (m);
        if (stops.empty())
            return juce::Colours::black;

        for (size_t i = 1; i < stops.size(); ++i)
        {
            if (t <= stops[i].first)
            {
                const auto& a = stops[i - 1];
                const auto& b = stops[i];
                const float span = b.first - a.first;
                const float u = span > 0.0001f ? (t - a.first) / span : 0.0f;
                return a.second.interpolatedWith (b.second, juce::jlimit (0.0f, 1.0f, u));
            }
        }
        return stops.back().second;
    }
}
