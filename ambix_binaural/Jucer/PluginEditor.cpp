/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 4.0.1

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Ambix_binauralAudioProcessorEditor::Ambix_binauralAudioProcessorEditor (Ambix_binauralAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (gr_hp = new GroupComponent ("new group",
                                                   TRANS("high pass")));
    gr_hp->setColour (GroupComponent::outlineColourId, Colour (0x66ffffff));
    gr_hp->setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("Ambisonics input channels: ")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredRight);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (box_presets = new ComboBox ("new combo box"));
    box_presets->setTooltip (TRANS("choose preset to load"));
    box_presets->setEditableText (false);
    box_presets->setJustificationType (Justification::centredLeft);
    box_presets->setTextWhenNothingSelected (TRANS("no preset loaded"));
    box_presets->setTextWhenNoChoicesAvailable (TRANS("(no presets)"));
    box_presets->addListener (this);

    addAndMakeVisible (label5 = new Label ("new label",
                                           TRANS("Preset")));
    label5->setFont (Font (15.00f, Font::plain));
    label5->setJustificationType (Justification::centredRight);
    label5->setEditable (false, false, false);
    label5->setColour (Label::textColourId, Colours::white);
    label5->setColour (TextEditor::textColourId, Colours::white);
    label5->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (txt_debug = new TextEditor ("new text editor"));
    txt_debug->setMultiLine (true);
    txt_debug->setReturnKeyStartsNewLine (false);
    txt_debug->setReadOnly (true);
    txt_debug->setScrollbarsShown (true);
    txt_debug->setCaretVisible (false);
    txt_debug->setPopupMenuEnabled (true);
    txt_debug->setText (TRANS("debug window"));

    addAndMakeVisible (btn_open = new TextButton ("new button"));
    btn_open->setTooltip (TRANS("open a preset from file"));
    btn_open->setButtonText (TRANS("open"));
    btn_open->addListener (this);
    btn_open->setColour (TextButton::buttonColourId, Colours::white);
    btn_open->setColour (TextButton::buttonOnColourId, Colours::blue);

    addAndMakeVisible (label2 = new Label ("new label",
                                           TRANS("Virtual speakers: ")));
    label2->setFont (Font (15.00f, Font::plain));
    label2->setJustificationType (Justification::centredRight);
    label2->setEditable (false, false, false);
    label2->setColour (Label::textColourId, Colours::white);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label3 = new Label ("new label",
                                           TRANS("Impulse responses: ")));
    label3->setFont (Font (15.00f, Font::plain));
    label3->setJustificationType (Justification::centredRight);
    label3->setEditable (false, false, false);
    label3->setColour (Label::textColourId, Colours::white);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label4 = new Label ("new label",
                                           TRANS("debug window")));
    label4->setFont (Font (10.00f, Font::plain));
    label4->setJustificationType (Justification::centredLeft);
    label4->setEditable (false, false, false);
    label4->setColour (Label::textColourId, Colours::white);
    label4->setColour (TextEditor::textColourId, Colours::black);
    label4->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (num_ch = new Label ("new label",
                                           TRANS("0")));
    num_ch->setFont (Font (15.00f, Font::plain));
    num_ch->setJustificationType (Justification::centredRight);
    num_ch->setEditable (false, false, false);
    num_ch->setColour (Label::textColourId, Colours::white);
    num_ch->setColour (TextEditor::textColourId, Colours::black);
    num_ch->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (num_spk = new Label ("new label",
                                            TRANS("0")));
    num_spk->setFont (Font (15.00f, Font::plain));
    num_spk->setJustificationType (Justification::centredRight);
    num_spk->setEditable (false, false, false);
    num_spk->setColour (Label::textColourId, Colours::white);
    num_spk->setColour (TextEditor::textColourId, Colours::black);
    num_spk->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (num_hrtf = new Label ("new label",
                                             TRANS("0")));
    num_hrtf->setFont (Font (15.00f, Font::plain));
    num_hrtf->setJustificationType (Justification::centredRight);
    num_hrtf->setEditable (false, false, false);
    num_hrtf->setColour (Label::textColourId, Colours::white);
    num_hrtf->setColour (TextEditor::textColourId, Colours::black);
    num_hrtf->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btn_preset_folder = new TextButton ("new button"));
    btn_preset_folder->setTooltip (TRANS("choose another preset folder"));
    btn_preset_folder->setButtonText (TRANS("preset folder"));
    btn_preset_folder->addListener (this);
    btn_preset_folder->setColour (TextButton::buttonColourId, Colours::white);
    btn_preset_folder->setColour (TextButton::buttonOnColourId, Colours::blue);

    addAndMakeVisible (tgl_load_irs = new ToggleButton ("new toggle button"));
    tgl_load_irs->setTooltip (TRANS("load impulse responses if new preset being loaded - deactivate if IRs already loaded - for fast decoder matrix switching"));
    tgl_load_irs->setButtonText (TRANS("load IRs"));
    tgl_load_irs->addListener (this);
    tgl_load_irs->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (sld_hpf = new Slider ("new slider"));
    sld_hpf->setTooltip (TRANS("loudspeaker high pass cut off frequency"));
    sld_hpf->setRange (0, 200, 1);
    sld_hpf->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_hpf->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sld_hpf->setColour (Slider::rotarySliderFillColourId, Colour (0x7fffffff));
    sld_hpf->setColour (Slider::rotarySliderOutlineColourId, Colour (0x66ffffff));
    sld_hpf->addListener (this);

    addAndMakeVisible (box_hp_order = new ComboBox ("new combo box"));
    box_hp_order->setTooltip (TRANS("high pass order"));
    box_hp_order->setEditableText (false);
    box_hp_order->setJustificationType (Justification::centredLeft);
    box_hp_order->setTextWhenNothingSelected (TRANS("HP OFF"));
    box_hp_order->setTextWhenNoChoicesAvailable (TRANS("HP OFF"));
    box_hp_order->addItem (TRANS("HP OFF"), 1);
    box_hp_order->addItem (TRANS("2nd"), 2);
    box_hp_order->addItem (TRANS("4th"), 3);
    box_hp_order->addListener (this);

    addAndMakeVisible (groupComponent2 = new GroupComponent ("new group",
                                                             TRANS("sub out")));
    groupComponent2->setColour (GroupComponent::outlineColourId, Colour (0x66ffffff));
    groupComponent2->setColour (GroupComponent::textColourId, Colours::white);

    addAndMakeVisible (sld_lpf = new Slider ("new slider"));
    sld_lpf->setTooltip (TRANS("sub low-pass cut off frequency"));
    sld_lpf->setRange (0, 200, 1);
    sld_lpf->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_lpf->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sld_lpf->setColour (Slider::rotarySliderFillColourId, Colour (0x7fffffff));
    sld_lpf->setColour (Slider::rotarySliderOutlineColourId, Colour (0x66ffffff));
    sld_lpf->addListener (this);

    addAndMakeVisible (box_lp_order = new ComboBox ("new combo box"));
    box_lp_order->setTooltip (TRANS("sub low-pass order"));
    box_lp_order->setEditableText (false);
    box_lp_order->setJustificationType (Justification::centredLeft);
    box_lp_order->setTextWhenNothingSelected (TRANS("LP OFF"));
    box_lp_order->setTextWhenNoChoicesAvailable (TRANS("LP OFF"));
    box_lp_order->addItem (TRANS("LP OFF"), 1);
    box_lp_order->addItem (TRANS("2nd"), 2);
    box_lp_order->addItem (TRANS("4th"), 3);
    box_lp_order->addListener (this);

    addAndMakeVisible (box_sub_output = new ComboBox ("new combo box"));
    box_sub_output->setTooltip (TRANS("sub output channel"));
    box_sub_output->setEditableText (false);
    box_sub_output->setJustificationType (Justification::centredLeft);
    box_sub_output->setTextWhenNothingSelected (TRANS("4"));
    box_sub_output->setTextWhenNoChoicesAvailable (TRANS("4"));
    box_sub_output->addItem (TRANS("1"), 1);
    box_sub_output->addItem (TRANS("2"), 2);
    box_sub_output->addItem (TRANS("3"), 3);
    box_sub_output->addListener (this);

    addAndMakeVisible (box_sub_output2 = new ComboBox ("new combo box"));
    box_sub_output2->setTooltip (TRANS("sub signal origin"));
    box_sub_output2->setEditableText (false);
    box_sub_output2->setJustificationType (Justification::centredLeft);
    box_sub_output2->setTextWhenNothingSelected (TRANS("SUB SEND OFF"));
    box_sub_output2->setTextWhenNoChoicesAvailable (TRANS("SUB SEND OFF"));
    box_sub_output2->addItem (TRANS("SUB SEND OFF"), 1);
    box_sub_output2->addItem (TRANS("omni (W)"), 2);
    box_sub_output2->addItem (TRANS("sum of all LS"), 3);
    box_sub_output2->addListener (this);

    addAndMakeVisible (sld_sub_vol = new Slider ("new slider"));
    sld_sub_vol->setTooltip (TRANS("sub volume"));
    sld_sub_vol->setRange (-99, 6, 1);
    sld_sub_vol->setSliderStyle (Slider::LinearHorizontal);
    sld_sub_vol->setTextBoxStyle (Slider::TextBoxLeft, false, 50, 20);
    sld_sub_vol->setColour (Slider::thumbColourId, Colour (0xff949494));
    sld_sub_vol->setColour (Slider::rotarySliderFillColourId, Colour (0x7fffffff));
    sld_sub_vol->setColour (Slider::rotarySliderOutlineColourId, Colour (0x66ffffff));
    sld_sub_vol->addListener (this);

    addAndMakeVisible (sld_gain = new Slider ("new slider"));
    sld_gain->setTooltip (TRANS("Output Volume"));
    sld_gain->setRange (-99, 20, 0.1);
    sld_gain->setSliderStyle (Slider::LinearVertical);
    sld_gain->setTextBoxStyle (Slider::TextBoxBelow, false, 48, 20);
    sld_gain->setColour (Slider::thumbColourId, Colours::white);
    sld_gain->addListener (this);
    sld_gain->setSkewFactor (0.8);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (650, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Ambix_binauralAudioProcessorEditor::~Ambix_binauralAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    gr_hp = nullptr;
    label = nullptr;
    box_presets = nullptr;
    label5 = nullptr;
    txt_debug = nullptr;
    btn_open = nullptr;
    label2 = nullptr;
    label3 = nullptr;
    label4 = nullptr;
    num_ch = nullptr;
    num_spk = nullptr;
    num_hrtf = nullptr;
    btn_preset_folder = nullptr;
    tgl_load_irs = nullptr;
    sld_hpf = nullptr;
    box_hp_order = nullptr;
    groupComponent2 = nullptr;
    sld_lpf = nullptr;
    box_lp_order = nullptr;
    box_sub_output = nullptr;
    box_sub_output2 = nullptr;
    sld_sub_vol = nullptr;
    sld_gain = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Ambix_binauralAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       static_cast<float> (proportionOfWidth (0.6400f)), static_cast<float> (proportionOfHeight (0.6933f)),
                                       Colours::black,
                                       static_cast<float> (proportionOfWidth (0.1143f)), static_cast<float> (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 650, 300);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 650, 300, 1);

    g.setColour (Colour (0x410000ff));
    g.fillRoundedRectangle (18.0f, 100.0f, 222.0f, 76.0f, 10.000f);

    g.setColour (Colours::white);
    g.setFont (Font (17.20f, Font::bold));
    g.drawText (TRANS("AMBIX-BINAURAL-DECODER"),
                1, 4, 343, 30,
                Justification::centred, true);

    g.setColour (Colours::white);
    g.setFont (Font (12.40f, Font::plain));
    g.drawText (TRANS("listening to Ambisonics with headphones"),
                1, 28, 343, 30,
                Justification::centred, true);

    g.setColour (Colours::white);
    g.setFont (Font (12.40f, Font::plain));
    g.drawText (TRANS("Volume [dB]"),
                338, 276, 65, 23,
                Justification::centred, true);

    
    /* Version text */
    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string,
                getWidth()-51, getHeight()-11, 50, 10,
                Justification::bottomRight, true);
    
    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Ambix_binauralAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    gr_hp->setBounds (407, 195, 96, 72);
    label->setBounds (16, 104, 184, 24);
    box_presets->setBounds (72, 64, 200, 24);
    label5->setBounds (8, 64, 56, 24);
    txt_debug->setBounds (16, 184, 320, 96);
    btn_open->setBounds (280, 64, 56, 24);
    label2->setBounds (72, 128, 127, 24);
    label3->setBounds (48, 152, 152, 24);
    label4->setBounds (24, 280, 64, 16);
    num_ch->setBounds (192, 104, 40, 24);
    num_spk->setBounds (192, 128, 40, 24);
    num_hrtf->setBounds (192, 152, 40, 24);
    btn_preset_folder->setBounds (248, 96, 94, 24);
    tgl_load_irs->setBounds (256, 152, 80, 24);
    sld_hpf->setBounds (415, 211, 88, 22);
    box_hp_order->setBounds (415, 235, 80, 24);
    groupComponent2->setBounds (511, 195, 184, 96);
    sld_lpf->setBounds (519, 211, 88, 22);
    box_lp_order->setBounds (615, 211, 64, 22);
    box_sub_output->setBounds (631, 235, 48, 22);
    box_sub_output2->setBounds (519, 235, 104, 24);
    sld_sub_vol->setBounds (519, 259, 160, 24);
    sld_gain->setBounds (343, 18, 48, 254);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Ambix_binauralAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == box_presets)
    {
        //[UserComboBoxCode_box_presets] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_presets]
    }
    else if (comboBoxThatHasChanged == box_hp_order)
    {
        //[UserComboBoxCode_box_hp_order] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_hp_order]
    }
    else if (comboBoxThatHasChanged == box_lp_order)
    {
        //[UserComboBoxCode_box_lp_order] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_lp_order]
    }
    else if (comboBoxThatHasChanged == box_sub_output)
    {
        //[UserComboBoxCode_box_sub_output] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_sub_output]
    }
    else if (comboBoxThatHasChanged == box_sub_output2)
    {
        //[UserComboBoxCode_box_sub_output2] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_sub_output2]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void Ambix_binauralAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btn_open)
    {
        //[UserButtonCode_btn_open] -- add your button handler code here..
        //[/UserButtonCode_btn_open]
    }
    else if (buttonThatWasClicked == btn_preset_folder)
    {
        //[UserButtonCode_btn_preset_folder] -- add your button handler code here..
        //[/UserButtonCode_btn_preset_folder]
    }
    else if (buttonThatWasClicked == tgl_load_irs)
    {
        //[UserButtonCode_tgl_load_irs] -- add your button handler code here..
        //[/UserButtonCode_tgl_load_irs]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void Ambix_binauralAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sld_hpf)
    {
        //[UserSliderCode_sld_hpf] -- add your slider handling code here..
        //[/UserSliderCode_sld_hpf]
    }
    else if (sliderThatWasMoved == sld_lpf)
    {
        //[UserSliderCode_sld_lpf] -- add your slider handling code here..
        //[/UserSliderCode_sld_lpf]
    }
    else if (sliderThatWasMoved == sld_sub_vol)
    {
        //[UserSliderCode_sld_sub_vol] -- add your slider handling code here..
        //[/UserSliderCode_sld_sub_vol]
    }
    else if (sliderThatWasMoved == sld_gain)
    {
        //[UserSliderCode_sld_gain] -- add your slider handling code here..
        //[/UserSliderCode_sld_gain]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Ambix_binauralAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="Ambix_binauralAudioProcessor* ownerFilter"
                 variableInitialisers="AudioProcessorEditor (ownerFilter)" snapPixels="8"
                 snapActive="0" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="650" initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 650 300" fill=" radial: 64% 69.333%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="18 100 222 76" cornerSize="10" fill="solid: 410000ff" hasStroke="0"/>
    <TEXT pos="1 4 343 30" fill="solid: ffffffff" hasStroke="0" text="AMBIX-BINAURAL-DECODER"
          fontname="Default font" fontsize="17.199999999999999289" bold="1"
          italic="0" justification="36"/>
    <TEXT pos="1 28 343 30" fill="solid: ffffffff" hasStroke="0" text="listening to Ambisonics with headphones"
          fontname="Default font" fontsize="12.400000000000000355" bold="0"
          italic="0" justification="36"/>
    <TEXT pos="338 276 65 23" fill="solid: ffffffff" hasStroke="0" text="Volume [dB]"
          fontname="Default font" fontsize="12.400000000000000355" bold="0"
          italic="0" justification="36"/>
  </BACKGROUND>
  <GROUPCOMPONENT name="new group" id="93b72dd4e7eb70f2" memberName="gr_hp" virtualName=""
                  explicitFocusOrder="0" pos="407 195 96 72" outlinecol="66ffffff"
                  textcol="ffffffff" title="high pass"/>
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
  <TOGGLEBUTTON name="new toggle button" id="5fbf315b518c7f3" memberName="tgl_load_irs"
                virtualName="" explicitFocusOrder="0" pos="256 152 80 24" tooltip="load impulse responses if new preset being loaded - deactivate if IRs already loaded - for fast decoder matrix switching"
                txtcol="ffffffff" buttonText="load IRs" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <SLIDER name="new slider" id="93c06746c73b55e6" memberName="sld_hpf"
          virtualName="" explicitFocusOrder="0" pos="415 211 88 22" tooltip="loudspeaker high pass cut off frequency"
          rotarysliderfill="7fffffff" rotaryslideroutline="66ffffff" min="0"
          max="200" int="1" style="RotaryHorizontalVerticalDrag" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <COMBOBOX name="new combo box" id="4355679e656fdcb6" memberName="box_hp_order"
            virtualName="" explicitFocusOrder="0" pos="415 235 80 24" tooltip="high pass order"
            editable="0" layout="33" items="HP OFF&#10;2nd&#10;4th" textWhenNonSelected="HP OFF"
            textWhenNoItems="HP OFF"/>
  <GROUPCOMPONENT name="new group" id="f2a608de1e2dcc2f" memberName="groupComponent2"
                  virtualName="" explicitFocusOrder="0" pos="511 195 184 96" outlinecol="66ffffff"
                  textcol="ffffffff" title="sub out"/>
  <SLIDER name="new slider" id="887c3af75adba9d7" memberName="sld_lpf"
          virtualName="" explicitFocusOrder="0" pos="519 211 88 22" tooltip="sub low-pass cut off frequency"
          rotarysliderfill="7fffffff" rotaryslideroutline="66ffffff" min="0"
          max="200" int="1" style="RotaryHorizontalVerticalDrag" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <COMBOBOX name="new combo box" id="bddf00b0dabff700" memberName="box_lp_order"
            virtualName="" explicitFocusOrder="0" pos="615 211 64 22" tooltip="sub low-pass order"
            editable="0" layout="33" items="LP OFF&#10;2nd&#10;4th" textWhenNonSelected="LP OFF"
            textWhenNoItems="LP OFF"/>
  <COMBOBOX name="new combo box" id="82e9180ea7c9d1a3" memberName="box_sub_output"
            virtualName="" explicitFocusOrder="0" pos="631 235 48 22" tooltip="sub output channel"
            editable="0" layout="33" items="1&#10;2&#10;3" textWhenNonSelected="4"
            textWhenNoItems="4"/>
  <COMBOBOX name="new combo box" id="5bcc5f10478e7389" memberName="box_sub_output2"
            virtualName="" explicitFocusOrder="0" pos="519 235 104 24" tooltip="sub signal origin"
            editable="0" layout="33" items="SUB SEND OFF&#10;omni (W)&#10;sum of all LS"
            textWhenNonSelected="SUB SEND OFF" textWhenNoItems="SUB SEND OFF"/>
  <SLIDER name="new slider" id="6c2fc8e1fc0081ff" memberName="sld_sub_vol"
          virtualName="" explicitFocusOrder="0" pos="519 259 160 24" tooltip="sub volume"
          thumbcol="ff949494" rotarysliderfill="7fffffff" rotaryslideroutline="66ffffff"
          min="-99" max="6" int="1" style="LinearHorizontal" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="50" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="26fc90b8b12b56a0" memberName="sld_gain"
          virtualName="" explicitFocusOrder="0" pos="343 18 48 254" tooltip="Output Volume"
          thumbcol="ffffffff" min="-99" max="20" int="0.10000000000000000555"
          style="LinearVertical" textBoxPos="TextBoxBelow" textBoxEditable="1"
          textBoxWidth="48" textBoxHeight="20" skewFactor="0.80000000000000004441"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
