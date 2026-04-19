#include "LevelMapping.h"

namespace LevelMapping
{
    namespace
    {
        using Stop = std::pair<float, juce::Colour>;

        // Each colormap is a list of (t, colour) control points in [0,1]
        // which we linearly interpolate in RGB. Covers the common MATLAB set.
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

        const std::vector<Stop>& stopsFor (ColorMap m) noexcept
        {
            switch (m)
            {
                case ColorMap::Jet:     return kJet;
                case ColorMap::Parula:  return kParula;
                case ColorMap::Turbo:   return kTurbo;
                case ColorMap::Viridis: return kViridis;
                case ColorMap::Hot:     return kHot;
                case ColorMap::Cool:    return kCool;
                case ColorMap::Gray:    return kGray;
                case ColorMap::Hsv:     return kJet; // handled separately
            }
            return kJet;
        }
    }

    juce::StringArray getColorMapNames()
    {
        return { "jet", "parula", "turbo", "viridis", "hot", "cool", "gray", "hsv" };
    }

    ColorMap colorMapFromName (const juce::String& name)
    {
        const auto n = name.trim().toLowerCase();
        if (n == "jet")     return ColorMap::Jet;
        if (n == "parula")  return ColorMap::Parula;
        if (n == "turbo")   return ColorMap::Turbo;
        if (n == "viridis") return ColorMap::Viridis;
        if (n == "hot")     return ColorMap::Hot;
        if (n == "cool")    return ColorMap::Cool;
        if (n == "gray" || n == "grey") return ColorMap::Gray;
        if (n == "hsv")     return ColorMap::Hsv;
        return ColorMap::Jet;
    }

    juce::String colorMapToName (ColorMap map)
    {
        switch (map)
        {
            case ColorMap::Jet:     return "jet";
            case ColorMap::Parula:  return "parula";
            case ColorMap::Turbo:   return "turbo";
            case ColorMap::Viridis: return "viridis";
            case ColorMap::Hot:     return "hot";
            case ColorMap::Cool:    return "cool";
            case ColorMap::Gray:    return "gray";
            case ColorMap::Hsv:     return "hsv";
        }
        return "jet";
    }

    juce::Colour sampleColorMap (ColorMap map, float t) noexcept
    {
        t = juce::jlimit (0.0f, 1.0f, t);

        if (map == ColorMap::Hsv)
            return juce::Colour::fromHSV (t, 1.0f, 1.0f, 1.0f);

        const auto& stops = stopsFor (map);
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

    juce::Colour peakRingColour (float peakHoldDb) noexcept
    {
        const float t = juce::jlimit (0.0f, 1.0f, (peakHoldDb + 20.0f) / 20.0f);
        const auto cold = juce::Colours::white.withAlpha (0.0f);
        const auto warm = juce::Colour::fromRGBA (0xFF, 0x33, 0x22, 0xFF);
        return cold.interpolatedWith (warm, t);
    }
}
