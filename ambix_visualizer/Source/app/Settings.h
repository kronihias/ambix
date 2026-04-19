#pragma once

#include <JuceHeader.h>

#include "../view/LevelMapping.h"

struct AppSettings
{
    // Visual size of source pucks. Small keeps the existing radius range;
    // Medium/Large scale it up for distant viewing or touchscreens. The
    // combo lives on both the 2D and 3D view's top bar; either view mutates
    // the setting and syncs the sibling.
    enum class PuckSize { Small = 0, Medium = 1, Large = 2 };

    int  listenPort           { 7130 };
    bool autoClearEnabled     { false };
    int  autoClearTimeoutSec  { 10 };

    PuckSize puckSize         { PuckSize::Small };

    LevelMapping::ColorMap colorMap { LevelMapping::ColorMap::Jet };
    float minDb                { -60.0f };

    bool  upperHemisphereOnly    { false };
    float upperHemisphereFloorDeg { -10.0f }; // show ~10° below the equator

    // Mirror the view horizontally. Useful when looking at the visualisation
    // from the audience side rather than the listener side.
    bool  flipLeftRight          { false };

    // Tab + 3D view state
    int   currentTabIndex        { 0 };
    bool  sphereLocked           { false };
    float sphereYawDeg           { 0.0f };
    float spherePitchDeg         { 0.0f };

    // Network discovery (NSD / JUCE NetworkServiceDiscovery). The visualizer
    // only *browses* — it doesn't advertise itself. Identity sent along with
    // /ambi_enc_subscribe is (uuid, hostname).
    juce::String visualizerUuid;                  // stable identity across sessions
    // Set of encoder-NSD instanceIDs the user has opted to subscribe to.
    // Serialised as a "\n"-separated string.
    juce::String subscribedEncoderIds;

    // Convenience: visible ambix elevation range for both views
    float minVisibleEleDeg() const noexcept
    {
        return upperHemisphereOnly ? upperHemisphereFloorDeg : -90.0f;
    }
    float maxVisibleEleDeg() const noexcept { return 90.0f; }
};

class SettingsStore
{
public:
    SettingsStore();

    AppSettings load();
    void save (const AppSettings& s);

    juce::File getFile() const;
    bool       fileExists() const;

private:
    juce::ApplicationProperties appProps;
};
