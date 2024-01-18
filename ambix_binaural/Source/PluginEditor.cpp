/*
  ==============================================================================

   This file is part of the ambix Ambisonic plug-in suite.
   Copyright (c) 2013/2014 - Matthias Kronlachner
   www.matthiaskronlachner.com

   Permission is granted to use this software under the terms of:
   the GPL v2 (or any later version)

   Details of these licenses can be found at: www.gnu.org/licenses

   ambix is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ==============================================================================
*/

#include "PluginEditor.h"


#define Q(x) #x
#define QUOTE(x) Q(x)

//==============================================================================
Ambix_binauralAudioProcessorEditor::Ambix_binauralAudioProcessorEditor (Ambix_binauralAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
    _width(0)
{
    setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears (700); // tooltip delay

    addAndMakeVisible (label);
    label.setText("Ambisonics input channels: ", dontSendNotification);
    label.setFont (Font (15.0000f, Font::plain));
    label.setJustificationType (Justification::centredRight);
    label.setEditable (false, false, false);
    label.setColour (Label::textColourId, Colours::white);
    label.setColour (TextEditor::textColourId, Colours::black);
    label.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (txt_preset);

    txt_preset.setReadOnly(true);
    txt_preset.setPopupMenuEnabled(false);

    addAndMakeVisible (label5);
    label5.setText("Preset", dontSendNotification);
    label5.setFont (Font (15.0000f, Font::plain));
    label5.setJustificationType (Justification::centredRight);
    label5.setEditable (false, false, false);
    label5.setColour (Label::textColourId, Colours::white);
    label5.setColour (TextEditor::textColourId, Colours::white);
    label5.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (txt_debug);
    txt_debug.setMultiLine (true);
    txt_debug.setReturnKeyStartsNewLine (false);
    txt_debug.setReadOnly (true);
    txt_debug.setScrollbarsShown (true);
    txt_debug.setCaretVisible (false);
    txt_debug.setPopupMenuEnabled (true);
    txt_debug.setText ("debug window");
    txt_debug.setFont(Font(10,1));

    addAndMakeVisible (btn_open);
    btn_open.setTooltip ("browse presets or open from file");
    btn_open.setButtonText ("open");
    btn_open.addListener (this);
    btn_open.setColour (TextButton::buttonColourId, Colours::white);
    btn_open.setColour (TextButton::buttonOnColourId, Colours::blue);

    addAndMakeVisible (label2);
    label2.setText("Virtual loudspeakers: ", dontSendNotification);
    label2.setFont (Font (15.0000f, Font::plain));
    label2.setJustificationType (Justification::centredRight);
    label2.setEditable (false, false, false);
    label2.setColour (Label::textColourId, Colours::white);
    label2.setColour (TextEditor::textColourId, Colours::black);
    label2.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label3);
    label3.setText("Impulse responses: ", dontSendNotification);
    label3.setFont (Font (15.0000f, Font::plain));
    label3.setJustificationType (Justification::centredRight);
    label3.setEditable (false, false, false);
    label3.setColour (Label::textColourId, Colours::white);
    label3.setColour (TextEditor::textColourId, Colours::black);
    label3.setColour (TextEditor::backgroundColourId, Colour (0x0));

#if BINAURAL_DECODER
#else
    label2.setText("Loudspeakers: ", dontSendNotification);
    label3.setText("", dontSendNotification);
#endif

    addAndMakeVisible (label4);
    label4.setText("debug window", dontSendNotification);
    label4.setFont (Font (10.0000f, Font::plain));
    label4.setJustificationType (Justification::centredLeft);
    label4.setEditable (false, false, false);
    label4.setColour (Label::textColourId, Colours::white);
    label4.setColour (TextEditor::textColourId, Colours::black);
    label4.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (num_ch);
    num_ch.setText("0", dontSendNotification);
    num_ch.setFont (Font (15.0000f, Font::plain));
    num_ch.setJustificationType (Justification::centredRight);
    num_ch.setEditable (false, false, false);
    num_ch.setColour (Label::textColourId, Colours::white);
    num_ch.setColour (TextEditor::textColourId, Colours::black);
    num_ch.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (num_spk);
    num_spk.setText("0", dontSendNotification);
    num_spk.setFont (Font (15.0000f, Font::plain));
    num_spk.setJustificationType (Justification::centredRight);
    num_spk.setEditable (false, false, false);
    num_spk.setColour (Label::textColourId, Colours::white);
    num_spk.setColour (TextEditor::textColourId, Colours::black);
    num_spk.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (num_hrtf);
    num_hrtf.setText("", dontSendNotification);
    num_hrtf.setFont (Font (15.0000f, Font::plain));
    num_hrtf.setJustificationType (Justification::centredRight);
    num_hrtf.setEditable (false, false, false);
    num_hrtf.setColour (Label::textColourId, Colours::white);
    num_hrtf.setColour (TextEditor::textColourId, Colours::black);
    num_hrtf.setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (btn_preset_folder);
    btn_preset_folder.setTooltip ("choose another preset folder");
    btn_preset_folder.setButtonText ("preset folder");
    btn_preset_folder.addListener (this);
    btn_preset_folder.setColour (TextButton::buttonColourId, Colours::white);
    btn_preset_folder.setColour (TextButton::buttonOnColourId, Colours::blue);

    addAndMakeVisible (sld_gain);
    sld_gain.setTooltip (TRANS("Output Volume"));
    sld_gain.setRange (-99, 20, 0.1);
    sld_gain.setSliderStyle (Slider::LinearVertical);
    sld_gain.setTextBoxStyle (Slider::TextBoxBelow, false, 45, 20);
    sld_gain.setColour (Slider::thumbColourId, Colours::white);
    sld_gain.addListener (this);
    sld_gain.setSkewFactor (1.6f);
    sld_gain.setDoubleClickReturnValue(true, 0.f);

    addAndMakeVisible(tgl_save_preset);
    tgl_save_preset.setButtonText(TRANS("Save preset within project"));
    tgl_save_preset.setTooltip(TRANS("this will save the preset and data within the project and reload it from there; CAUTION: may slow down saving/opening your project and increases project file size!"));
    tgl_save_preset.addListener(this);
    tgl_save_preset.setToggleState(true, dontSendNotification);
    tgl_save_preset.setColour(ToggleButton::textColourId, Colours::white);

#if BINAURAL_DECODER
    addAndMakeVisible (tgl_load_irs);
    tgl_load_irs.setTooltip (TRANS("load impulse responses if new preset being loaded - deactivate if IRs already loaded - for fast decoder matrix switching"));
    tgl_load_irs.setButtonText (TRANS("load IRs"));
    tgl_load_irs.addListener (this);
    tgl_load_irs.setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible(box_conv_buffer);
    box_conv_buffer.setTooltip("set higher buffer size to optimize CPU performance but increased latency");
    box_conv_buffer.addListener(this);
    box_conv_buffer.setEditableText (false);
    box_conv_buffer.setJustificationType (Justification::centredLeft);
#endif

    setSize (350, 325);

    DrawMeters();

    UpdateText();

    UpdatePresets();

    txt_preset.setText(ownerFilter->box_preset_str);
    txt_preset.setCaretPosition(txt_preset.getTotalNumChars()-1);
    txt_preset.setTooltip(txt_preset.getText());

    sld_gain.setValue(ParamToDB(ownerFilter->getParameter(0)), dontSendNotification);

    startTimer (100);

    ownerFilter->addChangeListener(this); // listen to changes of processor
}

Ambix_binauralAudioProcessorEditor::~Ambix_binauralAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]
    stopTimer();

    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    ourProcessor->removeChangeListener(this);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Ambix_binauralAudioProcessorEditor::paint (Graphics& g)
{

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       (float) (proportionOfWidth (0.6400f)), (float) (proportionOfHeight (0.6933f)),
                                       Colours::black,
                                       (float) (proportionOfWidth (0.1143f)), (float) (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 350 + _width, 325);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 350, 325, 1);

    g.setColour (Colour (0x410000ff));
    g.fillRoundedRectangle (18.0f, 128.0f, 217.0f, 76.0f, 10.0000f);

    g.setColour (Colours::white);

    g.setFont (Font (12.40f, Font::plain));
    g.drawText (TRANS("Volume [dB]"),
                353, 282, 65, 23,
                Justification::centred, true);

    g.setFont (Font (17.2000f, Font::bold));
#if BINAURAL_DECODER
    g.drawText ("AMBIX-BINAURAL-DECODER",
                1, 4, 343, 30,
                Justification::centred, true);

  g.setFont (Font (12.4000f, Font::plain));
  g.drawText ("listening to Ambisonics with headphones",
              1, 28, 343, 30,
              Justification::centred, true);

#else
  g.drawText ("AMBIX-DECODER",
              1, 4, 343, 30,
              Justification::centred, true);
  g.setFont (Font (12.4000f, Font::plain));
  g.drawText ("playback Ambisonics with loudspeakers",
              1, 28, 343, 30,
              Justification::centred, true);
#endif


    /* Version text */
    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string,
                getWidth()-51, getHeight()-11, 50, 10,
                Justification::bottomRight, true);

}

void Ambix_binauralAudioProcessorEditor::resized()
{
    label.setBounds (16, 129, 184, 24);
    txt_preset.setBounds (72, 64, 200, 24);
    label5.setBounds (8, 64, 56, 24);
    txt_debug.setBounds (16, 217, 320, 96);
    btn_open.setBounds (280, 64, 56, 24);
    label2.setBounds (62, 153, 137, 24);
    label3.setBounds (48, 177, 152, 24);
    label4.setBounds (24, 305, 64, 16);
    num_ch.setBounds (192, 129, 40, 24);
    num_spk.setBounds (192, 153, 40, 24);
    num_hrtf.setBounds (192, 177, 40, 24);
    btn_preset_folder.setBounds (248, 96, 94, 24);
    sld_gain.setBounds (352, 18, 48, 264);
    tgl_save_preset.setBounds(16, 92, 200, 24);
#if BINAURAL_DECODER
    tgl_load_irs.setBounds (260, 125, 80, 24);
    box_conv_buffer.setBounds (271, 155, 65, 22);
#endif

}

void Ambix_binauralAudioProcessorEditor::timerCallback()
{
    UpdateMeters();
}

bool Ambix_binauralAudioProcessorEditor::isInterestedInFileDrag(StringArray const& files)
{
    // we allow dropping single .config files
    return (files.size() == 1) && (File(files[0]).getFileExtension() == ".config");
}

void Ambix_binauralAudioProcessorEditor::filesDropped(StringArray const& files, int /*x*/, int /*y*/)
{
    auto droppedFile = File(files[0]);

    if (!droppedFile.existsAsFile()) {
        return;
    }

    if (droppedFile.getFileExtension() == ".config") {
        Ambix_binauralAudioProcessor* ourProcessor = getProcessor();
        ourProcessor->LoadConfiguration(droppedFile);
        ourProcessor->lastDir = droppedFile.getParentDirectory();
    }
}

void Ambix_binauralAudioProcessorEditor::UpdateText()
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    num_spk.setText(String(ourProcessor->_AmbiSpeakers.size()), dontSendNotification);
#if BINAURAL_DECODER
    num_hrtf.setText(String(ourProcessor->num_conv), dontSendNotification);
#endif
    num_ch.setText(String(ourProcessor->_AmbiChannels), dontSendNotification);

    txt_debug.setText(ourProcessor->_DebugText, true);

    txt_preset.setText(ourProcessor->box_preset_str);
    txt_preset.setCaretPosition(txt_preset.getTotalNumChars()-1);
    txt_preset.setTooltip(txt_preset.getText());

    tgl_save_preset.setToggleState(ourProcessor->_storeConfigDataInProject.get(), dontSendNotification);

#if BINAURAL_DECODER
    tgl_load_irs.setToggleState(ourProcessor->_load_ir, dontSendNotification);

    box_conv_buffer.clear(dontSendNotification);

    unsigned int buf = jmax(ourProcessor->getBufferSize(), (unsigned int)1);
    unsigned int conv_buf = ourProcessor->getConvBufferSize();

    int sel = 0;
    unsigned int val = 0;

    for (int i=0; val < 8192; i++) {

        val = buf*pow(2,i);

        box_conv_buffer.addItem(String(val), i+1);

        if (val == conv_buf)
            sel = i;
    }

    box_conv_buffer.setSelectedItemIndex(sel, dontSendNotification);
#endif
}


void Ambix_binauralAudioProcessorEditor::UpdatePresets()
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    popup_submenu.clear(); // contains submenus

    popup_presets.clear(); // main menu

    String lastSubdir;
    StringArray Subdirs; // hold name of subdirs

    int j = 1;

    for (int i=0; i < ourProcessor->_presetFiles.size(); i++) {

        // add separator for new subfolder
        String subdir = ourProcessor->_presetFiles.getUnchecked(i).getParentDirectory().getFileName();

        if (!lastSubdir.equalsIgnoreCase(subdir)) {

            popup_submenu.add(new PopupMenu());
            Subdirs.add(subdir);

            j++;
            lastSubdir = subdir;
        }

        // add item to submenu
        // check if this preset is loaded

        if (ourProcessor->_configFile == ourProcessor->_presetFiles.getUnchecked(i))
        {
            popup_submenu.getLast()->addItem(i+1, ourProcessor->_presetFiles.getUnchecked(i).getFileNameWithoutExtension(), true, true);
        } else {
            popup_submenu.getLast()->addItem(i+1, ourProcessor->_presetFiles.getUnchecked(i).getFileNameWithoutExtension());
        }


    }

    // add all subdirs to main menu
    for (int i=0; i < popup_submenu.size(); i++) {
        if (Subdirs.getReference(i) == ourProcessor->_configFile.getParentDirectory().getFileName())
        {
            popup_presets.addSubMenu(Subdirs.getReference(i), *popup_submenu.getUnchecked(i), true, nullptr, true);
        } else {
            popup_presets.addSubMenu(Subdirs.getReference(i), *popup_submenu.getUnchecked(i));
        }

    }

    if (ourProcessor->activePreset.isNotEmpty())
    {
        popup_presets.addSeparator();
        popup_presets.addItem(-2, String("save preset to .zip file..."), ourProcessor->_readyToSaveConfiguration.get());
    }

    popup_presets.addSeparator();
    popup_presets.addItem(-1, String("open from file..."));

}


void Ambix_binauralAudioProcessorEditor::menuItemChosenCallback (int result, Ambix_binauralAudioProcessorEditor* demoComponent)
{
    // std::cout << "result: " << result << std::endl;

    Ambix_binauralAudioProcessor* ourProcessor = demoComponent->getProcessor();


    // file chooser....
    if (result == 0)
    {
        // do nothing
    }
    else if (result == -1)
    {
        FileChooser myChooser ("Please select the preset file to load...",
                               ourProcessor->lastDir,
                               "*.config");
        if (myChooser.browseForFileToOpen())
        {

            File mooseFile (myChooser.getResult());
            //ourProcessor->ScheduleConfiguration(mooseFile);
            ourProcessor->LoadConfiguration(mooseFile);

            ourProcessor->lastDir = mooseFile.getParentDirectory();
        }
    }
    else if (result == -2)
    {
        FileChooser myChooser("Save the loaded preset as .zip file...",
            ourProcessor->lastDir.getChildFile(ourProcessor->activePreset),
            "*.zip");
        if (myChooser.browseForFileToSave(true))
        {
            File mooseFile(myChooser.getResult());
            ourProcessor->SaveConfiguration(mooseFile);

            ourProcessor->lastDir = mooseFile.getParentDirectory();
        }
    }
    else // load preset
    {
        demoComponent->stopTimer();
        ourProcessor->LoadPreset(result - 1);
        // popup_presets
    }

}

void Ambix_binauralAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();
#if BINAURAL_DECODER
    if (comboBoxThatHasChanged == &box_conv_buffer)
    {
        int val = box_conv_buffer.getText().getIntValue();

        // std::cout << "set size: " << val << std::endl;
        ourProcessor->setConvBufferSize(val);
    }
#endif
}

void Ambix_binauralAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    if (sliderThatWasMoved == &sld_gain)
    {
        ourProcessor->setParameterNotifyingHost(0, DbToParam(sld_gain.getValue()));
    }
}

void Ambix_binauralAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    /*
    if (buttonThatWasClicked == &btn_open)
    {

        FileChooser myChooser ("Please select the preset file to load...",
                               ourProcessor->lastDir,
                               "*.config");
        if (myChooser.browseForFileToOpen())
        {

            File mooseFile (myChooser.getResult());
            ourProcessor->LoadConfiguration(mooseFile);

            UpdateText();

            ourProcessor->lastDir = mooseFile.getParentDirectory();
        }
    }
     */
    if (buttonThatWasClicked == &btn_open)
    {
            popup_presets.showMenuAsync(PopupMenu::Options().withTargetComponent (btn_open), ModalCallbackFunction::forComponent (menuItemChosenCallback, this));
    }
    else if (buttonThatWasClicked == &btn_preset_folder)
    {
        FileChooser myChooser ("Please select the new preset folder...",
                               ourProcessor->presetDir,
                               "");

        if (myChooser.browseForDirectory())
        {

            File mooseFile (myChooser.getResult());
            ourProcessor->presetDir = mooseFile;

            ourProcessor->SearchPresets(mooseFile);

            ourProcessor->lastDir = mooseFile.getParentDirectory();
            UpdatePresets();
        }
    }
    else if (buttonThatWasClicked == &tgl_save_preset)
    {
        ourProcessor->_storeConfigDataInProject = tgl_save_preset.getToggleState();
    }
#if BINAURAL_DECODER
    else if (buttonThatWasClicked == &tgl_load_irs)
    {
        ourProcessor->_load_ir = tgl_load_irs.getToggleState();
    }
#endif

}

void Ambix_binauralAudioProcessorEditor::DrawMeters()
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    if (_meters.size() != ourProcessor->_AmbiSpeakers.size()) {

        int xoffset = 17;

        // clear meters first?
        _meters.clear();
        _labels.clear();
        _scales.clear();

        _scales.add(new MyMeterScale());
        addChildComponent(_scales.getLast());
        _scales.getLast()->setVisible(true);
        _scales.getLast()->setBounds(xoffset+370, 53, 20, 170);


        for (int i=0; i < ourProcessor->_AmbiSpeakers.size(); i++) {
            _meters.add(new MyMeter());
            addChildComponent(_meters.getLast());
            _meters.getLast()->setBounds(xoffset+394 + 15*i, 60, 8, 163);
            _meters.getLast()->setVisible(true);
            if (Label* const LABEL = new Label ("new label", String (i+1)))
            {
                _labels.add(LABEL);
                addChildComponent(_labels.getUnchecked(i));
                _labels.getUnchecked(i)->setVisible(true);
                _labels.getUnchecked(i)->setFont (Font (11.0000f, Font::plain));
                _labels.getUnchecked(i)->setColour (Label::textColourId, Colours::white);
                _labels.getUnchecked(i)->setJustificationType (Justification::centred);
                _labels.getUnchecked(i)->setBounds(xoffset+385 + 15*i, 222, 25, 14);
            }
        }

        _width = _meters.size() * 15 + 87;

        _scales.add(new MyMeterScale());
        addChildComponent(_scales.getLast());
        _scales.getLast()->setVisible(true);
        _scales.getLast()->setBounds(xoffset+350 + _width - 40, 53, 20, 170);


    }

    // resize component
    setSize (350 + _width, 325);
}

void Ambix_binauralAudioProcessorEditor::UpdateMeters()
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    for (int i=0; i < std::min(_meters.size(), ourProcessor->_AmbiSpeakers.size()); i++) {
        //_meters.getUnchecked(i)->setPeak(ourProcessor->_AmbiSpeakers.getUnchecked(i)->getPeak());
        _meters.getUnchecked(i)->setValue(ourProcessor->_AmbiSpeakers.getUnchecked(i)->getRMS(), ourProcessor->_AmbiSpeakers.getUnchecked(i)->getPeak());
    }

}


void Ambix_binauralAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    Ambix_binauralAudioProcessor* ourProcessor = getProcessor();

    sld_gain.setValue(ParamToDB(ourProcessor->getParameter(0)), dontSendNotification);

    UpdateText();
    DrawMeters();
    UpdatePresets();
    repaint();
    startTimer(100);
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Ambix_binauralAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="Ambix_binauralAudioProcessor* ownerFilter"
                 variableInitialisers="AudioProcessorEditor (ownerFilter)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330000013" fixedSize="1"
                 initialWidth="350" initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 350 300" fill=" radial: 64% 69.333%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="18 100 222 76" cornerSize="10" fill="solid: 410000ff" hasStroke="0"/>
    <TEXT pos="1 4 343 30" fill="solid: ffffffff" hasStroke="0" text="AMBIX-BINAURAL-DECODER"
          fontname="Default font" fontsize="17.2" bold="1" italic="0" justification="36"/>
    <TEXT pos="1 28 343 30" fill="solid: ffffffff" hasStroke="0" text="listening to Ambisonics with headphones"
          fontname="Default font" fontsize="12.4" bold="0" italic="0" justification="36"/>
  </BACKGROUND>
  <HYPERLINKBUTTON name="new hyperlink" id="529bec77f5596b3c" memberName="hyperlinkButton"
                   virtualName="" explicitFocusOrder="0" pos="152 280 192 20" tooltip="http://www.matthiaskronlachner.com"
                   textCol="ccffffff" buttonText="(C) 2013 Matthias Kronlachner"
                   connectedEdges="0" needsCallback="0" radioGroupId="0" url="http://www.matthiaskronlachner.com"/>
  <LABEL name="new label" id="18d53e308ddb22b0" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="16 104 184 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Ambisonics input channels: "
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
  <COMBOBOX name="new combo box" id="d106da7f451db4f8" memberName="box_presets"
            virtualName="" explicitFocusOrder="0" pos="72 64 200 24" tooltip="choose preset to load"
            editable="0" layout="33" items="" textWhenNonSelected="no preset loaded"
            textWhenNoItems="(no presets)"/>
  <LABEL name="new label" id="2071080ca9cd2158" memberName="label5" virtualName=""
         explicitFocusOrder="0" pos="8 64 56 24" textCol="ffffffff" edTextCol="ffffffff"
         edBkgCol="0" labelText="Preset" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <TEXTEDITOR name="new text editor" id="98fa042e4fd535b2" memberName="txt_debug"
              virtualName="" explicitFocusOrder="0" pos="16 184 320 96" initialText="debug window"
              multiline="1" retKeyStartsLine="0" readonly="1" scrollbars="1"
              caret="0" popupmenu="1"/>
  <TEXTBUTTON name="new button" id="49a576a901d98e13" memberName="btn_open"
              virtualName="" explicitFocusOrder="0" pos="280 64 56 24" tooltip="open a preset from file"
              bgColOff="ffffffff" bgColOn="ff0000ff" buttonText="open" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="160ab19ae75e175a" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="72 128 127 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Virtual speakers: "
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="ed30e712755c4691" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="48 152 152 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Impulse responses: "
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="3d7615cef31a6dbc" memberName="label4" virtualName=""
         explicitFocusOrder="0" pos="24 280 64 16" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="debug window" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="10" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="a3003899448147b5" memberName="num_ch" virtualName=""
         explicitFocusOrder="0" pos="192 104 40 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="0" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="bb9c878542097fc6" memberName="num_spk" virtualName=""
         explicitFocusOrder="0" pos="192 128 40 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="0" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="988973b36b44defc" memberName="num_hrtf"
         virtualName="" explicitFocusOrder="0" pos="192 152 40 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="0" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <TEXTBUTTON name="new button" id="e6922b75c64169ac" memberName="btn_preset_folder"
              virtualName="" explicitFocusOrder="0" pos="248 96 94 24" tooltip="choose another preset folder"
              bgColOff="ffffffff" bgColOn="ff0000ff" buttonText="preset folder"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



//[EndFile] You can add extra defines here...
//[/EndFile]
