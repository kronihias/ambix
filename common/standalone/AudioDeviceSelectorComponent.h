/*
  ==============================================================================
   AudioDeviceSelectorComponent

   A vendored, mcfx-local copy of JUCE's AudioDeviceSelectorComponent
   (juce_AudioDeviceSelectorComponent.{h,cpp}). The only behavioural change vs
   stock JUCE is that the per-channel (stereo-pair) on/off list is replaced by a
   plain "how many input/output channels" ComboBox: multichannel mcfx plug-ins
   want a fixed number of consecutive channels, not individual pair toggles.

   Vendored instead of patching the JUCE submodule, and wrapped in namespace
   mcfx so its file-local helpers (SimpleDeviceManagerInputLevelMeter,
   AudioDeviceSettingsPanel, ChannelSelectorListBox, ...) do not ODR-clash with
   JUCE's own copies linked into the same standalone binary.

   Original JUCE source is licensed under the JUCE 8 licence / AGPLv3; see
   JUCE/modules/juce_audio_utils/gui/juce_AudioDeviceSelectorComponent.cpp.
  ==============================================================================
*/

#pragma once

namespace jsa
{
using namespace juce;

//==============================================================================
/**
    A component containing controls to let the user change the audio settings of
    an AudioDeviceManager object.

    Very easy to use - just create one of these and show it to the user.

    @see AudioDeviceManager

    @tags{Audio}
*/
class JUCE_API  AudioDeviceSelectorComponent  : public Component,
                                                private ChangeListener
{
public:
    //==============================================================================
    /** Creates the component.

        If your app needs only output channels, you might ask for a maximum of 0 input
        channels, and the component won't display any options for choosing the input
        channels. And likewise if you're doing an input-only app.

        @param deviceManager            the device manager that this component should control
        @param minAudioInputChannels    the minimum number of audio input channels that the application needs
        @param maxAudioInputChannels    the maximum number of audio input channels that the application needs
        @param minAudioOutputChannels   the minimum number of audio output channels that the application needs
        @param maxAudioOutputChannels   the maximum number of audio output channels that the application needs
        @param showMidiInputOptions     if true, the component will allow the user to select which midi inputs are enabled
        @param showMidiOutputSelector   if true, the component will let the user choose a default midi output device
        @param showChannelsAsStereoPairs    if true, channels will be treated as pairs; if false, channels will be
                                        treated as a set of separate mono channels.
        @param hideAdvancedOptionsWithButton    if true, only the minimum amount of UI components
                                        are shown, with an "advanced" button that shows the rest of them
    */
    AudioDeviceSelectorComponent (AudioDeviceManager& deviceManager,
                                  int minAudioInputChannels,
                                  int maxAudioInputChannels,
                                  int minAudioOutputChannels,
                                  int maxAudioOutputChannels,
                                  bool showMidiInputOptions,
                                  bool showMidiOutputSelector,
                                  bool showChannelsAsStereoPairs,
                                  bool hideAdvancedOptionsWithButton,
                                  bool restrictInputToAmbisonicCounts = false,
                                  bool restrictOutputToAmbisonicCounts = false,
                                  bool linkInputOutputChannelCounts = false,
                                  bool showMuteInputToggleButton = false,
                                  Value muteInputValueToControl = {});

    /** Destructor */
    ~AudioDeviceSelectorComponent() override;

    /** The device manager that this component is controlling */
    AudioDeviceManager& deviceManager;

    /** Sets the standard height used for items in the panel. */
    void setItemHeight (int itemHeight);

    /** Returns the standard height used for items in the panel. */
    int getItemHeight() const noexcept      { return itemHeight; }

    /** Returns the ListBox that's being used to show the midi inputs, or nullptr if there isn't one. */
    ListBox* getMidiInputSelectorListBox() const noexcept;

    //==============================================================================
    /** @internal */
    void resized() override;

    /** @internal */
    void childBoundsChanged (Component* child) override;

private:
    //==============================================================================
    void handleBluetoothButton();
    void updateDeviceType();
    void changeListenerCallback (ChangeBroadcaster*) override;
    void updateAllControls();

    std::unique_ptr<ComboBox> deviceTypeDropDown;
    std::unique_ptr<Label> deviceTypeDropDownLabel;
    std::unique_ptr<Component> audioDeviceSettingsComp;
    String audioDeviceSettingsCompType;
    int itemHeight = 0;
    const int minOutputChannels, maxOutputChannels, minInputChannels, maxInputChannels;
    const bool showChannelsAsStereoPairs;
    const bool hideAdvancedOptionsWithButton;
    const bool restrictInputToAmbisonic;
    const bool restrictOutputToAmbisonic;
    const bool linkInputOutputChannels;
    const bool showMuteInputToggle;

    class MidiInputSelectorComponentListBox;
    class MidiOutputSelector;

    Array<MidiDeviceInfo> currentMidiOutputs;
    std::unique_ptr<MidiInputSelectorComponentListBox> midiInputsList;
    std::unique_ptr<MidiOutputSelector> midiOutputSelector;
    std::unique_ptr<Label> midiInputsLabel, midiOutputLabel;
    std::unique_ptr<TextButton> bluetoothButton;

    // Optional "mute audio input" toggle (feedback protection / quick mute while
    // re-patching). Shown for every device type; bound to the host's mute Value.
    std::unique_ptr<ToggleButton> muteInputButton;
    std::unique_ptr<Label> muteInputLabel;
    Value muteInputValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDeviceSelectorComponent)
};


struct SimpleDeviceManagerInputLevelMeter final : public Component,
                                                  public Timer
{
    SimpleDeviceManagerInputLevelMeter (AudioDeviceManager& m)  : manager (m)
    {
        startTimerHz (20);
        inputLevelGetter = manager.getInputLevelGetter();
    }

    void timerCallback() override
    {
        if (isShowing())
        {
            auto newLevel = (float) inputLevelGetter->getCurrentLevel();

            if (std::abs (level - newLevel) > 0.005f)
            {
                level = newLevel;
                repaint();
            }
        }
        else
        {
            level = 0;
        }
    }

    void paint (Graphics& g) override
    {
        // (add a bit of a skew to make the level more obvious)
        getLookAndFeel().drawLevelMeter (g, getWidth(), getHeight(),
                                         (float) std::exp (std::log (level) / 3.0));
    }

    AudioDeviceManager& manager;
    AudioDeviceManager::LevelMeter::Ptr inputLevelGetter;
    float level = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDeviceManagerInputLevelMeter)
};

static void drawTextLayout (Graphics& g, Component& owner, StringRef text, const Rectangle<int>& textBounds, bool enabled)
{
    const auto textColour = owner.findColour (ListBox::textColourId, true).withMultipliedAlpha (enabled ? 1.0f : 0.6f);

    AttributedString attributedString { text };
    attributedString.setColour (textColour);
    attributedString.setFont (owner.withDefaultMetrics (FontOptions { (float) textBounds.getHeight() * 0.6f }));
    attributedString.setJustification (Justification::centredLeft);
    attributedString.setWordWrap (AttributedString::WordWrap::none);

    TextLayout textLayout;
    textLayout.createLayout (attributedString,
                             (float) textBounds.getWidth(),
                             (float) textBounds.getHeight());
    textLayout.draw (g, textBounds.toFloat());
}


//==============================================================================
class AudioDeviceSelectorComponent::MidiInputSelectorComponentListBox final : public ListBox,
                                                                              private ListBoxModel
{
public:
    MidiInputSelectorComponentListBox (AudioDeviceManager& dm, const String& noItems)
        : ListBox ({}, nullptr),
          deviceManager (dm),
          noItemsMessage (noItems)
    {
        updateDevices();
        setModel (this);
        setOutlineThickness (1);
    }

    void updateDevices()
    {
        items = MidiInput::getAvailableDevices();
    }

    int getNumRows() override
    {
        return items.size();
    }

    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (isPositiveAndBelow (row, items.size()))
        {
            if (rowIsSelected)
                g.fillAll (findColour (TextEditor::highlightColourId)
                               .withMultipliedAlpha (0.3f));

            auto item = items[row];
            bool enabled = deviceManager.isMidiInputDeviceEnabled (item.identifier);

            auto x = getTickX();
            auto tickW = (float) height * 0.75f;

            getLookAndFeel().drawTickBox (g, *this, (float) x - tickW, ((float) height - tickW) * 0.5f, tickW, tickW,
                                          enabled, true, true, false);

            drawTextLayout (g, *this, item.name, { x + 5, 0, width - x - 5, height }, enabled);
        }
    }

    void listBoxItemClicked (int row, const MouseEvent& e) override
    {
        selectRow (row);

        if (e.x < getTickX())
            flipEnablement (row);
    }

    void listBoxItemDoubleClicked (int row, const MouseEvent&) override
    {
        flipEnablement (row);
    }

    void returnKeyPressed (int row) override
    {
        flipEnablement (row);
    }

    void paint (Graphics& g) override
    {
        ListBox::paint (g);

        if (items.isEmpty())
        {
            g.setColour (Colours::grey);
            g.setFont (0.5f * (float) getRowHeight());
            g.drawText (noItemsMessage,
                        0, 0, getWidth(), getHeight() / 2,
                        Justification::centred, true);
        }
    }

    int getBestHeight (int preferredHeight)
    {
        auto extra = getOutlineThickness() * 2;

        return jmax (getRowHeight() * 2 + extra,
                     jmin (getRowHeight() * getNumRows() + extra,
                           preferredHeight));
    }

private:
    //==============================================================================
    AudioDeviceManager& deviceManager;
    const String noItemsMessage;
    Array<MidiDeviceInfo> items;

    void flipEnablement (const int row)
    {
        if (isPositiveAndBelow (row, items.size()))
        {
            auto identifier = items[row].identifier;
            deviceManager.setMidiInputDeviceEnabled (identifier, ! deviceManager.isMidiInputDeviceEnabled (identifier));
        }
    }

    int getTickX() const
    {
        return getRowHeight();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputSelectorComponentListBox)
};


//==============================================================================
struct AudioDeviceSetupDetails
{
    AudioDeviceManager* manager;
    int minNumInputChannels, maxNumInputChannels;
    int minNumOutputChannels, maxNumOutputChannels;
    bool useStereoPairs;
    // When set, the channel-count selector for that direction only offers valid
    // ambisonic channel counts (1, 4, 9, 16, 25, ... = (order+1)^2) instead of
    // every integer. Set by the host for buses whose AudioChannelSet is
    // ambisonic; harmless (and false) for channel-based plug-ins.
    bool restrictInputToAmbisonic = false;
    bool restrictOutputToAmbisonic = false;
    // When set, a single "Channels" selector drives input and output to the same
    // count (for plug-ins whose input and output buses are identical).
    bool linkInputOutputChannels = false;
};

static String getNoDeviceString()   { return "<< " + TRANS ("none") + " >>"; }

//==============================================================================
class AudioDeviceSelectorComponent::MidiOutputSelector final : public Component,
                                                               private ChangeListener
{
public:
    explicit MidiOutputSelector (AudioDeviceManager& dm)
        : deviceManager (dm)
    {
        deviceManager.addChangeListener (this);
        selector.onChange = [&]
        {
            const auto selectedId = selector.getSelectedId();
            jassert (selectedId != 0);

            const auto deviceId = selectedId == -1
                                ? String{}
                                : MidiOutput::getAvailableDevices()[selectedId - 1].identifier;
            deviceManager.setDefaultMidiOutputDevice (deviceId);
        };

        updateListOfDevices();
        addAndMakeVisible (selector);
    }

    ~MidiOutputSelector() final
    {
        deviceManager.removeChangeListener (this);
    }

    void resized() final { selector.setBounds (getLocalBounds()); }

private:
    void updateListOfDevices()
    {
        selector.clear();

        const auto midiOutputs = MidiOutput::getAvailableDevices();

        selector.addItem (getNoDeviceString(), -1);
        selector.setSelectedId (-1, dontSendNotification);
        selector.addSeparator();

        for (auto [id, midiOutput] : enumerate (midiOutputs, 1))
        {
            selector.addItem (midiOutput.name, id);

            if (midiOutput.identifier == deviceManager.getDefaultMidiOutputIdentifier())
                selector.setSelectedId (id, dontSendNotification);
        }
    }

    void changeListenerCallback (ChangeBroadcaster*) final { updateListOfDevices(); }

    ComboBox selector;
    AudioDeviceManager& deviceManager;
};

//==============================================================================
class AudioDeviceSettingsPanel : public Component,
                                 private ChangeListener
{
public:
    AudioDeviceSettingsPanel (AudioIODeviceType& t, AudioDeviceSetupDetails& setupDetails,
                              const bool hideAdvancedOptionsWithButton,
                              AudioDeviceSelectorComponent& p)
        : type (t), setup (setupDetails), parent (p)
    {
        if (hideAdvancedOptionsWithButton)
        {
            showAdvancedSettingsButton = std::make_unique <TextButton> (TRANS ("Show advanced settings..."));
            addAndMakeVisible (showAdvancedSettingsButton.get());
            showAdvancedSettingsButton->setClickingTogglesState (true);
            showAdvancedSettingsButton->onClick = [this] { toggleAdvancedSettings(); };
        }

        type.scanForDevices();

        setup.manager->addChangeListener (this);

        updateAllControls();
    }

    ~AudioDeviceSettingsPanel() override
    {
        setup.manager->removeChangeListener (this);
    }

    void resized() override
    {
        Rectangle<int> r (proportionOfWidth (0.35f), 0, proportionOfWidth (0.6f), 3000);

        const int maxListBoxHeight = 100;
        const int h = parent.getItemHeight();
        const int space = h / 4;

        if (outputDeviceDropDown != nullptr)
        {
            auto row = r.removeFromTop (h);

            if (testButton != nullptr)
            {
                testButton->changeWidthToFitText (h);
                testButton->setBounds (row.removeFromRight (testButton->getWidth()));
                row.removeFromRight (space);
            }

            outputDeviceDropDown->setBounds (row);
            r.removeFromTop (space);
        }

        if (inputDeviceDropDown != nullptr)
        {
            auto row = r.removeFromTop (h);

            inputLevelMeter->setBounds (row.removeFromRight (testButton != nullptr ? testButton->getWidth() : row.getWidth() / 6));
            row.removeFromRight (space);
            inputDeviceDropDown->setBounds (row);
            r.removeFromTop (space);
        }

        if (outputChanCountBox != nullptr)
        {
            outputChanCountBox->setBounds (r.removeFromTop (h));
            r.removeFromTop (space);
        }

        if (inputChanCountBox != nullptr)
        {
            inputChanCountBox->setBounds (r.removeFromTop (h));
            r.removeFromTop (space);
        }

        if (outputChanList != nullptr)
        {
            outputChanList->setRowHeight (jmin (22, h));
            outputChanList->setBounds (r.removeFromTop (outputChanList->getBestHeight (maxListBoxHeight)));
            outputChanLabel->setBounds (0, outputChanList->getBounds().getCentreY() - h / 2, r.getX(), h);
            r.removeFromTop (space);
        }

        if (inputChanList != nullptr)
        {
            inputChanList->setRowHeight (jmin (22, h));
            inputChanList->setBounds (r.removeFromTop (inputChanList->getBestHeight (maxListBoxHeight)));
            inputChanLabel->setBounds (0, inputChanList->getBounds().getCentreY() - h / 2, r.getX(), h);
            r.removeFromTop (space);
        }

        r.removeFromTop (space * 2);

        if (showAdvancedSettingsButton != nullptr
            && sampleRateDropDown != nullptr && bufferSizeDropDown != nullptr)
        {
            showAdvancedSettingsButton->setBounds (r.removeFromTop (h));
            r.removeFromTop (space);
            showAdvancedSettingsButton->changeWidthToFitText();
        }

        auto advancedSettingsVisible = showAdvancedSettingsButton == nullptr
                                          || showAdvancedSettingsButton->getToggleState();

        if (sampleRateDropDown != nullptr)
        {
            sampleRateDropDown->setVisible (advancedSettingsVisible);

            if (advancedSettingsVisible)
            {
                sampleRateDropDown->setBounds (r.removeFromTop (h));
                r.removeFromTop (space);
            }
        }

        if (bufferSizeDropDown != nullptr)
        {
            bufferSizeDropDown->setVisible (advancedSettingsVisible);

            if (advancedSettingsVisible)
            {
                bufferSizeDropDown->setBounds (r.removeFromTop (h));
                r.removeFromTop (space);
            }
        }

        r.removeFromTop (space);

        if (showUIButton != nullptr || resetDeviceButton != nullptr)
        {
            auto buttons = r.removeFromTop (h);

            if (showUIButton != nullptr)
            {
                showUIButton->setVisible (advancedSettingsVisible);
                showUIButton->changeWidthToFitText (h);
                showUIButton->setBounds (buttons.removeFromLeft (showUIButton->getWidth()));
                buttons.removeFromLeft (space);
            }

            if (resetDeviceButton != nullptr)
            {
                resetDeviceButton->setVisible (advancedSettingsVisible);
                resetDeviceButton->changeWidthToFitText (h);
                resetDeviceButton->setBounds (buttons.removeFromLeft (resetDeviceButton->getWidth()));
            }

            r.removeFromTop (space);
        }

        setSize (getWidth(), r.getY());
    }

    void updateConfig (bool updateOutputDevice, bool updateInputDevice, bool updateSampleRate, bool updateBufferSize)
    {
        auto config = setup.manager->getAudioDeviceSetup();
        String error;

        if (updateOutputDevice || updateInputDevice)
        {
            if (outputDeviceDropDown != nullptr)
                config.outputDeviceName = outputDeviceDropDown->getSelectedId() < 0 ? String()
                                                                                    : outputDeviceDropDown->getText();

            if (inputDeviceDropDown != nullptr)
                config.inputDeviceName = inputDeviceDropDown->getSelectedId() < 0 ? String()
                                                                                  : inputDeviceDropDown->getText();

            if (! type.hasSeparateInputsAndOutputs())
                config.inputDeviceName = config.outputDeviceName;

            if (updateInputDevice)
                config.useDefaultInputChannels = true;
            else
                config.useDefaultOutputChannels = true;

            error = setup.manager->setAudioDeviceSetup (config, true);

            updateSelectedInput();
            updateSelectedOutput();
            updateControlPanelButton();
            resized();
        }
        else if (updateSampleRate)
        {
            if (sampleRateDropDown->getSelectedId() > 0)
            {
                config.sampleRate = sampleRateDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }
        else if (updateBufferSize)
        {
            if (bufferSizeDropDown->getSelectedId() > 0)
            {
                config.bufferSize = bufferSizeDropDown->getSelectedId();
                error = setup.manager->setAudioDeviceSetup (config, true);
            }
        }

        if (error.isNotEmpty())
            messageBox = AlertWindow::showScopedAsync (MessageBoxOptions().withIconType (MessageBoxIconType::WarningIcon)
                                                                          .withTitle (TRANS ("Error when trying to open audio device!"))
                                                                          .withMessage (error)
                                                                          .withButton (TRANS ("OK")),
                                                       nullptr);
    }

    bool showDeviceControlPanel()
    {
        if (auto* device = setup.manager->getCurrentAudioDevice())
        {
            Component modalWindow;
            modalWindow.setOpaque (true);
            modalWindow.addToDesktop (0);
            modalWindow.enterModalState();

            return device->showControlPanel();
        }

        return false;
    }

    void toggleAdvancedSettings()
    {
        showAdvancedSettingsButton->setButtonText ((showAdvancedSettingsButton->getToggleState() ? "Hide " : "Show ")
                                                   + String ("advanced settings..."));
        resized();
    }

    void showDeviceUIPanel()
    {
        if (showDeviceControlPanel())
        {
            setup.manager->closeAudioDevice();
            setup.manager->restartLastAudioDevice();
            getTopLevelComponent()->toFront (true);
        }
    }

    void playTestSound()
    {
        setup.manager->playTestSound();
    }

    void updateAllControls()
    {
        updateOutputsComboBox();
        updateInputsComboBox();

        updateControlPanelButton();
        updateResetButton();

        if (auto* currentDevice = setup.manager->getCurrentAudioDevice())
        {
            // Linked mode: a single "Channels" selector drives both input and
            // output to the same count (for plug-ins where in == out). The
            // output box is reused as the combined control; the input box is
            // hidden. See channelCountChanged().
            const bool linked = setup.linkInputOutputChannels
                                 && setup.maxNumInputChannels > 0
                                 && setup.maxNumOutputChannels > 0;

            if (setup.maxNumOutputChannels > 0
                 && setup.minNumOutputChannels < currentDevice->getOutputChannelNames().size())
            {
                updateChannelCountBox (false);
            }
            else
            {
                outputChanLabel.reset();
                outputChanList.reset();
                outputChanCountBox.reset();
            }

            if (! linked
                 && setup.maxNumInputChannels > 0
                 && setup.minNumInputChannels < currentDevice->getInputChannelNames().size())
            {
                updateChannelCountBox (true);
            }
            else
            {
                inputChanLabel.reset();
                inputChanList.reset();
                inputChanCountBox.reset();
            }

            updateSampleRateComboBox (currentDevice);
            updateBufferSizeComboBox (currentDevice);
        }
        else
        {
            jassert (setup.manager->getCurrentAudioDevice() == nullptr); // not the correct device type!

            inputChanLabel.reset();
            outputChanLabel.reset();
            sampleRateLabel.reset();
            bufferSizeLabel.reset();

            inputChanList.reset();
            outputChanList.reset();
            inputChanCountBox.reset();
            outputChanCountBox.reset();
            sampleRateDropDown.reset();
            bufferSizeDropDown.reset();

            if (outputDeviceDropDown != nullptr)
                outputDeviceDropDown->setSelectedId (-1, dontSendNotification);

            if (inputDeviceDropDown != nullptr)
                inputDeviceDropDown->setSelectedId (-1, dontSendNotification);
        }

        sendLookAndFeelChange();
        resized();
        setSize (getWidth(), getLowestY() + 4);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        updateAllControls();
    }

    void resetDevice()
    {
        setup.manager->closeAudioDevice();
        setup.manager->restartLastAudioDevice();
    }

private:
    AudioIODeviceType& type;
    const AudioDeviceSetupDetails setup;
    AudioDeviceSelectorComponent& parent;

    std::unique_ptr<ComboBox> outputDeviceDropDown, inputDeviceDropDown, sampleRateDropDown, bufferSizeDropDown;
    std::unique_ptr<ComboBox> inputChanCountBox, outputChanCountBox;
    std::unique_ptr<Label> outputDeviceLabel, inputDeviceLabel, sampleRateLabel, bufferSizeLabel, inputChanLabel, outputChanLabel;
    std::unique_ptr<TextButton> testButton;
    std::unique_ptr<Component> inputLevelMeter;
    std::unique_ptr<TextButton> showUIButton, showAdvancedSettingsButton, resetDeviceButton;

    int findSelectedDeviceIndex (bool isInput) const
    {
        const auto device = setup.manager->getAudioDeviceSetup();
        const auto deviceName = isInput ? device.inputDeviceName : device.outputDeviceName;
        return type.getDeviceNames (isInput).indexOf (deviceName);
    }

    void updateSelectedInput()
    {
        showCorrectDeviceName (inputDeviceDropDown.get(), true);
    }

    void updateSelectedOutput()
    {
        constexpr auto isInput = false;
        showCorrectDeviceName (outputDeviceDropDown.get(), isInput);

        if (testButton != nullptr)
            testButton->setEnabled (findSelectedDeviceIndex (isInput) >= 0);
    }

    void showCorrectDeviceName (ComboBox* box, bool isInput)
    {
        if (box == nullptr)
            return;

        const auto index = findSelectedDeviceIndex (isInput);
        box->setSelectedId (index < 0 ? index : index + 1, dontSendNotification);
    }

    void addNamesToDeviceBox (ComboBox& combo, bool isInputs)
    {
        const StringArray devs (type.getDeviceNames (isInputs));

        combo.clear (dontSendNotification);

        for (int i = 0; i < devs.size(); ++i)
            combo.addItem (devs[i], i + 1);

        combo.addItem (getNoDeviceString(), -1);
        combo.setSelectedId (-1, dontSendNotification);
    }

    int getLowestY() const
    {
        int y = 0;

        for (auto* c : getChildren())
            y = jmax (y, c->getBottom());

        return y;
    }

    void updateControlPanelButton()
    {
        auto* currentDevice = setup.manager->getCurrentAudioDevice();
        showUIButton.reset();

        if (currentDevice != nullptr && currentDevice->hasControlPanel())
        {
            showUIButton = std::make_unique<TextButton> (TRANS ("Control Panel"),
                                                         TRANS ("Opens the device's own control panel"));
            addAndMakeVisible (showUIButton.get());
            showUIButton->onClick = [this] { showDeviceUIPanel(); };
        }

        resized();
    }

    void updateResetButton()
    {
        if (auto* currentDevice = setup.manager->getCurrentAudioDevice())
        {
            if (currentDevice->hasControlPanel())
            {
                if (resetDeviceButton == nullptr)
                {
                    resetDeviceButton = std::make_unique<TextButton> (TRANS ("Reset Device"),
                                                                      TRANS ("Resets the audio interface - sometimes needed after changing a device's properties in its custom control panel"));
                    addAndMakeVisible (resetDeviceButton.get());
                    resetDeviceButton->onClick = [this] { resetDevice(); };
                    resized();
                }

                return;
            }
        }

        resetDeviceButton.reset();
    }

    void updateOutputsComboBox()
    {
        // For JACK the app exposes its own client ports and never auto-connects,
        // so picking an external output "device" is meaningless - hide it (and
        // the Test button); only the channel-count selector matters.
        if (type.getTypeName() == "JACK")
        {
            outputDeviceDropDown.reset();
            outputDeviceLabel.reset();
            testButton.reset();
            return;
        }

        if (setup.maxNumOutputChannels > 0 || ! type.hasSeparateInputsAndOutputs())
        {
            if (outputDeviceDropDown == nullptr)
            {
                outputDeviceDropDown = std::make_unique<ComboBox>();
                outputDeviceDropDown->onChange = [this] { updateConfig (true, false, false, false); };

                addAndMakeVisible (outputDeviceDropDown.get());

                outputDeviceLabel = std::make_unique<Label> (String{}, type.hasSeparateInputsAndOutputs() ? TRANS ("Output:")
                                                                                                          : TRANS ("Device:"));
                outputDeviceLabel->attachToComponent (outputDeviceDropDown.get(), true);

                if (setup.maxNumOutputChannels > 0)
                {
                    testButton = std::make_unique<TextButton> (TRANS ("Test"), TRANS ("Plays a test tone"));
                    addAndMakeVisible (testButton.get());
                    testButton->onClick = [this] { playTestSound(); };
                }
            }

            addNamesToDeviceBox (*outputDeviceDropDown, false);
        }

        updateSelectedOutput();
    }

    void updateInputsComboBox()
    {
        // See updateOutputsComboBox: no external input "device" picker for JACK.
        if (type.getTypeName() == "JACK")
        {
            inputDeviceDropDown.reset();
            inputDeviceLabel.reset();
            inputLevelMeter.reset();
            return;
        }

        if (setup.maxNumInputChannels > 0 && type.hasSeparateInputsAndOutputs())
        {
            if (inputDeviceDropDown == nullptr)
            {
                inputDeviceDropDown = std::make_unique<ComboBox>();
                inputDeviceDropDown->onChange = [this] { updateConfig (false, true, false, false); };
                addAndMakeVisible (inputDeviceDropDown.get());

                inputDeviceLabel = std::make_unique<Label> (String{}, TRANS ("Input:"));
                inputDeviceLabel->attachToComponent (inputDeviceDropDown.get(), true);

                inputLevelMeter = std::make_unique<SimpleDeviceManagerInputLevelMeter> (*setup.manager);
                addAndMakeVisible (inputLevelMeter.get());
            }

            addNamesToDeviceBox (*inputDeviceDropDown, true);
        }

        updateSelectedInput();
    }

    //==============================================================================
    // mcfx: multichannel mcfx plug-ins want a fixed number of consecutive
    // channels, so the settings dialog offers a plain "how many channels"
    // selector instead of the per-channel (stereo-pair) on/off list.
    int maxChannelsForDevice (bool isInput) const
    {
        auto* dev = setup.manager->getCurrentAudioDevice();
        const int pluginMax = isInput ? setup.maxNumInputChannels : setup.maxNumOutputChannels;

        if (dev == nullptr)
            return pluginMax;

        // JACK ports are virtual: we can register as many as the plug-in
        // supports, independent of how many are currently open. Hardware drivers
        // are limited to the channels the device actually exposes.
        if (type.getTypeName() == "JACK")
            return pluginMax;

        const int deviceChannels = (isInput ? dev->getInputChannelNames()
                                            : dev->getOutputChannelNames()).size();
        return jmin (pluginMax, deviceChannels);
    }

    void updateChannelCountBox (bool isInput)
    {
        auto& box   = isInput ? inputChanCountBox : outputChanCountBox;
        auto& label = isInput ? inputChanLabel    : outputChanLabel;

        // the count selector replaces the channel list
        (isInput ? inputChanList : outputChanList).reset();

        if (box == nullptr)
        {
            box = std::make_unique<ComboBox>();
            addAndMakeVisible (box.get());
            // Editable so the user can just type a count instead of scrolling a
            // long list (up to 128). Typed values are clamped, and snapped to the
            // nearest valid ambisonic count for ambisonic buses (see
            // channelCountChanged / resolveChannelCount).
            box->setEditableText (true);
            box->onChange = [this, isInput] { channelCountChanged (isInput); };

            const String labelText = (! isInput && setup.linkInputOutputChannels)
                                         ? TRANS ("Channels:")
                                         : (isInput ? TRANS ("Input channels:")
                                                    : TRANS ("Output channels:"));
            label = std::make_unique<Label> (String{}, labelText);
            label->setJustificationType (Justification::centredRight);
            label->attachToComponent (box.get(), true);
        }

        const int maxCh = maxChannelsForDevice (isInput);
        // A plug-in can pin a direction (e.g. binaural's stereo output) by setting
        // min == max; the box then offers that single, unchangeable count.
        const int minCh = jlimit (1, jmax (1, maxCh),
                                  isInput ? setup.minNumInputChannels : setup.minNumOutputChannels);
        const bool ambisonicOnly = isInput ? setup.restrictInputToAmbisonic
                                           : setup.restrictOutputToAmbisonic;

        box->clear (dontSendNotification);
        if (ambisonicOnly)
        {
            // Only valid ambisonic channel counts from 1st order upward:
            // 4, 9, 16, 25, ... = (order+1)^2. 0th order (1 channel) is omitted.
            for (int order = 1; (order + 1) * (order + 1) <= maxCh; ++order)
            {
                const int n = (order + 1) * (order + 1);
                if (n >= minCh)
                    box->addItem (String (n), n);
            }
        }
        else
        {
            for (int i = minCh; i <= maxCh; ++i)
                box->addItem (String (i), i);
        }

        auto* dev = setup.manager->getCurrentAudioDevice();
        const int current = dev != nullptr ? (isInput ? dev->getActiveInputChannels()
                                                       : dev->getActiveOutputChannels()).countNumberOfSetBits()
                                           : 0;
        // Select the current count, snapping down to an available item when the
        // active count is not itself an offered (e.g. ambisonic) value.
        int selId = jlimit (1, jmax (1, maxCh), current);
        if (box->getNumItems() > 0 && box->indexOfItemId (selId) < 0)
        {
            int best = box->getItemId (0);
            for (int i = 0; i < box->getNumItems(); ++i)
            {
                const int id = box->getItemId (i);
                if (id <= selId)
                    best = id;
            }
            selId = best;
        }
        box->setSelectedId (selId, dontSendNotification);
    }

    // Resolve a typed/selected channel count to an allowed value: clamp to
    // [minCh, maxCh], and for an ambisonic bus snap to the nearest valid count
    // (1st order and up: 4, 9, 16, 25, ...).
    int resolveChannelCount (int n, int minCh, int maxCh, bool ambisonicOnly) const
    {
        const int lo = jlimit (1, jmax (1, maxCh), minCh);
        n = jlimit (lo, jmax (lo, maxCh), n);

        if (ambisonicOnly)
        {
            int best = jmax (lo, jmin (4, maxCh)), bestDist = maxCh + 1;
            for (int order = 1; (order + 1) * (order + 1) <= maxCh; ++order)
            {
                const int sq = (order + 1) * (order + 1);
                if (sq < lo) continue;
                const int d = sq > n ? sq - n : n - sq;
                if (d < bestDist) { bestDist = d; best = sq; }
            }
            n = best;
        }
        return n;
    }

    void channelCountChanged (bool isInput)
    {
        auto& box = (isInput ? inputChanCountBox : outputChanCountBox);
        if (box == nullptr)
            return;

        // Editable combo: the requested count is whatever the user typed/selected.
        const int requested = box->getText().getIntValue();
        if (requested <= 0)
            return;

        const int maxCh = maxChannelsForDevice (isInput);
        const int minCh = isInput ? setup.minNumInputChannels : setup.minNumOutputChannels;
        const bool ambisonicOnly = isInput ? setup.restrictInputToAmbisonic
                                           : setup.restrictOutputToAmbisonic;
        const int n = resolveChannelCount (requested, minCh, maxCh, ambisonicOnly);

        // Reflect the clamped / snapped value back in the box.
        box->setSelectedId (n, dontSendNotification);

        auto config = setup.manager->getAudioDeviceSetup();

        BigInteger bits;
        bits.setRange (0, n, true);

        if (setup.linkInputOutputChannels)
        {
            // Single "Channels" selector (the output box) drives both directions.
            config.inputChannels = bits;
            config.useDefaultInputChannels = false;
            config.outputChannels = bits;
            config.useDefaultOutputChannels = false;
        }
        else if (isInput)
        {
            config.inputChannels = bits;
            config.useDefaultInputChannels = false;
        }
        else
        {
            config.outputChannels = bits;
            config.useDefaultOutputChannels = false;
        }

        auto error = setup.manager->setAudioDeviceSetup (config, true);

        if (error.isNotEmpty())
            messageBox = AlertWindow::showScopedAsync (MessageBoxOptions().withIconType (MessageBoxIconType::WarningIcon)
                                                                          .withTitle (TRANS ("Error when trying to open audio device!"))
                                                                          .withMessage (error)
                                                                          .withButton (TRANS ("OK")),
                                                       nullptr);
    }

    void updateSampleRateComboBox (AudioIODevice* currentDevice)
    {
        if (sampleRateDropDown == nullptr)
        {
            sampleRateDropDown = std::make_unique<ComboBox>();
            addAndMakeVisible (sampleRateDropDown.get());

            sampleRateLabel = std::make_unique<Label> (String{}, TRANS ("Sample rate:"));
            sampleRateLabel->attachToComponent (sampleRateDropDown.get(), true);
        }
        else
        {
            sampleRateDropDown->clear();
            sampleRateDropDown->onChange = nullptr;
        }

        const auto getFrequencyString = [] (int rate) { return String (rate) + " Hz"; };

        for (auto rate : currentDevice->getAvailableSampleRates())
        {
            const auto intRate = roundToInt (rate);
            sampleRateDropDown->addItem (getFrequencyString (intRate), intRate);
        }

        const auto intRate = roundToInt (currentDevice->getCurrentSampleRate());
        sampleRateDropDown->setText (getFrequencyString (intRate), dontSendNotification);

        sampleRateDropDown->onChange = [this] { updateConfig (false, false, true, false); };
    }

    void updateBufferSizeComboBox (AudioIODevice* currentDevice)
    {
        if (bufferSizeDropDown == nullptr)
        {
            bufferSizeDropDown = std::make_unique<ComboBox>();
            addAndMakeVisible (bufferSizeDropDown.get());

            bufferSizeLabel = std::make_unique<Label> (String{}, TRANS ("Audio buffer size:"));
            bufferSizeLabel->attachToComponent (bufferSizeDropDown.get(), true);
        }
        else
        {
            bufferSizeDropDown->clear();
            bufferSizeDropDown->onChange = nullptr;
        }

        auto currentRate = currentDevice->getCurrentSampleRate();

        if (exactlyEqual (currentRate, 0.0))
            currentRate = 48000.0;

        for (auto bs : currentDevice->getAvailableBufferSizes())
            bufferSizeDropDown->addItem (getBufferSizeText (bs, currentRate), bs);

        const auto bufferSizeSamples = currentDevice->getCurrentBufferSizeSamples();
        bufferSizeDropDown->setText (getBufferSizeText (bufferSizeSamples, currentRate), dontSendNotification);

        bufferSizeDropDown->onChange = [this] { updateConfig (false, false, false, true); };
    }

    String getBufferSizeText (int bs, double currentRate) const
    {
        return String (bs) + " samples (" + String (bs * 1000.0 / currentRate, 1) + " ms)";
    }

public:
    //==============================================================================
    class ChannelSelectorListBox final : public ListBox,
                                         private ListBoxModel
    {
    public:
        enum BoxType
        {
            audioInputType,
            audioOutputType
        };

        //==============================================================================
        ChannelSelectorListBox (const AudioDeviceSetupDetails& setupDetails, BoxType boxType, const String& noItemsText)
           : ListBox ({}, nullptr), setup (setupDetails), type (boxType), noItemsMessage (noItemsText)
        {
            refresh();
            setModel (this);
            setOutlineThickness (1);
        }

        void refresh()
        {
            items.clear();

            if (auto* currentDevice = setup.manager->getCurrentAudioDevice())
            {
                if (type == audioInputType)
                    items = currentDevice->getInputChannelNames();
                else if (type == audioOutputType)
                    items = currentDevice->getOutputChannelNames();

                if (setup.useStereoPairs)
                {
                    StringArray pairs;

                    for (int i = 0; i < items.size(); i += 2)
                    {
                        auto& name = items[i];

                        if (i + 1 >= items.size())
                            pairs.add (name.trim());
                        else
                            pairs.add (getNameForChannelPair (name, items[i + 1]));
                    }

                    items = pairs;
                }
            }

            updateContent();
            repaint();
        }

        int getNumRows() override
        {
            return items.size();
        }

        void paintListBoxItem (int row, Graphics& g, int width, int height, bool) override
        {
            if (isPositiveAndBelow (row, items.size()))
            {
                g.fillAll (findColour (ListBox::backgroundColourId));

                auto item = items[row];
                bool enabled = false;
                auto config = setup.manager->getAudioDeviceSetup();

                if (setup.useStereoPairs)
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels[row * 2] || config.inputChannels[row * 2 + 1];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels[row * 2] || config.outputChannels[row * 2 + 1];
                }
                else
                {
                    if (type == audioInputType)
                        enabled = config.inputChannels[row];
                    else if (type == audioOutputType)
                        enabled = config.outputChannels[row];
                }

                auto x = getTickX();
                auto tickW = (float) height * 0.75f;

                getLookAndFeel().drawTickBox (g, *this, (float) x - tickW, ((float) height - tickW) * 0.5f, tickW, tickW,
                                              enabled, true, true, false);

                drawTextLayout (g, *this, item, { x + 5, 0, width - x - 5, height }, enabled);
            }
        }

        void listBoxItemClicked (int row, const MouseEvent& e) override
        {
            selectRow (row);

            if (e.x < getTickX())
                flipEnablement (row);
        }

        void listBoxItemDoubleClicked (int row, const MouseEvent&) override
        {
            flipEnablement (row);
        }

        void returnKeyPressed (int row) override
        {
            flipEnablement (row);
        }

        void paint (Graphics& g) override
        {
            ListBox::paint (g);

            if (items.isEmpty())
            {
                g.setColour (Colours::grey);
                g.setFont (0.5f * (float) getRowHeight());
                g.drawText (noItemsMessage,
                            0, 0, getWidth(), getHeight() / 2,
                            Justification::centred, true);
            }
        }

        int getBestHeight (int maxHeight)
        {
            return getRowHeight() * jlimit (2, jmax (2, maxHeight / getRowHeight()),
                                            getNumRows())
                       + getOutlineThickness() * 2;
        }

    private:
        //==============================================================================
        const AudioDeviceSetupDetails setup;
        const BoxType type;
        const String noItemsMessage;
        StringArray items;

        static String getNameForChannelPair (const String& name1, const String& name2)
        {
            String commonBit;

            for (int j = 0; j < name1.length(); ++j)
                if (name1.substring (0, j).equalsIgnoreCase (name2.substring (0, j)))
                    commonBit = name1.substring (0, j);

            // Make sure we only split the name at a space, because otherwise, things
            // like "input 11" + "input 12" would become "input 11 + 2"
            while (commonBit.isNotEmpty() && ! CharacterFunctions::isWhitespace (commonBit.getLastCharacter()))
                commonBit = commonBit.dropLastCharacters (1);

            return name1.trim() + " + " + name2.substring (commonBit.length()).trim();
        }

        void flipEnablement (int row)
        {
            jassert (type == audioInputType || type == audioOutputType);

            if (isPositiveAndBelow (row, items.size()))
            {
                auto config = setup.manager->getAudioDeviceSetup();

                if (setup.useStereoPairs)
                {
                    BigInteger bits;
                    auto& original = (type == audioInputType ? config.inputChannels
                                                             : config.outputChannels);

                    for (int i = 0; i < 256; i += 2)
                        bits.setBit (i / 2, original[i] || original[i + 1]);

                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (bits, row, setup.minNumInputChannels / 2, setup.maxNumInputChannels / 2);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (bits, row, setup.minNumOutputChannels / 2, setup.maxNumOutputChannels / 2);
                    }

                    for (int i = 0; i < 256; ++i)
                        original.setBit (i, bits[i / 2]);
                }
                else
                {
                    if (type == audioInputType)
                    {
                        config.useDefaultInputChannels = false;
                        flipBit (config.inputChannels, row, setup.minNumInputChannels, setup.maxNumInputChannels);
                    }
                    else
                    {
                        config.useDefaultOutputChannels = false;
                        flipBit (config.outputChannels, row, setup.minNumOutputChannels, setup.maxNumOutputChannels);
                    }
                }

                setup.manager->setAudioDeviceSetup (config, true);
            }
        }

        static void flipBit (BigInteger& chans, int index, int minNumber, int maxNumber)
        {
            auto numActive = chans.countNumberOfSetBits();

            if (chans[index])
            {
                if (numActive > minNumber)
                    chans.setBit (index, false);
            }
            else
            {
                if (numActive >= maxNumber)
                {
                    auto firstActiveChan = chans.findNextSetBit (0);
                    chans.clearBit (index > firstActiveChan ? firstActiveChan : chans.getHighestBit());
                }

                chans.setBit (index, true);
            }
        }

        int getTickX() const
        {
            return getRowHeight();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelSelectorListBox)
    };

private:
    std::unique_ptr<ChannelSelectorListBox> inputChanList, outputChanList;
    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDeviceSettingsPanel)
};


//==============================================================================
AudioDeviceSelectorComponent::AudioDeviceSelectorComponent (AudioDeviceManager& dm,
                                                            int minInputChannelsToUse,
                                                            int maxInputChannelsToUse,
                                                            int minOutputChannelsToUse,
                                                            int maxOutputChannelsToUse,
                                                            bool showMidiInputOptions,
                                                            bool showMidiOutputSelector,
                                                            bool showChannelsAsStereoPairsToUse,
                                                            bool hideAdvancedOptionsWithButtonToUse,
                                                            bool restrictInputToAmbisonicCounts,
                                                            bool restrictOutputToAmbisonicCounts,
                                                            bool linkInputOutputChannelCounts,
                                                            bool showMuteInputToggleButton,
                                                            Value muteInputValueToControl)
    : deviceManager (dm),
      itemHeight (24),
      minOutputChannels (minOutputChannelsToUse),
      maxOutputChannels (maxOutputChannelsToUse),
      minInputChannels (minInputChannelsToUse),
      maxInputChannels (maxInputChannelsToUse),
      showChannelsAsStereoPairs (showChannelsAsStereoPairsToUse),
      hideAdvancedOptionsWithButton (hideAdvancedOptionsWithButtonToUse),
      restrictInputToAmbisonic (restrictInputToAmbisonicCounts),
      restrictOutputToAmbisonic (restrictOutputToAmbisonicCounts),
      linkInputOutputChannels (linkInputOutputChannelCounts),
      showMuteInputToggle (showMuteInputToggleButton),
      muteInputValue (muteInputValueToControl)
{
    jassert (minOutputChannels >= 0 && minOutputChannels <= maxOutputChannels);
    jassert (minInputChannels >= 0 && minInputChannels <= maxInputChannels);

    if (showMuteInputToggle)
    {
        muteInputButton = std::make_unique<ToggleButton> (TRANS ("Mute audio input"));
        muteInputButton->getToggleStateValue().referTo (muteInputValue);
        addAndMakeVisible (muteInputButton.get());

        muteInputLabel = std::make_unique<Label> (String{}, TRANS ("Feedback Loop:"));
        muteInputLabel->setJustificationType (Justification::centredRight);
        muteInputLabel->attachToComponent (muteInputButton.get(), true);
    }

    const OwnedArray<AudioIODeviceType>& types = deviceManager.getAvailableDeviceTypes();

    if (types.size() > 1)
    {
        deviceTypeDropDown = std::make_unique<ComboBox>();

        for (int i = 0; i < types.size(); ++i)
            deviceTypeDropDown->addItem (types.getUnchecked (i)->getTypeName(), i + 1);

        addAndMakeVisible (deviceTypeDropDown.get());
        deviceTypeDropDown->onChange = [this] { updateDeviceType(); };

        deviceTypeDropDownLabel = std::make_unique<Label> (String{}, TRANS ("Audio device type:"));
        deviceTypeDropDownLabel->setJustificationType (Justification::centredRight);
        deviceTypeDropDownLabel->attachToComponent (deviceTypeDropDown.get(), true);
    }

    if (showMidiInputOptions)
    {
        midiInputsList = std::make_unique <MidiInputSelectorComponentListBox> (deviceManager,
                                                                               "(" + TRANS ("No MIDI inputs available") + ")");
        addAndMakeVisible (midiInputsList.get());

        midiInputsLabel = std::make_unique<Label> (String{}, TRANS ("Active MIDI inputs:"));
        midiInputsLabel->setJustificationType (Justification::topRight);
        midiInputsLabel->attachToComponent (midiInputsList.get(), true);

        if (BluetoothMidiDevicePairingDialogue::isAvailable())
        {
            bluetoothButton = std::make_unique<TextButton> (TRANS ("Bluetooth MIDI"), TRANS ("Scan for bluetooth MIDI devices"));
            addAndMakeVisible (bluetoothButton.get());
            bluetoothButton->onClick = [this] { handleBluetoothButton(); };
        }
    }
    else
    {
        midiInputsList.reset();
        midiInputsLabel.reset();
        bluetoothButton.reset();
    }

    if (showMidiOutputSelector)
    {
        midiOutputSelector = std::make_unique<MidiOutputSelector> (deviceManager);
        addAndMakeVisible (midiOutputSelector.get());

        midiOutputLabel = std::make_unique<Label> ("lm", TRANS ("MIDI Output:"));
        midiOutputLabel->attachToComponent (midiOutputSelector.get(), true);
    }
    else
    {
        midiOutputSelector.reset();
        midiOutputLabel.reset();
    }

    deviceManager.addChangeListener (this);
    updateAllControls();
}

AudioDeviceSelectorComponent::~AudioDeviceSelectorComponent()
{
    deviceManager.removeChangeListener (this);
}

void AudioDeviceSelectorComponent::setItemHeight (int newItemHeight)
{
    itemHeight = newItemHeight;
    resized();
}

void AudioDeviceSelectorComponent::resized()
{
    Rectangle<int> r (proportionOfWidth (0.35f), 15, proportionOfWidth (0.6f), 3000);
    auto space = itemHeight / 4;

    if (deviceTypeDropDown != nullptr)
    {
        deviceTypeDropDown->setBounds (r.removeFromTop (itemHeight));
        r.removeFromTop (space * 3);
    }

    if (audioDeviceSettingsComp != nullptr)
    {
        audioDeviceSettingsComp->resized();
        audioDeviceSettingsComp->setBounds (r.removeFromTop (audioDeviceSettingsComp->getHeight())
                                                .withX (0).withWidth (getWidth()));
        r.removeFromTop (space);
    }

    if (midiInputsList != nullptr)
    {
        midiInputsList->setRowHeight (jmin (22, itemHeight));
        midiInputsList->setBounds (r.removeFromTop (midiInputsList->getBestHeight (jmin (itemHeight * 8,
                                                                                         getHeight() - r.getY() - space - itemHeight))));
        r.removeFromTop (space);
    }

    if (bluetoothButton != nullptr)
    {
        bluetoothButton->setBounds (r.removeFromTop (24));
        r.removeFromTop (space);
    }

    if (midiOutputSelector != nullptr)
    {
        midiOutputSelector->setBounds (r.removeFromTop (itemHeight));
        r.removeFromTop (space);
    }

    if (muteInputButton != nullptr)
    {
        muteInputButton->setBounds (r.removeFromTop (itemHeight));
        r.removeFromTop (space);
    }

    r.removeFromTop (itemHeight);
    setSize (getWidth(), r.getY());
}

void AudioDeviceSelectorComponent::childBoundsChanged (Component* child)
{
    if (child == audioDeviceSettingsComp.get())
        resized();
}

void AudioDeviceSelectorComponent::updateDeviceType()
{
    if (auto* type = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown->getSelectedId() - 1])
    {
        audioDeviceSettingsComp.reset();
        deviceManager.setCurrentAudioDeviceType (type->getTypeName(), true);
        updateAllControls(); // needed in case the type hasn't actually changed
    }
}

void AudioDeviceSelectorComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateAllControls();
}

void AudioDeviceSelectorComponent::updateAllControls()
{
    if (deviceTypeDropDown != nullptr)
        deviceTypeDropDown->setText (deviceManager.getCurrentAudioDeviceType(), dontSendNotification);

    if (audioDeviceSettingsComp == nullptr
         || audioDeviceSettingsCompType != deviceManager.getCurrentAudioDeviceType())
    {
        audioDeviceSettingsCompType = deviceManager.getCurrentAudioDeviceType();
        audioDeviceSettingsComp.reset();

        if (auto* type = deviceManager.getAvailableDeviceTypes() [deviceTypeDropDown == nullptr
                                                                   ? 0 : deviceTypeDropDown->getSelectedId() - 1])
        {
            AudioDeviceSetupDetails details;
            details.manager = &deviceManager;
            details.minNumInputChannels = minInputChannels;
            details.maxNumInputChannels = maxInputChannels;
            details.minNumOutputChannels = minOutputChannels;
            details.maxNumOutputChannels = maxOutputChannels;
            details.useStereoPairs = showChannelsAsStereoPairs;
            details.restrictInputToAmbisonic  = restrictInputToAmbisonic;
            details.restrictOutputToAmbisonic = restrictOutputToAmbisonic;
            details.linkInputOutputChannels   = linkInputOutputChannels;

            audioDeviceSettingsComp = std::make_unique<AudioDeviceSettingsPanel> (*type, details, hideAdvancedOptionsWithButton, *this);
            addAndMakeVisible (audioDeviceSettingsComp.get());
        }
    }

    if (midiInputsList != nullptr)
    {
        midiInputsList->updateDevices();
        midiInputsList->updateContent();
        midiInputsList->repaint();
    }

    resized();
}

void AudioDeviceSelectorComponent::handleBluetoothButton()
{
    if (RuntimePermissions::isGranted (RuntimePermissions::bluetoothMidi))
    {
        BluetoothMidiDevicePairingDialogue::open();
    }
    else
    {
        RuntimePermissions::request (RuntimePermissions::bluetoothMidi, [] (auto)
        {
            if (RuntimePermissions::isGranted (RuntimePermissions::bluetoothMidi))
                BluetoothMidiDevicePairingDialogue::open();
        });
    }
}

ListBox* AudioDeviceSelectorComponent::getMidiInputSelectorListBox() const noexcept
{
    return midiInputsList.get();
}
} // namespace jsa
