/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

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
    addAndMakeVisible (hyperlinkButton = new HyperlinkButton (TRANS("(C) 2013 Matthias Kronlachner"),
                                                              URL ("http://www.matthiaskronlachner.com")));
    hyperlinkButton->setTooltip (TRANS("http://www.matthiaskronlachner.com"));
    hyperlinkButton->setButtonText (TRANS("(C) 2013 Matthias Kronlachner"));
    hyperlinkButton->setColour (HyperlinkButton::textColourId, Colour (0xccffffff));

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


    //[UserPreSize]
    //[/UserPreSize]

    setSize (350, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Ambix_binauralAudioProcessorEditor::~Ambix_binauralAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    hyperlinkButton = nullptr;
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
    g.fillRect (0, 0, 350, 300);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 350, 300, 1);

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

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Ambix_binauralAudioProcessorEditor::resized()
{
    hyperlinkButton->setBounds (152, 280, 192, 20);
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
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="350" initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 350 300" fill=" radial: 64% 69.333%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="18 100 222 76" cornerSize="10" fill="solid: 410000ff" hasStroke="0"/>
    <TEXT pos="1 4 343 30" fill="solid: ffffffff" hasStroke="0" text="AMBIX-BINAURAL-DECODER"
          fontname="Default font" fontsize="17.199999999999999289" bold="1"
          italic="0" justification="36"/>
    <TEXT pos="1 28 343 30" fill="solid: ffffffff" hasStroke="0" text="listening to Ambisonics with headphones"
          fontname="Default font" fontsize="12.400000000000000355" bold="0"
          italic="0" justification="36"/>
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
  <TOGGLEBUTTON name="new toggle button" id="5fbf315b518c7f3" memberName="tgl_load_irs"
                virtualName="" explicitFocusOrder="0" pos="256 152 80 24" tooltip="load impulse responses if new preset being loaded - deactivate if IRs already loaded - for fast decoder matrix switching"
                txtcol="ffffffff" buttonText="load IRs" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
