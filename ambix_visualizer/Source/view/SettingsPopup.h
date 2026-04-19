#pragma once

#include <JuceHeader.h>

#include "../app/Settings.h"

// Content panel for the settings CallOutBox. Owns its widgets and writes
// through to the passed-in AppSettings on every edit. When anything changes,
// the provided `onChange` callback fires so MainComponent can persist +
// propagate the new state to views / listener.
class SettingsPopup : public juce::Component
{
public:
    struct Callbacks
    {
        // All optional.
        std::function<void()>             onPersist;      // save to disk + broadcast
        std::function<void (int)>         onListenPortChanged;
        std::function<void()>             onAutoClearChanged;
        std::function<void()>             onColorOrRangeChanged;
    };

    SettingsPopup (AppSettings& s,
                   Callbacks cbs,
                   juce::String settingsFilePath,
                   bool settingsFileExists);

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    void commitListenPort();
    void commitMinDb();
    void commitAutoClear();
    void populateColormapBox();

    AppSettings& settings;
    Callbacks callbacks;

    juce::Label      portLabel       { {}, "Listen port:" };
    juce::TextEditor portEditor;

    juce::Label      colormapLabel   { {}, "Colormap:" };
    juce::ComboBox   colormapBox;

    juce::Label      minDbLabel      { {}, "Range (dB):" };
    juce::TextEditor minDbEditor;

    juce::ToggleButton autoClearToggle { "Auto-clear after" };
    juce::TextEditor   autoClearEditor;
    juce::Label        autoClearUnit  { {}, "s" };

    juce::Label      fileLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPopup)
};
