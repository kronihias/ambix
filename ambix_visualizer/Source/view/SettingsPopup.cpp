#include "SettingsPopup.h"

#include "LevelMapping.h"

namespace
{
    constexpr int kRowHeight    = 32;
    constexpr int kRowSpacing   = 6;
    constexpr int kPopupWidth   = 360;
    constexpr int kPadding      = 10;
    constexpr int kLabelColumn  = 110;
}

SettingsPopup::SettingsPopup (AppSettings& s,
                              Callbacks cbs,
                              juce::String settingsFilePath,
                              bool settingsFileExists)
    : settings (s), callbacks (std::move (cbs))
{
    // --- Listen port ---------------------------------------------------------
    portLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (portLabel);

    portEditor.setText (juce::String (settings.listenPort), juce::dontSendNotification);
    portEditor.setInputRestrictions (6, "0123456789");
    portEditor.onReturnKey = [this]() { commitListenPort(); };
    portEditor.onFocusLost = [this]() { commitListenPort(); };
    addAndMakeVisible (portEditor);

    // --- Colormap ------------------------------------------------------------
    colormapLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (colormapLabel);

    populateColormapBox();
    colormapBox.onChange = [this]()
    {
        const auto cm = LevelMapping::colorMapFromName (colormapBox.getText());
        if (cm != settings.colorMap)
        {
            settings.colorMap = cm;
            if (callbacks.onColorOrRangeChanged) callbacks.onColorOrRangeChanged();
            if (callbacks.onPersist) callbacks.onPersist();
        }
    };
    addAndMakeVisible (colormapBox);

    // --- Range ---------------------------------------------------------------
    minDbLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (minDbLabel);

    minDbEditor.setText (juce::String (settings.minDb, 1), juce::dontSendNotification);
    minDbEditor.setInputRestrictions (6, "-.0123456789");
    minDbEditor.onReturnKey = [this]() { commitMinDb(); };
    minDbEditor.onFocusLost = [this]() { commitMinDb(); };
    addAndMakeVisible (minDbEditor);

    // --- Auto-clear ----------------------------------------------------------
    autoClearToggle.setToggleState (settings.autoClearEnabled, juce::dontSendNotification);
    autoClearToggle.onClick       = [this]() { commitAutoClear(); };
    autoClearToggle.onStateChange = [this]() { commitAutoClear(); };
    addAndMakeVisible (autoClearToggle);

    autoClearEditor.setText (juce::String (settings.autoClearTimeoutSec), juce::dontSendNotification);
    autoClearEditor.setInputRestrictions (4, "0123456789");
    autoClearEditor.onReturnKey = [this]() { commitAutoClear(); };
    autoClearEditor.onFocusLost = [this]() { commitAutoClear(); };
    addAndMakeVisible (autoClearEditor);

    autoClearUnit.setText ("s", juce::dontSendNotification);
    addAndMakeVisible (autoClearUnit);

    // --- Settings file diagnostic -------------------------------------------
    fileLabel.setFont (juce::FontOptions (10.0f));
    fileLabel.setColour (juce::Label::textColourId,
                         juce::Colours::white.withAlpha (0.45f));
    fileLabel.setJustificationType (juce::Justification::centredLeft);
    fileLabel.setText ("Settings file (" + juce::String (settingsFileExists ? "exists" : "missing")
                           + "):\n" + settingsFilePath,
                       juce::dontSendNotification);
    fileLabel.setMinimumHorizontalScale (0.5f);
    addAndMakeVisible (fileLabel);

    // Decide total height now that we know how many rows we have.
    constexpr int rows = 4; // port, colormap, range, auto-clear
    const int bodyHeight = kPadding + rows * (kRowHeight + kRowSpacing)
                         + 44 /* file label */ + kPadding;
    setSize (kPopupWidth, bodyHeight);
}

void SettingsPopup::populateColormapBox()
{
    colormapBox.clear (juce::dontSendNotification);
    int id = 1;
    const auto current = LevelMapping::colorMapToName (settings.colorMap);
    for (const auto& n : LevelMapping::getColorMapNames())
    {
        colormapBox.addItem (n, id);
        if (n == current)
            colormapBox.setSelectedId (id, juce::dontSendNotification);
        ++id;
    }
}

void SettingsPopup::commitListenPort()
{
    const int p = portEditor.getText().getIntValue();
    if (p > 0 && p <= 65535 && p != settings.listenPort)
    {
        settings.listenPort = p;
        if (callbacks.onListenPortChanged) callbacks.onListenPortChanged (p);
        if (callbacks.onPersist) callbacks.onPersist();
    }
    portEditor.setText (juce::String (settings.listenPort), juce::dontSendNotification);
}

void SettingsPopup::commitMinDb()
{
    const float v = minDbEditor.getText().getFloatValue();
    const float clamped = juce::jlimit (-120.0f, -1.0f, v);
    if (std::abs (clamped - settings.minDb) > 0.05f)
    {
        settings.minDb = clamped;
        if (callbacks.onColorOrRangeChanged) callbacks.onColorOrRangeChanged();
        if (callbacks.onPersist) callbacks.onPersist();
    }
    minDbEditor.setText (juce::String (settings.minDb, 1), juce::dontSendNotification);
}

void SettingsPopup::commitAutoClear()
{
    const bool enabled = autoClearToggle.getToggleState();
    const int  seconds = juce::jlimit (1, 3600,
                                       juce::jmax (1, autoClearEditor.getText().getIntValue()));

    if (enabled != settings.autoClearEnabled
        || seconds != settings.autoClearTimeoutSec)
    {
        settings.autoClearEnabled    = enabled;
        settings.autoClearTimeoutSec = seconds;
        if (callbacks.onAutoClearChanged) callbacks.onAutoClearChanged();
        if (callbacks.onPersist) callbacks.onPersist();
    }

    autoClearEditor.setText (juce::String (settings.autoClearTimeoutSec), juce::dontSendNotification);
    autoClearEditor.setEnabled (enabled);
    autoClearUnit.setEnabled   (enabled);
}

void SettingsPopup::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1F1F1F));
}

void SettingsPopup::resized()
{
    auto area = getLocalBounds().reduced (kPadding);

    auto row = [&]() -> juce::Rectangle<int>
    {
        auto r = area.removeFromTop (kRowHeight);
        area.removeFromTop (kRowSpacing);
        return r;
    };

    // Listen port
    {
        auto r = row();
        portLabel.setBounds  (r.removeFromLeft (kLabelColumn));
        r.removeFromLeft (6);
        portEditor.setBounds (r.removeFromLeft (100));
    }
    // Colormap
    {
        auto r = row();
        colormapLabel.setBounds (r.removeFromLeft (kLabelColumn));
        r.removeFromLeft (6);
        colormapBox.setBounds   (r.removeFromLeft (130));
    }
    // Range
    {
        auto r = row();
        minDbLabel.setBounds  (r.removeFromLeft (kLabelColumn));
        r.removeFromLeft (6);
        minDbEditor.setBounds (r.removeFromLeft (80));
    }
    // Auto-clear
    {
        auto r = row();
        r.removeFromLeft (kLabelColumn + 6);
        autoClearToggle.setBounds (r.removeFromLeft (150));
        autoClearEditor.setBounds (r.removeFromLeft (56));
        r.removeFromLeft (4);
        autoClearUnit.setBounds   (r.removeFromLeft (16));
    }

    // Diagnostic at the bottom
    area.removeFromTop (kRowSpacing);
    fileLabel.setBounds (area);
}
