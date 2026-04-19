#include <JuceHeader.h>

#include "MainComponent.h"

class AmbixVisualizerApplication : public juce::JUCEApplication
{
public:
    AmbixVisualizerApplication() = default;

    const juce::String getApplicationName() override       { return "ambix Visualizer"; }
    const juce::String getApplicationVersion() override    { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted (const juce::String&) override {}

    // On iOS the OS may suspend / kill the app without giving destructors a
    // chance to run. Every setting change already writes through immediately
    // via SettingsStore::save(), but a value typed into a TextEditor that
    // hasn't yet lost focus is only committed on focus loss — unfocus
    // everything here so any pending edit is flushed to disk before sleep.
    void suspended() override
    {
        juce::Component::unfocusAllComponents();
    }
    void resumed() override {}

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : juce::DocumentWindow (name,
                                    juce::Desktop::getInstance().getDefaultLookAndFeel()
                                        .findColour (juce::ResizableWindow::backgroundColourId),
                                    juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (AmbixVisualizerApplication)
