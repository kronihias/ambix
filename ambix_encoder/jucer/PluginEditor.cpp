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
Ambix_encoderAudioProcessorEditor::Ambix_encoderAudioProcessorEditor (Ambix_encoderAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
    addAndMakeVisible (sld_el = new Slider ("new slider"));
    sld_el->setTooltip (TRANS("elevation"));
    sld_el->setRange (-180, 180, 1);
    sld_el->setSliderStyle (Slider::LinearVertical);
    sld_el->setTextBoxStyle (Slider::TextBoxBelow, false, 41, 20);
    sld_el->setColour (Slider::thumbColourId, Colours::grey);
    sld_el->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_el->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_el->addListener (this);

    addAndMakeVisible (opengl_component = new Component());
    opengl_component->setName ("new component");

    addAndMakeVisible (sld_az = new Slider ("new slider"));
    sld_az->setTooltip (TRANS("azimuth"));
    sld_az->setRange (-180, 180, 1);
    sld_az->setSliderStyle (Slider::LinearHorizontal);
    sld_az->setTextBoxStyle (Slider::TextBoxRight, false, 40, 20);
    sld_az->setColour (Slider::thumbColourId, Colours::grey);
    sld_az->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_az->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_az->addListener (this);

    addAndMakeVisible (sld_size = new Slider ("new slider"));
    sld_size->setTooltip (TRANS("higher order scaling - decrease spatial sharpness"));
    sld_size->setRange (0, 1, 0.01);
    sld_size->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_size->setTextBoxStyle (Slider::NoTextBox, false, 40, 20);
    sld_size->setColour (Slider::thumbColourId, Colours::white);
    sld_size->setColour (Slider::trackColourId, Colours::white);
    sld_size->setColour (Slider::rotarySliderFillColourId, Colours::white);
    sld_size->setColour (Slider::rotarySliderOutlineColourId, Colours::white);
    sld_size->setColour (Slider::textBoxTextColourId, Colours::white);
    sld_size->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_size->setColour (Slider::textBoxHighlightColourId, Colours::white);
    sld_size->addListener (this);

    addAndMakeVisible (sld_width = new Slider ("new slider"));
    sld_width->setTooltip (TRANS("multiple input sources are equally spread along this range"));
    sld_width->setRange (0, 360, 1);
    sld_width->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_width->setTextBoxStyle (Slider::NoTextBox, false, 41, 20);
    sld_width->setColour (Slider::thumbColourId, Colour (0xffe982cd));
    sld_width->setColour (Slider::rotarySliderFillColourId, Colours::white);
    sld_width->setColour (Slider::rotarySliderOutlineColourId, Colours::white);
    sld_width->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_width->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_width->addListener (this);

    addAndMakeVisible (sld_speed = new Slider ("new slider"));
    sld_speed->setTooltip (TRANS("movement speed in deg/sec"));
    sld_speed->setRange (0, 360, 1);
    sld_speed->setSliderStyle (Slider::LinearHorizontal);
    sld_speed->setTextBoxStyle (Slider::TextBoxRight, false, 40, 20);
    sld_speed->setColour (Slider::thumbColourId, Colour (0xffe98273));
    sld_speed->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_speed->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_speed->addListener (this);

    addAndMakeVisible (sld_el_move = new Slider ("new slider"));
    sld_el_move->setTooltip (TRANS("elevation movement speed"));
    sld_el_move->setRange (0, 1, 0.01);
    sld_el_move->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_el_move->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    sld_el_move->setColour (Slider::trackColourId, Colour (0xff2d7dff));
    sld_el_move->setColour (Slider::rotarySliderFillColourId, Colours::white);
    sld_el_move->setColour (Slider::rotarySliderOutlineColourId, Colours::azure);
    sld_el_move->addListener (this);

    addAndMakeVisible (sld_az_move = new Slider ("new slider"));
    sld_az_move->setTooltip (TRANS("azimuth movement speed"));
    sld_az_move->setRange (0, 1, 0.01);
    sld_az_move->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_az_move->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    sld_az_move->setColour (Slider::trackColourId, Colour (0xff2d7dff));
    sld_az_move->setColour (Slider::rotarySliderFillColourId, Colours::white);
    sld_az_move->setColour (Slider::rotarySliderOutlineColourId, Colours::azure);
    sld_az_move->addListener (this);

    addAndMakeVisible (txt_az_move = new TextEditor ("new text editor"));
    txt_az_move->setMultiLine (false);
    txt_az_move->setReturnKeyStartsNewLine (false);
    txt_az_move->setReadOnly (true);
    txt_az_move->setScrollbarsShown (false);
    txt_az_move->setCaretVisible (false);
    txt_az_move->setPopupMenuEnabled (false);
    txt_az_move->setColour (TextEditor::outlineColourId, Colour (0x706884ff));
    txt_az_move->setColour (TextEditor::shadowColourId, Colour (0x00000000));
    txt_az_move->setText (TRANS("-180 deg/s"));

    addAndMakeVisible (txt_el_move = new TextEditor ("new text editor"));
    txt_el_move->setTooltip (TRANS("elevation movement speed"));
    txt_el_move->setMultiLine (false);
    txt_el_move->setReturnKeyStartsNewLine (false);
    txt_el_move->setReadOnly (true);
    txt_el_move->setScrollbarsShown (false);
    txt_el_move->setCaretVisible (false);
    txt_el_move->setPopupMenuEnabled (false);
    txt_el_move->setColour (TextEditor::outlineColourId, Colour (0x706884ff));
    txt_el_move->setColour (TextEditor::shadowColourId, Colour (0x00000000));
    txt_el_move->setText (TRANS("-180 deg/s"));

    addAndMakeVisible (lbl_id = new Label ("new label",
                                           TRANS("ID: 1")));
    lbl_id->setFont (Font (15.00f, Font::plain));
    lbl_id->setJustificationType (Justification::centredRight);
    lbl_id->setEditable (false, false, false);
    lbl_id->setColour (Label::textColourId, Colour (0xff888888));
    lbl_id->setColour (TextEditor::textColourId, Colours::black);
    lbl_id->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (330, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Ambix_encoderAudioProcessorEditor::~Ambix_encoderAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    sld_el = nullptr;
    opengl_component = nullptr;
    sld_az = nullptr;
    sld_size = nullptr;
    sld_width = nullptr;
    sld_speed = nullptr;
    sld_el_move = nullptr;
    sld_az_move = nullptr;
    txt_az_move = nullptr;
    txt_el_move = nullptr;
    lbl_id = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Ambix_encoderAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       static_cast<float> (proportionOfWidth (0.6314f)), static_cast<float> (proportionOfHeight (0.5842f)),
                                       Colours::black,
                                       static_cast<float> (proportionOfWidth (0.1143f)), static_cast<float> (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 330, 400);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 330, 400, 1);

    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (165.0f, 310.0f, 154.0f, 77.0f, 4.000f);

    g.setColour (Colours::white);
    g.setFont (Font (17.20f, Font::bold));
    g.drawText (TRANS("AMBIX-ENCODER"),
                -6, 2, 343, 30,
                Justification::centred, true);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("elevation"),
                266, 29, 48, 16,
                Justification::centred, true);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("azimuth"),
                216, 272, 48, 16,
                Justification::centredRight, true);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("size"),
                31, 382, 23, 16,
                Justification::centredRight, true);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("max speed"),
                206, 372, 57, 16,
                Justification::centredRight, true);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("multiple source width"),
                61, 382, 95, 16,
                Justification::centredRight, true);

    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (24.0f, 310.0f, 226.0f, 43.0f, 4.000f);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("elevation move"),
                219, 338, 81, 16,
                Justification::centredRight, true);

    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    g.drawText (TRANS("azimuth move"),
                59, 338, 81, 16,
                Justification::centredRight, true);
    
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

void Ambix_encoderAudioProcessorEditor::resized()
{
    sld_el->setBounds (270, 38, 40, 232);
    opengl_component->setBounds (24, 34, 240, 240);
    sld_az->setBounds (27, 270, 282, 40);
    sld_size->setBounds (32, 355, 29, 29);
    sld_width->setBounds (93, 355, 29, 29);
    sld_speed->setBounds (169, 347, 140, 40);
    sld_el_move->setBounds (275, 312, 29, 29);
    sld_az_move->setBounds (118, 312, 29, 29);
    txt_az_move->setBounds (37, 316, 78, 22);
    txt_el_move->setBounds (191, 316, 78, 22);
    lbl_id->setBounds (271, 0, 57, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Ambix_encoderAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sld_el)
    {
        //[UserSliderCode_sld_el] -- add your slider handling code here..
        //[/UserSliderCode_sld_el]
    }
    else if (sliderThatWasMoved == sld_az)
    {
        //[UserSliderCode_sld_az] -- add your slider handling code here..
        //[/UserSliderCode_sld_az]
    }
    else if (sliderThatWasMoved == sld_size)
    {
        //[UserSliderCode_sld_size] -- add your slider handling code here..
        //[/UserSliderCode_sld_size]
    }
    else if (sliderThatWasMoved == sld_width)
    {
        //[UserSliderCode_sld_width] -- add your slider handling code here..
        //[/UserSliderCode_sld_width]
    }
    else if (sliderThatWasMoved == sld_speed)
    {
        //[UserSliderCode_sld_speed] -- add your slider handling code here..
        //[/UserSliderCode_sld_speed]
    }
    else if (sliderThatWasMoved == sld_el_move)
    {
        //[UserSliderCode_sld_el_move] -- add your slider handling code here..
        //[/UserSliderCode_sld_el_move]
    }
    else if (sliderThatWasMoved == sld_az_move)
    {
        //[UserSliderCode_sld_az_move] -- add your slider handling code here..
        //[/UserSliderCode_sld_az_move]
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

<JUCER_COMPONENT documentType="Component" className="Ambix_encoderAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="Ambix_encoderAudioProcessor* ownerFilter"
                 variableInitialisers="AudioProcessorEditor (ownerFilter)" snapPixels="8"
                 snapActive="0" snapShown="0" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="330" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 330 400" fill=" radial: 63.143% 58.421%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="165 310 154 77" cornerSize="4" fill="solid: ff2b1d69" hasStroke="0"/>
    <TEXT pos="-6 2 343 30" fill="solid: ffffffff" hasStroke="0" text="AMBIX-ENCODER"
          fontname="Default font" fontsize="17.199999999999999289" bold="1"
          italic="0" justification="36"/>
    <TEXT pos="266 29 48 16" fill="solid: ffffffff" hasStroke="0" text="elevation"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="36"/>
    <TEXT pos="216 272 48 16" fill="solid: ffffffff" hasStroke="0" text="azimuth"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="34"/>
    <TEXT pos="31 382 23 16" fill="solid: ffffffff" hasStroke="0" text="size"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="34"/>
    <TEXT pos="206 372 57 16" fill="solid: ffffffff" hasStroke="0" text="max speed"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="34"/>
    <TEXT pos="61 382 95 16" fill="solid: ffffffff" hasStroke="0" text="multiple source width"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="34"/>
    <ROUNDRECT pos="24 310 226 43" cornerSize="4" fill="solid: ff2b1d69" hasStroke="0"/>
    <TEXT pos="219 338 81 16" fill="solid: ffffffff" hasStroke="0" text="elevation move"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="34"/>
    <TEXT pos="59 338 81 16" fill="solid: ffffffff" hasStroke="0" text="azimuth move"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="34"/>
  </BACKGROUND>
  <SLIDER name="new slider" id="86bda7f5f29714ad" memberName="sld_el" virtualName=""
          explicitFocusOrder="0" pos="270 38 40 232" tooltip="elevation"
          thumbcol="ff808080" textboxtext="ff000000" textboxbkgd="ffffffff"
          min="-180" max="180" int="1" style="LinearVertical" textBoxPos="TextBoxBelow"
          textBoxEditable="1" textBoxWidth="41" textBoxHeight="20" skewFactor="1"/>
  <GENERICCOMPONENT name="new component" id="3391b824ef1c7ad9" memberName="opengl_component"
                    virtualName="" explicitFocusOrder="0" pos="24 34 240 240" class="Component"
                    params=""/>
  <SLIDER name="new slider" id="405bfd8311641135" memberName="sld_az" virtualName=""
          explicitFocusOrder="0" pos="27 270 282 40" tooltip="azimuth"
          thumbcol="ff808080" textboxtext="ff000000" textboxbkgd="ffffffff"
          min="-180" max="180" int="1" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="40" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="9fc511daef4de98d" memberName="sld_size"
          virtualName="" explicitFocusOrder="0" pos="32 355 29 29" tooltip="higher order scaling - decrease spatial sharpness"
          thumbcol="ffffffff" trackcol="ffffffff" rotarysliderfill="ffffffff"
          rotaryslideroutline="ffffffff" textboxtext="ffffffff" textboxbkgd="ffffffff"
          textboxhighlight="ffffffff" min="0" max="1" int="0.010000000000000000208"
          style="RotaryHorizontalVerticalDrag" textBoxPos="NoTextBox" textBoxEditable="1"
          textBoxWidth="40" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="823f23e392d249dc" memberName="sld_width"
          virtualName="" explicitFocusOrder="0" pos="93 355 29 29" tooltip="multiple input sources are equally spread along this range"
          thumbcol="ffe982cd" rotarysliderfill="ffffffff" rotaryslideroutline="ffffffff"
          textboxtext="ff000000" textboxbkgd="ffffffff" min="0" max="360"
          int="1" style="RotaryHorizontalVerticalDrag" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="41" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="7299b2ca93f14e38" memberName="sld_speed"
          virtualName="" explicitFocusOrder="0" pos="169 347 140 40" tooltip="movement speed in deg/sec"
          thumbcol="ffe98273" textboxtext="ff000000" textboxbkgd="ffffffff"
          min="0" max="360" int="1" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="40" textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="945d76eb6afdfcca" memberName="sld_el_move"
          virtualName="" explicitFocusOrder="0" pos="275 312 29 29" tooltip="elevation movement speed"
          trackcol="ff2d7dff" rotarysliderfill="ffffffff" rotaryslideroutline="fff0ffff"
          min="0" max="1" int="0.010000000000000000208" style="RotaryHorizontalVerticalDrag"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <SLIDER name="new slider" id="e8bf968ebd3f72b7" memberName="sld_az_move"
          virtualName="" explicitFocusOrder="0" pos="118 312 29 29" tooltip="azimuth movement speed"
          trackcol="ff2d7dff" rotarysliderfill="ffffffff" rotaryslideroutline="fff0ffff"
          min="0" max="1" int="0.010000000000000000208" style="RotaryHorizontalVerticalDrag"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <TEXTEDITOR name="new text editor" id="5e5851601ca3b525" memberName="txt_az_move"
              virtualName="" explicitFocusOrder="0" pos="37 316 78 22" outlinecol="706884ff"
              shadowcol="0" initialText="-180 deg/s" multiline="0" retKeyStartsLine="0"
              readonly="1" scrollbars="0" caret="0" popupmenu="0"/>
  <TEXTEDITOR name="new text editor" id="63e9b8df7ede2b03" memberName="txt_el_move"
              virtualName="" explicitFocusOrder="0" pos="191 316 78 22" tooltip="elevation movement speed"
              outlinecol="706884ff" shadowcol="0" initialText="-180 deg/s"
              multiline="0" retKeyStartsLine="0" readonly="1" scrollbars="0"
              caret="0" popupmenu="0"/>
  <LABEL name="new label" id="4f5581a62d8489b1" memberName="lbl_id" virtualName=""
         explicitFocusOrder="0" pos="271 0 57 24" textCol="ff888888" edTextCol="ff000000"
         edBkgCol="0" labelText="ID: 1" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
