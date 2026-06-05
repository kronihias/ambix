//==============================================================================
/**
 * OVERRIDE StandaloneFilterWindow for native OS standalone window
 *  
*/

#ifndef JUCE_STANDALONE_FILTER_WINDOW_H_INCLUDED
#define JUCE_STANDALONE_FILTER_WINDOW_H_INCLUDED

#include "JuceHeader.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#include "AudioDeviceSelectorComponent.h"
#if JSA_VENDORED_JACK
 #include "JackAudioDevice.h"
#endif

namespace jsa
{
using namespace juce;
//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplicationBase::initialise() method, and
    let it do its work. It will create your filter object using the same createPluginFilter() function
    that the other plugin wrappers use.

    @tags{Audio}
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                        public MenuBarModel,
                                        private Button::Listener
{
public:
    //==============================================================================
    typedef StandalonePluginHolder::PluginInOuts PluginInOuts;

    //==============================================================================
    /** Creates a window with a given title and colour.
        The settings object can be a PropertySet that the class should use to
        store its settings (it can also be null). If takeOwnershipOfSettings is
        true, then the settings object will be owned and deleted by this object.
    */
    StandaloneFilterWindow (const String& title,
                                  Colour backgroundColour,
                                  PropertySet* settingsToUse,
                                  bool takeOwnershipOfSettings,
                                  const String& preferredDefaultDeviceName = String(),
                                  const AudioDeviceManager::AudioDeviceSetup* preferredSetupOptions = nullptr,
                                  const Array<PluginInOuts>& constrainToConfiguration = {},
                                #if JUCE_ANDROID || JUCE_IOS
                                  bool autoOpenMidiDevices = true
                                #else
                                  bool autoOpenMidiDevices = false
                                #endif
                                ) : DocumentWindow (title,
                                                    backgroundColour,
                                                    DocumentWindow::minimiseButton | DocumentWindow::closeButton),
                                    optionsButton ("Options")
    {
       #if JUCE_IOS || JUCE_ANDROID
        setTitleBarHeight (0);
       #else
        Component::addAndMakeVisible (optionsButton);
        optionsButton.addListener (this);
        optionsButton.setTriggeredOnMouseDown (true);
       #if JUCE_WINDOWS
        setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);
        #else
        setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, true);
        #endif
       #endif
//        setTitleBarHeight (0);
//        clearContentComponent();
        
        pluginHolder.reset (new StandalonePluginHolder (settingsToUse, takeOwnershipOfSettings,
                                                        preferredDefaultDeviceName, preferredSetupOptions,
                                                        constrainToConfiguration, autoOpenMidiDevices));

       #if JSA_VENDORED_JACK
        // JUCE is built without its own JACK backend; register the vendored one.
        // The holder's ctor has already created the built-in device types (e.g.
        // ALSA), so this appends JACK to the list. Prefer JACK as the active type
        // on a fresh install BEFORE the reopen below, so the reopen actually
        // opens a JACK device (a saved setup is honoured by the reopen instead).
        registerVendoredJackDeviceType();
        preferJackBackendIfFreshInstall();
       #endif

       #ifdef JSA_STANDALONE_MAX_CHANNELS
        // JUCE's StandalonePluginHolder initialises the AudioDeviceManager
        // with the processor's default bus layout channel count. In the MC
        // build that default is stereo (required for VST3 compliance), so
        // the standalone only opens 2-in/2-out. Re-open the device with
        // JSA_STANDALONE_MAX_CHANNELS (== JSA_STANDALONE_MAX_CHANNELS == 128) channels so the user
        // can actually select multichannel devices like BlackHole 16ch.
        reopenDeviceManagerForMaxChannels();
       #endif

        // Start unmuted so live input always passes through; muting is an opt-in
        // toggle in the Audio Settings dialog (works on every backend, including
        // JACK, as a quick mute while re-patching).
        pluginHolder->getMuteInputValue().setValue (false);

        // Always use the native OS title bar so the window gets full desktop
        // integration (snapping, the system min/max/close buttons, the window
        // menu, HiDPI, and the macOS system menu bar). The app menu is shown
        // as an in-window menu bar on Linux/Windows (setMenuBar) and in the
        // system menu bar on macOS (setMacMainMenu); the redundant top-right
        // Options button is therefore hidden.
        setUsingNativeTitleBar (true);
        optionsButton.setVisible (false);

        if (auto* props = pluginHolder->settings.get())
        {
            const int x = props->getIntValue ("windowX", -100);
            const int y = props->getIntValue ("windowY", -100);

            if (x != -100 && y != -100)
                setBoundsConstrained ({ x, y, getWidth(), getHeight() });
            else
                centreWithSize (getWidth(), getHeight());
        }
        else
        {
            centreWithSize (getWidth(), getHeight());
        }
        #if JUCE_MAC
            setMacMainMenu(this);
        #else
            setMenuBar (this);
        #endif
        
        #if JUCE_IOS || JUCE_ANDROID
         setFullScreen (true);
         setContentOwned (new MainContentComponent (*this), false);
        #else
         setContentOwned (new MainContentComponent (*this), true);
        
        // addToDesktop();
        //end modified code
       #endif
    }


    ~StandaloneFilterWindow() override
    {
       #if (! JUCE_IOS) && (! JUCE_ANDROID)
        if (auto* props = pluginHolder->settings.get())
        {
            props->setValue ("windowX", getX());
            props->setValue ("windowY", getY());
        }
       #endif

        #if JUCE_MAC
            MenuBarModel::setMacMainMenu (0);
        #else
            setMenuBar (0);
        #endif

        pluginHolder->stopPlaying();
        clearContentComponent();
        pluginHolder = nullptr;
    }

    //==============================================================================
    StringArray getMenuBarNames() override
    {
        const char* const names[] = { "File", "Presets", "Options", nullptr };

        return StringArray (names);
    }

    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/) override
    {
        PopupMenu menu;

        if (topLevelMenuIndex == 0)
        {
            menu.addItem (1, TRANS("Load a saved state..."));
            menu.addItem (2, TRANS("Save current state"));
            menu.addItem (3, TRANS("Save current state as..."));
            menu.addSeparator();
            menu.addItem (4, TRANS("Reset to default state"));
            menu.addSeparator();
            menu.addItem (5, TRANS("Quit"));
        }
        else if (topLevelMenuIndex == 1)
        {
            if (getAudioProcessor() != nullptr && getAudioProcessor()->getNumPrograms() > 0)
            {
                for (int i=0; i < getAudioProcessor()->getNumPrograms(); i++)
                    menu.addItem (11+i, getAudioProcessor()->getProgramName(i));
            }
            else
                menu.addItem (10, TRANS("(none)"), false);
        }
        else if (topLevelMenuIndex == 2)
        {
            menu.addItem (6, TRANS("Audio Settings..."));
            // The mute-input toggle lives in the Audio Settings dialog (it works
            // for every backend, including JACK, as a quick mute while re-patching).
        }
        return menu;
    }

    void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override
    {
        if (menuItemID == 1)
            pluginHolder->askUserToLoadState();
        else if (menuItemID == 2)
            pluginHolder->savePluginState();
        else if (menuItemID == 3)
            pluginHolder->askUserToLoadState();
        else if (menuItemID == 4)
            resetToDefaultState();
        else if (menuItemID == 5)
            closeButtonPressed();
        else if (menuItemID == 6)
            showAudioSettingsDialogMC();
        else if (menuItemID >= 11)
          getAudioProcessor()->setCurrentProgram(menuItemID-11);
    }

    //==============================================================================
    AudioProcessor* getAudioProcessor() const noexcept      { return pluginHolder->processor.get(); }
    AudioDeviceManager& getDeviceManager() const noexcept   { return pluginHolder->deviceManager; }

    /** Deletes and re-creates the plugin, resetting it to its default state. */
    void resetToDefaultState()
    {
        pluginHolder->stopPlaying();
        clearContentComponent();
        pluginHolder->deletePlugin();

        if (auto* props = pluginHolder->settings.get())
            props->removeValue ("filterState");

        pluginHolder->createPlugin();
        setContentOwned (new MainContentComponent (*this), true);
        pluginHolder->startPlaying();

        resized();
    }

    //==============================================================================
    void closeButtonPressed() override
    {
        pluginHolder->savePluginState();

        JUCEApplicationBase::quit();
    }

    void handleMenuResult (int result)
    {
        switch (result)
        {
            case 1:  showAudioSettingsDialogMC(); break;
            case 2:  pluginHolder->askUserToSaveState(); break;
            case 3:  pluginHolder->askUserToLoadState(); break;
            case 4:  resetToDefaultState(); break;
            default: break;
        }
    }

    static void menuCallback (int result, StandaloneFilterWindow* button)
    {
        if (button != nullptr && result != 0)
            button->handleMenuResult (result);
    }

    void resized() override
    {
        DocumentWindow::resized();
        #if JUCE_WINDOWS
        optionsButton.setBounds (8, 6, 60, getTitleBarHeight() - 8);
        #else
        optionsButton.setBounds (getWidth()-68, 6, 60, getTitleBarHeight() - 8);
        #endif
    }

    virtual StandalonePluginHolder* getPluginHolder()    { return pluginHolder.get(); }

   #if JSA_VENDORED_JACK
    //==============================================================================
    /** Registers mcfx's vendored JACK backend on the holder's AudioDeviceManager.
        JUCE is compiled without JUCE_JACK, so it never creates its own JACK type;
        the built-in types (ALSA on Linux) were already created when the holder
        constructed and initialised the device manager, so addAudioDeviceType()
        simply appends JACK. Done before any re-initialise so a saved JACK setup
        restores correctly. Preferring JACK as the default backend (for a fresh
        install) is handled separately, after the device manager is fully set up. */
    void registerVendoredJackDeviceType()
    {
        if (pluginHolder == nullptr)
            return;

        auto* jackType = createJackAudioDeviceType();

        // Scan now so the device manager immediately sees JACK's available ports.
        // addAudioDeviceType() does not mark the type list as needing a rescan, so
        // the device manager's next initialise() would call getDeviceNames() on an
        // unscanned JACK type, conclude it has no devices, and fall back to ALSA.
        jackType->scanForDevices();

        pluginHolder->deviceManager.addAudioDeviceType (
            std::unique_ptr<AudioIODeviceType> (jackType));
    }

    /** On a fresh install (no saved audio setup) make JACK the active backend,
        since these are multichannel JACK tools. Called after reopen/init so the
        choice is not reset; an existing saved choice is left untouched. */
    void preferJackBackendIfFreshInstall()
    {
        if (pluginHolder == nullptr)
            return;

        bool hasSavedSetup = false;
        if (auto* props = pluginHolder->settings.get())
            hasSavedSetup = props->getXmlValue ("audioSetup") != nullptr;

        if (! hasSavedSetup)
            pluginHolder->deviceManager.setCurrentAudioDeviceType ("JACK", true);
    }
   #endif

    //==============================================================================
    /** Re-initialises the AudioDeviceManager with JSA_STANDALONE_MAX_CHANNELS inputs/outputs
        so the user can pick multichannel devices in the Audio Settings dialog.
        JUCE's StandalonePluginHolder only opens as many channels as the
        processor's default bus layout advertises (2 in the MC build). */
    void reopenDeviceManagerForMaxChannels()
    {
       #ifdef JSA_STANDALONE_MAX_CHANNELS
        if (pluginHolder == nullptr)
            return;

        pluginHolder->stopPlaying();

        std::unique_ptr<XmlElement> savedState;
        if (auto* props = pluginHolder->settings.get())
            savedState = props->getXmlValue ("audioSetup");

        const int maxCh = (int) JSA_STANDALONE_MAX_CHANNELS;  // == JSA_STANDALONE_MAX_CHANNELS

        // Initialise the device manager with the full max channel count.
        // Actual channel count opened on the device is still negotiated from
        // the user's device choice; this just raises the ceiling.
        pluginHolder->deviceManager.initialise (maxCh,        // numInputChannels
                                                maxCh,        // numOutputChannels
                                                savedState.get(),
                                                true);        // selectDefaultDeviceOnFailure

        pluginHolder->startPlaying();
       #endif
    }

    /** Shows the Audio/MIDI settings dialog. In the MC build this uses an
        AudioDeviceSelectorComponent configured with max = JSA_STANDALONE_MAX_CHANNELS so
        every channel of a multichannel device can be enabled. In the
        per-channel VST2 build this simply forwards to JUCE's default. */
    void showAudioSettingsDialogMC()
    {
       #ifdef JSA_STANDALONE_MAX_CHANNELS
        const int maxCh = (int) JSA_STANDALONE_MAX_CHANNELS;

        // For a bus whose AudioChannelSet is ambisonic, only offer valid
        // ambisonic channel counts (1, 4, 9, 16, ...). Channel-based plug-ins
        // (mcfx, or e.g. a discrete source/loudspeaker bus) keep the full range.
        auto busIsAmbisonic = [this] (bool isInput) -> bool
        {
            if (auto* p = getAudioProcessor())
                if (auto* bus = p->getBus (isInput, 0))
                    return bus->getDefaultLayout().getAmbisonicOrder() >= 0;
            return false;
        };

        // Link the input and output channel-count selectors into a single
        // "Channels" control when the plug-in's input and output buses are
        // identical (same AudioChannelSet) - which covers most symmetric
        // ambisonic / channel processors. Plug-ins that genuinely want
        // independent in/out counts but happen to share a default layout can
        // force them apart by defining JSA_STANDALONE_INDEPENDENT_IO.
        bool linkIO = false;
        if (auto* p = getAudioProcessor())
            if (auto* inBus = p->getBus (true, 0))
                if (auto* outBus = p->getBus (false, 0))
                    linkIO = (inBus->getDefaultLayout() == outBus->getDefaultLayout());
       #ifdef JSA_STANDALONE_INDEPENDENT_IO
        linkIO = false;
       #endif

        // jsa::AudioDeviceSelectorComponent is a vendored copy of JUCE's
        // AudioDeviceSelectorComponent that replaces the per-channel (stereo-pair)
        // on/off list with a plain input/output channel-count selector, which is
        // what multichannel plug-ins actually want. The stereo-pairs argument
        // is therefore irrelevant here.
        auto* selector = new jsa::AudioDeviceSelectorComponent (pluginHolder->deviceManager,
                                                           0, maxCh,   // min/max inputs
                                                           0, maxCh,   // min/max outputs
                                                           true,       // show MIDI inputs
                                                           getAudioProcessor() != nullptr
                                                               && getAudioProcessor()->producesMidi(),
                                                           false,      // show channels as stereo pairs (unused)
                                                           false,      // hide advanced options
                                                           busIsAmbisonic (true),    // restrict input counts to ambisonic
                                                           busIsAmbisonic (false),   // restrict output counts to ambisonic
                                                           linkIO,                   // single combined channel selector
                                                           true,                     // show "Mute audio input" toggle (all backends)
                                                           pluginHolder->getMuteInputValue());
        selector->setSize (500, 650);

        DialogWindow::LaunchOptions o;
        o.content.setOwned (selector);
        o.dialogTitle                   = TRANS ("Audio/MIDI Settings");
        o.dialogBackgroundColour        = o.content->getLookAndFeel()
                                              .findColour (ResizableWindow::backgroundColourId);
        o.escapeKeyTriggersCloseButton  = true;
        o.useNativeTitleBar             = true;
        o.resizable                     = true;

        o.launchAsync();
       #else
        pluginHolder->showAudioSettingsDialog();
       #endif
    }

    std::unique_ptr<StandalonePluginHolder> pluginHolder;

private:
    void buttonClicked (Button*) override
    {
        PopupMenu m;
        m.addItem (1, TRANS("Audio/MIDI Settings..."));
        m.addSeparator();
        m.addItem (2, TRANS("Save current state..."));
        m.addItem (3, TRANS("Load a saved state..."));
        m.addSeparator();
        m.addItem (4, TRANS("Reset to default state"));

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (menuCallback, this));
    }

    //==============================================================================
    class MainContentComponent  : public Component,
                                  private Value::Listener,
                                  private Button::Listener,
                                  private ComponentListener
    {
    public:
        MainContentComponent (StandaloneFilterWindow& filterWindow)
            : owner (filterWindow), notification (this),
              editor (owner.getAudioProcessor()->hasEditor() ? owner.getAudioProcessor()->createEditorIfNeeded()
                                                             : new GenericAudioProcessorEditor (*owner.getAudioProcessor()))
        {
            Value& inputMutedValue = owner.pluginHolder->getMuteInputValue();

            if (editor != nullptr)
            {
                editor->addComponentListener (this);
                componentMovedOrResized (*editor, false, true);

                addAndMakeVisible (editor.get());
            }

            addChildComponent (notification);

            // The JACK backend never auto-connects ports, so there is no feedback
            // loop to guard against: input is force-unmuted in the window ctor and
            // we never wire up / show the "Audio input is muted to avoid feedback
            // loop" notification.
            ignoreUnused (inputMutedValue);
            shouldShowNotification = false;

            inputMutedChanged (shouldShowNotification);
        }

        ~MainContentComponent() override
        {
            if (editor != nullptr)
            {
                editor->removeComponentListener (this);
                owner.pluginHolder->processor->editorBeingDeleted (editor.get());
                editor = nullptr;
            }
        }

        void resized() override
        {
            auto r = getLocalBounds();

            if (shouldShowNotification)
                notification.setBounds (r.removeFromTop (NotificationArea::height));

            if (editor != nullptr)
                editor->setBounds (editor->getLocalArea (this, r)
                                          .withPosition (r.getTopLeft().transformedBy (editor->getTransform().inverted())));
        }

    private:
        //==============================================================================
        class NotificationArea : public Component
        {
        public:
            enum { height = 30 };

            NotificationArea (Button::Listener* settingsButtonListener)
                : notification ("notification", "Audio input is muted to avoid feedback loop"),
                 #if JUCE_IOS || JUCE_ANDROID
                  settingsButton ("Unmute Input")
                 #else
                  settingsButton ("Settings...")
                 #endif
            {
                setOpaque (true);

                notification.setColour (Label::textColourId, Colours::black);

                settingsButton.addListener (settingsButtonListener);

                addAndMakeVisible (notification);
                addAndMakeVisible (settingsButton);
            }

            void paint (Graphics& g) override
            {
                auto r = getLocalBounds();

                g.setColour (Colours::darkgoldenrod);
                g.fillRect (r.removeFromBottom (1));

                g.setColour (Colours::lightgoldenrodyellow);
                g.fillRect (r);
            }

            void resized() override
            {
                auto r = getLocalBounds().reduced (5);

                settingsButton.setBounds (r.removeFromRight (70));
                notification.setBounds (r);
            }
        private:
            Label notification;
            TextButton settingsButton;
        };

        //==============================================================================
        void inputMutedChanged (bool newInputMutedValue)
        {
            shouldShowNotification = newInputMutedValue;
            notification.setVisible (shouldShowNotification);

           #if JUCE_IOS || JUCE_ANDROID
            resized();
           #else
            if (editor != nullptr)
            {
                auto rect = getSizeToContainEditor();

                setSize (rect.getWidth(),
                         rect.getHeight() + (shouldShowNotification ? NotificationArea::height : 0));
            }
           #endif
        }

        void valueChanged (Value& value) override     { inputMutedChanged (value.getValue()); }
        void buttonClicked (Button*) override
        {
           #if JUCE_IOS || JUCE_ANDROID
            owner.pluginHolder->getMuteInputValue().setValue (false);
           #else
            owner.showAudioSettingsDialogMC();
           #endif
        }

        //==============================================================================
        void componentMovedOrResized (Component&, bool, bool) override
        {
            if (editor != nullptr)
            {
                auto rect = getSizeToContainEditor();

                setSize (rect.getWidth(),
                         rect.getHeight() + (shouldShowNotification ? NotificationArea::height : 0));
            }
        }

        Rectangle<int> getSizeToContainEditor() const
        {
            if (editor != nullptr)
                return getLocalArea (editor.get(), editor->getLocalBounds());

            return {};
        }

        //==============================================================================
        StandaloneFilterWindow& owner;
        NotificationArea notification;
        std::unique_ptr<AudioProcessorEditor> editor;
        bool shouldShowNotification = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
    };

    //==============================================================================
    TextButton optionsButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow)
};

} // namespace jsa

#endif // JUCE_STANDALONE_FILTER_WINDOW_H_INCLUDED
