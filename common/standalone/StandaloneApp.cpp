#include <JuceHeader.h>

// #include "MainComponent.h"

#include "StandaloneFilterWindow.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

#define VAL(str) #str
#define TOSTRING(str) VAL(str)

namespace jsa
{
using namespace juce;

//==============================================================================
class StandaloneFilterApp  : public juce::JUCEApplication
{
public:
    //==============================================================================
    StandaloneFilterApp()
    {
        PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_Standalone;

        PropertiesFile::Options options;

        options.applicationName     = getApplicationName();
        options.filenameSuffix      = ".settings";
        // options.filenameSuffix = "xml";
        options.osxLibrarySubFolder = "Application Support";
       #if JUCE_LINUX
        options.folderName          = "~/.config";
       #else
        options.folderName          = "";
        // options.folderName = ".distrho";
       #endif

        // options.commonToAllUsers = false;
        // options.ignoreCaseOfKeyNames = true;

        appProperties.setStorageParameters (options);

        // setup window name
        pluginWindowName << getApplicationName() << " v" << ProjectInfo::versionString;
        #ifdef JUCE_DEBUG
        pluginWindowName << " (DEBUG)";
        #endif
    }

    // We inject these as compile definitions from the CMakeLists.txt
    // If you've enabled the juce header with `juce_generate_juce_header(<thisTarget>)`
    // you could `#include <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    virtual StandaloneFilterWindow* createWindow()
    {
        #ifdef JucePlugin_PreferredChannelConfigurations
        StandalonePluginHolder::PluginInOuts channels[] = { JucePlugin_PreferredChannelConfigurations };
        #endif

        return new StandaloneFilterWindow (pluginWindowName,
                                            LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                                            appProperties.getUserSettings(),
                                            false, {}, nullptr
                                            #ifdef JucePlugin_PreferredChannelConfigurations
                                            , juce::Array<StandalonePluginHolder::PluginInOuts> (channels, juce::numElementsInArray (channels))
                                            #else
                                            , {}
                                            #endif
                                            #if JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                                            , false
                                            #endif
                                            );
    }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused (commandLine);
        // create the window
        // mainWindow.reset (new MainWindow (getApplicationName()));
        mainWindow.reset(createWindow());

        #if JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
        Desktop::getInstance().setKioskModeComponent (mainWindow.get(), false);
        #endif

        // mainWindow->toFront (true); //unsure to leave it
        mainWindow->setVisible (true);

        // Process::setPriority (Process::HighPriority);
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr;
        appProperties.saveIfNeeded(); // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        if (mainWindow.get() != nullptr)
            mainWindow->pluginHolder->savePluginState();

        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            Timer::callAfterDelay (100, []()
            {
                if (auto app = JUCEApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused (commandLine);
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
   /*
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
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
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
        */
        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */
       /*
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };
    */
// private:
protected:
    String pluginWindowName;

    ApplicationProperties appProperties;
    std::unique_ptr<StandaloneFilterWindow> mainWindow;
    // std::unique_ptr<MainWindow> mainWindow;
};

} // namespace jsa

#if JucePlugin_Build_Standalone && JUCE_IOS

using namespace juce;

bool JUCE_CALLTYPE juce_isInterAppAudioConnected()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->isInterAppAudioConnected();

    return false;
}

void JUCE_CALLTYPE juce_switchToHostApplication()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        holder->switchToHostApplication();
}

Image JUCE_CALLTYPE juce_getIAAHostIcon (int size)
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->getIAAHostIcon (size);

    return Image();
}
#endif

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (jsa::StandaloneFilterApp)
