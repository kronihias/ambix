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

#include "Settings.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Settings::Settings (Ambix_encoderAudioProcessor& Processor)
    : _processor(Processor)
{
    addAndMakeVisible (txt_snd_ip = new TextEditor ("new text editor"));
    txt_snd_ip->setTooltip (TRANS("send ip address, specify multiple addresses by separating them with semicolon ;"));
    txt_snd_ip->addListener (this);
    txt_snd_ip->setMultiLine (false);
    txt_snd_ip->setReturnKeyStartsNewLine (false);
    txt_snd_ip->setReadOnly (false);
    txt_snd_ip->setScrollbarsShown (false);
    txt_snd_ip->setCaretVisible (true);
    txt_snd_ip->setPopupMenuEnabled (true);
    txt_snd_ip->setText (TRANS("127.0.0.1"));

    addAndMakeVisible (txt_snd_port = new TextEditor ("new text editor"));
    txt_snd_port->setTooltip (TRANS("send port, specify multiple ports by separating them with semicolon ;"));
    txt_snd_port->addListener (this);
    txt_snd_port->setMultiLine (false);
    txt_snd_port->setReturnKeyStartsNewLine (false);
    txt_snd_port->setReadOnly (false);
    txt_snd_port->setScrollbarsShown (false);
    txt_snd_port->setCaretVisible (true);
    txt_snd_port->setPopupMenuEnabled (true);
    txt_snd_port->setText (TRANS("8000"));

    addAndMakeVisible (tgl_snd_active = new ToggleButton ("new toggle button"));
    tgl_snd_active->setButtonText (TRANS("active"));
    tgl_snd_active->addListener (this);
    tgl_snd_active->setToggleState (true, dontSendNotification);
    tgl_snd_active->setColour (ToggleButton::textColourId, Colours::black);

    addAndMakeVisible (txt_rcv_port = new TextEditor ("new text editor"));
    txt_rcv_port->setTooltip (TRANS("osc receive port - open port is configured automatically!"));
    txt_rcv_port->setMultiLine (false);
    txt_rcv_port->setReturnKeyStartsNewLine (false);
    txt_rcv_port->setReadOnly (true);
    txt_rcv_port->setScrollbarsShown (false);
    txt_rcv_port->setCaretVisible (false);
    txt_rcv_port->setPopupMenuEnabled (true);
    txt_rcv_port->setText (TRANS("8000"));

    addAndMakeVisible (tgl_rcv_active = new ToggleButton ("new toggle button"));
    tgl_rcv_active->setButtonText (TRANS("active"));
    tgl_rcv_active->addListener (this);
    tgl_rcv_active->setToggleState (true, dontSendNotification);
    tgl_rcv_active->setColour (ToggleButton::textColourId, Colours::black);

    addAndMakeVisible (lbl_id = new Label ("new label",
                                           TRANS("ID: 1")));
    lbl_id->setFont (Font (15.00f, Font::plain));
    lbl_id->setJustificationType (Justification::centredRight);
    lbl_id->setEditable (false, false, false);
    lbl_id->setColour (Label::textColourId, Colour (0xff888888));
    lbl_id->setColour (TextEditor::textColourId, Colours::black);
    lbl_id->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider = new Slider ("new slider"));
    slider->setTooltip (TRANS("interval between two OSC messages are sent"));
    slider->setRange (1, 1000, 1);
    slider->setTextValueSuffix(" ms");
    slider->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider->setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    slider->setColour (Slider::thumbColourId, Colours::black);
    slider->setColour (Slider::rotarySliderFillColourId, Colours::black);
    slider->addListener (this);
    slider->setSkewFactor (0.6);
    slider->setDoubleClickReturnValue(true, 50.f);
    //[UserPreSize]
    //[/UserPreSize]

    setSize (203, 290);


    //[Constructor] You can add your own custom stuff here..
    updateSettings();
    
    String str_id = "ID: ";
    str_id << _processor.m_id;
    lbl_id->setText(str_id, dontSendNotification);
    //[/Constructor]
}

Settings::~Settings()
{
    _processor.myProperties.saveIfNeeded();
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    txt_snd_ip = nullptr;
    txt_snd_port = nullptr;
    tgl_snd_active = nullptr;
    txt_rcv_port = nullptr;
    tgl_rcv_active = nullptr;
    lbl_id = nullptr;
    slider = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Settings::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       static_cast<float> (proportionOfWidth (0.6314f)), static_cast<float> (proportionOfHeight (0.5842f)),
                                       Colours::black,
                                       static_cast<float> (proportionOfWidth (0.1143f)), static_cast<float> (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 220, 360);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 220, 360, 1);

    g.setColour (Colour (0xff8ea4aa));
    g.fillRoundedRectangle (19.0f, 45.0f, 163.0f, 90.0f, 4.000f);

    g.setColour (Colours::black);
    g.setFont (Font (14.20f, Font::bold));
    g.drawText (TRANS("OSC send"),
                101, 46, 80, 26,
                Justification::centredLeft, true);

    g.setColour (Colours::white);
    g.setFont (Font (17.20f, Font::bold));
    g.drawText (TRANS("ambix_encoder settings"),
                -2, 2, 208, 30,
                Justification::centred, true);

    g.setColour (Colours::black);
    g.setFont (Font (14.20f, Font::plain));
    g.drawText (TRANS("ip"),
                24, 78, 25, 26,
                Justification::centredRight, true);

    g.setColour (Colours::black);
    g.setFont (Font (14.20f, Font::plain));
    g.drawText (TRANS("port"),
                24, 101, 25, 26,
                Justification::centredRight, true);

    g.setColour (Colour (0xff8ea4aa));
    g.fillRoundedRectangle (19.0f, 149.0f, 163.0f, 58.0f, 4.000f);

    g.setColour (Colours::black);
    g.setFont (Font (14.20f, Font::bold));
    g.drawText (TRANS("OSC rcv"),
                101, 150, 80, 26,
                Justification::centredLeft, true);

    g.setColour (Colours::black);
    g.setFont (Font (14.20f, Font::plain));
    g.drawText (TRANS("port"),
                24, 175, 25, 26,
                Justification::centredRight, true);

    g.setColour (Colour (0xff8ea4aa));
    g.fillRoundedRectangle (19.0f, 220.0f, 163.0f, 52.0f, 4.000f);

    g.setColour (Colours::black);
    g.setFont (Font (14.20f, Font::bold));
    g.drawText (TRANS("OSC send interval"),
                43, 220, 133, 26,
                Justification::centredLeft, true);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Settings::resized()
{
    txt_snd_ip->setBounds (56, 80, 120, 20);
    txt_snd_port->setBounds (56, 106, 120, 20);
    tgl_snd_active->setBounds (24, 48, 80, 24);
    txt_rcv_port->setBounds (56, 178, 120, 20);
    tgl_rcv_active->setBounds (24, 152, 80, 24);
    lbl_id->setBounds (144, 24, 57, 24);
    slider->setBounds (57, 244, 96, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Settings::updateSettings()
{
    tgl_rcv_active->setToggleState(_processor.osc_in, dontSendNotification);
    
    tgl_snd_active->setToggleState(_processor.osc_out, dontSendNotification);
    
    txt_snd_ip->setText(_processor.osc_out_ip);
    
    txt_snd_port->setText(_processor.osc_out_port);
    
    txt_rcv_port->setText(_processor.osc_in_port);
    
    slider->setValue(_processor.osc_interval);
}

void Settings::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == tgl_snd_active)
    {
        //[UserButtonCode_tgl_snd_active] -- add your button handler code here..
        _processor.oscOut(tgl_snd_active->getToggleState());
        
        _processor.myProperties.getUserSettings()->setValue("osc_out", tgl_snd_active->getToggleState());
        
        //[/UserButtonCode_tgl_snd_active]
    }
    else if (buttonThatWasClicked == tgl_rcv_active)
    {
        //[UserButtonCode_tgl_rcv_active] -- add your button handler code here..
        _processor.oscIn(tgl_rcv_active->getToggleState());
        _processor.myProperties.getUserSettings()->setValue("osc_in", tgl_rcv_active->getToggleState());
        //[/UserButtonCode_tgl_rcv_active]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void Settings::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider)
    {
        //[UserSliderCode_slider] -- add your slider handling code here..
        _processor.myProperties.getUserSettings()->setValue("osc_out_interval", (int)slider->getValue());
        _processor.changeTimer((int)slider->getValue());
        //[/UserSliderCode_slider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void Settings::textEditorFocusLost (TextEditor& ed)
{
    updateOscSend();
}

void Settings::textEditorReturnKeyPressed (TextEditor& ed)
{
    updateOscSend();
}

void Settings::updateOscSend()
{
    _processor.myProperties.getUserSettings()->setValue("osc_out_ip", txt_snd_ip->getText());
    _processor.myProperties.getUserSettings()->setValue("osc_out_port", txt_snd_port->getText());
    
    
    if (  _processor.osc_out && ( !_processor.osc_out_ip.equalsIgnoreCase(txt_snd_ip->getText()) || !_processor.osc_out_port.equalsIgnoreCase(txt_snd_port->getText()) )  ) {
        
        _processor.osc_out_ip = txt_snd_ip->getText();
        _processor.osc_out_port = txt_snd_port->getText();
        
        _processor.oscOut(false);
        _processor.oscOut(true);
    }
    
    
}


//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Settings" componentName=""
                 parentClasses="public Component" constructorParams="Ambix_encoderAudioProcessor&amp; Processor"
                 variableInitialisers="_processor(Processor)" snapPixels="8" snapActive="0"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="203"
                 initialHeight="290">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 220 360" fill=" radial: 63.143% 58.421%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="19 45 163 90" cornerSize="4" fill="solid: ff8ea4aa" hasStroke="0"/>
    <TEXT pos="101 46 80 26" fill="solid: ff000000" hasStroke="0" text="OSC send"
          fontname="Default font" fontsize="14.199999999999999289" bold="1"
          italic="0" justification="33"/>
    <TEXT pos="19 5 166 30" fill="solid: ffffffff" hasStroke="0" text="OSC-spat settings"
          fontname="Default font" fontsize="17.199999999999999289" bold="1"
          italic="0" justification="36"/>
    <TEXT pos="24 78 25 26" fill="solid: ff000000" hasStroke="0" text="ip"
          fontname="Default font" fontsize="14.199999999999999289" bold="0"
          italic="0" justification="34"/>
    <TEXT pos="24 101 25 26" fill="solid: ff000000" hasStroke="0" text="port"
          fontname="Default font" fontsize="14.199999999999999289" bold="0"
          italic="0" justification="34"/>
    <ROUNDRECT pos="19 149 163 58" cornerSize="4" fill="solid: ff8ea4aa" hasStroke="0"/>
    <TEXT pos="101 150 80 26" fill="solid: ff000000" hasStroke="0" text="OSC rcv"
          fontname="Default font" fontsize="14.199999999999999289" bold="1"
          italic="0" justification="33"/>
    <TEXT pos="24 175 25 26" fill="solid: ff000000" hasStroke="0" text="port"
          fontname="Default font" fontsize="14.199999999999999289" bold="0"
          italic="0" justification="34"/>
    <ROUNDRECT pos="19 220 163 52" cornerSize="4" fill="solid: ff8ea4aa" hasStroke="0"/>
    <TEXT pos="43 220 133 26" fill="solid: ff000000" hasStroke="0" text="OSC send interval"
          fontname="Default font" fontsize="14.199999999999999289" bold="1"
          italic="0" justification="33"/>
  </BACKGROUND>
  <TEXTEDITOR name="new text editor" id="f2322e5b2ed5ee18" memberName="txt_snd_ip"
              virtualName="" explicitFocusOrder="0" pos="56 80 120 20" tooltip="send ip address, specify multiple addresses by separating them with semicolon ;"
              initialText="127.0.0.1" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="new text editor" id="709e9cc017f44b55" memberName="txt_snd_port"
              virtualName="" explicitFocusOrder="0" pos="56 106 120 20" tooltip="send port, specify multiple ports by separating them with semicolon ;"
              initialText="8000" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="0" caret="1" popupmenu="1"/>
  <TOGGLEBUTTON name="new toggle button" id="d81c1543282e087" memberName="tgl_snd_active"
                virtualName="" explicitFocusOrder="0" pos="24 48 80 24" txtcol="ff000000"
                buttonText="active" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="1"/>
  <TEXTEDITOR name="new text editor" id="d9e633ef474c1796" memberName="txt_rcv_port"
              virtualName="" explicitFocusOrder="0" pos="56 178 120 20" tooltip="osc receive port - open port is configured automatically!"
              initialText="8000" multiline="0" retKeyStartsLine="0" readonly="1"
              scrollbars="0" caret="0" popupmenu="1"/>
  <TOGGLEBUTTON name="new toggle button" id="3fa52a6bbb0d705b" memberName="tgl_rcv_active"
                virtualName="" explicitFocusOrder="0" pos="24 152 80 24" txtcol="ff000000"
                buttonText="active" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="1"/>
  <LABEL name="new label" id="4f5581a62d8489b1" memberName="lbl_id" virtualName=""
         explicitFocusOrder="0" pos="144 24 57 24" textCol="ff888888"
         edTextCol="ff000000" edBkgCol="0" labelText="ID: 1" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <SLIDER name="new slider" id="70bdbe1a6d977cf1" memberName="slider" virtualName=""
          explicitFocusOrder="0" pos="57 244 96 24" tooltip="interval between two OSC messages are sent"
          thumbcol="ff000000" rotarysliderfill="ff000000" min="1" max="200"
          int="1" style="RotaryHorizontalVerticalDrag" textBoxPos="TextBoxLeft"
          textBoxEditable="1" textBoxWidth="60" textBoxHeight="20" skewFactor="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
