/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  10 Jun 2013 5:40:13pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Ambix_converterAudioProcessorEditor::Ambix_converterAudioProcessorEditor (Ambix_converterAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      box_in_ch_seq (0),
      label (0),
      label2 (0),
      label3 (0),
      box_out_ch_seq (0),
      label4 (0),
      box_in_norm (0),
      box_out_norm (0),
      tgl_invert_cs (0),
      box_presets (0),
      label5 (0),
      tgl_flip (0),
      tgl_flop (0),
      tgl_flap (0),
      label6 (0)
{
    addAndMakeVisible (box_in_ch_seq = new ComboBox ("new combo box"));
    box_in_ch_seq->setTooltip ("channel input sequence");
    box_in_ch_seq->setEditableText (false);
    box_in_ch_seq->setJustificationType (Justification::centredLeft);
    box_in_ch_seq->setTextWhenNothingSelected ("ACN");
    box_in_ch_seq->setTextWhenNoChoicesAvailable ("(no choices)");
    box_in_ch_seq->addItem ("ACN", 1);
    box_in_ch_seq->addItem ("Furse-Malham", 2);
    box_in_ch_seq->addItem ("SID", 3);
    box_in_ch_seq->addListener (this);

    addAndMakeVisible (label = new Label ("new label",
                                          "Channel sequence"));
    label->setFont (Font (15.0000f, Font::plain));
    label->setJustificationType (Justification::centredRight);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::azure);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label2 = new Label ("new label",
                                           "Input"));
    label2->setFont (Font (15.0000f, Font::bold));
    label2->setJustificationType (Justification::centred);
    label2->setEditable (false, false, false);
    label2->setColour (Label::textColourId, Colours::black);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label3 = new Label ("new label",
                                           "Normalization"));
    label3->setFont (Font (15.0000f, Font::plain));
    label3->setJustificationType (Justification::centredRight);
    label3->setEditable (false, false, false);
    label3->setColour (Label::textColourId, Colours::azure);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (box_out_ch_seq = new ComboBox ("new combo box"));
    box_out_ch_seq->setTooltip ("channel output sequence");
    box_out_ch_seq->setEditableText (false);
    box_out_ch_seq->setJustificationType (Justification::centredLeft);
    box_out_ch_seq->setTextWhenNothingSelected ("ACN");
    box_out_ch_seq->setTextWhenNoChoicesAvailable ("(no choices)");
    box_out_ch_seq->addItem ("ACN", 1);
    box_out_ch_seq->addItem ("Furse-Malham", 2);
    box_out_ch_seq->addItem ("SID", 3);
    box_out_ch_seq->addListener (this);

    addAndMakeVisible (label4 = new Label ("new label",
                                           "Output"));
    label4->setFont (Font (15.0000f, Font::bold));
    label4->setJustificationType (Justification::centred);
    label4->setEditable (false, false, false);
    label4->setColour (Label::textColourId, Colours::black);
    label4->setColour (TextEditor::textColourId, Colours::black);
    label4->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (box_in_norm = new ComboBox ("new combo box"));
    box_in_norm->setTooltip ("channel input normalization scheme");
    box_in_norm->setEditableText (false);
    box_in_norm->setJustificationType (Justification::centredLeft);
    box_in_norm->setTextWhenNothingSelected ("SN3D");
    box_in_norm->setTextWhenNoChoicesAvailable ("(no choices)");
    box_in_norm->addItem ("SN3D", 1);
    box_in_norm->addItem ("Furse-Malham", 2);
    box_in_norm->addItem ("N3D", 3);
    box_in_norm->addListener (this);

    addAndMakeVisible (box_out_norm = new ComboBox ("new combo box"));
    box_out_norm->setTooltip ("channel output normalization scheme");
    box_out_norm->setEditableText (false);
    box_out_norm->setJustificationType (Justification::centredLeft);
    box_out_norm->setTextWhenNothingSelected ("SN3D");
    box_out_norm->setTextWhenNoChoicesAvailable ("(no choices)");
    box_out_norm->addItem ("SN3D", 1);
    box_out_norm->addItem ("Furse-Malham", 2);
    box_out_norm->addItem ("N3D", 3);
    box_out_norm->addListener (this);

    addAndMakeVisible (tgl_invert_cs = new ToggleButton ("new toggle button"));
    tgl_invert_cs->setTooltip ("only activate this if you know what you are doing!");
    tgl_invert_cs->setButtonText ("Invert Condon-Shortley");
    tgl_invert_cs->addListener (this);
    tgl_invert_cs->setColour (ToggleButton::textColourId, Colours::grey);

    addAndMakeVisible (box_presets = new ComboBox ("new combo box"));
    box_presets->setTooltip ("choose conversion scheme from several presets");
    box_presets->setEditableText (false);
    box_presets->setJustificationType (Justification::centredLeft);
    box_presets->setTextWhenNothingSelected (String::empty);
    box_presets->setTextWhenNoChoicesAvailable ("(no presets)");
    box_presets->addItem ("Wigware/B2X -> ambix", 1);
    box_presets->addItem ("ambix -> Wigware/B2X", 2);
    box_presets->addItem ("Universal Ambisonics -> ambix", 3);
    box_presets->addItem ("ambix -> Universal Ambisonics", 4);
    box_presets->addItem ("iem_ambi -> ambix", 5);
    box_presets->addItem ("ambix -> iem_ambi", 6);
    box_presets->addItem ("ICST -> ambix", 7);
    box_presets->addItem ("ambix -> ICST", 8);
    box_presets->addListener (this);

    addAndMakeVisible (label5 = new Label ("new label",
                                           "Presets"));
    label5->setFont (Font (15.0000f, Font::plain));
    label5->setJustificationType (Justification::centredRight);
    label5->setEditable (false, false, false);
    label5->setColour (Label::textColourId, Colours::azure);
    label5->setColour (TextEditor::textColourId, Colours::black);
    label5->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_flip = new ToggleButton ("new toggle button"));
    tgl_flip->setTooltip ("Mirror left-right (flip)");
    tgl_flip->setButtonText ("left <> right");
    tgl_flip->addListener (this);
    tgl_flip->setColour (ToggleButton::textColourId, Colours::azure);

    addAndMakeVisible (tgl_flop = new ToggleButton ("new toggle button"));
    tgl_flop->setTooltip ("Mirror front-back (flop)");
    tgl_flop->setButtonText ("front <> back");
    tgl_flop->addListener (this);
    tgl_flop->setColour (ToggleButton::textColourId, Colours::azure);

    addAndMakeVisible (tgl_flap = new ToggleButton ("new toggle button"));
    tgl_flap->setTooltip ("Mirror top-bottom (flap)");
    tgl_flap->setButtonText ("top <> bottom");
    tgl_flap->addListener (this);
    tgl_flap->setColour (ToggleButton::textColourId, Colours::azure);

    addAndMakeVisible (label6 = new Label ("new label",
                                           "Mirror"));
    label6->setFont (Font (15.0000f, Font::bold));
    label6->setJustificationType (Justification::centredLeft);
    label6->setEditable (false, false, false);
    label6->setColour (Label::textColourId, Colours::azure);
    label6->setColour (TextEditor::textColourId, Colours::black);
    label6->setColour (TextEditor::backgroundColourId, Colour (0x0));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (410, 305);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Ambix_converterAudioProcessorEditor::~Ambix_converterAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (box_in_ch_seq);
    deleteAndZero (label);
    deleteAndZero (label2);
    deleteAndZero (label3);
    deleteAndZero (box_out_ch_seq);
    deleteAndZero (label4);
    deleteAndZero (box_in_norm);
    deleteAndZero (box_out_norm);
    deleteAndZero (tgl_invert_cs);
    deleteAndZero (box_presets);
    deleteAndZero (label5);
    deleteAndZero (tgl_flip);
    deleteAndZero (tgl_flop);
    deleteAndZero (tgl_flap);
    deleteAndZero (label6);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Ambix_converterAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       (float) (proportionOfWidth (0.6400f)), (float) (proportionOfHeight (0.6933f)),
                                       Colours::black,
                                       (float) (proportionOfWidth (0.1143f)), (float) (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 410, 310);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 410, 310, 1);

    g.setColour (Colours::cadetblue);
    g.fillRoundedRectangle (272.0f, 110.0f, 133.0f, 106.0f, 10.0000f);

    g.setColour (Colours::grey);
    g.fillRoundedRectangle (128.0f, 110.0f, 133.0f, 106.0f, 10.0000f);

    g.setColour (Colours::azure);
    g.setFont (Font (17.2000f, Font::bold));
    g.drawText ("AMBIX-CONVERTER",
                11, 6, 380, 30,
                Justification::centred, true);

    g.setColour (Colours::azure);
    g.setFont (Font (12.4000f, Font::plain));
    g.drawText ("convert between Ambisonics formats",
                8, 28, 380, 30,
                Justification::centred, true);

    g.setColour (Colour (0xdc000000));
    g.fillRoundedRectangle (11.0f, 271.0f, 195.0f, 24.0f, 10.0000f);

    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (216.0f, 223.0f, 184.0f, 73.0f, 10.0000f);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Ambix_converterAudioProcessorEditor::resized()
{
    box_in_ch_seq->setBounds (136, 136, 120, 24);
    label->setBounds (0, 136, 127, 24);
    label2->setBounds (136, 112, 112, 24);
    label3->setBounds (16, 184, 111, 24);
    box_out_ch_seq->setBounds (280, 136, 120, 24);
    label4->setBounds (288, 112, 104, 24);
    box_in_norm->setBounds (136, 184, 120, 24);
    box_out_norm->setBounds (280, 184, 120, 24);
    tgl_invert_cs->setBounds (20, 271, 180, 24);
    box_presets->setBounds (112, 64, 288, 24);
    label5->setBounds (8, 64, 104, 24);
    tgl_flip->setBounds (280, 224, 120, 24);
    tgl_flop->setBounds (280, 248, 120, 24);
    tgl_flap->setBounds (280, 272, 120, 24);
    label6->setBounds (224, 224, 56, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Ambix_converterAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == box_in_ch_seq)
    {
        //[UserComboBoxCode_box_in_ch_seq] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_in_ch_seq]
    }
    else if (comboBoxThatHasChanged == box_out_ch_seq)
    {
        //[UserComboBoxCode_box_out_ch_seq] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_out_ch_seq]
    }
    else if (comboBoxThatHasChanged == box_in_norm)
    {
        //[UserComboBoxCode_box_in_norm] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_in_norm]
    }
    else if (comboBoxThatHasChanged == box_out_norm)
    {
        //[UserComboBoxCode_box_out_norm] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_out_norm]
    }
    else if (comboBoxThatHasChanged == box_presets)
    {
        //[UserComboBoxCode_box_presets] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_presets]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void Ambix_converterAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == tgl_invert_cs)
    {
        //[UserButtonCode_tgl_invert_cs] -- add your button handler code here..
        //[/UserButtonCode_tgl_invert_cs]
    }
    else if (buttonThatWasClicked == tgl_flip)
    {
        //[UserButtonCode_tgl_flip] -- add your button handler code here..
        //[/UserButtonCode_tgl_flip]
    }
    else if (buttonThatWasClicked == tgl_flop)
    {
        //[UserButtonCode_tgl_flop] -- add your button handler code here..
        //[/UserButtonCode_tgl_flop]
    }
    else if (buttonThatWasClicked == tgl_flap)
    {
        //[UserButtonCode_tgl_flap] -- add your button handler code here..
        //[/UserButtonCode_tgl_flap]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Ambix_converterAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="Ambix_converterAudioProcessor* ownerFilter"
                 variableInitialisers="AudioProcessorEditor (ownerFilter)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330000013" fixedSize="1"
                 initialWidth="410" initialHeight="305">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 410 310" fill=" radial: 64% 69.333%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="272 110 133 106" cornerSize="10" fill="solid: ff5f9ea0"
               hasStroke="0"/>
    <ROUNDRECT pos="128 110 133 106" cornerSize="10" fill="solid: ff808080"
               hasStroke="0"/>
    <TEXT pos="11 6 380 30" fill="solid: fff0ffff" hasStroke="0" text="AMBIX-CONVERTER"
          fontname="Default font" fontsize="17.2" bold="1" italic="0" justification="36"/>
    <TEXT pos="8 28 380 30" fill="solid: fff0ffff" hasStroke="0" text="convert between Ambisonics formats"
          fontname="Default font" fontsize="12.4" bold="0" italic="0" justification="36"/>
    <ROUNDRECT pos="11 271 195 24" cornerSize="10" fill="solid: dc000000" hasStroke="0"/>
    <ROUNDRECT pos="216 223 184 73" cornerSize="10" fill="solid: ff2b1d69" hasStroke="0"/>
  </BACKGROUND>
  <COMBOBOX name="new combo box" id="f1000b0699c42d32" memberName="box_in_ch_seq"
            virtualName="" explicitFocusOrder="0" pos="136 136 120 24" tooltip="channel input sequence"
            editable="0" layout="33" items="ACN&#10;Furse-Malham&#10;SID"
            textWhenNonSelected="ACN" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="18d53e308ddb22b0" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0 136 127 24" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Channel sequence"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="799520c015f0cd6c" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="136 112 112 24" textCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="Input" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="36"/>
  <LABEL name="new label" id="31661e2aabfa68fd" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="16 184 111 24" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Normalization" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <COMBOBOX name="new combo box" id="248109d38bcbf793" memberName="box_out_ch_seq"
            virtualName="" explicitFocusOrder="0" pos="280 136 120 24" tooltip="channel output sequence"
            editable="0" layout="33" items="ACN&#10;Furse-Malham&#10;SID"
            textWhenNonSelected="ACN" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="b555a3e046bcdb5a" memberName="label4" virtualName=""
         explicitFocusOrder="0" pos="288 112 104 24" textCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="Output" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="36"/>
  <COMBOBOX name="new combo box" id="fc31f754f89353ca" memberName="box_in_norm"
            virtualName="" explicitFocusOrder="0" pos="136 184 120 24" tooltip="channel input normalization scheme"
            editable="0" layout="33" items="SN3D&#10;Furse-Malham&#10;N3D"
            textWhenNonSelected="SN3D" textWhenNoItems="(no choices)"/>
  <COMBOBOX name="new combo box" id="370aea36432b7f84" memberName="box_out_norm"
            virtualName="" explicitFocusOrder="0" pos="280 184 120 24" tooltip="channel output normalization scheme"
            editable="0" layout="33" items="SN3D&#10;Furse-Malham&#10;N3D"
            textWhenNonSelected="SN3D" textWhenNoItems="(no choices)"/>
  <TOGGLEBUTTON name="new toggle button" id="f83af2c4bd8eaf30" memberName="tgl_invert_cs"
                virtualName="" explicitFocusOrder="0" pos="20 271 180 24" tooltip="only activate this if you know what you are doing!"
                txtcol="ff808080" buttonText="Invert Condon-Shortley" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <COMBOBOX name="new combo box" id="d106da7f451db4f8" memberName="box_presets"
            virtualName="" explicitFocusOrder="0" pos="112 64 288 24" tooltip="choose conversion scheme from several presets"
            editable="0" layout="33" items="Wigware/B2X -&gt; ambix&#10;ambix -&gt; Wigware/B2X&#10;Universal Ambisonics -&gt; ambix&#10;ambix -&gt; Universal Ambisonics&#10;iem_ambi -&gt; ambix&#10;ambix -&gt; iem_ambi&#10;ICST -&gt; ambix&#10;ambix -&gt; ICST"
            textWhenNonSelected="" textWhenNoItems="(no presets)"/>
  <LABEL name="new label" id="2071080ca9cd2158" memberName="label5" virtualName=""
         explicitFocusOrder="0" pos="8 64 104 24" textCol="fff0ffff" edTextCol="ff000000"
         edBkgCol="0" labelText="Presets" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <TOGGLEBUTTON name="new toggle button" id="2eb24f9d01b6c5f4" memberName="tgl_flip"
                virtualName="" explicitFocusOrder="0" pos="280 224 120 24" tooltip="Mirror left-right (flip)"
                txtcol="fff0ffff" buttonText="left &lt;&gt; right" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="3332afd90aee67c9" memberName="tgl_flop"
                virtualName="" explicitFocusOrder="0" pos="280 248 120 24" tooltip="Mirror front-back (flop)"
                txtcol="fff0ffff" buttonText="front &lt;&gt; back" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="5e710762de71b13a" memberName="tgl_flap"
                virtualName="" explicitFocusOrder="0" pos="280 272 120 24" tooltip="Mirror top-bottom (flap)"
                txtcol="fff0ffff" buttonText="top &lt;&gt; bottom" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <LABEL name="new label" id="6809c7d06268242f" memberName="label6" virtualName=""
         explicitFocusOrder="0" pos="224 224 56 24" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Mirror" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



//[EndFile] You can add extra defines here...
//[/EndFile]
