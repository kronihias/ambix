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

    addAndMakeVisible (btn_settings = new ImageButton ("new button"));
    btn_settings->setTooltip (TRANS("OSC settings"));
    btn_settings->setButtonText (TRANS("settings"));
    btn_settings->addListener (this);

    btn_settings->setImages (false, true, true,
                             ImageCache::getFromMemory (settings_png, settings_pngSize), 1.000f, Colour (0x00000000),
                             ImageCache::getFromMemory (settings_white_png, settings_white_pngSize), 1.000f, Colour (0x00000000),
                             ImageCache::getFromMemory (settings_png, settings_pngSize), 1.000f, Colour (0x00000000));

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
    btn_settings = nullptr;


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
    opengl_component->setBounds (23, 32, 240, 240);
    sld_az->setBounds (27, 270, 282, 40);
    sld_size->setBounds (32, 355, 29, 29);
    sld_width->setBounds (93, 355, 29, 29);
    sld_speed->setBounds (169, 347, 140, 40);
    sld_el_move->setBounds (275, 312, 29, 29);
    sld_az_move->setBounds (118, 312, 29, 29);
    txt_az_move->setBounds (37, 316, 78, 22);
    txt_el_move->setBounds (191, 316, 78, 22);
    lbl_id->setBounds (271, 0, 57, 24);
    btn_settings->setBounds (3, 3, 26, 25);
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

void Ambix_encoderAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btn_settings)
    {
        //[UserButtonCode_btn_settings] -- add your button handler code here..
        //[/UserButtonCode_btn_settings]
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
                    virtualName="" explicitFocusOrder="0" pos="23 32 240 240" class="Component"
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
  <IMAGEBUTTON name="new button" id="2100faa4a85025a6" memberName="btn_settings"
               virtualName="" explicitFocusOrder="0" pos="3 3 26 25" tooltip="OSC settings"
               buttonText="settings" connectedEdges="0" needsCallback="1" radioGroupId="0"
               keepProportions="1" resourceNormal="settings_png" opacityNormal="1"
               colourNormal="0" resourceOver="settings_white_png" opacityOver="1"
               colourOver="0" resourceDown="settings_png" opacityDown="1" colourDown="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: settings_png, 21502, "../../settings.png"
static const unsigned char resource_Ambix_encoderAudioProcessorEditor_settings_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,2,0,0,0,2,0,8,6,0,0,0,244,120,212,250,0,0,0,6,98,75,71,68,0,255,
0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,222,8,18,15,36,28,192,53,249,254,0,0,32,0,73,68,65,84,120,218,237,189,107,140,92,103,154,223,
247,188,167,206,165,122,170,217,106,178,69,177,217,108,94,37,74,188,105,36,238,72,26,105,174,156,93,26,11,59,187,155,100,87,213,69,32,27,3,6,130,24,136,237,15,73,144,108,108,195,235,76,188,27,196,49,144,
53,144,24,72,226,15,129,145,69,204,238,214,4,65,214,235,197,6,189,158,158,25,141,70,148,40,145,18,41,145,18,197,59,217,164,196,161,186,167,201,82,87,157,170,115,222,124,168,234,102,85,177,186,186,238,
117,46,191,63,32,216,203,105,254,216,117,206,243,190,191,231,125,207,169,115,68,8,33,132,16,18,187,168,78,1,199,143,31,75,212,254,217,220,220,188,7,15,94,189,164,211,233,137,218,63,155,157,157,93,128,
7,15,30,60,120,253,229,153,29,136,65,149,255,126,101,19,161,69,164,8,15,94,109,50,153,204,184,231,21,77,173,253,170,63,79,36,76,120,240,224,193,131,55,0,94,91,59,0,101,57,56,117,228,144,159,155,155,215,
240,224,213,22,107,177,88,176,181,214,107,60,165,148,54,77,203,157,158,158,190,11,15,30,60,120,240,250,207,107,185,1,56,126,252,152,33,34,67,117,228,176,50,55,55,239,195,131,87,153,169,169,169,237,133,
130,155,172,229,89,150,157,155,153,153,185,3,15,30,60,120,240,6,195,107,169,1,40,203,97,184,142,28,30,118,32,27,120,17,229,77,77,77,109,119,221,124,74,68,87,240,148,182,109,39,219,110,241,195,131,7,15,
30,188,206,121,45,53,0,229,155,193,54,137,136,81,241,199,190,136,60,104,231,166,48,120,209,230,165,211,233,137,66,33,63,172,181,84,108,83,137,182,44,231,97,59,55,189,192,131,7,15,30,188,238,240,50,153,
204,184,239,251,70,83,55,1,150,229,48,218,101,217,192,139,40,47,157,78,79,184,110,110,68,235,71,60,165,196,183,172,228,131,118,139,31,30,60,120,240,224,117,206,91,189,129,80,41,195,55,155,148,195,88,29,
57,44,117,32,27,120,17,229,165,211,233,137,124,126,101,115,45,207,182,135,150,219,45,126,120,240,224,193,131,215,57,111,106,106,106,123,177,88,176,148,82,190,200,6,95,3,172,144,131,93,241,199,158,136,
124,217,166,108,76,17,217,42,34,9,120,209,227,165,211,175,239,200,231,87,198,180,214,21,157,170,242,29,39,185,212,94,241,195,131,7,15,30,188,110,240,86,111,32,84,74,249,74,149,174,36,152,45,108,11,39,
164,244,29,240,123,115,115,243,45,127,119,177,44,155,109,53,255,38,188,136,240,210,233,215,119,228,114,43,91,69,116,69,51,161,60,199,73,222,159,157,125,227,54,60,120,240,224,193,27,20,47,61,225,186,249,
175,173,174,252,87,99,172,35,7,67,30,191,33,172,40,34,159,119,89,54,240,34,192,91,175,88,147,201,161,123,221,44,126,120,240,224,193,131,215,186,252,11,133,252,112,245,183,7,74,79,12,52,214,145,255,112,
141,252,61,100,8,143,193,4,15,30,60,120,225,146,127,249,6,194,170,135,6,173,62,49,208,172,145,131,146,199,31,2,227,75,233,154,48,50,132,199,224,132,7,15,30,188,144,240,242,249,149,81,17,165,149,146,213,
123,186,170,158,24,104,214,200,223,169,35,255,165,14,110,48,67,174,200,31,30,60,120,240,224,245,93,254,185,49,41,61,200,173,236,111,165,45,203,206,85,62,46,184,82,6,102,29,249,63,64,254,240,24,76,240,
224,193,131,23,54,158,136,82,202,43,253,191,82,247,137,129,70,89,16,9,121,252,241,175,200,31,30,131,9,30,60,120,240,66,204,43,61,52,200,121,88,239,113,193,245,190,6,216,201,179,228,145,43,242,135,7,15,
30,60,120,129,224,137,111,219,201,117,31,26,100,212,145,255,10,242,135,199,96,130,7,15,30,188,48,175,252,149,239,56,67,139,141,30,26,100,214,200,191,221,247,199,35,87,228,15,15,30,60,120,240,2,194,43,
61,52,168,241,19,3,171,30,244,131,252,225,49,152,224,193,131,7,47,30,60,37,29,4,185,34,127,120,240,224,193,131,23,78,94,219,13,0,114,69,254,240,224,193,131,7,47,188,188,182,26,0,228,138,252,225,193,131,
7,15,94,184,121,45,55,0,200,21,249,195,131,7,15,30,188,240,243,90,106,0,144,43,242,135,7,15,30,60,120,225,231,137,212,127,16,16,50,140,40,239,196,137,19,235,126,203,227,234,213,207,108,6,19,188,56,241,
26,141,135,147,39,79,42,142,31,188,168,242,50,153,204,184,239,251,70,83,59,0,200,53,218,242,111,54,181,147,34,131,9,94,24,121,173,140,133,147,39,79,42,142,31,188,168,201,223,243,138,166,82,134,191,225,
132,94,126,79,192,83,34,98,35,215,248,202,159,149,18,188,184,201,127,53,217,236,131,163,28,63,120,81,224,77,77,77,109,247,188,162,165,148,242,55,108,0,202,242,31,43,203,63,129,92,195,197,235,165,248,153,
44,225,197,65,254,171,185,123,247,246,222,77,155,54,141,113,62,224,133,89,254,133,130,155,84,74,249,134,97,120,13,27,128,178,252,71,69,196,42,11,39,129,92,145,127,179,249,201,79,126,156,222,191,255,233,
119,182,109,155,240,24,156,240,6,201,75,36,204,91,221,168,233,43,87,46,61,179,123,247,222,28,231,3,94,248,120,233,137,66,193,253,90,105,229,175,188,134,13,192,241,227,199,12,17,25,41,203,63,81,150,142,
70,174,200,191,147,112,15,1,188,176,202,191,157,58,230,124,192,11,142,252,243,195,34,162,149,50,188,213,6,224,141,55,126,116,75,173,35,255,225,138,85,127,66,74,95,23,188,135,92,145,127,175,39,81,6,59,
188,32,203,159,102,22,94,216,228,239,186,185,17,17,165,149,18,95,41,195,51,12,163,104,154,150,59,61,61,125,87,213,200,70,137,200,215,234,200,255,75,228,138,252,123,61,145,50,216,225,133,65,254,205,54,
1,156,15,120,131,230,229,243,185,209,146,252,149,87,110,0,138,182,237,172,76,79,79,223,21,169,120,16,80,89,254,78,29,249,47,205,205,205,123,200,26,249,247,50,158,87,156,100,176,195,11,139,252,217,201,
130,23,2,249,143,137,136,22,81,94,169,1,80,158,109,59,217,153,153,153,59,171,63,87,249,58,96,83,170,159,12,232,35,127,228,223,175,36,18,230,173,115,231,206,189,194,96,135,23,22,249,175,55,246,56,31,240,
130,192,211,90,175,249,93,41,209,181,242,95,219,1,40,223,241,95,249,159,33,34,89,228,31,14,94,20,26,128,202,228,243,249,207,23,22,110,238,60,117,234,116,129,193,14,47,168,242,175,183,19,192,249,128,23,
28,222,218,202,191,96,219,201,229,217,217,217,133,218,159,175,215,0,24,34,178,50,55,55,239,35,215,240,240,162,214,4,52,179,205,202,96,135,23,4,249,175,134,203,88,240,130,197,43,201,223,113,134,22,235,
201,95,164,250,18,128,72,233,171,126,200,63,132,188,86,191,154,20,166,156,56,113,66,111,212,224,48,216,145,255,160,243,241,199,231,71,57,31,240,130,194,83,74,249,141,228,95,219,0,104,17,201,35,255,240,
242,162,220,4,52,218,229,96,176,35,255,32,228,249,231,95,60,127,253,250,245,2,231,23,94,16,120,142,147,188,223,72,254,181,13,64,113,110,110,190,229,109,100,100,29,44,158,231,21,39,163,222,4,84,54,2,12,
118,228,31,164,28,58,244,252,7,183,111,223,94,230,252,194,11,3,175,163,21,35,178,14,22,175,178,24,82,169,145,211,18,241,204,207,255,120,211,203,47,191,244,12,131,29,249,7,49,60,49,16,94,208,121,6,242,
143,158,252,69,68,178,217,229,151,162,222,0,28,59,246,131,7,215,174,93,255,29,6,59,242,15,234,110,21,231,23,94,144,121,109,237,0,32,235,96,203,63,140,147,101,167,185,120,241,163,3,251,247,63,247,144,193,
142,252,195,182,19,192,249,133,55,40,94,203,59,0,200,58,60,242,159,157,125,227,118,212,111,12,92,205,129,3,135,47,182,35,7,38,15,228,63,200,157,0,206,47,188,65,242,90,146,3,178,14,151,252,219,221,142,
140,66,154,105,124,152,60,144,255,32,107,146,243,11,111,144,188,150,26,0,100,29,94,249,211,4,48,216,145,127,176,106,146,243,11,111,144,188,76,38,51,238,251,190,209,84,3,128,172,195,47,255,184,54,1,172,
188,144,127,208,194,19,3,225,13,90,254,158,87,52,149,50,252,13,239,1,40,63,38,120,43,178,14,191,252,55,90,21,71,53,60,55,0,249,7,41,215,175,95,77,114,126,225,13,130,55,53,53,181,189,88,44,216,107,164,
38,228,63,38,34,182,148,222,19,128,172,67,44,255,184,239,4,176,242,66,254,193,105,2,46,63,251,228,147,91,71,168,23,120,253,148,127,161,224,38,149,82,190,97,24,94,195,29,128,178,252,71,165,230,105,129,
200,58,252,242,95,149,97,220,26,128,146,84,152,60,144,255,224,179,123,247,211,159,82,47,240,250,199,75,79,184,110,62,85,187,232,87,235,200,203,16,145,17,17,177,202,43,127,83,74,239,10,64,214,17,144,127,
53,47,241,70,42,149,218,19,167,70,224,230,205,107,207,108,217,50,182,137,201,3,249,15,58,237,92,146,163,94,224,181,42,255,66,33,63,44,34,90,41,195,83,74,121,134,97,120,111,188,241,163,91,198,58,242,31,
174,89,249,123,200,58,138,242,23,17,241,94,143,219,78,192,206,157,123,62,75,165,54,157,97,242,64,254,131,78,171,151,226,168,23,120,173,175,252,115,35,90,63,90,236,43,165,116,34,97,22,165,70,242,114,252,
248,49,37,34,67,53,59,3,190,136,220,67,214,81,148,127,137,23,199,203,1,76,190,200,63,108,117,72,189,192,107,149,151,207,175,108,214,186,250,173,191,166,105,185,211,211,211,119,171,26,128,178,252,157,58,
242,191,143,172,163,43,255,184,61,49,144,201,23,249,135,177,14,169,23,120,173,203,63,55,86,189,200,87,218,178,236,220,170,252,107,119,0,204,58,242,95,154,155,155,247,144,117,180,229,191,250,39,52,1,76,
30,200,63,120,117,72,189,192,107,135,167,181,94,243,187,82,162,109,219,201,206,204,204,220,169,252,89,163,44,176,68,141,252,181,136,60,64,254,241,145,63,77,0,147,47,242,15,94,168,23,120,157,242,148,18,
223,178,156,135,181,242,175,221,1,168,148,255,67,228,31,63,249,211,4,84,55,1,76,30,200,127,144,53,72,189,192,235,156,39,190,109,39,151,103,103,103,23,234,253,29,85,177,3,144,40,55,4,238,220,220,188,143,
172,227,41,255,102,86,197,81,207,201,147,39,21,147,7,242,31,100,120,104,21,188,206,87,254,70,193,113,134,126,185,158,252,107,119,0,180,136,228,145,63,242,103,39,224,132,102,50,66,254,131,12,245,2,175,
83,158,227,36,239,55,146,127,109,3,80,156,155,155,111,121,197,135,252,163,41,255,202,149,72,28,39,224,84,106,211,25,38,35,228,63,168,124,248,225,185,87,169,63,120,189,230,117,180,194,67,254,209,150,127,
37,47,149,26,57,29,183,73,248,246,237,91,255,247,232,232,19,255,132,201,3,249,15,34,95,124,177,176,39,149,218,180,153,250,131,215,43,94,219,13,0,242,143,143,252,31,173,138,227,215,4,136,240,184,86,228,
63,168,6,244,246,236,248,248,182,255,28,121,193,235,21,207,104,167,48,145,127,252,228,207,19,3,153,60,144,127,127,179,99,199,142,116,59,199,143,250,131,215,179,6,0,249,199,83,254,60,49,144,39,6,34,255,
224,135,250,131,215,179,6,0,249,199,91,254,171,127,66,19,192,228,129,252,145,63,188,112,243,68,90,184,7,0,249,35,255,118,87,197,81,75,189,6,136,201,8,249,247,179,222,168,63,120,157,240,50,153,204,184,
239,251,70,83,59,0,200,31,249,147,245,27,31,38,35,228,207,206,19,188,48,201,223,243,138,166,72,19,151,0,202,79,9,220,138,252,145,127,59,43,147,168,135,201,8,249,83,127,240,194,194,155,154,154,218,94,44,
22,236,53,82,19,242,31,19,17,91,74,143,10,70,254,20,87,203,43,148,168,134,199,181,34,255,126,133,203,78,240,186,33,255,66,193,77,42,165,124,195,48,60,165,12,223,216,64,254,163,53,187,4,200,159,226,98,
39,160,156,159,255,252,205,175,51,25,33,255,126,132,203,78,240,58,227,165,39,92,55,159,170,93,244,27,235,200,208,16,145,77,200,31,249,55,203,203,102,31,28,125,251,237,183,254,118,156,38,229,239,125,239,
216,191,189,121,243,102,30,249,147,126,6,25,194,107,85,254,133,66,126,88,68,87,201,127,118,118,118,65,173,35,255,225,178,12,87,223,18,168,68,228,30,242,167,184,54,226,197,241,105,129,95,124,177,176,115,
108,236,41,141,252,73,175,195,101,39,120,173,175,252,115,35,34,74,43,37,190,82,134,103,24,70,209,52,45,119,122,122,250,174,81,35,67,37,34,67,53,219,4,62,242,167,184,154,229,125,241,197,194,206,184,77,
202,79,61,53,113,19,249,147,126,4,25,194,107,133,151,207,175,108,214,186,250,173,191,171,242,23,169,216,226,47,203,223,169,35,255,251,200,159,226,106,150,55,54,246,84,44,111,8,140,211,227,130,145,255,
32,131,12,225,53,43,255,220,152,84,93,198,87,218,178,236,220,170,252,165,250,127,20,179,142,252,151,230,230,230,61,228,79,113,181,194,227,73,129,200,159,244,38,31,124,240,225,24,50,132,215,12,79,107,189,
230,119,165,68,219,182,147,157,153,153,185,83,249,179,70,89,136,137,26,249,107,17,121,128,252,41,174,94,243,226,22,228,79,58,201,183,190,245,157,191,100,126,129,215,10,79,41,241,45,203,121,88,43,255,218,
29,128,74,249,63,68,254,20,87,39,60,118,1,144,63,233,77,82,169,77,103,144,33,188,230,120,226,219,118,114,121,118,118,118,161,222,223,49,234,200,127,101,110,110,222,71,254,20,23,43,255,238,53,1,200,159,
176,243,4,175,191,43,127,229,59,206,208,226,122,242,175,109,0,180,136,228,145,63,197,213,45,30,143,10,70,254,164,255,59,77,204,87,240,68,148,231,56,201,251,141,228,95,219,0,20,231,230,230,91,190,131,27,
249,83,172,100,253,9,26,249,19,154,79,120,65,229,25,34,34,115,115,243,30,242,167,24,144,127,119,115,224,192,179,14,231,151,12,106,23,128,249,10,94,83,13,64,59,65,254,20,87,51,137,243,101,128,23,95,252,
181,92,88,207,111,92,95,240,196,202,31,94,156,120,109,53,0,200,159,226,34,205,229,242,229,203,26,249,147,126,238,2,48,95,193,107,54,45,175,206,144,63,197,213,42,47,238,215,146,179,217,7,71,145,63,233,
101,86,119,218,152,175,224,181,146,150,118,0,144,63,197,213,14,239,163,143,206,253,48,206,147,115,84,191,183,77,130,21,230,43,120,173,198,68,254,20,67,175,121,191,252,229,253,115,76,207,193,63,191,172,
254,195,155,19,39,78,232,108,246,193,81,230,43,120,205,36,147,201,140,251,190,111,52,181,3,128,252,41,174,78,120,150,101,62,96,130,142,206,247,182,73,48,147,205,62,88,100,190,130,215,140,252,61,175,104,
138,52,113,9,160,252,158,128,173,200,159,226,106,151,167,148,250,138,233,185,185,38,0,249,147,118,99,219,95,251,11,230,43,120,141,50,53,53,181,189,88,44,216,171,255,183,209,132,252,199,68,36,129,252,41,
174,118,121,175,189,246,237,159,49,61,179,242,39,189,205,232,232,232,193,115,231,206,39,153,175,224,173,39,255,66,193,77,106,173,213,134,13,64,89,254,163,53,63,131,252,225,181,204,99,106,222,120,23,0,
249,147,110,228,213,87,191,245,102,59,223,186,97,190,138,58,47,61,225,186,249,148,212,124,243,207,88,71,174,134,136,108,66,254,20,23,178,97,229,79,168,63,120,225,150,127,161,144,31,22,209,85,242,159,157,
157,93,48,214,145,255,112,141,252,61,228,15,15,217,116,127,23,128,243,65,250,185,211,196,124,21,199,149,127,110,68,235,71,43,127,165,148,78,36,204,226,99,59,0,199,143,31,83,34,50,84,179,77,224,139,200,
61,228,15,15,217,176,242,39,209,105,2,152,175,162,207,203,231,87,54,107,93,253,214,95,211,180,220,233,233,233,187,82,41,250,178,252,157,178,92,19,229,255,148,136,44,205,205,205,123,200,31,94,187,60,190,
95,254,120,60,175,56,25,196,243,203,185,138,118,120,98,96,156,228,159,27,19,17,45,162,60,165,74,255,217,182,147,157,153,153,185,179,246,47,84,8,214,170,35,255,7,200,31,94,167,60,164,242,120,222,124,243,
167,127,237,232,209,23,3,249,189,109,206,23,169,109,24,152,255,194,200,19,81,74,121,34,202,51,12,85,180,237,228,131,74,249,175,53,0,229,59,254,43,255,51,68,36,139,252,225,117,131,135,80,234,39,155,93,
126,41,136,231,247,250,245,171,201,125,251,246,127,198,25,34,27,53,3,204,127,65,230,173,173,252,11,182,157,92,158,157,157,93,168,253,249,122,223,2,208,34,242,16,249,195,235,53,47,238,185,121,243,102,62,
136,231,119,219,182,109,163,231,207,127,240,34,103,136,52,106,228,153,255,194,192,19,127,61,249,215,107,0,180,136,172,204,205,205,251,200,31,30,242,239,109,14,28,56,124,46,168,231,119,239,222,189,230,
7,31,156,249,58,103,137,212,107,2,152,255,130,207,83,74,249,142,51,180,184,158,252,107,27,0,45,34,121,228,15,15,249,247,47,65,62,191,71,142,60,255,37,103,136,212,107,2,152,255,130,207,115,156,228,253,
70,242,175,109,0,138,115,115,243,45,95,171,69,254,240,72,231,43,170,160,158,223,102,110,2,35,241,203,229,203,151,21,243,95,248,121,134,136,200,220,220,188,135,252,225,33,255,96,103,80,231,151,38,128,212,
230,235,95,127,241,253,79,63,189,100,48,255,133,155,215,246,192,70,254,240,122,181,202,141,99,54,146,44,223,230,32,97,173,93,230,211,224,242,140,118,78,56,242,135,71,162,191,242,239,198,68,79,162,159,
160,95,198,130,215,197,6,0,249,195,107,149,199,20,25,126,249,211,4,144,110,52,1,204,167,193,226,181,212,0,32,127,120,237,241,72,59,19,104,80,207,47,77,0,105,167,9,96,62,13,22,175,165,6,0,249,195,107,135,
119,235,214,45,151,169,49,252,43,255,218,120,94,113,242,157,119,222,254,38,103,138,52,211,4,48,159,6,139,151,201,100,198,211,233,244,68,83,13,0,242,135,215,46,111,120,120,211,31,49,45,70,243,252,30,62,
124,200,123,231,157,183,191,199,25,35,141,154,0,230,211,224,201,223,243,138,102,83,59,0,229,247,4,108,69,254,240,218,225,237,216,49,249,31,50,37,70,247,252,30,62,124,232,171,119,223,61,245,10,103,142,
212,107,2,152,79,131,197,155,154,154,218,94,44,22,236,53,82,19,242,31,19,17,91,74,47,9,66,254,240,154,230,221,189,123,247,225,211,79,63,123,145,233,176,249,100,179,15,142,134,177,94,18,9,243,22,103,143,
68,165,158,163,42,255,66,193,77,42,165,124,195,48,60,165,12,223,216,64,254,163,53,187,4,200,31,94,211,60,228,223,78,194,89,47,220,24,72,234,229,211,79,63,181,152,79,131,192,75,79,184,110,62,85,187,232,
55,214,145,181,33,34,155,144,63,188,78,120,76,127,157,36,124,245,66,19,64,106,115,244,232,55,222,225,137,129,131,151,127,161,144,31,22,209,85,227,115,118,118,118,193,88,71,254,195,53,242,247,144,63,188,
86,120,169,212,166,51,76,127,173,103,121,249,87,95,134,185,94,104,2,72,189,38,32,149,218,116,134,249,116,80,43,255,220,136,214,143,86,254,74,41,157,72,152,197,199,118,0,142,31,63,166,68,100,168,102,155,
192,23,145,123,200,31,94,43,60,166,189,246,114,233,210,149,103,195,94,47,52,1,164,94,120,98,96,255,121,249,252,202,102,173,171,223,250,107,154,150,59,61,61,125,87,42,69,95,150,191,83,150,117,162,252,159,
18,145,165,185,185,249,150,183,115,145,127,124,121,220,16,214,89,162,242,108,117,222,29,64,218,173,111,230,211,110,200,63,55,38,34,90,68,121,74,149,254,179,109,39,59,51,51,115,103,245,231,42,59,3,179,
206,202,31,249,195,235,57,143,116,22,158,24,72,162,180,19,192,124,218,29,158,214,122,205,239,74,137,174,149,255,90,3,80,190,227,191,114,176,106,17,121,208,15,249,159,56,113,66,175,254,199,201,11,63,143,
85,31,242,175,140,231,21,39,207,159,255,224,8,103,138,52,211,4,48,159,118,159,167,148,248,150,229,60,172,149,127,237,14,64,165,252,31,246,75,254,141,254,111,78,30,43,127,38,199,240,215,203,222,189,123,
147,151,46,93,60,196,153,37,204,247,253,230,137,111,219,201,229,217,217,217,133,122,127,199,168,35,255,149,185,185,121,191,223,242,175,253,115,78,94,248,120,172,254,89,249,175,199,155,152,152,248,218,
39,159,124,252,28,103,142,48,223,247,107,229,175,124,199,25,90,92,79,254,181,13,128,22,145,252,32,229,191,154,61,123,118,13,81,12,200,159,68,171,94,158,121,230,217,44,103,143,212,243,1,243,105,247,121,
142,147,188,223,72,254,82,43,236,185,185,249,150,39,241,110,203,95,68,228,213,87,191,245,213,205,155,215,158,217,178,101,203,40,197,16,45,30,137,167,252,43,120,138,134,145,212,230,211,79,63,181,158,125,
118,191,207,124,218,95,94,71,119,233,246,66,254,149,185,125,251,250,211,163,163,91,70,56,121,193,230,49,153,119,63,245,238,160,143,90,253,81,55,164,50,231,206,125,112,116,223,190,125,194,252,220,63,94,
219,13,64,175,229,191,154,11,23,206,59,31,124,112,222,229,228,5,151,199,68,222,251,6,32,170,245,71,237,144,102,154,95,230,231,222,240,140,118,78,80,183,191,231,223,40,7,15,30,201,243,4,41,228,31,231,68,
185,254,120,86,0,233,180,41,100,126,110,159,215,242,224,107,71,254,221,146,4,79,144,66,254,113,91,9,197,169,254,168,39,194,124,223,95,94,75,59,0,131,148,127,51,44,138,1,249,179,242,15,47,207,243,138,147,
55,110,92,221,203,153,39,204,247,189,231,181,180,3,48,104,249,111,212,25,82,12,253,229,209,0,244,62,217,236,131,163,113,173,191,155,55,111,230,15,28,56,124,142,42,32,204,247,221,231,101,50,153,113,223,
247,141,166,118,0,250,121,205,191,157,206,144,98,64,254,209,76,124,235,111,231,206,93,166,231,21,39,169,1,194,124,223,125,249,123,94,209,108,106,7,160,252,158,128,167,68,196,110,85,254,189,22,69,220,174,
145,34,255,184,237,0,44,191,68,253,81,123,132,249,190,91,188,169,169,169,237,158,87,180,148,82,190,82,134,175,154,144,255,88,89,254,137,118,86,254,189,28,180,23,46,156,119,118,237,218,125,136,98,64,254,
209,109,0,168,191,65,207,51,36,72,99,34,190,151,197,186,33,255,66,193,77,42,165,124,195,48,60,165,12,223,216,64,254,163,82,125,163,224,192,182,253,235,229,224,193,35,249,108,246,193,34,197,128,252,163,
25,234,175,217,120,94,113,50,155,125,112,52,155,93,126,41,155,93,126,233,221,119,79,125,135,250,137,94,174,92,185,194,248,104,139,151,158,112,221,124,74,106,118,253,141,117,228,111,136,200,166,32,203,
127,53,79,61,53,113,141,98,24,44,143,244,38,212,95,251,188,67,135,14,101,61,175,56,201,115,6,162,149,231,159,127,225,204,167,159,94,50,152,159,91,147,127,161,144,31,22,209,85,99,97,118,118,118,193,92,
71,254,195,53,242,247,68,228,94,208,228,191,154,84,106,228,180,231,21,39,41,134,222,240,88,253,15,38,212,95,119,120,149,77,0,181,28,254,28,61,250,141,119,106,207,43,227,163,209,202,63,55,34,162,180,82,
162,69,68,148,82,58,145,48,139,143,237,0,28,63,126,76,137,200,80,205,54,129,223,174,252,211,233,215,119,244,171,40,18,9,243,22,197,128,252,227,28,234,121,99,222,201,147,39,21,187,2,209,8,79,12,220,152,
151,207,175,108,214,186,250,173,191,166,105,185,211,211,211,119,171,26,128,178,252,157,58,242,191,223,174,252,75,31,38,152,5,193,100,137,252,145,127,124,121,52,2,241,106,2,226,41,255,220,88,245,34,95,
105,203,178,115,171,242,175,221,1,48,235,200,127,105,110,110,222,235,206,135,9,78,65,48,89,34,127,228,15,111,181,17,240,188,226,228,135,31,158,125,158,74,138,102,19,16,215,241,161,181,94,243,187,82,162,
109,219,201,206,204,204,220,169,252,89,163,188,250,79,212,200,95,139,200,131,176,201,191,153,130,96,178,68,254,200,31,94,45,239,233,167,247,57,217,236,242,75,111,190,249,211,223,162,170,162,211,4,48,62,
68,148,18,223,178,156,135,181,242,175,221,1,168,148,255,195,110,201,255,194,133,143,255,32,40,5,65,49,32,127,228,15,175,17,239,232,209,163,183,175,94,253,204,166,186,194,223,4,80,207,34,34,226,219,118,
114,121,118,118,118,161,222,223,49,234,200,127,101,110,110,222,239,214,135,57,120,240,208,63,13,66,65,80,12,200,31,249,195,107,134,119,234,212,233,2,247,7,132,187,9,160,158,69,148,82,190,227,12,45,174,
39,255,218,6,64,139,72,190,155,242,79,38,135,238,13,122,32,157,56,113,66,83,12,200,31,249,195,107,149,71,19,16,222,38,128,122,86,158,227,36,239,55,146,127,109,3,80,156,155,155,215,131,254,48,189,72,233,
43,130,76,110,200,31,249,195,107,141,71,19,16,206,240,196,192,230,120,170,31,147,81,80,4,195,179,213,17,127,152,82,43,31,100,61,56,30,99,37,124,57,123,246,189,87,246,239,127,182,64,61,247,160,1,8,235,
91,188,120,98,32,19,26,242,135,215,14,143,49,19,141,177,68,61,63,138,209,107,249,7,45,137,132,121,43,238,79,144,98,34,11,87,144,117,48,120,92,14,8,103,120,98,96,23,27,128,168,188,72,38,174,79,144,66,254,
200,31,94,103,111,29,188,116,233,226,126,42,51,154,77,64,220,234,217,232,199,63,30,212,206,57,78,79,144,58,113,226,132,70,254,200,31,94,231,188,137,137,137,39,222,123,239,221,215,168,208,104,53,1,113,
124,139,171,234,215,63,30,100,249,212,107,80,162,38,127,134,127,184,226,121,197,73,100,29,108,94,42,53,114,154,74,13,95,162,62,223,55,147,76,38,51,238,251,190,97,12,226,31,15,122,103,24,149,98,96,213,
143,252,145,117,239,120,158,87,156,164,90,153,239,195,40,127,207,43,154,34,77,92,2,72,167,211,19,165,183,10,69,83,254,181,69,17,37,249,51,212,195,25,228,26,237,109,87,194,124,63,40,222,212,212,212,246,
98,177,176,246,168,107,181,177,252,87,54,107,173,173,71,191,64,251,255,120,24,164,148,205,62,56,26,230,98,64,252,225,206,194,194,237,153,39,158,216,244,63,34,87,154,109,194,124,223,109,249,23,10,110,82,
41,229,27,134,225,41,101,248,70,35,249,187,110,110,164,122,151,160,179,15,19,134,130,56,115,230,204,14,228,79,6,149,124,126,229,31,33,87,86,254,164,63,185,116,233,83,43,30,245,156,158,112,221,124,170,
118,209,175,214,235,20,92,55,183,201,247,181,41,162,19,90,151,126,137,78,63,204,187,239,190,123,224,216,177,95,255,215,65,47,138,95,253,106,233,99,211,76,252,71,97,41,6,196,31,165,21,201,242,75,200,53,
124,60,198,96,120,19,253,39,6,166,39,10,133,252,176,136,104,165,12,79,41,229,25,134,225,189,241,198,143,110,153,245,229,159,79,105,253,168,57,40,189,85,40,121,191,211,15,243,220,115,251,79,133,161,32,
158,120,98,244,144,136,156,105,212,36,5,161,24,202,15,53,98,4,71,70,254,217,107,200,149,149,63,233,111,94,124,241,27,239,136,68,245,137,129,171,59,249,74,43,37,186,236,115,157,72,152,69,145,154,155,0,
51,153,204,120,161,224,38,69,116,229,129,232,138,252,69,68,70,70,158,216,18,182,226,104,165,179,239,87,49,164,82,155,206,148,94,112,68,162,148,27,55,174,254,14,114,69,254,36,248,115,125,88,234,185,116,
15,95,245,91,127,77,211,114,167,167,167,239,86,173,110,51,153,204,120,177,88,176,125,223,55,181,246,19,165,191,164,149,109,39,151,55,122,165,96,171,242,10,107,129,52,234,16,123,93,12,151,47,95,214,95,
255,250,209,247,24,166,81,222,1,120,112,20,185,134,151,199,101,128,104,164,153,157,128,112,200,63,55,38,34,90,68,121,74,149,254,179,109,39,59,51,51,115,103,245,231,214,58,3,207,43,154,90,235,138,109,127,
209,221,150,127,50,57,116,47,236,29,98,189,239,214,247,170,24,62,250,232,163,77,169,212,200,233,84,106,228,52,242,143,118,22,22,110,207,32,87,86,254,36,248,59,1,97,169,103,173,181,81,237,243,106,249,175,
237,0,164,211,233,9,173,125,195,247,253,132,214,58,161,181,54,44,203,254,170,219,242,143,242,221,234,111,191,253,139,255,52,159,207,125,153,72,36,114,190,239,251,223,251,222,177,127,203,80,34,205,230,
244,233,119,134,62,251,236,74,14,185,134,151,199,14,64,244,119,2,194,85,207,107,43,255,194,122,139,121,179,14,163,110,167,64,39,221,56,175,190,250,218,255,206,144,33,237,6,249,135,159,119,225,194,121,
231,224,193,35,121,170,57,58,59,1,149,77,64,56,235,89,252,70,59,249,85,55,1,42,165,180,101,217,57,228,79,72,176,131,172,131,199,219,181,107,247,33,42,51,122,77,64,88,235,185,244,237,189,161,197,70,59,
249,85,13,128,105,90,110,63,228,207,123,181,9,105,127,60,32,235,80,172,188,72,132,154,128,48,214,115,233,219,123,141,47,227,175,53,0,137,132,89,92,253,106,0,43,127,66,88,249,195,67,254,164,148,168,62,
49,80,13,114,50,226,166,25,66,154,223,1,64,174,193,231,241,138,224,232,38,138,79,12,52,131,178,18,33,4,249,35,215,176,243,168,232,232,38,138,79,12,52,218,57,16,200,159,144,254,5,185,134,135,71,181,70,
63,81,122,98,160,49,200,127,156,155,1,9,171,127,228,26,103,30,137,118,19,16,244,250,51,6,61,152,206,158,125,255,5,202,137,16,100,136,252,73,148,154,128,48,212,159,10,194,96,226,198,25,194,46,0,114,141,
2,143,27,155,25,195,97,168,231,76,38,51,238,251,190,97,4,97,48,189,255,254,187,175,82,70,132,85,4,114,101,229,79,194,158,48,200,223,243,138,102,83,59,0,233,116,122,34,159,95,121,82,107,191,167,223,131,
12,243,91,2,9,233,214,42,2,185,134,151,199,234,159,241,27,244,122,158,154,154,218,238,121,69,75,41,229,43,101,248,170,9,249,111,214,90,91,143,126,129,222,124,24,222,111,79,226,30,207,43,78,34,215,240,
242,104,0,24,191,65,151,127,161,224,38,149,82,190,97,24,158,82,134,111,52,146,191,235,230,70,164,234,70,193,222,125,24,190,17,64,226,158,123,247,238,26,200,149,109,127,18,206,156,57,243,254,120,112,235,
57,61,225,186,249,148,212,236,250,27,235,119,10,249,97,173,251,35,127,74,135,16,145,241,241,201,27,75,75,139,139,200,21,249,147,240,229,165,151,94,57,157,205,62,252,101,16,229,95,40,228,135,69,116,149,
252,103,103,103,23,140,122,242,119,221,124,74,235,71,157,130,82,202,239,199,135,97,23,128,196,61,59,118,236,190,188,184,184,248,75,228,26,46,30,219,255,68,68,228,169,167,38,174,5,111,229,159,27,169,241,
185,78,36,204,162,72,205,163,128,51,153,204,184,235,230,147,53,157,130,239,56,201,37,58,105,66,250,147,201,201,221,215,86,7,11,114,101,229,79,194,149,84,106,211,153,160,60,46,56,159,95,25,21,81,90,41,
89,125,76,181,54,77,203,93,125,241,159,81,41,255,98,177,96,87,78,58,74,73,249,125,194,253,27,76,236,2,16,210,250,170,18,89,15,142,199,234,159,116,90,19,189,145,127,110,76,170,239,225,211,150,101,231,42,
223,250,187,246,63,122,94,209,212,90,87,202,95,219,118,114,121,163,247,9,211,73,19,50,216,137,4,89,179,242,39,225,77,175,234,89,107,109,84,251,220,201,206,204,204,220,169,252,89,163,244,23,210,19,213,
8,165,45,203,121,56,40,249,179,11,64,72,115,77,0,178,30,44,143,213,63,9,122,243,174,148,248,150,229,60,172,149,191,72,253,111,1,104,219,118,178,131,94,249,211,4,16,210,120,50,65,214,172,252,9,43,255,198,
60,241,27,237,228,87,53,0,74,149,174,17,212,235,20,6,241,97,78,159,62,245,50,101,66,200,227,77,0,178,30,60,143,213,63,105,183,70,250,179,242,87,229,123,248,214,95,204,87,53,0,166,105,185,65,145,127,46,
183,178,245,224,193,131,12,48,66,106,38,20,100,205,202,159,132,183,9,232,87,61,59,78,242,254,70,59,249,107,13,64,34,97,22,43,239,14,12,202,224,188,115,231,230,62,74,136,144,71,185,115,231,182,137,172,
7,203,99,245,79,218,73,208,234,89,133,225,195,240,162,32,66,170,115,251,246,245,167,71,71,55,111,70,214,131,225,209,0,144,86,19,196,119,5,24,237,126,152,126,14,78,110,8,36,164,58,60,49,16,249,147,112,
37,136,245,220,86,3,192,53,57,66,6,159,201,201,221,215,218,121,139,38,242,71,254,164,255,89,90,250,114,57,104,245,108,132,101,112,178,11,64,72,231,82,66,254,200,159,12,38,169,212,19,255,38,104,227,195,
8,211,224,164,9,32,164,125,57,33,255,206,120,132,116,146,209,209,209,131,65,171,103,21,198,193,73,39,78,72,253,172,215,36,35,255,206,120,204,57,164,215,99,180,159,245,156,201,100,198,125,223,55,140,40,
12,78,66,200,250,162,66,254,157,241,246,236,217,53,68,101,145,65,165,23,242,247,188,162,217,212,14,64,58,157,158,200,231,87,158,212,218,183,130,52,56,233,200,9,217,120,149,129,252,59,231,181,115,163,37,
33,221,216,5,232,118,61,79,77,77,109,247,188,162,165,148,242,149,50,124,213,132,252,55,107,173,173,71,191,0,219,114,132,132,33,65,252,222,113,216,120,119,239,126,254,207,119,236,216,241,58,213,68,250,
221,0,244,66,254,133,130,155,84,74,249,134,97,120,74,25,190,209,72,254,174,155,27,145,234,247,9,7,106,112,102,179,15,142,82,74,132,212,15,79,12,236,124,229,143,252,73,47,178,209,226,181,251,245,156,158,
112,221,124,74,106,118,253,141,245,59,133,252,176,214,193,149,255,42,239,230,205,107,207,80,78,132,60,158,201,201,221,215,150,150,22,23,145,127,91,43,255,63,161,130,200,32,210,11,249,23,10,249,97,17,93,
37,255,217,217,217,5,163,158,252,93,55,159,210,250,81,167,160,148,242,131,58,216,183,108,217,50,202,91,3,9,169,159,29,59,118,95,254,213,175,150,238,33,255,150,87,254,105,170,135,68,65,254,174,155,27,169,
241,185,78,36,204,226,99,59,0,153,76,102,188,80,112,147,53,157,130,95,122,171,80,112,7,251,209,163,223,184,67,233,16,82,63,19,19,187,174,243,196,192,230,229,79,197,144,126,164,31,111,9,44,221,195,87,229,
121,109,154,150,187,250,226,63,163,82,254,197,98,193,22,169,236,20,164,252,62,225,224,15,118,30,18,68,72,107,19,14,242,71,254,36,170,43,255,215,119,228,243,185,177,234,69,190,210,150,101,231,42,223,250,
187,246,63,122,94,209,212,90,87,202,95,219,118,114,121,163,247,9,7,105,176,211,4,16,210,121,19,128,252,9,9,183,252,115,185,149,173,90,107,163,218,231,78,118,102,168,83,39,99,0,0,32,0,73,68,65,84,102,166,
106,183,220,40,253,133,244,68,53,66,105,203,114,30,134,73,254,52,1,132,116,222,4,32,127,66,250,151,3,7,158,117,122,61,62,148,18,223,178,156,135,181,242,23,169,255,45,0,109,219,78,54,140,242,167,9,32,164,
253,38,128,187,253,9,233,111,22,23,151,254,70,111,199,135,248,141,118,242,213,234,14,128,214,190,161,181,54,18,9,179,80,175,83,8,227,96,231,65,65,132,52,78,156,159,24,200,202,159,4,33,217,236,242,75,189,
89,249,27,5,199,25,250,101,163,197,188,89,245,127,84,220,29,24,165,193,78,8,89,191,73,142,227,19,3,145,63,9,86,186,63,62,74,223,222,107,188,147,191,118,9,32,145,48,139,81,147,255,201,147,39,213,219,111,
191,245,53,138,139,144,245,243,249,231,11,9,228,79,200,96,114,227,198,141,236,160,198,71,71,215,202,131,62,216,143,31,63,102,138,200,182,66,161,96,111,223,190,243,10,165,70,72,253,220,188,121,237,153,
45,91,182,140,34,127,66,250,159,65,189,34,216,104,247,23,14,139,252,69,196,180,44,203,255,226,139,133,61,148,25,33,245,179,115,231,158,207,238,223,191,127,31,249,19,18,252,116,107,124,24,131,252,199,251,
33,255,181,15,106,24,197,171,87,63,179,41,29,66,234,103,215,174,189,87,163,250,196,64,228,79,144,127,23,26,128,48,202,95,68,138,34,242,249,169,83,167,11,148,15,33,141,19,165,39,6,30,63,126,204,188,126,
253,250,223,231,172,146,40,140,185,110,143,15,53,200,193,217,79,249,207,205,205,23,219,157,228,8,137,99,54,186,46,25,134,249,96,121,57,251,207,247,237,123,250,239,112,54,9,227,173,131,6,32,74,242,167,
9,32,164,179,73,9,249,147,122,53,194,156,218,155,6,160,219,227,35,147,201,140,251,190,111,168,65,12,206,32,200,159,130,37,164,189,137,9,249,147,141,132,197,188,26,220,102,59,147,201,140,123,94,209,84,
202,240,141,141,7,103,122,162,244,86,161,232,201,191,153,109,23,66,8,242,39,213,115,230,70,243,38,243,106,48,199,219,212,212,212,246,242,91,127,69,100,131,155,0,75,242,95,217,92,249,86,161,40,201,159,
98,37,164,249,177,17,14,249,127,133,252,7,40,254,202,120,94,113,50,155,125,112,244,230,205,155,127,198,209,219,56,181,187,38,189,144,127,161,224,38,43,223,250,107,52,146,191,235,230,70,170,127,38,122,
242,175,45,86,202,144,144,48,203,127,31,242,239,113,45,180,122,126,183,108,121,226,135,111,191,253,214,175,115,20,7,55,62,74,62,207,167,164,230,190,63,99,253,78,33,63,172,117,60,228,95,249,251,101,179,
203,47,189,249,230,79,127,139,18,36,76,248,225,144,127,58,253,250,14,228,31,76,249,175,254,217,243,207,63,191,120,237,218,213,93,28,205,193,200,191,80,200,15,139,232,170,243,56,59,59,187,160,234,201,223,
117,243,41,173,117,66,107,189,250,140,112,85,122,177,64,180,229,95,249,251,165,82,155,206,80,138,4,249,135,225,45,129,249,159,165,82,169,189,156,185,193,138,191,217,243,203,13,130,235,167,251,47,230,90,
221,201,87,90,41,241,149,50,60,195,48,138,171,47,254,171,218,1,200,100,50,227,133,130,155,172,233,20,252,184,201,63,153,28,186,215,234,245,46,66,144,127,255,231,131,235,215,111,252,125,228,31,30,249,183,
203,142,75,186,61,222,74,247,240,85,237,244,235,202,183,254,26,149,242,47,223,29,184,118,114,148,18,223,113,134,22,227,38,127,138,149,32,127,182,253,227,152,159,255,252,103,169,126,188,152,134,121,117,
189,116,83,254,185,49,169,190,135,79,91,150,157,171,124,235,175,81,177,245,96,86,222,29,168,148,104,219,78,46,111,244,62,225,40,203,159,98,37,200,63,152,242,207,229,242,63,67,254,221,141,231,21,39,111,
222,188,253,85,191,206,47,243,234,227,89,90,90,92,236,214,120,171,252,246,94,201,231,78,118,102,102,230,78,229,207,26,165,191,144,158,168,70,40,109,89,206,67,228,95,93,172,20,44,65,254,193,144,63,219,
254,221,203,79,127,58,255,215,61,175,56,57,136,243,203,156,90,157,219,183,239,236,232,246,120,83,74,124,203,114,30,214,202,95,164,254,183,0,180,109,59,89,228,79,193,18,228,207,53,255,104,231,157,119,222,
126,245,208,161,3,115,131,60,191,204,169,143,146,74,13,127,175,187,227,77,252,70,59,249,85,13,128,82,165,107,4,245,58,5,228,95,93,176,158,87,156,124,239,189,119,191,65,201,18,228,223,159,249,32,159,47,
158,100,219,191,123,185,112,225,252,193,125,251,246,188,55,232,249,84,164,116,249,129,51,34,114,248,240,145,63,234,222,202,95,149,239,225,91,127,49,95,213,0,152,166,229,34,255,230,121,7,14,60,167,178,
217,229,151,174,92,185,252,191,82,186,4,249,247,86,254,59,118,76,254,30,103,174,123,242,223,186,245,201,207,130,52,159,126,241,197,194,30,206,76,235,89,239,124,148,190,189,215,120,39,127,173,1,72,36,204,
98,229,221,129,200,191,121,222,182,109,79,253,111,23,46,156,119,40,69,130,252,123,242,132,191,63,65,254,221,203,185,115,103,143,4,77,254,34,58,145,74,13,63,249,201,39,23,14,115,134,250,119,62,212,32,255,
241,40,200,159,135,92,16,228,207,202,159,149,127,247,234,37,238,15,97,107,246,158,136,110,156,15,163,221,95,18,249,215,231,241,109,1,130,252,145,63,242,111,159,199,252,217,191,243,161,6,249,143,71,77,
254,245,194,142,0,65,254,200,31,249,183,206,139,235,220,185,81,3,212,205,243,97,4,97,176,71,85,254,236,8,16,228,223,250,124,192,53,255,238,38,168,215,252,187,193,99,229,223,217,241,51,6,61,216,163,44,
255,218,73,216,243,138,147,55,110,92,221,155,205,102,175,81,202,4,249,215,95,249,239,219,183,239,239,114,230,186,183,242,223,190,125,252,147,48,206,167,113,93,56,173,183,243,209,139,230,73,13,114,176,
199,69,254,235,241,78,159,126,111,251,247,191,255,131,63,67,82,165,129,206,229,18,228,207,202,191,123,249,248,227,115,135,158,122,106,235,37,46,163,134,123,236,246,226,124,100,50,153,113,223,247,13,99,
80,131,61,238,242,23,81,222,119,190,243,221,51,113,190,60,80,123,121,132,75,37,241,149,63,219,254,221,205,185,115,103,143,68,65,254,164,55,242,247,188,162,217,212,14,64,58,157,158,200,231,87,158,212,218,
183,144,127,239,121,81,239,118,155,145,60,59,1,172,252,9,43,255,56,207,9,171,99,184,219,231,99,106,106,106,187,231,21,45,165,148,175,148,225,155,77,200,127,115,229,91,133,144,127,111,121,149,147,119,20,
138,62,159,207,127,126,234,212,219,79,47,44,220,201,182,82,252,52,1,200,159,32,127,86,254,221,149,127,161,224,38,75,242,47,77,19,102,35,249,187,110,110,68,170,223,39,140,252,251,200,171,156,204,39,38,
182,167,158,124,114,235,223,61,116,232,208,255,16,244,194,205,102,179,215,110,222,188,254,91,59,119,238,180,146,201,161,123,173,200,127,53,158,87,156,44,22,189,247,28,199,217,198,84,128,252,73,188,229,
239,121,197,201,159,253,236,167,47,29,59,246,235,255,15,242,111,231,124,164,39,92,55,255,53,165,148,95,249,231,106,189,78,193,117,115,155,124,95,155,34,58,161,117,233,151,64,254,193,225,93,185,114,37,
247,252,243,47,158,15,210,74,255,214,173,235,223,31,31,31,31,238,230,231,93,90,90,254,7,59,118,76,254,46,211,123,180,228,191,188,252,213,159,112,183,127,247,114,238,220,217,35,97,189,219,191,21,94,42,
53,114,58,14,231,51,155,125,112,180,155,242,47,20,242,195,34,162,149,50,60,165,148,103,24,134,247,198,27,63,186,165,234,203,63,159,210,122,85,252,58,33,34,170,244,98,1,228,31,100,222,32,182,205,61,175,
56,217,235,207,107,154,206,159,179,19,192,202,159,196,111,229,95,203,59,123,246,131,173,223,254,246,119,255,34,250,13,192,242,75,221,91,249,231,70,68,148,86,74,124,165,12,207,48,140,162,105,90,238,244,
244,244,221,170,6,32,147,201,140,187,110,126,72,107,223,212,90,140,114,3,160,28,39,185,132,252,195,205,91,109,14,230,231,255,221,191,191,115,231,238,191,241,244,211,79,255,237,78,68,211,239,207,155,72,
152,183,152,234,145,63,137,175,252,87,121,81,127,87,192,210,210,210,199,150,101,252,205,110,28,191,124,62,55,90,146,191,242,202,13,64,209,182,157,149,213,23,255,153,149,242,47,22,11,182,84,92,22,80,74,
124,219,30,90,222,232,149,130,200,63,28,215,208,114,185,149,173,47,191,252,82,66,68,254,101,54,187,252,47,67,246,121,185,49,16,249,147,152,203,63,153,28,186,23,131,83,107,116,73,254,99,34,162,69,196,43,
243,180,101,217,185,202,183,254,26,21,130,48,181,214,149,242,215,182,157,68,254,240,2,195,227,57,1,225,149,63,223,243,239,110,162,252,61,255,141,120,81,159,7,70,71,71,15,116,227,248,85,126,123,175,228,
115,39,59,51,51,115,231,177,78,35,157,78,79,84,35,148,182,44,231,33,242,135,23,52,30,77,0,143,247,101,229,127,238,80,28,110,248,235,6,47,172,233,230,241,83,74,124,203,114,30,214,202,191,106,7,160,34,218,
182,157,44,242,135,23,84,94,220,159,158,24,54,249,179,242,239,174,252,227,186,242,143,211,121,238,222,241,19,191,209,78,126,85,3,160,84,233,26,65,189,78,1,249,195,11,218,91,22,145,127,224,183,253,255,
39,228,223,189,196,121,219,159,241,223,206,202,95,249,142,51,180,216,104,49,95,213,0,152,166,229,34,127,120,97,225,197,105,18,8,163,252,247,237,219,247,247,152,154,187,39,127,182,253,73,43,199,175,244,
213,253,198,59,249,107,13,64,34,97,22,43,239,14,68,254,240,194,192,243,188,226,228,194,194,194,52,242,15,220,53,127,228,223,165,112,205,159,5,64,175,206,135,41,34,210,206,245,126,228,15,47,40,188,39,158,
24,254,103,31,125,116,254,227,195,135,143,252,16,249,15,94,254,108,251,119,87,254,108,251,147,94,29,63,163,221,127,28,249,195,11,18,111,207,158,93,127,30,181,157,0,228,143,252,145,63,233,229,241,107,171,
1,64,254,240,130,200,219,182,237,169,255,18,249,15,70,254,220,240,215,221,112,195,95,235,227,4,249,183,126,252,140,118,6,59,242,135,23,84,94,20,38,4,174,249,179,242,231,154,127,243,188,159,254,116,254,
175,35,255,246,142,159,209,234,96,71,254,240,130,206,11,115,19,192,202,159,149,63,43,255,214,120,7,14,60,123,1,249,183,119,217,196,68,254,240,162,200,59,121,242,100,232,222,29,16,210,149,63,242,239,238,
202,31,249,183,200,75,165,134,159,68,254,173,37,147,201,140,251,190,111,24,205,14,118,228,15,47,108,188,48,237,4,176,242,103,229,207,202,191,125,222,252,252,191,251,15,144,127,243,242,247,188,162,41,210,
196,37,128,227,199,143,37,68,100,43,242,135,23,70,94,24,154,0,174,249,179,242,231,154,127,103,188,109,219,158,250,11,228,191,113,166,166,166,182,151,223,250,43,27,54,0,101,249,143,137,72,2,249,195,11,
43,47,200,77,0,95,245,67,254,172,252,59,231,125,240,193,121,23,249,111,44,255,66,193,77,86,190,245,215,216,64,254,163,53,63,131,252,225,133,146,23,196,39,6,34,127,228,143,252,7,195,139,155,252,211,233,
244,132,235,230,83,34,82,181,24,50,214,25,236,134,136,108,66,254,240,162,196,43,63,49,240,31,35,255,214,229,255,249,231,191,252,111,144,127,247,194,53,127,228,191,94,174,94,253,204,238,182,252,11,133,
252,176,136,174,146,255,236,236,236,130,177,142,252,135,107,228,239,33,127,120,81,224,5,225,137,129,97,148,255,225,195,71,254,9,218,238,158,252,185,230,223,125,94,84,30,10,244,212,83,219,14,119,119,229,
159,27,209,250,209,202,95,41,165,19,9,179,248,216,14,192,241,227,199,148,136,12,213,108,19,248,34,114,143,98,133,23,21,222,32,159,24,24,198,109,127,228,223,189,112,195,31,43,255,141,211,189,227,151,207,
175,108,214,186,202,243,218,52,45,119,245,197,127,102,141,252,157,58,242,95,154,155,155,247,40,46,120,17,227,245,253,57,1,225,147,191,247,175,217,246,239,174,252,217,246,71,254,205,167,83,249,231,198,
68,68,75,105,7,95,68,148,182,44,59,87,249,214,223,202,206,192,68,254,240,226,196,235,231,150,97,56,175,249,239,120,29,109,119,39,92,243,71,254,253,148,127,46,183,178,85,107,189,230,119,165,68,219,182,
147,157,153,153,185,83,249,179,70,121,192,39,106,228,175,69,228,1,242,135,23,117,94,63,154,128,48,174,252,217,246,239,238,202,159,109,255,254,240,162,113,31,64,119,143,159,82,226,91,150,243,176,86,254,
181,59,0,149,242,127,136,252,225,197,133,215,203,73,35,156,219,254,172,252,187,41,127,86,254,172,252,91,73,119,143,159,248,182,157,92,158,157,157,93,168,247,119,140,58,242,95,153,155,155,247,41,46,120,
172,28,144,63,65,254,200,191,191,233,222,202,95,249,142,51,180,184,158,252,107,27,0,45,34,121,228,15,47,174,188,110,54,1,33,188,230,255,7,200,191,123,225,154,63,242,239,87,214,59,126,142,147,188,223,72,
254,181,13,64,113,110,110,94,83,92,240,226,204,235,198,19,3,67,122,205,255,143,152,74,187,183,242,231,154,255,224,120,63,255,249,207,126,61,238,242,111,246,248,169,65,254,227,240,224,5,149,119,237,218,
141,127,239,240,225,35,63,140,131,252,89,249,119,87,254,172,252,7,203,91,94,94,250,213,246,237,59,175,132,181,134,154,221,137,236,198,241,51,218,253,37,41,86,120,81,230,237,217,179,235,207,191,250,42,
123,165,221,129,139,252,145,63,227,109,48,188,145,145,39,182,176,242,111,46,38,242,135,7,111,189,107,104,206,247,68,228,86,171,29,123,88,174,249,31,62,124,4,249,119,41,31,126,120,246,249,137,137,113,228,
31,24,94,116,87,255,221,60,126,38,197,5,15,94,227,39,6,70,233,243,86,92,243,71,254,93,92,249,35,255,96,241,88,249,55,23,131,226,130,7,47,30,60,238,246,239,205,202,159,109,255,224,241,144,127,151,119,0,
40,46,120,240,88,249,19,86,254,97,224,157,56,113,2,249,55,72,38,147,25,247,125,223,48,40,46,120,240,226,33,127,86,254,221,149,63,43,255,104,240,226,40,127,207,43,154,34,77,92,2,72,167,211,19,165,183,10,
81,92,240,224,33,127,130,252,131,207,139,198,59,1,186,127,252,166,166,166,182,23,139,5,123,245,255,54,54,150,255,202,230,202,183,10,81,92,240,224,33,127,228,143,252,89,249,135,79,254,133,130,155,212,90,
171,13,27,128,116,58,61,225,186,185,145,234,159,161,184,224,193,67,254,200,159,250,67,254,97,146,127,201,231,249,148,212,124,171,201,88,191,83,200,15,107,141,252,225,193,11,163,252,63,255,252,254,127,
141,252,187,23,238,246,71,254,189,206,234,101,139,94,200,191,80,200,15,139,232,42,249,207,206,206,46,152,245,228,239,186,249,148,214,143,58,133,210,91,133,146,247,41,46,120,240,194,33,255,195,135,15,255,
49,218,238,158,252,39,38,198,47,34,127,228,31,206,149,127,110,68,68,105,165,68,151,125,174,19,9,179,40,181,219,1,153,76,102,220,117,243,67,90,251,166,214,98,104,173,19,34,90,57,78,114,137,226,130,7,143,
109,255,184,133,109,255,240,242,78,156,56,161,195,84,107,158,87,156,236,246,241,203,231,115,163,37,249,43,79,41,241,149,50,138,182,237,172,76,79,79,223,21,169,120,14,64,38,147,25,47,223,29,88,177,242,
23,223,182,135,150,55,122,165,32,197,10,15,30,242,71,254,212,51,43,255,246,211,3,249,143,137,136,22,145,242,147,17,149,182,44,59,183,42,127,145,138,123,0,60,175,104,86,222,29,168,148,104,219,78,34,127,
120,240,144,63,242,167,254,144,127,15,115,251,246,237,55,186,125,252,42,191,189,87,242,185,147,157,153,153,185,83,249,179,70,233,47,164,39,170,17,74,91,150,243,16,249,195,131,135,252,145,63,245,135,252,
123,155,155,55,111,252,139,94,29,63,165,196,183,44,231,97,173,252,171,118,0,42,162,109,219,201,34,127,120,240,144,63,242,167,254,144,127,239,179,115,231,196,141,222,28,63,241,27,237,228,87,53,0,74,149,
174,17,212,235,20,40,46,120,240,248,170,95,148,195,87,253,162,197,187,113,227,234,222,176,212,222,232,232,230,205,221,95,249,43,223,113,134,22,27,45,230,171,190,6,104,154,150,91,121,131,0,197,5,15,94,
112,87,254,135,15,31,70,254,93,92,249,243,98,159,104,241,198,198,198,18,225,169,192,238,31,191,210,87,247,27,239,228,175,237,0,36,18,102,17,249,195,131,199,182,127,28,229,207,202,63,202,188,224,103,80,
199,175,163,23,38,80,92,240,224,33,127,228,79,61,7,149,151,74,141,156,14,67,29,182,243,242,162,110,28,63,163,221,95,152,226,130,7,15,249,35,127,234,57,200,188,168,214,110,183,142,159,49,200,127,28,30,
60,120,200,31,249,195,235,21,47,138,171,255,110,30,63,131,226,130,7,47,240,119,251,255,87,200,191,123,225,110,127,120,113,95,249,175,198,164,24,224,193,227,110,255,56,173,252,185,219,31,30,242,111,177,
1,160,184,224,193,235,191,252,89,249,119,87,254,172,252,225,33,255,210,187,127,124,223,55,12,138,1,30,60,228,143,252,169,63,228,31,31,249,123,94,209,20,105,226,30,128,116,58,61,81,122,171,16,197,5,15,
30,242,71,254,212,51,242,15,171,252,167,166,166,182,151,223,250,43,27,54,0,37,249,175,108,174,124,171,16,197,5,15,30,242,71,254,212,51,242,239,60,141,190,1,208,11,249,23,10,110,178,242,173,191,70,35,249,
187,110,110,164,250,103,40,46,120,240,122,37,127,238,246,239,110,184,219,31,30,43,255,74,159,231,83,82,243,240,63,115,189,78,193,117,115,195,90,35,127,120,240,250,181,242,231,110,255,238,174,252,185,219,
31,30,242,47,201,191,80,200,15,139,136,174,244,255,236,236,236,130,89,95,254,249,148,214,143,126,178,244,86,161,228,125,138,11,30,188,158,200,255,255,98,229,223,93,249,179,242,135,135,252,43,119,242,149,
86,74,116,217,231,58,145,48,139,34,53,151,0,50,153,204,120,161,224,38,69,116,229,54,1,242,135,7,175,183,242,79,163,109,228,15,15,249,119,251,248,149,238,225,171,242,188,174,124,235,175,89,41,255,242,221,
129,21,43,127,241,109,123,104,121,163,87,10,82,92,240,226,204,75,36,204,91,39,78,156,88,251,211,102,30,237,185,122,205,255,240,225,195,200,191,75,249,240,195,179,207,179,237,15,47,44,169,156,39,122,35,
255,220,152,136,104,17,41,191,19,65,105,203,178,115,149,111,253,93,107,0,60,175,104,86,222,29,168,148,104,203,74,62,64,254,240,224,61,226,93,190,124,89,111,222,60,246,195,157,59,119,253,214,122,63,127,
226,196,9,221,104,208,167,211,175,239,184,120,241,147,191,121,248,240,225,255,30,109,119,85,254,23,195,34,255,202,26,217,168,97,100,252,182,199,107,52,14,163,190,242,47,241,68,148,42,189,16,73,41,209,
182,237,100,103,102,102,238,84,254,172,42,253,133,244,132,214,190,225,251,126,66,107,157,208,90,27,150,101,127,133,252,225,197,157,119,252,248,49,211,117,221,137,175,190,114,255,209,51,207,60,243,159,
160,90,228,223,77,249,111,212,4,48,126,219,231,5,189,1,88,93,12,244,238,248,41,79,41,229,41,165,10,182,157,172,187,147,95,239,107,128,218,182,157,44,242,135,7,239,245,29,249,124,241,228,196,196,174,235,
200,31,249,247,74,254,235,253,57,227,183,51,94,124,87,254,149,60,241,215,147,255,99,13,128,82,165,107,4,181,219,4,20,23,188,184,241,78,156,56,161,19,9,243,214,142,29,147,191,135,102,145,127,47,229,95,
175,9,96,252,118,206,59,117,234,23,223,10,106,237,122,94,113,178,215,199,175,244,237,189,161,197,70,139,249,170,175,1,86,222,29,72,113,193,139,179,252,209,43,242,239,167,252,43,107,175,31,114,136,3,175,
80,40,108,9,106,253,246,227,248,149,190,189,215,120,39,127,109,7,32,145,48,139,200,31,30,242,71,254,65,79,249,33,63,145,147,255,106,110,223,190,105,49,126,59,231,37,18,137,61,193,173,226,96,28,63,213,
201,71,64,54,240,144,63,97,229,223,61,249,175,230,230,205,107,207,108,217,178,101,148,241,219,62,175,80,240,255,207,209,209,209,131,65,171,225,183,222,250,249,95,123,225,133,231,23,131,112,252,140,118,
63,4,178,129,135,252,9,242,239,190,252,69,68,118,238,220,243,217,253,251,247,239,51,126,219,231,93,190,124,233,63,14,98,29,63,243,204,222,75,65,57,126,38,242,135,135,252,145,127,208,19,182,103,251,119,
163,166,118,237,218,123,117,21,206,248,109,157,247,226,139,71,239,5,177,150,83,169,225,39,131,114,252,140,184,20,3,60,120,200,63,188,43,255,48,61,222,183,219,53,213,10,143,249,160,154,215,204,83,57,251,
159,224,28,63,35,78,197,0,15,30,242,15,229,202,255,98,92,229,223,74,173,50,31,212,231,5,169,9,120,235,173,55,191,31,164,231,36,168,184,21,3,60,120,200,63,60,242,143,243,202,191,94,120,98,96,251,188,32,
140,121,207,43,78,6,225,248,101,50,153,113,223,247,13,21,215,98,128,135,252,9,242,15,147,252,215,107,2,152,15,154,231,13,122,236,183,179,27,209,11,249,123,94,209,84,202,240,141,141,255,241,244,68,233,
173,66,20,23,60,228,79,144,255,32,229,95,251,239,49,31,180,198,11,230,61,1,253,59,126,83,83,83,219,203,111,253,45,145,54,150,255,202,102,173,117,197,131,41,40,46,120,200,159,32,255,65,215,20,79,12,108,
159,55,168,243,214,74,3,210,11,249,23,10,110,82,41,229,27,134,225,53,220,1,72,167,211,19,174,155,27,145,170,27,5,41,46,120,200,159,32,255,32,212,20,79,12,108,159,231,121,197,201,133,133,27,187,227,34,
255,146,207,243,169,218,69,191,177,126,167,144,31,214,26,249,195,67,254,164,247,137,251,87,253,218,201,174,93,123,175,126,249,229,151,75,204,7,237,241,158,120,98,116,235,205,155,215,158,9,218,88,232,133,
252,11,133,252,176,136,174,146,255,236,236,236,130,81,79,254,174,155,79,105,253,168,83,80,74,249,20,23,60,228,79,122,181,242,231,171,126,237,133,39,6,118,198,219,178,101,203,232,221,187,183,118,69,89,
254,174,155,27,169,241,185,78,36,204,162,72,205,147,0,51,153,204,184,235,230,147,53,157,130,239,56,201,37,138,11,30,242,39,189,144,63,43,255,206,119,2,86,127,89,230,131,214,121,91,183,142,251,209,148,
255,235,59,242,249,149,81,17,165,149,18,175,252,199,186,242,173,191,70,165,252,203,119,7,86,116,10,82,126,159,48,178,129,135,252,9,242,15,242,241,228,137,129,237,243,122,253,237,128,141,248,189,145,127,
110,76,170,239,225,211,150,101,231,42,223,250,187,246,63,122,94,209,212,90,87,202,95,219,118,114,121,163,247,9,83,92,240,144,63,65,254,225,105,2,152,95,234,243,78,158,60,169,6,241,53,193,94,125,94,173,
181,81,237,115,39,59,51,51,115,167,242,103,141,210,95,72,79,84,35,148,182,44,231,33,242,135,135,252,9,242,15,87,77,53,250,125,153,95,54,230,245,179,9,232,199,231,85,74,124,203,114,30,214,202,95,164,254,
183,0,180,109,59,89,228,15,15,249,19,228,31,206,154,170,247,123,51,191,52,207,235,71,19,208,159,207,43,126,163,157,252,170,6,64,169,210,53,130,122,157,2,178,129,135,252,9,242,15,103,19,192,252,210,58,
175,91,77,64,61,78,127,86,254,170,124,15,223,250,139,249,170,6,192,52,45,23,249,195,67,254,164,219,225,123,254,131,107,2,152,95,218,231,245,98,39,160,95,159,215,113,146,247,55,218,201,95,107,0,18,9,179,
88,121,119,32,197,0,15,249,147,110,173,252,249,158,255,224,114,231,206,109,147,249,165,125,94,54,251,224,104,216,228,223,44,79,69,233,195,192,67,254,36,120,242,103,229,63,248,220,190,125,253,233,209,209,
205,155,153,95,218,231,125,249,229,175,254,241,206,157,59,127,187,221,29,132,32,126,94,163,221,130,66,94,240,144,63,65,254,225,200,142,29,187,47,47,46,46,254,146,249,165,125,222,150,45,163,127,24,53,95,
26,81,250,48,240,144,63,65,254,200,191,126,38,39,119,95,75,36,204,91,204,47,237,243,218,185,39,32,200,159,87,81,12,240,144,63,65,254,241,74,179,34,99,190,170,207,107,166,102,78,158,60,169,130,254,121,
121,118,52,60,228,79,186,150,15,63,60,251,60,55,252,69,163,9,96,190,106,204,219,168,118,60,175,56,25,228,207,219,82,3,64,49,192,67,254,132,149,127,60,154,0,230,171,230,120,141,106,168,244,237,129,96,126,
222,76,38,51,238,251,190,97,196,249,228,193,67,254,4,249,199,57,60,49,176,55,79,12,12,186,252,61,175,104,138,52,113,19,96,58,157,158,40,189,85,136,98,128,135,252,9,242,143,114,19,192,124,213,58,175,126,
19,16,204,207,59,53,53,181,189,252,214,223,18,105,99,249,175,108,214,90,91,143,126,1,138,1,30,242,39,200,63,106,9,250,53,235,176,204,87,111,189,245,230,111,190,240,194,215,239,7,81,254,133,130,155,84,
74,249,134,97,120,74,25,190,217,72,254,174,155,27,169,222,37,160,24,224,33,127,82,74,249,134,63,228,31,145,220,185,115,219,220,188,121,51,243,85,7,79,12,252,224,131,15,159,124,225,133,175,47,6,239,247,
75,79,184,110,254,107,74,41,191,242,207,213,122,157,130,235,230,54,249,190,54,69,116,66,235,210,47,65,49,192,67,254,132,149,127,116,195,19,3,163,200,75,79,20,10,249,97,17,209,74,25,158,82,202,51,12,195,
123,227,141,31,221,50,234,203,63,159,210,250,81,115,160,148,242,57,216,240,144,63,65,254,209,14,79,12,140,158,252,93,55,55,82,227,115,157,72,152,69,17,145,170,75,0,153,76,102,220,117,243,73,17,93,185,
51,224,59,78,114,137,131,13,15,249,19,228,31,253,76,78,238,190,182,122,48,153,175,194,205,203,231,87,70,69,148,86,74,188,242,31,107,211,180,220,213,23,255,25,149,242,47,223,29,88,209,41,72,249,125,194,
28,108,120,200,31,249,35,255,56,165,149,227,199,252,23,68,249,231,198,164,250,30,62,109,89,118,174,242,173,191,107,255,163,231,21,77,173,117,165,252,181,109,39,151,55,122,159,48,7,27,30,242,71,254,200,
63,190,77,0,243,95,48,121,90,107,163,218,231,78,118,102,102,230,78,229,207,26,165,191,144,158,168,70,40,109,89,206,67,228,15,15,249,19,228,79,19,192,124,21,94,158,82,226,91,150,243,176,86,254,34,245,31,
4,164,109,219,201,34,127,120,200,159,32,127,178,222,113,101,254,11,3,79,252,70,59,249,85,13,128,82,165,107,4,245,58,5,14,54,60,228,31,175,100,179,217,171,207,61,119,112,25,249,147,218,227,203,252,23,134,
149,191,42,223,195,183,254,98,94,149,254,114,122,66,107,223,72,36,204,98,229,13,2,28,108,120,200,63,222,9,203,251,207,169,169,254,133,39,6,70,135,183,182,3,128,252,225,33,127,210,105,144,127,244,115,231,
206,109,147,249,47,26,60,21,182,193,14,15,249,147,96,238,2,32,255,248,132,39,6,70,131,103,180,91,0,28,108,120,200,159,32,255,120,134,39,6,70,131,103,34,127,120,200,159,32,127,210,106,120,98,96,248,121,
6,7,7,30,242,39,200,159,180,27,158,24,24,94,158,193,193,129,135,252,9,242,39,189,110,2,152,79,131,197,107,105,235,134,131,13,15,249,199,51,245,110,2,68,254,164,217,90,97,62,13,30,47,147,201,140,251,190,
111,24,28,28,120,200,159,176,242,39,189,218,9,96,62,13,158,252,61,175,104,138,52,113,9,32,157,78,79,148,222,42,196,193,134,135,252,145,63,242,39,205,207,1,204,167,193,226,77,77,77,109,47,191,245,87,68,
54,248,22,64,73,254,43,155,43,223,42,196,193,134,135,252,145,63,242,39,27,205,5,60,49,48,120,242,47,20,220,164,82,202,87,170,116,165,198,104,36,127,215,205,141,84,255,12,7,27,30,242,71,254,193,171,63,
18,188,240,196,192,32,241,210,19,174,155,79,73,205,125,127,198,250,157,66,126,88,107,228,15,15,249,35,127,228,79,90,207,228,228,238,107,75,75,139,139,204,167,131,151,127,161,144,31,22,209,85,242,159,157,
157,93,48,234,201,223,117,243,41,173,31,117,10,74,41,159,131,13,15,249,199,51,200,159,180,27,158,24,24,132,149,127,110,164,198,231,58,145,48,139,143,237,0,100,50,153,241,66,193,77,214,116,10,190,227,36,
239,115,176,225,33,255,248,229,246,237,219,179,200,159,116,186,19,144,72,152,183,152,79,251,207,43,221,195,87,229,121,109,154,150,187,250,226,63,163,82,254,229,187,3,43,58,5,41,191,79,152,131,13,15,249,
199,49,139,139,191,252,99,228,79,186,17,158,24,216,111,249,231,198,170,23,249,74,91,150,157,171,124,235,239,218,255,232,121,69,83,107,93,41,127,109,219,201,229,217,217,217,5,14,54,60,228,31,207,236,221,
187,215,68,254,164,159,77,0,243,115,119,120,149,223,222,43,249,220,201,206,204,204,220,169,252,89,163,244,23,210,19,213,8,165,45,203,121,136,252,225,33,127,130,252,73,191,154,0,230,231,238,243,148,18,
223,178,156,135,181,242,23,169,255,45,0,109,219,78,22,249,195,67,254,241,206,197,139,31,61,143,252,73,191,154,0,230,231,94,240,196,111,180,147,95,213,0,40,85,186,70,80,175,83,224,96,35,127,166,173,120,
101,231,206,93,230,32,235,143,154,139,79,19,192,252,220,139,149,191,42,223,195,183,254,98,94,149,254,114,122,66,107,223,72,36,204,98,229,13,2,28,108,120,76,196,241,204,133,11,231,15,62,251,236,129,7,131,
174,63,106,47,250,225,137,129,131,227,173,237,0,32,127,120,76,192,100,53,65,144,255,170,28,62,250,232,163,127,200,25,137,110,120,98,224,224,120,170,147,19,199,193,70,254,36,122,185,122,245,51,251,212,
169,211,133,32,213,243,59,239,188,123,224,7,63,248,141,63,229,236,68,51,183,111,95,127,122,116,116,243,102,230,231,254,242,218,110,0,56,216,200,159,68,47,215,174,93,221,245,246,219,167,110,6,177,158,175,
93,187,249,155,135,15,31,254,99,206,82,52,115,235,214,245,61,219,183,239,40,50,63,247,143,103,180,115,162,56,216,200,159,68,51,65,149,191,136,242,14,28,120,238,95,113,134,162,27,158,24,216,127,158,226,
224,192,67,254,68,68,228,228,201,147,161,152,15,168,81,106,145,249,190,59,60,197,193,129,135,252,73,88,228,79,173,82,147,204,247,221,123,174,134,226,224,192,99,66,101,162,13,99,61,83,179,241,173,77,230,
251,206,120,153,76,102,220,247,125,195,224,224,192,99,34,101,130,13,99,61,183,243,187,147,112,133,39,6,246,70,254,158,87,52,69,154,184,9,48,157,78,79,148,222,42,196,193,70,254,4,249,7,171,158,105,2,226,
213,4,48,223,119,198,155,154,154,218,94,126,235,111,137,180,177,252,87,54,107,173,173,71,191,0,7,27,249,19,228,31,172,122,166,150,163,31,158,24,216,185,252,11,5,55,169,148,242,13,195,240,148,50,124,163,
145,252,93,55,55,82,189,75,192,193,70,254,4,249,7,175,158,121,98,96,244,195,19,3,59,225,165,39,92,55,159,170,93,244,27,235,119,10,249,97,173,145,63,242,39,200,63,28,227,99,207,158,157,127,249,227,31,255,
213,239,115,150,163,153,201,201,221,215,150,150,22,23,153,239,91,151,127,161,144,31,22,209,85,99,127,118,118,118,193,168,39,127,215,205,167,180,126,212,41,40,165,124,14,54,242,39,200,63,232,227,227,149,
87,94,190,120,251,246,237,89,206,118,52,179,99,199,238,203,55,110,220,200,50,223,183,178,242,207,141,212,248,92,39,18,102,81,106,183,3,50,153,204,184,235,230,135,180,246,77,173,197,208,90,39,68,180,114,
156,228,18,7,27,249,19,228,31,150,241,209,206,19,229,72,180,107,58,142,254,200,231,115,163,34,74,43,165,60,165,196,87,202,40,218,182,179,178,250,226,63,85,41,255,98,177,96,251,190,111,106,237,39,74,219,
255,90,217,118,114,185,209,251,132,57,216,200,159,32,255,32,242,168,121,234,59,230,242,31,19,17,45,162,188,82,3,160,60,219,118,178,51,51,51,119,86,127,110,237,18,128,231,21,77,173,117,197,54,129,104,228,
143,252,9,242,15,235,248,224,43,130,241,200,70,243,91,92,253,161,181,54,170,125,94,45,255,181,6,32,157,78,79,84,35,148,182,44,231,33,242,71,254,4,249,135,121,124,208,4,196,187,9,192,31,34,74,137,111,89,
206,195,90,249,87,237,0,84,68,219,182,147,69,254,200,159,32,255,40,140,15,154,128,248,52,1,60,52,168,150,39,126,163,157,124,181,186,3,160,181,111,104,173,141,68,194,44,212,235,20,144,43,242,39,200,63,
204,60,198,68,124,194,67,131,68,148,50,10,142,51,244,203,70,139,249,170,29,0,211,180,92,228,143,252,9,242,143,226,248,96,39,32,62,185,126,253,198,31,196,221,31,142,147,188,191,209,78,254,90,3,144,72,152,
197,213,175,6,48,121,32,127,130,252,163,56,62,120,98,96,60,178,111,223,190,191,151,74,141,156,254,242,203,47,151,240,199,250,233,168,35,70,214,200,159,32,255,48,242,222,121,231,221,3,63,248,193,111,252,
41,149,195,248,136,243,248,104,187,1,64,214,200,159,48,185,133,153,119,237,218,205,223,60,124,248,240,31,83,65,140,147,184,142,143,182,26,0,38,15,228,79,144,127,20,120,60,49,144,49,19,231,241,193,228,
129,252,9,242,143,245,120,99,12,49,118,226,58,62,120,150,50,242,39,200,63,246,227,141,177,196,24,138,219,248,104,169,1,96,242,64,254,4,249,71,153,199,152,138,239,120,138,219,248,200,100,50,227,190,239,
27,138,201,3,249,19,228,15,143,177,21,231,100,179,15,142,198,73,254,158,87,52,149,50,124,99,227,127,60,61,81,122,171,16,147,7,242,39,200,63,218,60,30,22,20,207,44,46,46,253,48,14,227,99,106,106,106,123,
177,88,176,215,72,27,203,127,101,179,214,218,122,244,11,48,121,32,127,130,252,163,205,99,172,197,51,151,46,93,220,191,111,223,51,43,81,149,127,161,224,38,149,82,190,97,24,94,195,29,128,116,58,61,225,186,
185,17,169,122,92,48,147,7,242,39,200,63,250,60,158,24,24,207,236,223,127,224,82,59,95,13,13,254,248,72,79,184,110,62,85,187,232,55,214,239,20,242,195,90,35,127,228,79,144,127,60,121,123,246,236,252,203,
31,255,248,175,126,159,42,140,95,90,153,115,195,32,255,66,33,63,44,162,171,230,146,217,217,217,5,163,158,252,93,55,159,210,250,81,167,160,148,242,153,60,144,63,65,254,113,227,189,242,202,203,23,217,9,
136,111,19,176,209,252,27,142,149,127,110,164,198,231,58,145,48,139,82,187,29,144,201,100,198,93,55,63,164,181,111,106,45,134,214,58,33,162,149,227,36,151,152,60,144,63,65,254,60,49,144,48,249,245,199,
254,0,0,19,115,73,68,65,84,30,195,83,207,249,124,110,84,68,105,165,148,167,148,248,74,25,69,219,118,86,86,95,252,103,84,202,191,124,119,96,69,167,32,190,227,12,45,50,121,32,127,130,252,227,204,227,219,
1,236,6,132,80,254,99,82,125,15,159,182,44,59,87,249,214,223,181,162,126,253,245,223,155,244,125,63,161,181,78,104,237,39,68,68,89,150,243,112,163,247,9,51,121,32,127,130,252,227,194,99,108,18,207,43,
78,134,163,158,69,148,82,158,136,242,12,67,21,109,59,249,96,102,102,230,78,229,207,26,165,191,144,158,168,70,40,141,252,145,63,65,254,240,58,63,71,36,90,249,249,207,127,126,56,76,245,172,148,248,150,229,
60,172,149,191,72,253,111,1,104,219,118,178,200,31,249,19,228,15,175,59,231,138,68,39,223,251,222,247,255,50,149,26,57,29,142,122,22,223,182,147,203,235,249,188,170,1,80,170,116,141,160,94,167,192,228,
129,252,9,242,135,71,19,64,74,73,165,70,78,167,82,155,206,4,119,229,175,202,247,240,173,191,152,175,106,0,76,211,114,145,63,242,39,200,31,94,111,206,29,137,94,90,157,191,251,85,207,142,147,188,191,209,
78,254,90,3,144,72,152,197,202,187,3,25,236,200,159,32,127,120,141,195,19,3,201,234,60,222,204,92,30,180,122,238,168,131,101,242,64,254,4,249,195,211,137,171,87,175,255,206,145,35,207,255,33,149,77,214,
27,219,65,172,231,182,27,0,38,15,228,79,144,63,188,71,188,207,63,191,247,183,246,237,123,250,239,80,225,173,229,212,169,95,124,255,200,145,195,217,141,206,71,152,230,204,218,49,30,212,122,110,171,1,96,
176,35,127,130,252,225,241,196,192,118,243,241,199,231,14,237,222,189,251,107,157,156,143,160,207,163,171,99,61,200,245,204,100,132,252,9,242,135,215,69,30,99,124,253,92,184,112,222,217,181,107,247,161,
110,158,143,115,231,62,28,123,241,197,95,251,32,168,159,57,155,125,112,52,168,245,108,48,216,145,63,65,254,240,186,199,227,219,1,245,227,121,197,201,110,203,63,151,91,217,186,127,255,51,86,54,187,252,
210,219,111,191,245,27,65,252,220,239,191,255,254,174,32,214,115,75,59,0,12,118,228,79,144,63,188,230,121,140,249,71,245,223,175,243,241,197,23,159,127,180,119,239,51,110,16,143,195,221,187,183,118,109,
221,58,238,7,161,158,51,153,204,184,239,251,134,193,96,71,254,4,249,195,235,62,143,157,128,254,202,63,153,28,186,119,234,212,233,66,80,143,251,248,248,228,141,118,238,17,233,133,252,61,175,104,138,52,
113,9,32,157,78,79,148,222,42,196,96,71,254,4,249,195,227,137,129,193,148,127,88,46,195,180,226,132,110,31,191,169,169,169,237,229,183,254,150,72,27,203,127,101,179,214,218,122,244,11,48,216,145,63,65,
254,240,152,11,130,41,255,48,29,247,70,243,68,47,228,95,40,184,73,165,148,111,24,134,167,148,225,27,141,228,239,186,185,145,234,93,2,6,59,3,158,32,127,120,173,242,60,175,56,121,229,202,229,127,129,252,
185,12,211,140,39,186,127,252,210,19,174,155,79,213,46,250,141,245,59,133,252,176,214,200,31,249,19,228,15,175,27,188,109,219,182,254,31,63,254,241,95,253,62,242,239,239,249,8,219,101,152,94,200,191,80,
200,15,139,232,170,227,48,59,59,187,96,212,147,191,235,230,83,90,63,234,20,148,82,62,131,29,249,19,228,15,175,51,222,43,175,188,124,49,170,59,1,65,148,127,216,210,155,149,127,110,164,198,231,58,145,48,
139,82,187,29,144,201,100,198,93,55,63,164,181,111,106,45,134,214,58,33,162,149,227,36,151,24,236,200,159,32,127,120,221,225,69,241,137,129,158,87,156,100,126,110,127,238,232,69,253,229,243,185,81,17,
165,149,82,158,82,226,43,101,20,109,219,89,89,125,241,159,170,148,127,177,88,176,125,223,55,181,246,19,165,237,127,173,108,59,185,188,209,43,5,25,236,200,31,249,51,62,224,181,198,139,210,156,17,116,249,
175,242,82,169,77,103,226,114,252,74,223,222,19,45,162,188,82,3,160,60,219,118,178,51,51,51,119,86,127,206,168,248,5,76,173,117,197,54,129,104,228,143,252,9,242,135,215,27,94,84,190,34,120,245,234,103,
118,88,206,199,252,252,143,79,4,241,24,246,226,243,106,173,141,106,159,87,203,127,109,7,32,157,78,79,104,237,27,190,239,39,180,214,9,173,181,97,89,246,87,200,31,249,19,228,15,175,183,188,176,207,33,65,
126,214,125,61,94,42,53,114,58,120,199,112,249,165,238,127,222,181,149,127,97,189,197,124,189,111,1,104,219,118,178,200,31,249,19,228,15,175,247,188,48,239,4,92,184,112,254,96,216,206,71,176,143,104,183,
63,175,248,141,118,242,171,26,0,165,148,182,44,59,87,187,77,192,96,71,254,4,249,195,235,29,47,172,77,192,174,93,187,82,97,59,31,65,60,142,55,110,220,112,187,253,121,149,82,190,227,12,45,54,90,204,87,53,
0,166,105,185,200,31,249,19,228,15,175,255,188,176,53,1,247,238,125,113,38,140,231,35,136,199,121,211,166,209,255,182,219,159,215,113,146,247,55,218,201,95,107,0,18,9,179,184,250,213,0,6,39,242,39,200,
31,94,255,121,97,122,98,224,229,203,151,255,139,168,159,143,126,101,114,114,242,119,7,241,121,77,145,210,19,129,218,249,165,145,63,65,254,140,15,120,61,121,98,224,47,126,240,131,223,248,211,32,143,137,
125,251,118,95,21,81,155,145,255,96,210,141,207,107,12,242,31,15,19,111,124,124,219,48,26,68,254,140,15,120,253,224,189,242,202,203,23,63,250,232,163,127,24,228,113,145,74,109,66,254,93,204,32,222,18,
104,34,255,230,120,81,124,114,23,65,254,240,130,203,59,112,224,185,127,37,34,127,28,212,177,129,252,195,187,242,111,123,7,32,142,131,243,244,233,119,119,162,66,228,207,248,128,199,13,107,143,130,252,195,
45,255,150,27,128,184,14,206,111,126,243,181,95,160,67,228,207,248,128,199,19,3,131,35,175,78,120,97,60,166,189,104,118,12,6,103,99,222,141,27,215,61,33,200,159,241,1,111,128,188,176,55,1,65,59,31,65,
189,161,123,189,223,171,219,199,239,248,241,99,234,248,241,99,9,131,193,217,152,119,240,224,145,15,81,34,242,103,124,192,227,137,129,209,144,127,220,143,223,241,227,199,148,148,239,255,51,54,254,199,211,
19,165,183,10,197,121,112,18,228,143,188,224,13,158,23,182,38,0,249,7,78,254,134,136,56,82,126,15,144,177,177,252,87,54,87,190,85,40,78,131,115,105,105,113,17,45,34,127,228,5,47,72,188,176,52,1,200,191,
189,172,94,6,232,145,252,135,86,229,223,176,1,72,167,211,19,174,155,27,169,254,153,120,13,206,43,87,174,61,141,26,145,63,242,130,23,52,94,16,158,24,216,232,58,58,242,15,220,202,63,33,34,195,149,242,95,
183,1,152,154,154,218,94,40,228,135,181,142,175,252,69,148,247,221,239,126,255,255,67,143,200,31,121,193,11,234,19,3,207,159,63,247,223,177,242,111,157,23,244,121,169,7,242,223,84,35,127,61,55,55,239,
25,245,228,239,186,249,148,214,143,126,88,41,229,199,113,112,162,71,228,143,108,224,5,153,183,119,239,238,255,23,249,183,198,187,125,251,166,21,228,121,233,250,245,43,251,186,44,255,209,154,197,190,22,
145,226,99,59,0,153,76,102,188,80,112,147,34,186,114,98,244,75,111,21,98,112,18,228,143,188,224,5,141,119,245,234,103,246,160,198,81,229,101,128,176,28,191,229,229,135,127,43,168,243,210,194,194,141,221,
79,62,249,228,150,46,201,223,20,145,177,58,242,207,207,205,205,235,170,6,32,147,201,140,23,139,5,187,114,155,64,41,41,191,79,152,193,73,144,63,178,129,23,68,222,169,83,167,11,172,252,155,231,29,57,242,
252,31,6,113,94,186,113,227,234,222,39,158,24,221,218,69,249,111,173,35,255,149,85,249,87,53,0,158,87,52,181,214,149,242,215,182,157,92,110,231,77,129,200,159,32,127,120,240,250,199,27,228,55,3,194,116,
252,62,251,236,114,32,231,165,171,87,63,123,122,108,108,108,172,139,242,223,38,34,149,95,101,247,69,228,225,220,220,188,95,249,179,70,233,0,165,39,170,17,74,91,150,243,16,249,19,228,143,108,224,133,131,
55,168,38,224,237,183,223,126,38,44,199,239,133,23,142,158,14,226,202,255,169,167,158,218,220,101,249,155,53,242,127,80,43,127,145,250,223,2,208,182,237,100,145,63,65,254,200,6,30,243,213,70,249,246,183,
191,51,31,134,227,119,241,226,39,129,187,249,239,202,149,75,207,244,96,229,95,43,255,165,185,185,249,186,143,180,175,106,0,148,82,218,178,236,220,204,204,204,29,6,19,65,254,200,6,94,184,120,131,218,5,
248,240,195,115,175,5,253,248,125,227,27,47,7,234,165,110,215,175,95,217,183,109,219,182,209,30,202,223,19,145,251,235,201,255,177,6,192,52,45,23,249,19,228,143,108,224,133,151,55,136,38,224,181,215,190,
253,63,167,82,155,206,4,245,248,45,46,46,255,131,32,205,75,151,47,127,242,76,151,239,246,175,149,127,81,68,238,53,146,127,85,3,144,72,152,197,233,233,233,187,12,38,130,252,145,13,188,112,243,6,181,19,
208,234,91,246,250,113,252,206,158,253,112,120,114,114,242,119,131,180,242,31,31,223,222,203,149,127,81,68,62,159,155,155,47,110,244,247,59,42,146,56,12,166,160,190,54,146,32,127,120,240,26,229,192,129,
103,157,23,95,252,181,92,80,199,94,63,142,223,245,235,215,191,58,116,232,249,143,131,180,242,15,138,252,171,118,0,24,76,4,249,35,27,120,209,225,237,223,191,255,224,153,51,239,189,18,196,93,128,126,28,
191,207,63,255,124,41,72,242,191,116,233,226,254,32,201,191,237,6,0,249,19,228,15,15,94,240,121,207,62,187,223,159,159,255,113,38,72,77,64,63,142,223,229,203,87,242,251,246,237,255,44,40,243,210,245,235,
87,246,77,76,76,60,17,36,249,139,180,113,9,32,110,131,137,75,0,200,31,30,188,176,243,82,169,77,103,130,48,46,251,241,121,207,159,255,200,250,230,55,95,11,204,29,255,151,46,93,220,31,68,249,183,220,0,196,
117,48,13,114,240,16,228,15,15,94,167,188,68,194,188,53,232,241,217,143,207,155,74,141,4,234,65,63,159,126,122,225,217,29,59,118,140,4,81,254,45,53,0,113,30,76,65,43,42,130,252,225,193,107,149,55,232,
221,204,235,215,175,236,235,214,87,223,106,63,239,185,115,231,159,120,245,213,111,253,85,144,230,165,160,221,240,87,195,83,34,98,40,6,211,198,188,79,62,249,196,252,181,95,123,249,109,84,139,252,225,193,
11,51,111,208,77,192,210,210,210,133,7,15,150,126,123,203,150,177,77,221,248,188,23,47,94,180,191,241,141,87,222,10,218,188,244,201,39,31,63,55,57,57,185,41,192,242,55,69,196,87,27,31,236,244,68,62,191,
242,164,214,190,21,231,193,196,101,0,228,15,15,94,20,120,65,187,175,169,149,113,156,78,191,190,227,211,79,47,110,218,180,233,137,127,58,57,185,243,119,130,56,47,5,92,254,134,136,216,82,122,68,176,167,
154,144,255,102,173,181,245,168,192,184,134,70,144,63,60,120,97,229,113,99,115,239,114,241,226,71,7,118,238,220,57,28,96,249,15,173,202,95,68,60,163,145,252,93,55,55,34,85,95,21,100,48,17,228,15,15,94,
152,121,131,124,117,112,148,115,233,210,197,253,1,150,127,66,68,134,165,230,190,191,186,13,192,212,212,212,246,66,33,63,172,53,242,15,194,251,182,9,242,135,7,175,155,60,230,179,238,203,63,168,95,245,43,
203,127,83,141,252,245,220,220,252,227,59,0,83,83,83,219,93,55,159,210,250,209,15,43,165,124,6,19,65,254,240,224,69,135,71,19,16,27,249,143,214,44,246,117,153,89,189,3,144,201,100,198,11,5,55,41,162,43,
11,195,119,156,228,125,6,19,3,6,249,195,131,23,45,222,173,91,215,247,48,187,68,86,254,166,136,140,213,145,127,126,110,110,94,87,53,0,153,76,102,188,88,44,216,149,219,4,74,137,239,56,67,139,12,38,130,252,
225,193,139,30,111,243,230,205,79,190,255,254,187,175,50,203,180,158,79,63,189,240,108,192,229,191,181,142,252,87,86,229,95,213,0,120,94,209,212,90,87,202,95,219,118,114,121,118,118,118,129,193,196,46,
0,242,135,7,47,154,188,231,158,123,174,120,229,202,149,255,133,217,166,53,249,7,245,9,127,21,188,138,122,17,95,68,30,206,205,205,251,149,63,171,74,5,145,158,208,218,55,124,223,79,104,173,19,90,107,195,
178,236,175,144,255,250,225,171,52,200,31,30,188,40,241,248,170,115,164,228,111,74,233,171,126,69,17,41,136,200,210,220,220,188,87,251,243,245,190,5,160,109,219,201,34,127,130,252,225,193,131,71,66,41,
255,202,149,127,93,249,63,214,0,40,165,180,101,217,185,153,153,153,59,20,127,227,120,94,113,146,225,128,252,225,193,139,10,143,203,155,141,115,225,194,249,131,33,147,191,39,34,247,215,147,255,99,13,128,
105,90,46,242,111,158,247,197,23,11,123,24,22,200,31,30,188,168,240,104,2,214,151,255,174,93,187,82,33,146,127,81,68,238,53,146,127,85,3,144,72,152,197,233,233,233,187,12,166,230,121,169,212,240,147,31,
126,120,230,27,12,15,228,15,15,94,84,120,52,1,213,249,248,227,115,135,66,40,255,166,120,29,157,104,6,83,137,199,139,130,144,63,60,120,81,226,113,147,243,35,249,239,222,189,251,107,81,148,127,213,14,0,
197,223,62,143,142,25,249,195,131,23,37,30,115,90,244,229,223,118,3,192,96,234,140,71,144,63,60,120,65,231,197,185,9,136,131,252,219,106,0,24,76,245,121,116,204,200,31,30,188,168,241,226,248,109,167,143,
62,250,240,112,28,228,223,114,3,192,96,106,204,163,9,64,254,240,224,69,141,119,253,250,229,103,227,36,255,61,123,246,12,197,65,254,34,45,220,4,200,96,106,158,199,13,52,200,31,30,188,40,241,206,159,255,
40,245,205,111,190,246,19,228,31,13,249,31,63,126,76,137,136,161,40,254,222,240,104,2,144,63,60,120,81,226,69,249,219,78,49,148,191,41,34,190,177,113,49,164,39,242,249,220,24,131,137,27,104,144,63,60,
120,241,229,69,117,78,59,127,254,131,35,49,146,191,33,34,142,148,119,255,141,141,229,191,178,89,107,109,48,152,184,129,6,249,195,131,7,47,106,242,223,187,119,111,50,70,242,31,146,138,75,255,70,35,249,
187,110,110,164,250,103,40,254,86,121,217,236,242,75,200,31,249,195,131,199,183,157,130,149,179,103,223,127,33,70,242,79,136,200,176,212,220,247,87,183,1,152,154,154,218,94,40,228,135,181,70,254,221,224,
197,185,9,64,254,240,224,241,109,167,32,202,127,255,254,103,172,24,201,127,83,141,252,245,220,220,188,103,212,147,191,235,230,83,90,63,250,97,165,148,79,241,119,198,139,227,229,0,228,15,15,94,52,121,97,
110,2,98,40,255,209,154,197,190,46,51,171,119,0,50,153,204,120,161,224,38,69,116,229,201,245,29,39,121,159,226,239,156,23,167,27,3,145,63,60,120,92,243,71,254,3,231,141,213,145,127,126,110,110,94,87,53,
0,153,76,102,188,88,44,216,149,219,4,74,137,239,56,67,139,20,127,247,120,113,104,2,144,63,60,120,209,231,133,109,46,59,123,246,189,23,99,38,255,173,117,228,191,178,42,255,170,6,192,243,138,166,214,186,
82,254,218,182,147,203,179,179,179,11,20,127,119,121,81,110,2,144,63,60,120,241,225,133,101,46,43,201,127,191,25,35,249,111,19,145,138,243,43,190,136,60,156,155,155,247,43,127,214,40,21,68,122,162,26,
161,180,101,57,15,145,127,239,120,81,108,2,144,63,60,120,241,227,101,179,15,142,34,255,192,201,223,172,145,255,131,90,249,139,212,255,22,128,182,109,39,139,252,123,207,139,82,19,128,252,225,193,139,47,
239,179,207,62,57,24,196,121,233,204,153,247,142,34,127,89,154,155,155,247,234,253,157,170,6,64,41,165,45,203,206,205,204,204,220,161,248,251,195,59,121,242,164,10,123,35,128,252,225,193,139,55,111,251,
246,237,169,55,223,252,233,241,160,201,255,217,103,247,119,229,243,134,84,254,158,136,220,95,79,254,143,53,0,166,105,185,200,127,112,219,104,23,47,126,124,4,249,83,47,240,224,133,145,247,218,107,223,186,
136,252,3,197,187,215,72,254,85,13,64,34,97,22,167,167,167,239,82,252,131,227,237,220,57,153,204,102,151,95,186,125,251,214,143,144,63,60,120,240,194,198,11,194,110,38,242,111,158,215,209,201,162,248,
123,199,75,36,204,91,200,31,30,60,120,97,227,13,242,77,168,167,79,159,122,249,224,193,131,26,249,55,199,107,187,1,160,248,163,63,152,144,63,60,120,240,218,225,13,98,222,66,254,173,243,218,106,0,40,254,
254,243,130,210,8,32,127,120,240,224,53,195,235,231,156,117,234,212,47,190,117,228,200,97,23,249,183,198,99,50,15,25,111,80,141,192,79,127,250,147,225,133,133,59,89,206,7,60,120,240,90,225,245,122,206,
122,247,221,83,175,28,58,116,208,71,254,173,243,20,197,26,78,94,191,26,129,159,252,228,199,191,253,157,239,124,247,12,231,3,30,60,120,157,242,62,255,124,33,113,229,202,181,231,191,243,157,239,253,27,228,
63,88,94,75,13,0,197,31,76,222,231,159,223,93,177,172,228,63,219,185,115,231,111,119,83,252,191,248,197,91,255,217,211,79,239,249,51,195,48,238,182,83,92,156,95,120,240,224,109,196,75,165,54,157,65,254,
3,225,41,17,49,20,197,26,29,222,217,179,103,63,253,246,183,191,155,149,14,242,222,123,239,190,118,224,192,115,5,206,7,60,120,240,250,205,107,118,103,243,157,119,222,254,230,225,195,135,60,228,223,182,
252,77,17,241,213,198,39,47,61,145,207,175,60,169,181,111,81,172,209,188,27,247,139,47,22,118,166,82,195,219,56,126,240,224,193,11,250,61,79,200,191,35,158,33,34,182,148,30,17,236,169,38,228,191,89,107,
109,61,58,129,20,43,60,120,240,224,193,235,47,239,196,137,19,250,236,217,247,95,136,217,43,125,187,45,255,161,85,249,55,108,0,210,233,244,132,235,230,70,180,214,150,214,58,81,58,137,20,43,60,120,240,224,
193,11,55,47,134,242,79,136,72,170,82,254,34,226,213,123,27,160,76,77,77,109,47,20,242,195,90,87,190,43,128,226,130,7,15,30,60,120,200,63,132,242,223,36,213,55,253,235,185,185,121,207,172,39,127,215,205,
167,180,126,244,195,74,41,223,113,146,247,41,46,120,240,224,193,131,135,252,67,37,255,81,17,209,229,85,191,148,255,255,69,145,154,183,1,102,50,153,241,66,193,77,138,232,202,78,1,249,195,131,7,15,30,60,
228,31,62,222,88,141,231,181,136,228,231,230,230,181,84,254,67,153,76,102,188,88,44,216,149,219,4,74,137,111,219,67,203,179,179,179,11,20,23,60,120,240,224,193,67,254,161,225,109,173,179,242,95,89,149,
127,85,3,224,121,69,83,107,93,41,127,109,89,201,7,200,31,30,60,120,240,224,33,255,208,241,84,153,35,82,186,249,239,225,220,220,188,95,249,179,70,233,128,167,39,170,17,74,91,150,243,16,249,195,131,7,15,
30,60,228,31,106,158,47,34,15,106,229,191,214,0,212,68,219,182,147,69,254,240,224,193,131,7,15,249,135,94,254,75,115,115,243,94,189,191,83,245,45,0,165,148,54,77,43,55,51,51,115,135,226,130,7,15,30,60,
120,200,63,180,60,79,68,238,175,39,255,199,118,0,76,211,114,145,63,60,120,240,224,193,67,254,161,231,221,107,36,255,170,6,32,145,48,139,211,211,211,119,41,46,120,240,224,193,131,135,252,163,207,83,210,
65,40,86,120,240,224,193,131,135,252,195,201,107,187,1,160,88,225,193,131,7,15,30,242,15,47,207,108,87,254,249,124,110,108,245,164,137,116,254,184,96,120,240,224,193,131,7,175,77,25,110,45,47,104,87,175,
121,123,82,186,6,94,132,215,197,29,128,213,87,4,75,245,13,132,190,227,12,45,182,247,213,65,120,240,224,193,131,7,175,117,94,249,89,247,181,143,187,245,101,131,187,223,225,181,209,0,60,122,69,240,163,127,
188,244,184,224,228,114,187,197,0,15,30,60,120,240,224,181,41,195,209,58,50,92,234,64,174,177,225,181,212,0,164,211,233,137,242,43,130,107,30,23,220,238,19,3,225,193,131,7,15,30,188,182,229,191,169,142,
12,31,116,32,215,56,241,148,136,24,77,53,0,171,175,8,174,126,75,160,210,182,237,100,219,121,110,0,60,120,240,224,193,131,215,14,239,248,241,99,134,136,12,215,44,96,181,212,121,214,61,188,117,229,111,138,
136,175,154,57,121,165,87,4,87,255,227,150,101,231,218,45,6,120,240,224,193,131,7,175,77,25,14,213,145,225,74,7,114,141,27,207,46,239,30,120,13,191,5,80,126,69,176,165,148,242,31,109,219,40,109,154,150,
219,206,67,131,224,193,131,7,15,30,188,118,120,229,149,235,170,188,42,101,152,175,124,197,45,188,13,121,143,46,195,52,58,121,158,87,124,172,65,104,247,137,129,240,224,193,131,7,15,94,7,242,55,235,172,
132,139,29,200,48,238,188,245,119,0,124,223,55,148,50,170,182,24,218,185,89,3,30,60,120,240,224,193,235,132,39,165,155,223,170,120,237,222,249,14,239,17,239,255,7,124,143,79,131,130,186,8,224,0,0,0,0,
73,69,78,68,174,66,96,130,0,0};

const char* Ambix_encoderAudioProcessorEditor::settings_png = (const char*) resource_Ambix_encoderAudioProcessorEditor_settings_png;
const int Ambix_encoderAudioProcessorEditor::settings_pngSize = 21502;

// JUCER_RESOURCE: settings_white_png, 21725, "../../settings_white.png"
static const unsigned char resource_Ambix_encoderAudioProcessorEditor_settings_white_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,2,0,0,0,2,0,8,6,0,0,0,244,120,212,250,0,0,0,4,103,65,77,
65,0,0,177,143,11,252,97,5,0,0,0,6,98,75,71,68,0,255,0,255,0,255,160,189,167,147,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,0,7,116,73,77,69,7,222,8,18,15,38,42,61,185,14,229,0,0,32,
0,73,68,65,84,120,218,237,189,109,108,92,87,154,223,249,156,91,247,222,42,154,20,77,154,118,155,164,218,122,177,45,89,178,36,75,234,182,37,181,95,36,119,172,96,208,51,30,206,236,100,138,181,192,238,151,
0,193,238,98,147,124,89,32,59,217,9,146,236,32,153,0,131,5,54,11,100,243,33,223,22,216,32,25,146,114,128,112,220,99,4,203,70,211,146,219,109,217,234,200,150,220,109,89,150,100,89,178,37,185,109,89,26,
137,52,171,110,213,189,103,63,84,149,116,235,186,72,214,123,221,151,223,31,16,102,90,46,254,197,186,247,121,158,223,115,206,61,247,28,17,132,16,66,8,37,78,170,93,131,163,71,95,78,5,255,110,97,97,209,197,
15,191,122,202,102,179,147,193,191,155,155,155,187,134,31,126,248,225,135,95,111,253,204,54,192,160,42,63,239,111,34,180,136,148,240,195,47,168,92,46,55,238,186,37,83,107,175,230,239,83,41,19,63,252,240,
195,15,191,62,248,181,52,3,80,129,67,186,14,28,10,11,11,139,26,63,252,130,193,90,42,21,109,173,245,61,63,165,148,54,77,203,153,153,153,185,129,31,126,248,225,135,95,239,253,154,110,0,142,30,125,217,16,
145,129,58,112,88,89,88,88,244,240,195,207,175,233,233,233,137,98,209,201,4,253,44,203,206,207,206,206,94,199,15,63,252,240,195,175,63,126,77,53,0,21,56,12,213,129,195,82,27,176,193,47,166,126,211,211,
211,19,142,83,24,20,209,62,63,165,109,59,189,220,106,240,227,135,31,126,248,225,215,190,95,83,13,64,101,49,216,6,17,49,124,127,237,137,200,221,86,22,133,225,23,111,191,108,54,59,89,44,22,134,180,22,223,
52,149,104,203,74,47,181,178,232,5,63,252,240,195,15,191,206,248,229,114,185,113,207,243,140,134,22,1,86,224,48,210,97,216,224,23,83,191,108,54,59,233,56,249,97,173,239,251,41,37,158,101,101,238,182,26,
252,248,225,135,31,126,248,181,239,87,93,64,168,148,225,153,13,194,97,172,14,28,110,183,1,27,252,98,234,151,205,102,39,11,133,149,209,160,159,109,15,220,105,53,248,241,195,15,63,252,240,107,223,111,122,
122,122,162,84,42,90,74,41,79,100,157,215,0,125,112,176,125,127,237,138,200,55,45,194,198,20,145,71,68,36,133,95,252,252,178,217,63,222,88,40,172,140,105,173,125,157,170,242,210,233,204,237,214,130,31,
63,252,240,195,15,191,78,248,85,23,16,42,165,60,165,202,79,18,204,38,166,133,83,82,126,7,252,171,133,133,197,166,223,93,172,192,230,209,192,191,137,95,76,252,178,217,63,222,152,207,175,60,34,162,125,205,
132,114,211,233,204,205,185,185,99,95,224,135,31,126,248,225,215,47,191,236,164,227,20,30,168,142,252,171,50,86,129,131,33,223,93,16,86,18,145,47,59,12,27,252,98,224,183,90,176,102,50,3,95,117,50,248,
241,195,15,63,252,240,107,30,254,197,98,97,168,246,237,129,242,142,129,198,42,240,31,10,192,223,5,134,248,145,76,248,225,135,31,126,209,130,127,101,1,97,205,166,65,213,29,3,205,0,28,148,124,119,19,24,
79,202,207,132,129,33,126,36,39,126,248,225,135,95,68,252,10,133,149,17,17,165,149,146,234,154,174,154,29,3,205,0,252,211,117,224,127,187,141,5,102,192,21,248,227,135,31,126,248,225,215,115,248,231,199,
164,188,145,91,133,223,74,91,150,157,247,111,23,236,135,129,89,7,254,119,129,63,126,36,19,126,248,225,135,95,212,252,68,148,82,110,249,255,74,221,29,3,141,10,32,82,242,221,237,95,129,63,126,36,19,126,
248,225,135,95,132,253,202,155,6,165,151,234,109,23,92,239,53,192,118,246,146,7,174,192,31,63,252,240,195,15,191,80,248,137,103,219,153,85,55,13,50,234,192,127,5,248,227,71,50,225,135,31,126,248,69,121,
228,175,188,116,122,224,214,90,155,6,153,1,248,183,122,126,60,112,5,254,248,225,135,31,126,248,133,196,175,188,105,208,218,59,6,214,108,244,3,252,241,35,153,240,195,15,63,252,146,225,167,164,13,1,87,224,
143,31,126,248,225,135,95,52,253,90,110,0,128,43,240,199,15,63,252,240,195,47,186,126,45,53,0,192,21,248,227,135,31,126,248,225,23,109,191,166,27,0,224,10,252,241,195,15,63,252,240,139,190,95,83,13,0,
112,5,254,248,225,135,31,126,248,69,223,79,164,254,70,64,192,48,166,126,90,235,85,223,242,56,116,232,57,187,94,112,205,207,191,126,90,41,165,72,38,252,226,230,183,86,62,4,99,158,235,135,95,156,252,114,
185,220,184,231,121,70,67,133,29,184,198,27,254,13,79,23,53,216,8,144,156,248,133,217,175,153,92,80,74,41,174,31,126,113,131,191,235,150,76,165,12,207,104,0,54,41,17,121,4,184,38,27,254,85,159,160,72,
78,252,226,10,255,106,204,115,253,240,139,139,223,244,244,244,68,169,84,180,239,57,53,0,255,49,17,177,69,36,5,92,163,229,215,41,240,55,163,169,169,87,247,147,156,248,133,201,175,250,24,171,157,124,248,
241,143,15,111,221,176,97,195,24,247,3,191,40,195,191,88,116,50,74,41,207,48,12,87,41,195,83,235,192,127,68,68,172,10,112,82,192,21,248,55,170,201,201,241,236,182,109,79,188,251,230,155,191,248,140,53,
4,248,245,19,254,157,138,233,131,7,159,125,242,228,201,83,23,136,103,252,162,231,151,157,44,22,157,7,148,82,158,82,202,93,179,1,56,122,244,101,67,68,134,43,240,79,85,160,163,129,43,240,111,71,172,33,192,
47,170,240,111,37,142,185,31,248,133,7,254,133,33,17,209,74,25,110,181,1,56,118,236,181,207,141,85,224,63,36,181,231,4,184,192,21,248,183,171,181,214,14,144,236,248,69,1,254,205,230,24,247,3,191,126,195,
223,113,242,195,90,223,127,220,175,148,210,169,148,89,146,0,228,229,232,209,151,149,136,12,72,237,218,0,79,68,190,2,174,192,191,27,205,0,201,142,95,148,224,223,76,174,113,63,240,235,183,95,161,176,50,
170,117,13,231,181,105,90,206,204,204,204,141,154,6,160,2,255,116,29,248,223,4,174,192,191,23,179,2,36,59,126,81,128,127,35,57,199,253,192,175,255,240,207,143,213,14,242,149,182,44,59,95,133,127,112,6,
192,172,3,255,219,11,11,139,46,176,6,254,189,208,217,179,103,15,144,236,248,69,1,254,107,229,30,247,3,191,48,248,105,173,239,241,93,41,209,182,157,94,158,157,157,189,238,255,172,170,0,39,37,229,197,126,
213,63,134,136,44,3,255,104,248,197,165,1,240,235,208,161,231,236,147,39,79,21,73,118,252,194,10,255,154,66,90,89,24,200,253,192,47,60,126,202,85,74,185,74,169,162,109,103,238,204,205,205,93,11,126,190,
222,70,64,90,68,150,128,127,116,252,90,121,45,41,236,122,231,157,247,156,245,22,12,146,236,248,133,1,254,213,38,156,199,88,248,133,207,79,188,213,224,95,175,1,208,34,178,178,176,176,232,1,215,104,249,
197,177,9,8,22,87,146,29,191,48,194,191,170,221,187,119,238,230,254,226,23,22,63,165,148,151,78,15,220,90,13,254,193,6,64,139,72,1,248,71,215,47,206,77,64,181,17,32,217,241,11,35,252,69,68,62,252,240,
163,15,63,251,236,179,34,247,23,191,48,248,165,211,153,155,107,193,95,130,192,89,88,88,108,250,89,50,176,14,151,95,220,85,109,2,120,230,10,252,195,24,159,31,124,240,225,7,63,252,225,190,237,191,250,213,
251,231,217,49,16,191,176,251,181,53,98,4,214,225,242,243,7,195,252,252,79,79,197,189,25,152,152,24,223,240,220,115,207,62,73,178,3,255,48,138,29,3,241,11,187,159,209,106,112,3,235,240,194,95,68,100,106,
234,247,158,141,123,3,112,253,250,141,187,151,47,127,54,69,178,3,255,48,207,86,113,127,241,11,171,95,75,51,0,192,58,220,240,143,98,177,108,87,251,247,63,179,227,244,233,51,231,152,118,5,254,81,155,9,224,
254,226,215,47,191,166,103,0,128,117,52,224,159,201,12,124,165,42,74,66,3,112,250,244,153,115,173,252,28,197,3,248,247,115,38,128,251,139,95,63,253,154,130,3,176,142,14,252,131,193,16,199,205,130,218,
25,117,81,60,128,127,191,99,146,251,139,95,63,253,154,106,0,128,117,116,225,79,19,64,178,3,255,112,197,36,247,23,191,126,250,229,114,185,113,207,243,140,134,30,1,0,235,232,195,191,209,81,113,156,180,218,
6,66,20,15,224,223,239,152,228,254,226,215,79,248,187,110,201,20,105,96,13,64,229,156,128,71,128,117,180,225,159,212,38,32,56,243,65,241,0,254,253,214,193,131,188,186,138,95,127,252,166,167,167,39,74,
165,162,93,253,223,70,3,240,31,147,242,1,65,192,58,226,240,79,122,19,192,200,11,248,135,65,39,79,158,186,240,245,215,95,221,33,94,240,235,53,252,139,69,39,163,181,86,235,54,0,21,248,143,4,62,3,172,35,
14,127,68,241,0,254,253,215,219,111,191,123,158,120,193,175,119,126,217,73,199,41,12,74,96,221,159,90,5,94,134,136,12,139,136,85,25,253,155,82,62,43,0,88,199,0,254,181,126,169,99,243,243,243,91,146,212,
2,188,248,226,143,158,124,232,161,177,13,20,15,224,223,111,177,111,5,126,189,128,127,177,88,24,18,17,173,148,225,42,165,92,195,48,220,99,199,94,251,220,92,5,254,67,129,145,191,43,34,95,1,235,184,193,95,
68,196,253,99,17,57,149,164,162,251,214,91,191,188,64,241,5,254,161,152,143,210,90,55,19,135,196,11,126,205,143,252,243,195,34,74,43,37,213,115,84,116,42,101,150,36,0,121,57,122,244,101,37,34,3,129,153,
1,15,248,199,21,254,101,63,73,168,154,125,45,146,98,4,252,251,25,135,196,11,126,205,250,21,10,43,163,90,215,158,250,107,154,150,51,51,51,115,163,166,1,168,192,63,93,7,254,55,129,117,124,225,159,180,29,
3,41,190,192,63,138,113,72,188,224,215,60,252,243,99,181,131,124,165,45,203,206,87,225,31,156,1,48,235,192,255,246,194,194,98,211,35,68,224,31,29,248,251,253,104,2,40,30,192,63,124,113,72,188,224,215,
138,159,214,218,184,95,219,69,219,118,122,121,118,118,246,186,255,179,70,5,96,169,0,252,181,136,220,5,254,201,129,63,77,0,197,23,248,135,79,196,11,126,237,250,41,37,158,101,165,151,130,240,15,206,0,248,
225,191,4,252,147,7,127,154,128,218,38,128,226,1,252,251,25,131,196,11,126,237,251,137,103,219,153,59,115,115,115,215,234,253,140,81,7,254,43,11,11,139,30,176,78,38,252,105,2,202,5,152,226,1,252,251,25,
131,108,90,133,95,251,35,127,229,165,211,3,183,86,131,127,176,1,208,34,82,0,254,192,159,38,128,226,11,252,251,171,169,169,87,247,19,127,248,181,227,151,78,103,110,174,5,255,96,3,80,90,88,88,108,250,180,
56,224,31,79,248,39,93,181,176,162,24,1,255,222,234,204,153,179,135,136,63,252,186,237,215,214,8,15,248,199,27,254,126,191,249,249,159,38,106,179,32,17,145,195,135,95,250,79,35,35,15,254,11,138,7,240,
239,135,94,121,229,200,150,193,193,13,163,196,31,126,221,242,107,185,1,0,254,201,129,255,253,81,113,242,154,0,17,118,12,4,254,253,106,64,15,207,29,63,126,60,75,252,225,215,45,63,163,149,192,4,254,201,
131,63,59,6,82,60,128,127,111,117,252,248,241,108,43,63,71,252,225,215,181,6,0,248,39,19,254,236,24,200,142,129,192,63,252,34,254,240,235,90,3,0,252,147,11,127,118,12,100,199,64,224,15,252,241,139,143,
159,72,19,107,0,128,63,240,111,117,84,28,55,213,107,128,40,70,192,191,151,241,70,252,225,215,142,95,46,151,27,247,60,207,104,104,6,0,248,3,127,180,122,227,67,49,2,254,204,60,225,23,37,248,187,110,201,
20,105,224,17,64,229,156,128,71,128,63,240,111,101,100,18,119,81,140,128,63,241,135,95,84,252,166,167,167,39,74,165,162,93,253,223,70,3,240,31,19,145,20,240,7,254,52,1,181,163,50,118,12,4,254,192,31,191,
40,193,191,88,116,50,90,107,181,110,3,80,129,255,72,224,51,192,159,224,162,9,168,104,227,198,137,223,5,254,192,191,87,13,39,48,196,175,117,191,236,164,227,20,6,37,176,238,207,88,5,134,134,136,108,0,254,
192,191,81,191,169,169,87,247,111,221,186,249,127,76,82,81,254,226,139,235,127,125,245,234,213,2,240,71,140,252,241,11,51,252,139,197,194,144,136,174,129,255,220,220,220,53,99,21,248,15,5,224,239,2,127,
130,107,61,191,61,123,118,255,42,105,197,248,244,233,51,103,151,151,151,190,4,254,168,23,179,0,60,118,194,175,249,145,127,126,88,235,251,35,127,165,148,78,165,204,146,4,167,3,142,30,125,89,137,200,3,21,
24,166,42,127,148,136,124,3,252,9,174,70,252,150,151,151,190,252,217,207,222,188,154,180,226,156,164,237,90,129,127,255,196,41,129,248,53,227,87,40,228,71,68,148,86,74,185,74,137,167,148,81,178,237,244,
202,204,204,204,13,241,143,242,43,240,79,7,154,2,79,68,110,2,127,130,171,81,191,36,194,191,58,58,3,254,168,7,145,6,12,241,107,20,254,99,82,51,147,175,180,101,217,249,42,252,165,246,63,138,89,7,254,183,
23,22,22,155,222,3,30,248,39,55,88,217,46,24,248,163,238,233,131,15,206,140,1,67,252,26,241,211,90,27,247,103,40,69,219,118,122,121,118,118,246,186,255,179,70,5,136,169,0,252,181,136,220,5,254,4,87,183,
253,146,38,224,143,218,209,103,159,93,253,47,212,23,252,154,241,83,74,60,203,74,47,5,225,47,85,232,87,26,128,234,31,67,68,86,22,22,22,61,224,79,112,181,227,199,86,193,192,31,245,54,198,168,87,248,221,
247,83,174,82,170,152,78,15,220,154,155,155,187,86,239,103,130,111,1,104,224,79,112,49,242,111,79,245,26,31,224,143,152,121,194,175,183,35,127,229,173,5,255,96,3,160,69,164,0,252,9,174,78,249,177,85,48,
240,71,189,107,50,169,87,248,249,253,210,233,204,205,181,224,31,108,0,74,11,11,139,77,79,217,2,127,130,21,173,94,160,129,63,162,249,196,47,172,126,166,136,72,43,139,253,128,63,193,69,121,93,91,59,118,
108,79,71,249,109,142,164,174,225,136,82,147,185,214,44,27,245,10,191,245,100,180,26,124,192,159,224,106,68,73,126,12,240,209,71,31,231,163,122,127,129,63,35,127,252,226,239,215,82,3,0,252,9,46,212,152,
46,94,188,168,129,63,234,230,44,0,245,10,191,86,253,154,110,0,128,63,193,213,110,129,74,146,126,253,235,115,191,2,254,168,87,77,0,245,10,191,102,212,212,244,44,240,39,184,90,241,187,124,249,242,31,158,
57,243,225,63,79,114,145,142,202,123,219,52,0,209,141,47,234,21,126,205,202,4,254,4,67,183,253,190,254,250,230,89,74,52,240,71,221,157,5,224,160,32,252,26,85,46,151,27,247,60,207,104,232,17,0,240,39,184,
218,241,179,44,243,46,5,58,62,239,109,163,112,106,121,249,238,45,234,21,126,141,192,223,117,75,166,72,3,107,0,42,219,4,63,2,252,9,174,86,253,148,82,223,82,158,27,107,2,128,63,106,85,182,253,192,27,212,
43,252,214,210,244,244,244,68,169,84,180,239,57,53,0,255,49,17,177,165,124,78,0,240,199,175,105,63,54,149,241,93,141,16,191,183,205,35,128,232,107,235,214,205,47,126,250,233,103,111,113,86,0,126,245,224,
95,44,58,25,165,148,103,24,134,171,148,225,169,117,224,63,34,34,86,5,176,41,224,143,31,240,239,78,19,16,134,251,75,3,144,140,70,147,122,149,68,191,236,100,177,232,60,160,148,242,148,82,110,181,1,48,86,
129,171,33,34,27,36,176,85,48,240,199,175,89,63,206,3,136,254,253,69,196,31,126,81,135,127,97,72,68,215,212,226,185,185,185,107,198,42,240,31,10,192,223,5,254,248,1,155,206,40,74,239,109,163,232,199,24,
245,37,217,240,119,156,252,176,214,247,31,247,43,165,116,42,101,150,36,0,121,57,122,244,101,37,34,3,82,187,54,192,19,145,175,128,63,126,192,134,145,63,138,79,19,64,189,138,191,95,161,176,50,170,117,237,
169,191,166,105,57,51,51,51,55,106,26,128,10,252,211,117,224,127,19,248,227,7,108,58,95,156,195,118,63,120,92,19,207,56,243,107,189,248,155,159,127,253,52,245,42,46,240,207,143,213,14,242,149,182,44,59,
95,133,191,248,97,127,244,232,203,254,197,126,169,202,127,187,219,202,73,129,192,31,191,86,166,35,147,164,199,30,219,248,183,247,239,223,23,202,247,182,185,95,168,153,166,144,250,23,86,63,17,165,148,43,
162,92,195,80,37,219,206,220,157,157,157,189,238,255,172,81,1,108,42,48,242,215,192,31,63,70,254,221,211,213,171,95,252,127,97,189,191,7,15,62,251,36,119,8,5,103,16,168,87,81,221,135,69,60,203,74,47,5,
225,47,82,127,35,32,45,34,75,192,31,63,224,223,237,38,224,106,33,140,247,247,209,71,31,29,217,179,231,233,125,220,33,180,214,204,16,245,47,10,126,226,217,118,230,206,220,220,220,181,186,51,60,190,25,128,
84,165,33,112,22,22,22,61,224,143,95,167,252,152,82,94,91,97,221,180,229,194,133,139,206,111,126,115,238,12,119,8,5,227,149,250,23,133,145,191,81,76,167,7,190,94,13,254,193,25,0,45,34,5,224,143,31,35,
127,224,47,162,92,224,143,86,107,232,169,127,225,247,75,167,51,55,215,130,127,176,1,40,45,44,44,54,61,82,3,254,248,161,246,138,105,88,239,175,170,136,187,132,130,186,120,241,162,162,254,69,223,175,173,
228,6,254,248,117,3,114,204,2,132,239,254,114,15,81,80,59,118,108,63,176,125,251,246,34,245,47,186,126,45,55,0,192,31,63,26,128,222,53,1,156,21,128,162,222,192,82,79,195,231,103,180,114,195,129,63,126,
168,119,10,203,253,229,113,0,234,68,99,72,61,13,143,95,211,13,0,240,199,143,209,127,242,224,79,19,128,58,145,231,212,211,112,249,53,213,0,0,127,252,154,245,155,154,122,117,63,229,177,181,2,26,214,251,
75,19,128,90,105,2,168,167,225,242,147,0,120,129,63,126,29,247,251,252,243,207,29,74,99,244,71,254,245,244,196,19,91,15,94,188,248,233,73,238,22,242,55,1,245,26,68,234,105,184,252,114,185,220,184,231,
121,70,67,51,0,192,31,191,86,253,134,134,54,252,75,202,98,60,103,118,118,237,122,218,125,226,137,173,135,185,99,104,173,153,0,234,105,248,224,239,186,37,83,164,129,71,0,149,93,2,31,1,254,248,181,226,119,
252,248,137,255,134,146,24,223,251,187,107,215,211,223,62,249,228,227,7,184,115,168,94,19,64,61,13,151,223,244,244,244,68,169,84,180,171,255,219,104,0,254,99,82,222,38,24,248,227,215,148,223,141,27,55,
150,40,133,205,21,205,40,198,203,133,11,151,222,229,238,161,184,196,115,156,225,95,44,58,25,173,181,90,183,1,168,192,127,36,240,25,224,143,95,195,126,239,190,251,171,115,148,193,166,203,102,228,226,133,
29,3,209,106,58,127,254,188,69,61,13,131,95,118,210,113,10,131,18,216,251,199,88,5,214,134,136,108,0,254,248,181,227,71,249,107,71,209,139,23,154,0,20,212,185,115,231,223,61,127,254,19,131,122,218,95,
248,23,139,133,33,17,93,147,159,115,115,115,215,204,85,224,63,20,128,191,43,34,95,1,127,252,26,245,155,159,127,253,52,229,175,121,221,185,243,55,223,12,15,143,60,24,213,120,81,74,41,246,126,64,193,38,
160,213,6,145,122,218,137,145,127,126,88,68,105,165,68,87,238,131,78,165,204,210,119,102,0,142,30,125,89,137,200,64,96,154,192,3,254,248,1,255,222,232,147,79,46,109,143,122,188,48,19,128,234,137,29,3,
123,239,87,40,172,140,106,93,123,234,175,105,90,206,204,204,204,13,241,131,190,2,255,116,5,214,169,202,31,37,34,183,23,22,22,155,158,206,5,254,192,31,181,166,184,140,148,152,9,64,173,198,55,245,180,19,
240,207,143,137,136,22,81,174,82,229,63,182,157,94,158,157,157,189,94,253,156,191,51,48,235,140,252,129,63,126,77,249,49,250,235,189,216,49,16,197,105,38,128,122,218,25,63,173,181,113,63,23,69,7,225,127,
175,1,168,172,248,247,39,171,22,145,187,189,128,191,246,137,155,23,125,63,70,125,192,63,168,61,123,158,222,205,157,66,141,52,1,212,211,206,251,41,37,158,101,165,151,130,240,151,42,244,43,13,64,245,143,
33,34,43,11,11,139,94,47,224,191,214,168,129,155,23,61,63,26,128,206,40,110,211,164,215,174,93,251,246,212,169,211,191,225,206,34,234,125,175,252,148,171,148,42,166,211,3,183,230,230,230,174,213,251,153,
224,107,128,186,159,240,247,255,61,55,15,248,163,248,196,203,228,228,228,3,63,248,193,222,167,184,115,136,122,223,171,145,191,242,214,130,127,176,1,208,34,82,232,39,252,171,218,178,101,211,0,193,0,252,
81,188,226,229,191,254,215,15,62,230,238,161,122,60,160,158,118,222,47,157,206,220,92,11,254,193,6,160,180,176,176,216,116,17,239,52,252,69,68,62,253,244,179,111,191,249,230,230,93,130,33,158,71,80,162,
228,193,223,191,99,32,139,3,81,80,236,24,216,31,191,182,18,177,27,240,247,235,240,225,231,159,24,25,121,104,152,155,23,110,63,70,255,157,87,18,142,84,37,110,144,95,187,119,63,189,255,241,199,31,23,234,
115,239,252,90,110,0,186,13,255,170,246,237,219,147,254,224,131,15,29,110,94,120,253,40,228,221,111,0,226,26,127,196,14,106,164,249,165,62,119,199,175,165,6,160,149,247,252,219,77,244,102,130,130,96,0,
254,113,42,130,113,143,63,98,8,81,239,251,227,215,116,3,208,15,248,55,19,20,4,3,240,143,83,17,76,82,252,17,79,136,122,223,91,63,35,42,240,111,196,139,96,0,254,113,82,210,86,71,139,136,188,240,194,161,
173,220,121,68,189,239,77,190,53,60,3,208,111,248,175,215,25,18,12,189,245,163,1,232,190,166,166,94,221,159,212,248,187,122,245,106,225,244,233,51,103,137,2,68,189,239,188,95,46,151,27,247,60,207,104,
104,6,160,211,123,251,119,186,51,36,24,128,127,76,199,64,137,141,191,199,30,219,100,114,255,17,245,190,59,240,119,221,146,41,210,192,35,128,202,54,193,143,132,5,254,193,160,32,24,128,127,2,198,64,137,
61,88,138,189,3,16,245,190,115,126,211,211,211,19,165,82,209,174,254,111,163,1,248,143,73,249,140,128,166,225,223,237,164,221,187,119,183,77,48,0,127,224,159,140,248,83,107,136,56,73,70,19,64,125,110,
15,254,197,162,147,209,90,223,203,23,115,29,248,143,4,154,132,190,143,252,253,122,255,253,179,133,87,94,57,178,101,112,112,232,97,130,1,248,3,255,228,198,179,72,237,154,137,223,252,230,163,204,133,11,
151,222,34,134,226,165,75,151,46,201,227,143,111,37,63,154,246,203,78,58,78,225,1,165,84,205,86,255,106,21,248,27,34,50,44,34,86,101,244,111,74,249,172,128,166,225,223,11,112,76,77,253,222,179,4,67,119,
253,104,0,250,212,2,176,41,74,203,126,243,243,175,159,86,74,41,98,55,94,218,177,99,251,129,237,219,183,23,169,207,141,195,191,88,44,12,137,136,86,202,112,149,82,174,97,24,238,177,99,175,125,110,174,2,
255,161,192,200,223,21,145,175,194,50,242,15,106,126,254,167,167,40,150,192,31,248,19,207,126,191,234,245,243,95,71,98,57,250,58,119,238,252,187,228,71,51,35,255,252,176,136,210,74,137,174,92,55,157,74,
153,37,9,64,94,142,30,125,89,137,200,64,96,102,192,107,21,254,97,79,54,138,37,240,143,147,136,231,245,253,88,51,16,31,53,91,155,146,152,31,133,194,202,168,214,181,167,254,154,166,229,204,204,204,220,168,
105,0,42,240,79,215,129,255,205,86,224,159,205,254,241,198,242,51,185,112,6,4,197,18,248,3,255,228,250,209,8,36,171,9,72,38,252,243,99,181,131,124,165,45,203,206,87,225,31,156,1,48,235,192,255,246,194,
194,162,219,153,47,19,158,128,160,88,2,127,224,143,95,181,17,16,17,217,181,107,231,30,34,41,158,77,64,82,243,67,107,109,220,143,115,209,182,157,94,158,157,157,189,94,19,255,149,209,127,74,202,139,253,
170,127,12,17,89,238,4,252,171,207,231,123,173,213,186,123,138,37,240,143,138,216,11,189,247,126,167,79,191,63,126,245,234,23,175,19,125,241,200,151,100,199,179,114,149,82,174,82,170,104,219,153,59,115,
115,115,215,130,159,175,183,15,128,22,145,165,78,141,252,247,237,219,251,39,97,233,10,41,150,192,159,145,63,126,107,249,237,223,191,255,139,67,135,158,179,137,174,232,207,4,16,207,34,34,226,173,6,255,
122,13,128,22,145,149,133,133,69,175,83,95,230,253,247,63,248,139,48,4,4,193,0,252,129,63,126,141,248,157,60,121,170,200,250,128,104,55,1,196,179,136,82,202,75,167,7,110,173,6,255,96,3,160,69,164,208,
73,248,251,95,197,233,103,64,176,131,20,240,7,254,248,53,235,71,19,16,221,38,128,120,86,110,58,157,185,185,22,252,131,13,64,105,97,97,81,247,251,203,116,49,44,40,110,192,31,248,227,215,244,246,195,68,
93,244,116,233,210,37,33,158,27,120,27,166,23,197,40,44,128,97,199,64,192,31,37,5,225,3,172,251,231,71,174,68,79,59,119,110,63,176,109,27,59,6,118,165,1,104,230,31,15,91,242,176,131,20,5,13,248,227,215,
172,31,57,19,143,92,34,158,239,203,104,229,130,70,103,218,191,51,137,12,252,81,63,5,172,195,225,199,227,128,104,138,29,3,59,216,0,68,29,254,205,6,5,240,71,192,31,63,191,158,125,118,255,54,34,51,158,77,
64,210,226,89,245,234,31,15,43,120,214,234,234,227,20,12,128,63,90,82,74,41,96,29,94,191,115,231,62,182,206,159,191,240,75,34,53,122,121,69,60,223,151,153,180,145,127,189,198,36,238,59,72,1,255,104,198,
165,255,124,123,96,29,46,191,29,59,158,42,18,165,212,251,168,198,115,46,151,27,247,60,207,48,250,241,143,135,125,122,40,46,193,80,221,3,129,180,143,158,128,127,248,253,136,82,234,125,84,225,239,186,37,
83,164,129,53,0,217,108,118,178,124,170,80,60,225,31,12,138,56,193,159,84,7,254,192,186,123,126,44,10,164,222,71,45,158,167,167,167,39,74,165,226,189,173,174,141,245,225,191,50,234,63,85,168,221,83,183,
194,30,20,81,15,6,70,253,209,214,145,35,135,103,129,107,116,252,104,2,162,221,4,36,13,254,197,162,147,209,90,171,117,27,128,108,54,59,233,56,249,225,218,207,180,254,143,71,5,74,167,79,159,222,24,101,248,
147,214,209,86,161,176,242,79,129,107,60,23,92,161,240,233,147,79,206,91,201,136,231,236,164,227,20,6,37,176,240,95,173,214,41,56,78,126,131,231,105,83,68,167,180,46,255,18,237,126,153,247,222,123,111,
199,245,235,95,254,199,176,7,197,239,254,238,79,126,99,154,169,255,110,126,254,245,211,97,222,68,162,250,251,1,254,248,168,188,91,37,112,141,154,31,57,24,93,197,127,199,192,236,100,177,88,24,18,17,173,
148,225,42,165,92,195,48,220,99,199,94,251,220,172,15,255,194,160,214,247,155,131,242,169,66,153,155,237,126,153,167,158,218,118,50,10,1,241,215,127,253,198,211,34,114,186,149,159,237,37,252,41,60,113,
131,255,212,101,224,202,200,31,245,86,31,125,116,254,221,10,231,98,184,99,96,117,38,95,105,165,68,87,190,167,78,165,204,146,72,224,17,64,46,151,27,47,22,157,140,136,246,95,136,142,192,95,68,100,120,248,
193,135,162,22,28,205,0,182,215,240,71,241,210,149,43,159,78,1,87,224,143,194,95,235,163,18,207,229,53,124,181,167,254,154,166,229,204,204,204,220,16,241,237,3,144,203,229,198,43,171,3,125,35,127,241,
108,123,224,206,122,71,10,54,243,101,162,28,24,253,220,68,226,226,197,139,250,215,191,62,247,43,210,52,190,218,180,105,115,10,184,70,211,143,71,113,241,105,2,26,153,9,136,6,252,243,99,34,162,69,164,194,
93,165,45,203,206,87,225,95,211,0,184,110,201,244,175,14,84,74,180,101,101,238,118,18,254,81,31,185,250,19,220,31,36,171,109,218,210,234,26,130,170,223,175,127,253,235,13,23,47,126,122,156,180,140,191,
142,28,57,60,251,232,163,143,2,87,70,254,40,228,77,64,116,226,89,68,169,242,160,91,41,209,182,157,94,158,157,157,189,94,211,184,150,127,32,59,169,181,103,120,158,151,210,90,167,180,214,134,101,217,223,
118,18,254,113,95,173,190,117,235,150,255,161,80,200,127,147,74,165,242,158,231,121,95,124,113,253,175,73,37,212,168,182,109,123,98,224,194,133,75,121,224,26,93,63,102,0,226,165,232,239,24,168,92,165,
148,171,148,42,218,118,166,238,76,126,189,6,192,176,44,59,31,236,20,58,249,101,72,20,132,214,47,54,192,53,90,126,123,247,238,182,223,127,255,108,129,104,142,103,94,70,47,158,203,240,79,167,7,110,173,54,
152,55,3,95,86,155,166,213,85,248,35,132,218,23,176,14,171,31,138,147,170,143,3,162,24,207,229,183,247,86,135,191,72,224,45,0,211,180,156,94,192,159,221,179,16,106,61,31,128,117,20,252,80,156,154,128,
40,198,115,249,237,189,181,31,227,223,107,0,82,41,179,228,95,29,200,200,31,33,70,254,248,1,127,20,223,29,3,85,63,139,17,107,1,16,106,124,6,0,184,134,223,111,126,254,167,167,136,232,120,42,142,59,6,154,
97,25,137,32,4,252,129,107,212,253,136,232,248,42,142,59,6,26,173,92,8,224,143,80,239,4,92,163,225,199,14,157,201,80,156,118,12,52,250,249,143,179,24,16,49,250,7,174,113,241,163,158,209,4,68,45,158,141,
126,39,211,206,157,79,237,37,156,16,2,174,236,24,136,226,212,4,68,33,254,204,126,39,211,182,109,79,178,122,22,37,182,128,172,54,106,4,174,192,31,69,55,135,195,30,127,185,92,110,220,243,60,195,8,67,50,
61,245,212,147,135,8,37,196,40,2,184,2,127,20,117,69,1,254,174,91,50,69,26,120,4,144,205,102,39,203,167,10,117,239,203,60,245,212,14,182,207,68,52,1,192,53,210,126,188,214,76,254,134,61,158,167,167,167,
39,42,167,254,202,186,13,64,25,254,43,163,90,107,163,155,95,134,213,179,136,34,82,22,112,101,228,143,200,223,110,193,191,88,116,50,254,83,127,141,181,224,239,56,249,225,218,207,116,111,245,44,43,104,81,
210,245,227,31,191,180,9,184,2,127,20,77,109,219,246,196,179,225,141,231,236,164,227,20,6,37,176,249,159,177,122,167,80,24,210,186,251,240,39,153,16,42,235,231,63,63,113,229,246,237,91,183,128,43,240,
71,209,211,39,159,92,60,181,188,188,244,117,24,225,95,44,22,134,68,116,13,252,231,230,230,174,169,122,240,119,156,194,160,214,58,165,181,78,85,126,9,85,62,88,160,251,95,134,231,104,40,233,122,233,165,
231,183,156,56,241,246,101,142,8,142,142,31,117,11,85,53,53,245,234,254,112,141,252,243,195,34,74,43,37,158,82,134,107,24,70,201,52,45,103,102,102,230,70,77,129,201,229,114,227,142,83,24,208,218,51,181,
22,163,210,0,168,116,58,115,187,87,95,134,68,66,168,146,41,156,18,24,25,63,234,22,106,39,119,187,21,207,133,66,126,164,12,127,229,86,26,128,146,109,167,87,170,7,255,25,126,248,87,86,7,170,251,95,66,42,
231,9,247,46,153,88,11,128,80,243,80,1,214,192,31,69,51,119,187,8,255,49,169,93,195,167,45,203,206,251,79,253,189,247,31,93,183,100,250,87,7,42,37,218,182,51,119,214,59,79,184,23,95,6,33,10,9,176,142,
138,31,66,97,136,103,255,219,123,101,158,167,151,103,103,103,175,251,63,107,148,127,32,59,25,24,135,107,203,74,47,245,11,254,204,2,32,212,88,19,0,172,251,235,199,232,31,133,189,121,87,74,60,203,74,47,
5,225,47,82,255,45,0,109,219,233,229,126,143,252,105,2,16,90,187,152,0,107,70,254,136,145,255,218,126,226,173,53,147,111,4,160,171,45,203,206,215,235,20,250,241,101,182,109,123,252,57,194,4,161,239,54,
1,192,186,255,126,140,254,81,171,49,210,155,145,191,170,172,225,91,125,48,95,211,0,152,166,229,132,5,254,249,252,202,35,59,119,238,36,193,16,10,20,20,118,12,100,228,143,162,219,4,244,42,158,203,175,238,
175,61,147,127,175,1,72,165,204,146,127,117,96,88,146,243,229,151,95,124,156,16,66,232,190,94,122,233,249,45,192,186,191,126,140,254,81,43,10,91,60,171,40,124,25,206,10,64,168,86,135,15,63,255,196,200,
200,232,40,176,238,143,31,13,0,106,69,83,83,175,238,15,83,60,155,81,232,100,148,82,138,132,67,232,190,142,31,127,251,34,59,6,2,127,4,252,219,241,51,90,249,34,60,147,67,168,255,58,113,226,237,203,113,204,
95,224,143,226,168,219,183,191,185,19,182,120,54,162,146,156,188,22,136,80,251,80,2,254,192,31,245,71,131,131,15,190,30,182,252,48,162,148,156,52,1,8,181,14,39,224,223,158,31,66,237,232,141,55,222,216,
25,182,120,86,81,76,78,58,113,132,86,73,232,85,154,100,224,223,158,31,53,7,117,59,71,123,25,207,185,92,110,220,243,60,195,136,67,114,34,132,86,7,21,240,111,207,111,203,150,77,3,68,22,234,151,186,1,127,
215,45,153,13,205,0,100,179,217,201,66,97,229,97,173,61,43,76,201,73,71,142,208,250,163,12,224,207,200,31,69,119,22,160,211,241,60,61,61,61,225,186,37,75,41,229,41,101,120,70,3,240,31,245,159,42,20,150,
228,100,61,0,66,107,131,139,29,3,219,135,255,225,195,135,143,17,77,40,14,35,255,233,233,233,137,98,209,201,212,156,250,187,22,252,29,39,63,172,181,182,180,214,169,242,47,17,190,100,103,147,32,132,234,
235,165,151,158,223,50,58,58,250,48,240,103,228,143,162,53,11,208,249,252,200,78,22,139,206,3,229,145,191,114,13,195,112,87,157,1,40,119,10,133,33,173,197,8,123,178,191,248,226,143,158,36,148,16,250,174,
78,156,120,251,242,237,219,183,110,1,255,150,70,254,115,68,16,138,195,200,191,12,255,194,144,136,174,105,56,230,230,230,174,169,122,240,119,156,194,160,214,58,117,127,228,47,170,124,176,64,56,147,253,
163,143,62,82,159,124,114,233,61,66,7,161,239,234,200,145,23,54,191,249,230,47,62,99,199,64,70,254,40,220,51,0,221,128,191,227,228,135,69,148,86,74,60,165,12,215,48,140,146,105,90,206,204,204,204,141,
154,95,32,151,203,141,59,78,97,64,107,207,212,90,140,74,3,160,210,233,204,237,48,39,59,143,1,16,106,173,224,0,127,224,143,194,145,147,221,200,143,66,33,63,82,134,191,114,43,13,64,201,182,211,43,213,131,
255,12,63,252,75,165,162,45,190,117,1,74,73,229,60,225,112,39,187,170,136,112,66,168,51,128,3,254,8,245,78,93,130,255,152,212,108,246,167,180,101,217,121,255,169,191,247,254,163,235,150,204,154,213,221,
78,40,71,0,0,32,0,73,68,65,84,129,74,180,109,103,238,172,119,158,112,152,146,157,38,0,161,246,65,7,252,17,138,54,252,243,249,149,71,252,111,239,149,121,158,94,158,157,157,189,238,255,172,81,254,129,236,
100,96,98,66,91,86,122,41,74,240,167,9,64,168,125,224,1,127,132,122,167,29,59,182,167,187,157,31,74,137,103,89,233,165,32,252,69,234,159,5,160,109,59,189,28,69,248,211,4,32,212,58,248,88,237,143,80,111,
117,235,214,237,223,237,110,126,136,183,214,76,190,17,0,167,182,44,59,95,175,83,136,90,178,211,4,32,212,120,19,144,212,145,255,241,227,199,179,68,2,234,151,110,220,248,242,63,117,111,228,175,42,107,248,
86,31,204,215,52,0,166,105,57,113,128,63,66,168,113,16,38,109,199,64,166,253,81,248,212,249,252,40,191,186,191,246,76,254,189,6,32,149,50,75,254,213,129,113,72,118,165,148,218,186,117,243,3,4,23,66,171,
235,200,145,23,54,3,127,132,250,163,43,87,174,44,247,109,166,188,157,95,60,236,201,126,244,232,203,166,136,60,90,44,22,237,197,197,183,46,17,106,8,213,215,139,47,254,232,201,135,30,122,104,4,248,35,212,
135,241,127,159,54,233,106,185,1,136,10,252,69,196,20,17,241,60,79,126,246,179,55,47,19,106,8,213,215,11,47,28,218,250,139,95,188,243,105,28,119,12,4,254,40,78,13,64,167,242,163,165,6,32,106,240,175,168,
116,247,238,210,111,223,121,231,61,135,112,67,40,124,197,8,248,35,114,174,183,249,97,36,5,254,34,242,229,201,147,167,138,132,26,66,157,3,102,20,234,193,193,131,7,254,45,119,21,197,33,231,58,157,31,177,
234,244,215,130,255,194,194,98,137,81,1,66,157,27,149,68,161,30,220,185,179,252,127,157,60,249,238,223,231,110,34,242,173,141,6,32,78,240,167,9,64,168,189,162,4,252,81,189,24,161,166,118,167,1,232,116,
126,228,114,185,113,207,243,12,163,31,201,25,6,248,55,210,113,33,148,116,69,113,199,64,224,223,59,96,5,107,40,7,179,117,94,221,128,191,235,150,76,145,6,214,0,100,179,217,201,242,169,66,241,130,63,77,0,
66,253,47,70,192,63,30,224,167,174,70,35,223,166,167,167,39,42,167,254,202,186,13,64,25,254,43,163,254,83,133,226,4,127,130,21,161,198,115,35,26,240,255,22,248,247,17,252,65,77,77,189,186,255,197,23,95,
252,43,174,222,250,10,206,182,117,3,254,197,162,147,241,159,250,107,172,5,127,199,201,15,215,126,38,126,240,15,6,43,97,136,80,148,225,127,18,248,119,57,22,26,189,191,229,122,170,83,15,61,244,224,159,109,
221,186,249,111,113,21,251,151,31,101,158,23,6,37,176,238,79,173,214,41,56,78,126,131,231,105,83,68,167,180,46,255,18,113,133,127,240,247,59,125,250,253,241,171,87,191,120,157,48,68,20,252,104,28,20,244,
217,103,87,254,55,224,31,30,248,215,187,191,95,125,245,245,205,95,254,242,157,43,92,209,181,175,117,55,224,95,44,22,134,68,68,43,101,184,74,41,215,48,12,247,216,177,215,62,55,234,195,191,48,168,245,253,
230,64,41,229,37,5,254,34,34,251,247,239,231,192,33,68,193,151,168,156,18,88,56,1,252,187,19,7,157,220,20,234,157,119,78,94,229,113,235,218,234,252,193,92,229,153,252,0,207,117,42,101,150,36,0,81,201,
229,114,227,142,83,200,136,104,255,77,242,210,233,204,237,164,192,191,250,251,85,3,149,215,90,16,240,15,239,180,127,101,228,191,149,59,215,223,81,127,51,247,87,41,165,168,171,245,85,125,108,210,169,124,
43,20,86,70,68,148,86,74,220,106,143,97,154,150,83,61,248,207,240,195,191,178,58,208,215,41,72,229,60,225,100,193,63,24,172,132,37,2,254,225,27,249,243,204,191,243,218,180,233,251,131,189,56,11,130,186,
186,234,28,64,7,225,159,31,147,218,53,124,218,178,236,188,255,212,223,123,48,117,221,146,233,95,29,168,148,104,203,202,220,93,239,60,225,56,195,159,142,21,1,255,112,194,191,50,237,207,200,191,195,186,
122,245,139,111,123,117,127,169,171,223,213,237,219,183,110,141,140,140,142,118,38,223,68,148,82,110,149,231,182,157,94,158,157,157,189,238,255,172,81,254,129,236,100,224,214,104,203,74,47,1,255,218,96,
165,107,69,192,63,28,240,159,159,159,7,254,29,210,198,141,19,63,105,117,84,222,246,121,244,212,212,26,125,241,197,245,141,157,206,55,165,196,179,172,244,82,16,254,18,128,234,189,57,136,122,157,66,146,
225,79,215,138,128,63,207,252,227,168,39,158,216,122,232,233,167,119,252,170,159,71,64,83,83,239,107,112,112,232,112,38,51,240,255,116,46,223,196,179,237,204,157,213,6,243,170,58,3,160,181,103,104,173,
141,84,202,44,2,255,245,165,181,214,219,183,63,249,195,243,231,47,252,138,176,69,192,191,251,245,160,80,40,253,229,241,227,39,254,14,119,174,51,218,183,111,207,206,71,30,121,248,66,88,234,41,119,164,245,
89,145,250,35,127,163,152,78,15,124,189,214,76,126,77,3,144,74,153,37,255,2,1,224,223,152,223,151,95,126,245,247,78,158,124,247,127,34,116,17,240,7,254,192,191,53,191,229,229,187,183,126,246,179,55,47,
147,155,170,103,167,244,222,91,33,8,252,91,247,123,244,209,239,253,187,125,251,246,164,41,43,8,248,119,101,135,191,127,13,252,59,167,221,187,119,238,14,27,252,69,116,106,112,112,232,225,31,252,96,223,
46,238,80,239,238,135,41,34,210,202,98,63,224,95,235,247,193,7,31,58,60,203,66,192,191,243,35,255,147,39,79,2,255,14,142,252,39,38,198,47,132,181,158,126,255,251,223,183,185,75,189,187,31,70,63,255,241,
184,192,63,184,111,0,43,91,17,240,239,12,252,25,249,119,22,254,97,28,249,7,253,168,159,189,187,31,170,159,255,120,220,224,95,79,204,8,32,224,15,252,129,127,243,126,73,173,157,235,53,64,157,188,31,38,240,
239,174,31,91,10,35,224,223,92,61,184,115,231,219,127,205,180,127,231,180,123,247,206,221,97,158,246,111,199,143,145,127,123,215,207,232,119,178,199,25,254,245,138,240,11,47,28,218,58,53,53,117,153,80,
70,192,191,254,200,255,228,201,147,255,128,59,215,185,145,255,196,196,248,199,81,172,167,73,125,20,176,218,96,177,27,205,147,234,103,178,39,5,254,171,249,157,58,245,171,137,107,215,110,252,21,144,98,150,
4,248,51,237,223,105,237,221,187,251,233,239,125,239,145,79,120,140,26,237,220,237,198,253,200,229,114,227,158,231,25,102,191,146,61,233,240,23,81,238,139,47,190,116,58,201,111,14,4,131,156,183,40,146,
11,127,166,253,59,171,202,180,255,39,76,251,71,95,221,128,191,235,150,76,165,12,207,92,255,31,207,78,22,10,43,99,192,191,123,126,254,98,29,119,0,174,55,173,71,19,144,204,145,63,240,239,236,200,63,78,240,
79,114,77,232,244,253,152,158,158,158,40,149,138,150,82,202,19,89,103,13,64,5,254,163,90,107,3,248,247,198,79,249,20,151,32,222,184,113,114,168,153,239,196,107,64,201,130,63,211,254,157,133,127,28,166,
253,81,119,224,95,44,58,25,255,169,191,198,90,240,119,156,252,112,237,103,128,127,175,59,223,170,54,110,156,28,218,187,119,239,63,142,66,224,78,77,77,93,222,191,127,239,238,169,169,87,247,139,136,92,187,
118,125,153,116,6,254,192,31,248,183,3,255,137,137,71,255,48,41,247,82,107,173,59,123,63,178,147,142,83,24,148,192,186,63,181,90,167,224,56,249,13,158,167,77,17,157,210,186,252,75,0,255,240,248,93,186,
116,41,255,225,135,31,125,24,166,160,61,112,224,135,59,198,199,199,135,58,249,125,111,223,190,243,167,199,143,159,248,35,202,123,188,224,95,121,230,207,106,255,14,169,242,204,255,227,184,215,211,249,249,
159,158,74,194,253,44,15,158,58,7,255,98,177,48,36,34,90,41,195,85,74,185,134,97,184,199,142,189,246,185,89,31,254,133,65,173,239,55,7,74,41,47,157,206,220,4,254,225,241,123,250,233,221,183,131,83,229,
253,122,78,86,13,214,78,195,95,68,100,100,100,248,95,137,8,13,64,204,70,254,60,243,239,236,200,63,174,11,254,130,126,155,54,125,255,39,87,174,124,254,70,2,230,0,58,56,242,207,15,139,40,173,148,232,74,
189,208,169,148,89,18,9,60,2,200,229,114,227,197,162,147,17,209,126,176,0,255,136,248,169,53,228,155,70,251,131,3,7,14,252,187,86,65,19,244,236,108,167,90,255,251,82,226,227,5,127,166,253,59,11,255,184,
78,251,215,243,219,183,111,223,141,184,223,211,159,252,228,39,191,233,212,245,43,175,225,171,225,188,54,77,203,169,30,252,167,252,240,47,149,138,182,231,121,166,214,94,170,252,67,90,217,118,230,78,43,
135,5,1,255,112,249,105,173,117,61,88,207,207,191,126,186,83,231,79,119,251,251,242,118,0,240,71,201,133,127,181,94,37,160,1,56,103,89,198,127,223,62,252,243,99,34,162,69,148,171,84,249,143,109,167,151,
103,103,103,175,87,63,119,175,51,112,221,146,233,95,29,168,148,104,224,31,31,191,213,70,234,81,129,127,16,120,192,159,35,125,147,172,221,187,119,238,78,26,252,171,245,42,238,117,224,141,55,222,216,209,
137,235,231,127,123,175,204,243,90,248,223,155,1,200,102,179,147,90,123,134,231,121,41,173,117,74,107,109,88,150,253,45,240,199,47,140,126,73,159,9,96,228,207,200,63,137,240,79,218,65,65,237,15,206,238,
141,252,139,171,13,230,235,189,6,168,109,59,189,12,252,241,11,171,95,146,103,2,128,63,240,79,58,252,147,150,231,237,93,63,241,214,154,201,55,3,255,168,54,77,43,31,156,38,0,254,248,133,241,148,197,164,
205,4,68,112,218,255,255,100,181,127,231,20,231,237,125,91,89,240,204,154,160,181,175,95,249,237,189,129,91,107,13,230,107,102,0,76,211,114,128,63,126,81,241,75,210,76,64,68,225,255,15,41,205,29,133,255,
199,212,83,212,232,245,43,191,189,183,246,76,254,189,6,32,149,50,75,213,87,3,128,63,126,81,74,246,35,71,142,204,0,255,208,189,231,15,252,59,164,202,123,254,192,63,193,3,128,110,221,143,182,46,32,240,199,
47,44,126,151,47,95,249,189,51,103,206,254,25,240,239,63,252,121,230,223,89,248,243,204,127,117,197,245,49,64,35,205,77,71,182,155,7,254,248,197,197,239,111,254,102,233,31,189,249,230,155,57,224,15,252,
129,127,50,234,65,82,27,128,78,93,63,179,149,95,14,248,227,23,70,63,224,223,63,248,179,224,175,179,98,193,95,227,121,146,180,197,128,157,188,126,6,240,199,47,46,126,113,217,36,132,103,254,140,252,121,
230,223,184,223,198,141,19,63,1,254,173,93,63,179,217,100,7,254,248,133,221,47,202,163,2,70,254,140,252,25,249,55,231,183,99,199,246,143,128,127,107,11,168,205,102,146,29,248,227,23,21,191,40,54,1,156,
234,199,200,31,248,55,239,55,56,56,244,48,240,111,78,185,92,110,220,243,60,195,104,52,217,129,63,126,81,243,139,210,227,128,40,142,252,89,240,215,217,145,63,11,254,90,247,155,152,120,244,15,129,127,227,
240,119,221,146,217,208,12,192,209,163,47,167,68,228,17,224,143,95,20,253,162,48,19,192,200,159,145,63,35,255,246,252,30,125,244,123,111,0,255,245,53,61,61,61,81,42,21,45,165,148,39,178,206,34,192,10,
252,199,68,36,5,252,241,139,170,95,152,103,2,120,213,15,248,51,242,111,223,239,131,15,62,116,128,255,250,240,47,22,157,140,255,212,95,99,29,248,143,4,62,3,252,241,139,164,159,72,248,118,12,4,254,192,31,
248,247,175,30,132,89,90,107,221,217,235,151,157,116,156,194,160,4,246,254,81,171,36,187,33,34,195,34,98,85,70,255,166,136,104,224,143,95,212,253,194,178,99,96,212,224,255,229,151,95,255,227,51,103,206,
254,11,176,221,25,177,183,127,231,253,226,178,31,192,161,67,207,217,223,251,222,163,187,58,9,255,98,177,48,36,34,90,41,195,85,74,185,134,97,184,199,142,189,246,185,177,10,252,135,2,35,127,23,248,227,23,
7,191,45,91,54,253,180,223,51,1,192,31,248,3,255,206,251,197,229,108,128,78,195,223,113,242,195,90,223,31,236,43,165,116,42,101,150,68,2,139,0,143,30,125,89,137,200,64,96,102,192,19,145,111,8,86,252,226,
226,215,207,29,3,163,56,237,127,230,204,89,166,253,59,36,22,252,245,207,47,58,234,220,245,43,20,86,70,68,148,86,74,220,170,185,105,90,78,245,224,63,51,0,255,116,29,248,223,94,88,88,116,9,46,252,226,226,
87,133,112,175,167,12,163,7,127,247,63,242,204,191,179,240,231,153,63,240,111,162,98,180,9,255,252,152,148,31,221,87,248,173,180,101,217,121,255,169,191,254,25,0,179,14,252,239,2,127,252,226,234,215,203,
87,4,35,58,237,255,199,96,187,51,98,135,63,224,223,75,248,151,175,159,136,82,202,45,255,95,209,182,157,94,158,157,157,189,46,1,232,87,87,252,251,225,175,69,100,9,248,227,23,119,191,94,52,1,81,28,249,3,
255,206,142,252,129,127,111,252,226,113,56,80,167,174,223,61,248,123,150,149,94,10,194,95,170,208,175,52,0,213,63,134,136,172,44,44,44,122,4,23,126,73,241,235,86,209,136,230,180,255,113,224,223,65,248,
51,237,223,91,191,56,188,13,208,202,130,198,239,94,63,229,42,165,138,233,244,192,173,185,185,185,107,245,126,38,248,22,128,6,254,248,37,117,228,0,252,129,63,240,143,135,31,240,23,81,74,121,107,193,95,
164,118,13,128,22,145,194,194,194,162,38,184,240,75,162,95,39,167,15,35,248,204,255,79,152,246,239,156,120,230,15,252,123,165,213,174,95,58,157,185,185,22,252,131,51,0,37,224,143,31,197,163,253,29,3,35,
250,204,255,95,82,74,59,55,242,231,61,255,254,249,109,218,244,253,191,149,116,248,55,60,243,217,207,127,28,63,252,226,182,99,32,211,254,192,159,105,255,254,250,221,185,115,251,111,22,23,223,186,20,213,
24,106,244,17,64,39,174,159,209,234,47,73,176,226,23,103,191,45,91,54,253,244,15,254,96,234,82,171,137,11,252,129,63,249,214,31,191,225,225,7,31,98,228,223,152,204,126,254,227,248,225,23,102,191,255,252,
159,231,31,111,165,99,231,153,127,242,180,107,215,206,61,147,147,60,243,15,143,95,124,71,255,157,188,126,38,193,133,31,126,107,239,24,24,167,239,203,123,254,221,25,249,3,255,112,249,49,242,239,66,3,64,
112,225,135,95,180,225,207,200,159,145,127,220,253,230,231,95,63,13,252,59,220,0,16,92,248,225,199,200,31,49,242,143,194,204,93,92,142,6,238,214,245,203,229,114,227,158,231,25,6,193,133,31,126,201,128,
63,11,254,58,11,127,22,252,197,195,47,137,240,119,221,146,41,210,192,91,0,217,108,118,178,124,170,16,193,133,31,126,192,31,1,255,240,251,117,99,103,207,56,192,127,122,122,122,162,84,42,218,213,255,109,
172,15,255,149,81,173,181,65,112,225,135,31,240,7,254,192,159,145,127,116,225,95,44,58,25,173,181,90,183,1,200,102,179,147,142,147,31,174,253,12,193,133,31,126,192,31,248,19,127,192,63,74,240,47,243,188,
48,40,129,205,255,204,213,58,5,199,201,15,105,13,252,241,195,47,138,240,255,242,203,155,255,235,153,51,103,128,127,135,196,106,127,224,223,109,85,31,91,116,3,254,197,98,97,72,68,180,159,255,115,115,115,
215,204,250,240,47,12,106,125,255,147,229,83,133,50,55,9,46,252,240,139,12,252,255,28,108,119,20,254,231,128,63,240,143,230,200,63,63,44,162,180,82,162,43,60,215,169,148,89,18,9,60,2,200,229,114,227,197,
162,147,17,209,254,105,2,224,143,31,126,18,165,87,253,128,127,167,84,121,213,15,248,3,255,174,75,107,173,59,125,253,202,107,248,106,56,175,77,211,114,102,102,102,110,136,248,30,1,228,114,185,241,202,234,
64,223,200,95,60,219,30,184,179,222,145,130,4,23,126,248,133,3,254,60,243,239,44,252,121,230,15,252,123,165,169,169,87,247,119,22,254,249,49,17,209,34,82,217,25,81,105,203,178,243,85,248,215,204,0,184,
110,201,244,175,14,84,74,180,109,103,128,63,126,248,1,127,224,79,252,1,255,46,234,240,225,195,199,58,125,253,252,111,239,149,121,158,94,158,157,157,189,238,255,172,81,254,129,236,100,173,133,210,150,149,
94,2,254,248,225,7,252,129,63,241,7,252,187,171,171,87,175,252,219,110,93,63,165,196,179,172,244,82,16,254,53,51,0,62,105,219,78,47,3,127,252,240,3,254,192,159,248,3,254,221,215,99,143,77,94,233,206,245,
19,111,173,153,252,154,6,64,169,242,51,130,122,157,2,193,133,31,126,225,91,237,15,252,59,167,93,187,118,238,1,254,241,241,123,225,133,67,91,163,18,123,35,35,163,163,157,31,249,43,47,157,30,184,181,214,
96,190,230,53,64,255,234,64,130,11,63,252,66,191,218,31,248,119,112,228,207,123,254,241,242,27,27,27,75,69,39,2,59,127,253,202,111,239,173,61,147,127,175,1,72,165,204,18,240,199,15,63,166,253,147,8,127,
70,254,113,246,11,191,230,231,95,63,221,202,249,5,237,94,63,83,164,188,35,80,43,191,52,193,133,31,126,192,31,248,19,207,192,191,205,241,127,31,224,47,210,192,105,128,4,23,126,248,1,127,224,79,60,71,209,
47,174,177,219,169,235,167,250,249,143,227,135,31,126,192,31,248,227,215,13,191,249,249,215,79,199,113,244,223,201,235,103,18,92,248,225,23,250,213,254,255,136,5,127,157,19,7,251,36,195,79,41,165,180,
214,154,145,127,135,26,0,130,11,63,252,122,63,242,7,254,157,29,249,3,127,252,128,127,147,13,0,193,133,31,126,189,135,63,211,254,157,133,63,211,254,248,1,255,242,217,63,158,231,25,6,193,128,31,126,192,
31,248,19,127,192,63,57,240,119,221,146,41,210,192,91,0,217,108,118,178,124,170,16,193,133,31,126,192,31,248,19,207,192,63,170,240,159,158,158,158,168,156,250,43,235,54,0,101,248,175,140,250,79,21,34,
184,240,195,15,248,3,127,226,25,248,183,175,181,222,0,232,6,252,139,69,39,227,63,245,215,92,11,254,142,147,31,174,109,18,8,46,252,240,235,22,252,89,237,223,89,177,218,31,63,70,254,126,158,23,30,80,74,
121,254,191,55,87,235,20,28,39,63,164,53,240,199,15,191,94,141,252,129,127,103,71,254,192,31,63,224,95,134,127,177,88,24,18,17,237,223,250,103,110,110,238,154,89,31,254,133,65,173,239,127,178,124,170,
80,230,38,193,133,31,126,93,129,255,127,96,218,191,179,240,103,218,31,63,224,239,159,201,87,90,41,209,21,158,235,84,202,44,125,103,6,32,151,203,141,59,78,33,35,162,253,207,37,188,116,58,115,155,224,194,
15,191,213,119,28,107,101,47,111,31,252,179,96,27,248,227,7,252,59,125,253,10,133,149,145,10,252,171,219,34,107,255,169,191,166,31,254,149,213,129,190,145,191,120,182,61,112,167,149,195,130,8,86,252,146,
2,127,17,17,255,142,99,141,52,3,190,103,254,192,191,67,226,153,63,126,81,146,191,78,116,7,254,249,49,17,209,34,85,248,43,109,89,118,222,127,234,239,189,6,192,117,75,166,127,117,160,82,162,45,43,115,23,
248,227,135,223,125,191,139,23,47,234,209,209,177,63,123,235,173,95,188,186,218,231,215,218,126,180,186,61,233,51,207,60,243,167,103,206,156,249,87,96,187,163,240,63,23,21,248,55,211,48,146,191,173,249,
69,101,27,224,238,93,63,17,165,202,7,34,41,37,218,182,211,203,179,179,179,215,107,234,81,249,7,178,147,90,123,134,231,121,41,173,117,74,107,109,88,150,253,45,240,199,47,233,126,71,143,190,108,58,142,51,
249,237,183,206,63,125,239,189,247,254,30,168,5,254,157,132,255,122,77,0,249,219,186,95,216,27,0,165,148,234,238,245,83,174,82,202,85,74,21,109,59,83,119,38,191,222,62,0,218,182,211,203,192,31,191,164,
251,105,173,117,161,80,250,203,55,223,252,197,103,192,31,248,119,11,254,171,253,61,249,219,158,95,216,165,181,214,221,191,126,226,173,6,255,239,52,0,74,149,159,17,4,167,9,8,46,252,146,8,127,17,145,227,
199,79,252,29,48,11,252,187,9,255,122,77,0,249,219,190,223,227,143,111,121,62,204,241,59,53,245,234,254,110,94,191,242,219,123,3,183,214,26,204,215,52,0,166,105,57,192,31,63,224,31,175,35,68,129,127,248,
225,239,143,189,222,140,12,227,239,87,44,22,31,74,42,252,69,148,91,126,117,127,237,153,252,123,13,64,42,101,150,252,171,3,9,46,252,128,63,10,163,42,155,252,196,14,254,85,189,240,194,161,173,228,111,251,
126,169,84,106,75,120,163,56,28,215,79,181,243,21,128,13,126,192,31,49,242,239,28,252,171,122,241,197,31,61,249,208,67,15,141,144,191,173,251,21,139,222,255,251,198,27,111,236,12,91,12,111,222,188,233,
111,239,221,187,231,86,24,174,95,203,13,0,176,193,15,248,35,224,223,121,248,251,103,2,126,241,139,119,62,109,101,147,41,234,129,200,185,115,31,203,249,243,23,78,133,45,142,95,121,229,200,150,193,193,161,
135,195,112,253,76,224,143,31,240,7,254,97,87,212,246,246,239,68,76,253,226,23,239,124,74,125,110,221,239,252,249,11,167,195,24,203,97,129,191,200,58,199,1,19,92,248,1,127,20,134,145,127,148,182,247,237,
116,76,53,227,71,61,184,239,167,42,10,95,68,135,231,250,25,73,9,6,252,240,3,254,145,29,249,159,75,42,252,155,137,85,234,65,125,191,48,53,1,155,55,63,118,36,76,251,36,168,164,5,3,126,248,1,255,232,192,
63,201,35,255,186,5,155,29,3,91,246,11,75,206,135,97,77,71,46,151,27,247,60,207,48,146,26,12,248,1,127,4,252,163,4,255,213,254,29,234,65,99,126,97,152,9,8,11,252,93,183,100,138,52,240,8,32,155,205,78,
150,79,21,34,184,240,3,254,8,248,247,11,254,245,98,152,122,208,156,95,56,215,4,244,238,250,77,79,79,79,84,78,253,149,117,27,128,50,252,87,70,181,214,6,193,133,31,240,71,192,191,191,240,247,199,50,59,6,
182,230,23,149,38,160,27,240,47,22,157,140,255,212,95,99,45,248,59,78,126,184,246,51,4,23,126,192,31,1,255,48,196,20,59,6,182,183,0,238,200,145,23,54,247,242,126,53,211,120,116,254,250,101,39,29,167,48,
40,129,117,127,106,181,78,193,113,242,27,60,79,155,34,58,165,117,249,151,32,184,240,3,254,168,27,74,218,38,63,157,18,59,6,182,231,247,205,55,223,220,126,235,173,95,94,8,83,3,208,13,248,23,139,133,33,17,
209,74,25,174,82,202,53,12,195,61,118,236,181,207,85,125,248,23,6,181,174,130,95,167,68,68,149,15,22,32,184,240,3,254,136,145,127,152,174,31,59,6,182,231,119,247,238,157,223,254,252,231,39,174,132,161,
1,232,206,200,63,63,44,162,180,82,226,41,101,184,134,97,148,76,211,114,102,102,102,110,212,252,66,185,92,110,220,113,10,3,90,123,166,214,98,84,26,0,149,78,103,110,3,27,252,128,63,2,254,225,141,169,102,
27,0,234,75,217,111,126,254,245,211,97,184,63,221,248,190,133,66,126,164,12,127,229,86,26,128,146,109,167,87,170,7,255,25,126,248,87,86,7,170,251,191,176,84,206,19,6,54,248,1,127,4,252,195,124,61,217,
49,176,53,191,94,236,24,216,39,248,143,73,237,26,62,109,89,118,222,127,234,239,189,255,232,186,37,211,191,58,80,41,209,182,157,185,179,222,121,194,4,23,126,192,31,1,255,232,52,1,212,151,250,126,253,218,
58,184,91,223,215,255,246,94,153,231,233,229,217,217,217,235,254,207,26,229,31,200,78,6,250,21,109,89,233,37,224,143,31,240,71,192,63,90,49,181,214,239,75,125,89,223,175,151,77,64,47,190,175,82,226,89,
86,122,41,8,127,145,250,175,1,106,219,78,47,3,127,252,128,63,2,254,209,140,41,118,12,108,207,175,23,77,64,111,190,175,120,107,205,228,27,129,47,173,45,203,206,215,235,20,128,13,126,192,31,1,255,104,54,
1,212,151,230,253,58,213,4,212,243,233,205,200,95,85,214,240,173,62,152,175,105,0,76,211,114,128,63,126,192,31,117,90,73,63,210,183,159,77,0,59,6,182,238,215,141,153,128,94,125,223,242,171,251,107,207,
228,223,107,0,82,41,179,228,95,29,72,48,224,7,252,81,167,70,254,108,242,211,63,189,244,210,243,91,168,47,173,251,77,77,189,186,63,106,240,111,248,108,132,56,125,25,252,128,63,10,31,252,25,249,247,95,135,
15,63,255,196,200,200,232,40,245,165,117,191,111,190,249,155,127,254,214,91,111,253,126,171,51,8,161,124,204,1,252,241,3,254,8,248,199,63,166,94,122,233,249,45,39,78,188,125,153,29,3,91,247,107,118,211,
160,234,181,14,235,247,53,90,9,36,130,1,63,224,143,128,127,180,116,226,196,219,151,169,247,237,249,197,173,121,162,19,196,15,248,35,224,159,48,245,239,96,154,120,248,53,18,51,74,41,21,246,239,107,36,241,
230,225,7,252,81,119,196,106,255,104,136,29,3,219,243,107,164,129,10,251,219,23,77,205,0,16,12,248,1,127,196,200,63,25,51,1,212,171,198,252,214,138,161,242,219,3,225,252,190,185,92,110,220,243,60,195,
72,242,205,195,15,248,35,224,159,100,177,99,96,119,118,12,12,59,252,93,183,100,138,52,240,8,32,155,205,78,150,79,21,34,24,240,3,254,8,248,199,185,9,160,94,53,239,87,191,9,8,231,247,157,158,158,158,168,
156,250,43,235,54,0,101,248,175,140,250,79,21,34,24,240,3,254,8,248,199,175,9,96,199,192,214,253,252,77,192,230,205,143,253,78,88,225,95,44,58,153,154,83,127,215,130,191,227,228,135,181,214,150,214,58,
85,254,37,8,6,252,128,63,42,107,215,174,157,123,216,225,47,62,122,233,165,231,183,140,142,142,62,76,189,106,221,239,131,15,206,60,188,119,239,158,91,225,251,253,178,147,197,162,243,128,82,202,83,74,185,
134,97,184,74,25,158,90,173,83,112,156,252,6,207,211,166,136,78,149,27,0,17,130,1,63,224,143,24,249,199,87,236,24,24,71,191,236,100,177,88,24,18,17,173,148,225,86,27,128,99,199,94,251,220,172,15,255,194,
160,214,247,103,7,202,167,10,101,110,114,177,241,3,254,8,248,199,87,199,143,191,125,145,29,3,227,5,127,199,201,15,139,40,173,148,232,10,207,117,42,101,150,68,2,143,0,114,185,220,184,227,20,6,180,246,76,
173,197,168,76,253,171,116,58,115,155,139,141,31,240,71,192,63,57,106,182,1,160,254,133,207,175,80,200,143,148,225,175,92,165,196,83,202,40,217,118,122,165,122,240,159,225,135,127,101,117,160,111,228,
47,149,243,132,185,216,248,1,127,224,15,252,147,164,102,174,31,245,47,148,240,31,147,154,133,254,74,91,150,157,247,159,250,123,239,63,186,110,201,172,89,29,168,68,219,118,230,206,122,231,9,115,177,241,
3,254,192,31,248,39,183,9,160,254,133,211,207,255,246,94,153,231,233,229,217,217,217,235,254,207,26,229,31,200,78,6,38,127,180,101,165,151,128,63,126,192,31,1,127,154,0,234,85,116,253,148,18,207,178,210,
75,65,248,139,212,223,7,64,219,118,122,25,248,227,7,252,17,240,71,171,93,87,234,95,20,252,196,91,107,38,191,166,1,80,170,252,140,160,94,167,192,197,198,15,248,39,75,83,83,83,159,62,245,212,206,59,192,
31,5,175,47,245,47,10,35,127,85,89,195,183,250,96,94,149,127,56,59,169,181,103,164,82,102,201,191,64,128,139,141,31,240,79,182,162,242,42,24,49,213,235,230,48,188,123,221,227,215,184,223,189,25,0,224,
143,31,240,71,237,10,248,199,95,47,189,244,252,22,234,95,60,252,84,212,146,29,63,224,143,194,57,11,0,252,147,35,118,12,140,201,65,70,192,31,63,224,143,218,109,0,128,127,50,103,2,216,49,48,218,126,45,53,
0,92,108,252,128,63,13,0,240,71,205,204,16,81,79,195,231,103,112,113,240,3,254,168,85,1,127,196,142,129,209,245,51,184,56,248,1,127,4,252,81,183,155,0,234,105,184,252,68,154,120,4,192,197,198,15,248,39,
83,245,166,120,129,63,106,52,86,168,167,225,243,203,229,114,227,158,231,25,6,23,7,63,224,143,24,249,163,110,205,4,80,79,195,7,127,215,45,153,34,13,60,2,200,102,179,147,229,83,133,184,216,248,1,127,224,
15,252,81,227,53,128,122,26,46,191,233,233,233,137,202,169,191,178,110,3,80,134,255,202,168,255,84,33,46,54,126,192,31,248,3,127,180,94,45,208,90,107,234,105,184,224,95,44,58,25,255,169,191,198,90,240,
119,156,252,112,237,103,184,216,248,1,127,224,31,190,248,67,225,19,59,6,134,201,47,59,233,56,133,65,9,172,251,51,86,239,20,10,67,90,3,127,252,128,63,163,185,112,143,228,80,56,117,226,196,219,151,111,223,
190,117,139,122,218,127,248,23,139,133,33,17,93,3,255,185,185,185,107,102,61,248,59,78,97,80,235,251,157,66,249,84,161,204,77,46,54,126,192,63,121,10,243,193,47,40,220,58,126,252,237,139,236,24,216,239,
145,127,126,88,68,105,165,68,87,120,174,83,41,179,36,193,233,128,92,46,55,238,56,133,1,173,61,83,107,49,180,214,41,17,173,210,233,204,109,46,54,126,192,63,121,58,124,248,240,220,200,200,134,191,232,87,
252,17,99,241,17,59,6,246,222,175,80,200,143,148,225,175,92,165,196,83,202,40,217,118,122,165,122,240,159,225,135,127,101,117,160,111,228,47,149,243,132,185,216,248,81,152,147,168,91,183,190,254,115,70,
254,168,19,98,199,192,158,195,127,76,106,215,240,105,203,178,243,254,83,127,239,253,71,215,45,153,254,213,129,74,137,182,237,204,157,185,185,185,107,92,108,252,128,127,50,181,117,235,86,19,248,163,94,
54,1,212,231,206,248,249,223,222,43,243,60,189,60,59,59,123,221,255,89,163,252,3,217,201,192,100,141,182,172,244,18,240,199,15,248,35,224,143,122,213,4,80,159,59,239,167,148,120,150,149,94,10,194,95,164,
254,91,0,218,182,211,203,192,31,63,224,159,108,237,223,255,204,30,224,143,122,213,4,80,159,187,225,39,222,90,51,249,53,13,128,82,229,103,4,245,58,5,46,54,240,167,108,37,75,143,61,182,201,236,103,252,17,
115,201,105,2,168,207,221,24,249,171,202,26,190,213,7,243,53,13,128,105,90,14,240,199,143,66,140,246,237,219,179,179,223,241,215,202,107,99,40,122,77,0,59,6,118,199,175,252,234,254,218,51,249,247,26,128,
84,202,44,249,87,7,114,177,241,3,254,201,213,251,239,159,253,40,44,211,254,207,60,243,204,63,225,142,196,87,236,24,216,63,191,182,58,108,46,54,240,71,241,211,161,67,207,217,39,79,158,42,134,41,158,223,
125,247,189,29,55,110,252,246,223,115,119,226,169,195,135,159,127,98,100,100,116,148,250,220,91,191,150,27,0,46,54,240,71,241,211,143,126,116,104,211,59,239,156,188,26,198,120,190,124,249,234,239,156,
57,115,230,207,185,75,241,157,9,96,199,192,222,250,181,212,0,112,177,129,63,138,167,194,92,124,231,231,95,63,205,29,34,6,169,247,157,243,51,184,56,248,1,127,20,118,248,103,50,3,95,169,138,184,83,241,22,
59,6,246,206,143,78,11,63,224,143,36,106,211,174,196,42,49,73,189,111,207,175,169,6,128,139,13,252,17,240,15,83,60,19,179,201,141,77,234,125,123,126,185,92,110,220,243,60,195,224,226,224,71,33,165,192,
70,49,158,121,28,16,127,177,99,96,119,224,239,186,37,83,164,129,53,0,217,108,118,178,124,170,16,23,27,248,35,224,31,178,247,152,105,2,18,213,4,80,239,219,243,155,158,158,158,168,156,250,43,235,54,0,101,
248,175,140,250,79,21,226,98,3,127,4,252,195,20,207,52,1,201,104,2,216,49,176,125,248,23,139,78,198,127,234,175,177,22,252,29,39,63,92,251,25,46,54,240,71,192,63,124,241,44,194,142,129,113,23,59,6,182,
227,151,157,116,156,194,160,4,214,253,169,213,58,5,199,201,111,240,60,109,138,232,148,214,229,95,130,139,13,252,17,240,15,115,126,176,99,96,188,197,142,129,173,193,191,88,44,12,137,136,86,202,112,149,
82,174,97,24,238,177,99,175,125,174,234,195,191,48,168,117,21,252,58,37,34,170,124,176,0,23,27,248,35,224,31,238,252,184,125,251,238,159,28,63,126,60,203,29,143,167,246,237,219,179,243,253,247,207,126,
196,142,129,141,142,252,243,195,34,74,43,37,158,82,134,107,24,70,201,52,45,103,102,102,230,70,205,5,204,229,114,227,142,83,24,208,218,51,181,22,163,210,0,168,116,58,115,27,184,2,127,4,252,163,144,31,236,
24,72,76,83,239,203,126,133,66,126,164,12,127,229,86,26,128,146,109,167,87,170,7,255,153,126,248,87,86,7,170,251,23,88,60,219,30,184,179,222,145,130,92,108,224,143,128,127,88,252,170,223,151,152,143,183,
170,247,183,145,248,78,40,252,199,68,68,139,136,91,241,211,150,101,231,253,167,254,222,91,224,231,186,37,211,191,58,80,41,209,182,157,1,254,192,31,1,255,72,230,7,111,7,36,171,17,160,222,215,250,249,223,
222,43,243,60,189,60,59,59,123,221,255,89,163,252,3,217,201,64,217,208,150,149,94,2,254,192,31,1,255,40,231,7,77,64,178,155,0,248,81,158,201,183,172,244,82,16,254,53,51,0,254,107,105,219,233,101,224,15,
252,17,240,143,67,126,208,4,36,167,9,96,211,160,160,159,120,107,205,228,27,129,68,209,150,101,231,235,117,10,192,21,248,35,224,31,85,63,154,128,228,53,2,140,252,149,151,78,15,220,90,107,48,95,211,0,152,
166,229,0,127,224,143,128,127,28,243,131,38,32,57,58,120,240,224,191,73,58,63,202,175,238,175,61,147,127,175,1,72,165,204,146,127,117,32,197,3,248,35,224,31,55,63,17,118,12,76,130,78,158,60,249,15,231,
231,127,122,234,155,111,190,185,13,63,214,168,25,237,92,100,96,13,252,17,240,143,162,31,59,6,146,31,228,71,27,13,0,176,6,254,136,226,22,101,191,203,151,175,254,206,153,51,103,254,156,8,34,79,146,154,31,
70,43,23,146,226,1,252,17,240,143,186,31,240,79,150,130,111,9,144,31,45,52,0,20,15,224,143,128,127,28,252,84,69,68,82,242,26,1,242,163,242,147,20,15,224,143,128,127,210,243,141,92,34,135,146,184,32,150,
103,34,192,31,1,127,242,141,156,74,116,62,37,45,63,114,185,220,184,231,121,134,65,241,0,254,8,248,227,199,62,1,73,85,210,54,13,202,229,114,227,174,91,50,69,26,88,3,144,205,102,39,203,167,10,81,60,128,
63,2,254,236,24,136,226,167,91,183,110,255,89,18,242,99,122,122,122,162,114,234,111,217,105,125,248,175,140,106,173,173,251,191,0,197,3,248,35,224,31,111,63,114,45,153,122,246,217,253,219,78,157,58,253,
73,28,243,109,122,122,122,162,88,116,50,74,41,207,48,12,87,41,195,51,214,130,191,227,228,135,107,247,197,247,100,0,0,20,66,73,68,65,84,103,9,40,30,192,31,1,255,248,251,137,176,99,96,18,117,234,212,233,
79,90,249,185,240,231,71,118,210,113,10,131,193,65,191,90,173,83,112,156,252,6,207,211,166,136,78,105,93,254,37,40,30,192,31,1,255,36,249,177,99,32,185,23,7,248,23,139,133,33,17,209,74,25,174,82,202,53,
12,195,61,118,236,181,207,141,250,240,47,12,106,125,191,57,80,74,121,20,15,224,143,128,127,210,252,14,28,120,238,28,51,1,201,84,35,27,7,69,99,228,159,31,14,240,92,167,82,102,73,36,176,8,48,151,203,141,
23,139,78,70,68,251,11,143,87,62,85,136,226,1,252,17,240,103,199,64,148,188,70,32,170,241,92,94,195,87,195,121,109,154,150,83,61,248,207,240,195,191,178,58,208,215,41,72,229,60,97,138,7,240,71,192,159,
29,3,17,77,64,116,224,159,31,171,29,228,43,109,89,118,222,127,234,239,189,255,232,186,37,83,107,237,135,191,182,237,204,157,245,206,19,166,120,0,127,4,252,147,224,71,19,64,19,80,85,20,226,89,107,109,212,
242,60,189,60,59,59,123,221,255,89,163,252,3,217,201,64,25,210,150,149,94,2,254,192,31,1,127,252,218,187,71,40,94,218,184,113,242,119,162,20,207,74,137,103,89,233,165,32,252,69,234,111,4,164,109,59,189,
12,252,129,63,2,254,248,117,230,94,161,248,232,139,47,174,253,151,249,249,159,158,138,70,60,139,183,214,76,190,25,8,108,109,154,86,190,94,167,64,241,0,254,8,248,227,119,255,158,145,179,201,86,181,9,8,
107,254,42,165,42,107,248,86,31,204,215,204,0,152,166,229,0,127,224,143,128,63,126,221,185,119,40,126,106,182,126,247,42,158,203,111,239,173,61,147,127,175,1,72,165,204,146,127,117,32,201,14,252,17,240,
199,111,125,177,79,0,106,100,207,128,80,206,100,181,243,165,41,30,192,31,1,127,252,116,234,211,79,63,155,58,123,246,195,127,70,100,163,213,114,59,140,241,220,114,3,64,241,0,254,8,248,227,119,223,239,203,
47,191,250,187,39,79,190,251,247,137,240,230,244,248,227,91,142,236,222,189,107,89,68,185,243,243,175,159,94,45,63,162,84,51,131,223,33,172,241,108,2,127,224,143,128,63,126,237,251,1,255,198,181,119,239,
238,167,55,111,222,252,128,136,200,238,221,187,196,191,233,82,163,185,19,230,58,170,181,214,213,223,55,204,241,108,144,236,192,31,1,127,252,218,247,99,199,192,245,181,111,223,158,244,212,212,171,251,171,
240,111,231,126,84,175,245,206,157,79,237,13,107,19,16,246,77,131,20,201,14,252,17,240,199,175,179,126,228,122,125,77,77,189,186,191,91,247,227,236,217,15,31,252,244,211,207,126,22,182,239,252,253,239,
79,254,193,15,126,176,255,139,176,197,115,83,13,0,201,14,252,17,240,199,175,113,63,114,254,126,252,107,173,117,55,225,95,245,251,237,111,191,252,245,59,239,188,231,132,241,58,252,248,199,47,109,250,249,
207,79,92,9,67,61,200,229,114,227,158,231,25,138,100,7,254,8,248,227,215,29,191,164,231,190,82,74,245,227,126,132,249,186,55,91,19,186,1,127,215,45,153,74,25,158,177,254,63,158,157,44,159,42,68,178,3,
127,4,252,241,107,118,199,64,224,207,193,77,173,50,161,211,215,111,122,122,122,162,114,234,175,136,172,179,8,176,12,255,149,81,255,169,66,36,59,240,71,192,31,63,118,12,12,43,252,163,210,4,172,199,135,
110,192,191,88,116,50,254,83,127,141,181,224,239,56,249,225,218,207,144,236,192,31,1,127,252,90,137,139,131,7,15,252,91,224,207,193,77,141,112,162,243,215,47,59,233,56,133,65,9,172,251,83,171,117,10,142,
147,223,224,121,218,20,209,41,173,203,191,4,201,14,252,17,240,199,175,117,191,119,223,125,111,199,141,27,191,253,247,192,191,183,247,35,74,107,2,186,1,255,98,177,48,36,34,90,41,195,85,74,185,134,97,184,
199,142,189,246,185,81,31,254,133,65,173,239,55,7,74,41,143,100,7,254,8,248,227,215,158,223,129,3,207,157,139,235,76,64,88,225,31,37,117,103,228,159,31,14,240,92,167,82,102,73,36,240,8,32,151,203,141,
23,139,78,70,68,251,11,153,87,62,85,136,100,7,254,8,248,227,215,174,95,92,119,12,12,243,166,55,81,88,135,209,141,248,43,175,225,171,225,188,54,77,203,169,30,252,103,248,225,95,89,29,232,235,20,164,114,
158,48,201,14,252,17,240,199,175,19,126,113,221,49,176,23,239,249,183,227,87,254,253,194,217,56,117,163,121,42,191,189,87,179,134,79,91,150,157,247,159,250,123,239,44,0,215,45,153,254,213,129,74,137,182,
172,204,221,245,206,19,38,217,129,63,240,39,63,240,107,222,175,186,65,78,212,115,224,208,161,231,236,239,125,239,209,93,81,184,31,19,19,227,255,237,245,235,55,254,50,25,205,147,136,82,202,173,242,220,
182,211,203,179,179,179,215,253,159,53,202,63,144,157,12,148,53,109,89,233,37,224,15,252,17,240,199,175,123,126,113,152,9,136,10,252,69,68,158,123,238,135,23,194,121,21,187,119,253,148,18,207,178,210,
75,65,248,139,212,127,13,80,219,118,122,25,248,3,127,4,252,241,235,190,95,148,155,128,125,251,246,236,140,218,253,8,121,101,233,240,247,21,207,182,51,119,86,227,185,17,8,68,109,89,118,190,94,167,64,178,
3,127,4,252,241,235,142,95,84,155,128,77,155,54,13,70,233,126,204,207,191,126,58,140,215,241,202,149,43,78,231,71,254,170,178,134,111,245,193,124,77,3,96,154,150,3,252,129,63,2,254,248,245,222,47,106,
77,192,209,163,175,156,142,218,253,8,235,2,204,13,27,70,254,247,78,127,223,242,219,123,107,207,228,223,107,0,82,41,179,228,95,29,72,114,2,127,4,252,241,235,173,159,72,116,118,12,188,120,241,226,255,146,
132,251,209,11,157,56,113,226,143,250,50,243,212,206,47,13,252,17,240,39,63,240,75,230,142,129,175,188,114,100,203,224,224,134,209,40,222,143,48,214,242,126,156,18,104,180,250,203,38,45,57,199,199,31,
29,2,131,192,159,252,192,175,23,126,7,14,60,119,238,153,103,158,249,39,97,206,139,168,194,63,172,234,199,41,129,45,205,0,48,242,71,192,159,252,192,47,153,11,214,226,144,31,97,173,233,141,92,207,78,94,
63,131,228,92,223,239,241,199,183,252,8,20,2,127,242,3,63,22,172,197,59,63,194,174,78,127,95,131,228,92,223,239,210,165,203,191,4,135,192,159,252,192,175,95,59,6,2,175,206,250,69,241,154,118,163,217,49,
73,206,181,253,174,92,249,204,21,4,252,201,15,252,250,188,99,96,148,31,69,134,237,126,132,245,90,106,173,117,189,250,211,233,235,119,244,232,203,74,68,12,131,228,92,219,239,253,247,207,158,1,137,192,159,
252,192,143,29,3,227,1,255,164,95,191,10,252,77,145,6,30,1,100,179,217,201,242,169,66,73,78,78,4,252,129,23,126,253,247,139,90,19,0,252,67,7,127,67,68,210,82,121,1,192,88,31,254,43,163,90,107,35,137,201,
121,251,246,173,91,96,17,248,3,47,252,194,228,23,149,38,0,248,183,166,234,227,137,46,193,127,64,124,111,255,25,107,193,223,113,242,195,181,159,73,86,114,94,186,116,249,9,208,8,252,129,23,126,97,132,87,
191,119,12,92,235,57,58,240,111,255,218,118,24,254,41,17,25,146,192,171,255,117,11,223,244,244,244,132,227,228,55,120,158,54,69,116,74,235,242,47,193,123,184,8,248,3,47,252,194,227,247,233,167,159,77,
157,61,251,225,63,11,83,254,176,175,75,251,154,154,122,117,127,135,225,191,65,68,180,136,184,149,63,165,133,133,197,162,81,31,254,133,65,173,239,55,7,74,41,15,248,35,224,15,188,240,11,151,223,214,173,
155,231,195,148,83,81,184,126,47,188,112,104,107,152,235,210,243,207,31,124,188,195,240,31,145,218,217,126,45,34,37,9,252,165,228,114,185,241,98,209,201,136,104,127,97,244,202,167,10,37,111,19,14,16,9,
252,129,13,126,97,247,59,116,232,57,187,95,121,228,31,77,71,229,250,221,185,179,244,119,195,90,151,142,28,121,97,243,195,15,63,252,80,135,224,111,138,200,88,29,248,23,22,22,22,117,77,3,144,203,229,198,
75,165,162,45,226,31,249,75,229,60,97,146,19,1,127,96,131,95,24,253,78,158,60,85,100,228,223,184,95,63,31,153,172,165,23,94,56,180,245,193,7,71,30,233,32,252,31,169,3,255,149,42,252,107,26,0,215,45,153,
90,107,63,252,181,109,103,238,172,119,158,48,201,137,128,63,126,248,245,215,175,159,51,150,157,94,176,214,205,235,119,225,194,197,80,214,165,67,135,158,123,98,108,108,108,172,131,240,127,84,68,252,175,
178,123,34,178,180,176,176,232,249,63,107,148,47,80,118,50,80,38,181,101,165,151,128,63,2,254,192,6,191,104,248,245,171,9,216,180,233,177,151,163,114,253,126,243,155,115,167,194,56,242,255,222,247,190,
55,218,97,248,155,1,248,223,13,194,95,164,254,107,128,218,182,211,203,192,31,1,127,96,131,31,245,106,61,93,185,114,117,49,10,215,239,220,185,143,173,176,213,165,131,7,159,125,178,11,35,255,32,252,111,
47,44,44,214,221,210,222,8,20,73,109,89,118,126,118,118,246,58,201,132,128,63,176,193,47,90,126,253,154,5,56,115,230,236,143,194,126,253,206,159,191,16,170,67,221,158,127,254,224,227,143,62,250,232,72,
23,225,239,138,200,205,213,224,47,129,15,139,105,90,206,204,204,204,13,146,9,1,127,96,131,95,52,253,250,113,112,208,229,203,87,254,77,152,243,237,214,173,59,127,26,166,186,116,224,192,15,158,28,31,159,
232,228,106,255,32,252,75,34,242,213,90,240,175,105,0,82,41,179,4,252,17,240,7,54,248,69,223,175,95,167,7,174,118,154,93,63,175,223,251,239,159,25,186,114,229,234,31,133,105,228,223,3,248,127,185,176,
176,88,90,55,78,218,249,34,73,72,166,40,31,193,9,252,137,103,252,146,235,183,99,199,246,244,71,31,125,156,15,107,238,245,226,250,125,246,217,103,223,126,240,193,135,191,9,217,200,191,155,211,254,13,195,
95,164,129,211,0,73,38,4,252,129,13,126,209,243,219,182,109,219,206,29,59,182,31,232,199,44,64,24,174,223,151,95,126,121,59,76,240,127,246,217,253,219,194,4,255,150,27,0,224,143,128,63,126,248,133,223,
111,251,246,109,222,196,196,120,46,76,77,64,47,174,223,197,139,151,10,39,79,158,186,16,150,186,244,252,243,7,31,159,156,156,124,48,76,240,151,128,1,201,132,128,63,176,193,47,102,126,207,61,247,236,249,
126,228,99,181,9,240,231,101,47,190,239,135,31,254,218,186,116,233,114,104,222,247,127,246,217,253,219,38,39,39,67,241,204,255,59,53,147,100,90,223,143,131,129,128,63,126,248,113,176,89,123,249,217,139,
239,59,63,255,211,80,109,244,243,195,31,238,219,190,113,227,198,225,48,194,191,169,25,0,146,9,1,127,252,240,139,238,193,102,253,92,208,172,181,214,207,63,127,240,241,135,31,126,184,43,223,247,236,217,
15,31,252,244,211,207,126,22,166,186,116,224,192,15,158,220,184,113,99,168,158,249,251,252,148,136,24,138,100,90,223,239,227,143,63,54,63,254,248,194,59,160,22,248,227,135,95,148,253,250,253,86,211,79,
126,242,147,143,238,222,189,253,251,15,61,52,182,97,126,254,245,211,237,230,239,185,115,231,236,243,231,47,190,29,182,186,244,131,31,236,125,234,251,223,255,254,134,16,195,223,20,17,79,173,127,177,179,
147,133,194,202,195,90,123,22,211,104,8,248,227,135,95,180,253,194,246,106,115,51,121,172,181,214,251,246,237,217,185,97,195,131,127,113,226,196,91,83,97,172,75,33,135,191,33,34,182,148,183,8,118,85,3,
240,31,213,90,91,247,3,12,248,35,224,143,31,126,81,245,99,111,147,238,105,255,254,103,118,60,246,216,99,67,33,134,255,64,21,254,34,226,26,107,193,223,113,242,195,82,243,170,96,178,159,161,33,224,143,31,
126,81,247,163,158,117,71,207,62,187,127,91,136,225,159,18,145,33,9,44,252,175,219,0,76,79,79,79,20,139,133,33,173,129,127,24,206,219,70,192,31,63,252,58,233,71,61,235,60,252,195,248,158,191,15,254,27,
2,240,215,11,11,139,223,157,1,152,158,158,158,112,156,194,160,214,247,63,172,148,242,72,38,4,252,241,195,47,62,126,52,1,137,129,255,72,96,176,175,43,158,181,51,0,185,92,110,188,88,116,50,34,218,31,24,
94,58,157,185,73,50,145,48,192,31,63,252,226,229,247,210,75,207,111,161,186,196,22,254,166,136,140,213,129,127,97,97,97,81,215,52,0,185,92,110,188,84,42,218,254,105,2,165,196,75,167,7,110,145,76,8,248,
227,135,95,252,252,70,71,71,31,126,234,169,39,15,81,101,154,215,15,127,184,111,123,200,225,255,72,29,248,175,84,225,95,211,0,184,110,201,212,90,251,225,175,109,59,115,103,110,110,238,26,201,196,44,0,240,
199,15,191,120,250,61,245,212,83,165,131,7,15,254,223,84,155,230,224,31,214,29,254,124,126,254,77,236,60,17,89,90,88,88,244,252,159,53,202,1,145,157,12,148,93,109,89,233,37,224,79,19,0,252,241,195,47,
254,126,39,79,158,252,7,84,156,88,193,223,12,192,255,110,16,254,34,245,223,2,208,182,157,94,6,254,8,248,227,135,95,50,252,24,212,196,26,254,183,23,22,22,221,122,63,99,4,138,174,182,44,59,63,59,59,123,
157,100,66,192,31,63,252,146,227,71,19,176,182,246,237,219,179,51,98,240,119,69,228,230,106,240,255,78,3,96,154,150,3,252,27,243,155,154,122,117,255,43,175,28,217,66,90,0,127,252,240,139,139,31,77,192,
234,240,223,180,105,211,96,132,224,95,18,145,175,214,130,127,77,3,144,74,153,165,153,153,153,27,36,83,227,126,131,131,67,15,239,218,181,227,135,164,7,240,199,15,191,184,248,209,4,212,106,239,222,221,79,
71,16,254,13,249,181,117,163,73,166,178,31,103,5,0,127,252,240,139,147,31,231,5,220,135,255,230,205,155,31,136,35,252,107,102,0,8,254,214,253,232,152,129,63,126,248,197,201,143,154,22,127,248,183,220,
0,144,76,237,249,33,224,143,31,126,97,247,75,114,19,144,4,248,183,212,0,144,76,245,253,232,152,129,63,126,248,49,88,137,190,158,121,102,215,174,36,192,95,164,201,53,0,36,211,250,126,60,59,3,254,248,225,
23,39,191,175,191,254,237,242,219,111,191,123,62,41,240,223,178,101,203,64,18,224,223,84,3,64,50,53,238,71,19,0,252,241,195,47,78,126,31,126,248,235,193,75,151,46,191,9,252,227,1,255,163,71,95,86,34,98,
24,4,127,231,253,120,28,0,252,241,195,47,78,126,187,119,239,190,3,252,99,5,127,83,164,129,53,0,217,108,118,178,80,200,143,145,76,44,160,1,254,248,225,151,92,191,184,214,180,61,123,158,222,157,32,248,27,
34,146,150,202,236,191,177,62,252,87,70,181,214,6,201,196,2,26,224,143,31,126,248,197,13,254,91,183,110,205,36,8,254,3,226,123,244,175,214,130,191,227,228,135,181,214,150,214,58,85,14,10,130,191,21,191,
249,249,159,158,2,254,196,11,126,248,197,193,47,46,107,156,118,238,124,106,239,182,109,79,90,9,129,127,74,68,6,165,124,56,144,91,253,83,119,6,96,122,122,122,162,88,44,12,105,45,140,252,59,224,55,53,245,
123,207,2,127,226,5,63,252,226,224,23,135,71,1,9,132,255,134,192,128,95,47,44,44,126,183,1,152,158,158,158,112,156,194,160,214,190,105,2,165,60,130,191,61,63,224,79,188,224,135,95,92,252,162,220,4,36,
16,254,35,82,251,184,95,87,60,107,215,0,228,114,185,241,98,209,201,136,104,255,205,245,210,233,204,77,130,191,61,63,85,17,240,39,94,240,195,143,103,254,192,191,71,126,99,117,224,95,88,88,88,212,53,13,
64,46,151,27,47,149,138,182,127,154,64,41,241,210,233,129,91,4,127,231,252,146,208,4,0,127,252,240,139,191,95,212,106,217,206,157,219,247,37,12,254,143,212,129,255,74,21,254,53,13,128,235,150,76,173,181,
31,254,218,182,51,119,230,230,230,174,17,252,157,245,139,115,19,0,252,241,195,47,57,126,81,169,101,101,248,111,51,19,4,255,71,69,196,119,127,197,19,145,165,133,133,69,207,255,89,163,28,16,217,201,192,
109,213,150,149,94,2,254,221,243,139,99,19,0,252,241,195,47,121,126,83,83,175,238,7,254,161,131,191,25,128,255,221,32,252,69,234,239,3,160,109,59,189,12,252,187,239,23,167,38,0,248,227,135,95,114,253,
158,123,238,7,59,195,88,151,118,236,216,190,31,248,203,237,133,133,197,186,11,209,141,64,17,215,150,101,231,103,103,103,175,19,252,189,241,139,195,226,64,224,143,31,126,201,246,155,152,152,24,124,236,
177,141,71,195,6,255,237,219,183,117,228,251,70,20,254,174,136,220,92,13,254,223,105,0,76,211,114,128,127,255,166,209,246,239,223,187,27,248,19,47,248,225,23,69,191,171,87,191,88,0,254,161,242,251,106,
45,248,215,52,0,169,148,89,154,153,153,185,65,240,247,207,239,177,199,190,159,153,154,250,189,103,15,31,126,233,53,224,143,31,126,248,69,201,47,44,179,153,192,191,113,191,182,110,22,193,223,29,191,249,
249,215,79,3,127,252,240,195,47,138,126,253,220,42,120,219,182,199,159,219,185,115,167,6,254,141,249,25,173,94,104,130,191,251,157,116,88,215,6,0,127,252,240,195,111,53,191,126,213,45,224,223,188,95,75,
13,0,193,223,219,100,10,83,35,0,252,241,195,15,191,245,252,122,93,179,30,127,124,203,243,192,191,121,63,131,96,141,134,95,191,27,129,141,27,39,135,128,63,126,248,225,215,168,95,175,106,214,147,79,62,126,
96,247,238,93,14,240,111,222,207,32,88,163,229,215,235,70,96,114,114,252,247,69,68,174,93,187,190,204,253,192,15,63,252,218,121,213,249,200,145,23,54,63,246,216,198,87,59,9,255,167,159,222,233,1,255,230,
253,68,154,88,4,72,240,135,203,79,107,173,167,166,94,221,255,229,151,55,86,44,43,243,127,188,245,214,91,191,223,73,240,111,217,178,249,127,126,226,137,45,127,101,24,198,141,86,130,139,251,139,31,126,248,
173,231,215,206,130,103,224,223,150,159,18,17,67,17,172,241,241,123,255,253,247,207,95,185,242,249,178,180,161,237,219,159,252,209,142,29,79,21,185,31,248,225,135,95,175,253,26,125,131,224,137,39,182,
30,220,181,235,105,23,248,183,12,127,83,68,60,115,253,155,151,157,44,20,86,198,8,214,240,251,93,189,250,197,183,193,199,3,141,36,212,43,175,28,121,108,112,112,232,81,17,145,29,59,158,226,126,224,135,31,
126,161,56,40,173,94,253,2,254,109,249,25,34,98,75,121,139,224,181,31,1,84,224,63,170,181,182,238,223,64,130,53,46,126,213,199,8,92,63,252,240,195,47,10,245,106,231,206,167,246,38,236,72,223,78,195,127,
160,2,127,87,68,92,181,22,252,29,39,63,172,181,182,180,214,169,242,77,36,88,241,195,15,63,252,240,139,182,95,2,225,159,18,145,65,63,252,69,196,173,251,22,192,244,244,244,68,177,88,24,210,218,255,150,0,
193,133,31,126,248,225,135,31,240,143,32,252,55,72,237,140,191,94,88,88,116,205,122,240,119,156,194,160,214,247,63,172,148,242,210,233,204,77,130,11,63,252,240,195,15,63,224,31,41,248,143,136,136,174,
140,250,165,242,255,151,68,2,251,0,228,114,185,241,98,209,201,136,104,127,167,0,252,241,195,15,63,252,240,3,254,209,243,27,11,112,94,139,72,97,97,97,81,139,255,31,202,229,114,227,165,82,209,246,79,19,
40,37,158,109,15,220,153,155,155,187,70,112,225,135,31,126,248,225,7,252,35,227,247,72,157,145,255,74,21,254,53,13,128,235,150,76,173,181,31,254,218,178,50,119,129,63,126,248,225,135,31,126,192,63,114,
126,170,226,35,82,94,252,183,180,176,176,232,249,63,107,148,47,120,118,178,214,66,105,203,74,47,1,127,252,240,195,15,63,252,128,127,164,253,60,17,185,27,132,255,189,6,32,32,109,219,233,101,224,143,31,
126,248,225,135,31,240,143,60,252,111,47,44,44,186,245,126,166,230,45,0,165,148,54,77,43,63,59,59,123,157,224,194,15,63,252,240,195,15,248,71,214,207,21,145,155,171,193,255,59,51,0,166,105,57,192,31,63,
252,240,195,15,63,224,31,121,191,175,214,130,127,77,3,144,74,153,165,153,153,153,27,4,23,126,248,225,135,31,126,192,63,254,126,109,157,43,79,176,226,135,31,126,248,225,7,252,163,233,215,114,3,64,176,226,
135,31,126,248,225,7,252,163,235,103,74,11,202,102,255,120,99,161,144,31,171,222,52,145,246,183,11,198,15,63,252,240,195,15,191,22,97,248,72,101,64,91,125,230,237,74,249,25,120,9,191,14,206,0,84,143,8,
150,218,5,132,94,58,61,112,171,181,87,7,241,195,15,63,252,240,195,175,121,191,202,94,247,193,237,110,61,89,103,245,59,126,45,52,0,247,143,8,190,255,143,151,183,11,206,220,105,53,24,240,195,15,63,252,240,
195,175,69,24,142,212,129,225,237,54,224,154,24,191,166,26,128,108,54,59,89,57,34,56,176,93,112,171,59,6,226,135,31,126,248,225,135,95,203,240,223,80,7,134,119,219,128,107,146,252,148,136,24,13,53,0,213,
35,130,107,79,9,84,218,182,211,203,173,236,27,128,31,126,248,225,135,31,126,173,248,29,61,250,178,33,34,67,129,1,172,150,58,123,221,227,183,42,252,77,17,241,84,35,55,175,124,68,112,237,63,110,89,118,190,
213,96,192,15,63,252,240,195,15,191,22,97,56,80,7,134,43,109,192,53,105,126,118,101,246,192,93,243,45,128,202,17,193,150,82,202,187,63,109,163,180,105,90,78,43,155,6,225,135,31,126,248,225,135,95,43,126,
149,145,107,21,94,126,24,22,252,71,220,226,183,174,223,253,199,48,107,221,60,215,45,125,167,65,104,117,199,64,252,240,195,15,63,252,240,107,3,254,102,157,145,112,169,13,24,38,221,111,245,25,0,207,243,
12,165,140,154,41,134,86,22,107,224,135,31,126,248,225,135,95,59,126,82,94,252,86,227,215,234,202,119,252,238,251,253,255,0,183,164,66,192,181,16,92,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_encoderAudioProcessorEditor::settings_white_png = (const char*) resource_Ambix_encoderAudioProcessorEditor_settings_white_png;
const int Ambix_encoderAudioProcessorEditor::settings_white_pngSize = 21725;


//[EndFile] You can add extra defines here...
//[/EndFile]
