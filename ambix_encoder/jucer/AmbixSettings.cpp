/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 May 2013 7:19:37pm

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

#include "AmbixSettings.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
AmbixSettings::AmbixSettings ()
    : txt_osc_out_ip (0),
      label (0),
      label2 (0),
      txt_osc_out_ip2 (0),
      txt_osc_out_ip3 (0),
      label3 (0),
      label4 (0),
      hyperlinkButton (0)
{
    addAndMakeVisible (txt_osc_out_ip = new TextEditor ("new text editor"));
    txt_osc_out_ip->setTooltip ("send OSC to this IP address");
    txt_osc_out_ip->setMultiLine (false);
    txt_osc_out_ip->setReturnKeyStartsNewLine (false);
    txt_osc_out_ip->setReadOnly (false);
    txt_osc_out_ip->setScrollbarsShown (false);
    txt_osc_out_ip->setCaretVisible (true);
    txt_osc_out_ip->setPopupMenuEnabled (true);
    txt_osc_out_ip->setColour (TextEditor::highlightColourId, Colour (0xff2b1d69));
    txt_osc_out_ip->setText ("127.0.0.1");

    addAndMakeVisible (label = new Label ("new label",
                                          "send to ip"));
    label->setFont (Font (15.0000f, Font::plain));
    label->setJustificationType (Justification::centredRight);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::white);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label2 = new Label ("new label",
                                           "send to port"));
    label2->setFont (Font (15.0000f, Font::plain));
    label2->setJustificationType (Justification::centredRight);
    label2->setEditable (false, false, false);
    label2->setColour (Label::textColourId, Colours::white);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (txt_osc_out_ip2 = new TextEditor ("new text editor"));
    txt_osc_out_ip2->setTooltip ("send OSC to this IP address");
    txt_osc_out_ip2->setMultiLine (false);
    txt_osc_out_ip2->setReturnKeyStartsNewLine (false);
    txt_osc_out_ip2->setReadOnly (false);
    txt_osc_out_ip2->setScrollbarsShown (false);
    txt_osc_out_ip2->setCaretVisible (true);
    txt_osc_out_ip2->setPopupMenuEnabled (true);
    txt_osc_out_ip2->setColour (TextEditor::highlightColourId, Colour (0xff2b1d69));
    txt_osc_out_ip2->setText ("8000");

    addAndMakeVisible (txt_osc_out_ip3 = new TextEditor ("new text editor"));
    txt_osc_out_ip3->setTooltip ("send OSC to this IP address");
    txt_osc_out_ip3->setMultiLine (false);
    txt_osc_out_ip3->setReturnKeyStartsNewLine (false);
    txt_osc_out_ip3->setReadOnly (false);
    txt_osc_out_ip3->setScrollbarsShown (false);
    txt_osc_out_ip3->setCaretVisible (true);
    txt_osc_out_ip3->setPopupMenuEnabled (true);
    txt_osc_out_ip3->setColour (TextEditor::highlightColourId, Colour (0xff2b1d69));
    txt_osc_out_ip3->setText ("8000");

    addAndMakeVisible (label3 = new Label ("new label",
                                           "receive port"));
    label3->setFont (Font (15.0000f, Font::plain));
    label3->setJustificationType (Justification::centredRight);
    label3->setEditable (false, false, false);
    label3->setColour (Label::textColourId, Colours::white);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label4 = new Label ("new label",
                                           "AMBIX plug-in suite by\nMatthias Kronlachner"));
    label4->setFont (Font (15.0000f, Font::plain));
    label4->setJustificationType (Justification::centred);
    label4->setEditable (false, false, false);
    label4->setColour (Label::textColourId, Colours::azure);
    label4->setColour (TextEditor::textColourId, Colours::black);
    label4->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (hyperlinkButton = new HyperlinkButton ("www.matthiaskronlachner.com",
                                                              URL ("http://www.matthiaskronlachner.com")));
    hyperlinkButton->setTooltip ("http://www.matthiaskronlachner.com");
    hyperlinkButton->setButtonText ("www.matthiaskronlachner.com");
    hyperlinkButton->setColour (HyperlinkButton::textColourId, Colours::azure);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (320, 300);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

AmbixSettings::~AmbixSettings()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (txt_osc_out_ip);
    deleteAndZero (label);
    deleteAndZero (label2);
    deleteAndZero (txt_osc_out_ip2);
    deleteAndZero (txt_osc_out_ip3);
    deleteAndZero (label3);
    deleteAndZero (label4);
    deleteAndZero (hyperlinkButton);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AmbixSettings::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       (float) (proportionOfWidth (0.6314f)), (float) (proportionOfHeight (0.5842f)),
                                       Colours::black,
                                       (float) (proportionOfWidth (0.1143f)), (float) (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 320, 300);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 320, 300, 1);

    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (20.0f, 206.0f, 276.0f, 81.0f, 4.0000f);

    g.setColour (Colours::white);
    g.setFont (Font (17.2000f, Font::bold));
    g.drawText ("AMBIX-SETTINGS",
                -6, 2, 343, 30,
                Justification::centred, true);

    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (20.0f, 58.0f, 276.0f, 126.0f, 4.0000f);

    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("Open Sound Control",
                24, 57, 90, 21,
                Justification::centred, true);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AmbixSettings::resized()
{
    txt_osc_out_ip->setBounds (127, 82, 99, 24);
    label->setBounds (42, 82, 82, 24);
    label2->setBounds (34, 111, 90, 24);
    txt_osc_out_ip2->setBounds (127, 112, 48, 24);
    txt_osc_out_ip3->setBounds (127, 149, 48, 23);
    label3->setBounds (35, 148, 90, 23);
    label4->setBounds (26, 213, 264, 45);
    hyperlinkButton->setBounds (33, 262, 255, 19);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AmbixSettings" componentName=""
                 parentClasses="" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="0" snapShown="0" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="320" initialHeight="300">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 320 300" fill=" radial: 63.143% 58.421%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="20 206 276 81" cornerSize="4" fill="solid: ff2b1d69" hasStroke="0"/>
    <TEXT pos="-6 2 343 30" fill="solid: ffffffff" hasStroke="0" text="AMBIX-SETTINGS"
          fontname="Default font" fontsize="17.2" bold="1" italic="0" justification="36"/>
    <ROUNDRECT pos="20 58 276 126" cornerSize="4" fill="solid: ff2b1d69" hasStroke="0"/>
    <TEXT pos="24 57 90 21" fill="solid: ffffffff" hasStroke="0" text="Open Sound Control"
          fontname="Default font" fontsize="10" bold="0" italic="0" justification="36"/>
  </BACKGROUND>
  <TEXTEDITOR name="new text editor" id="1825f38c54d767de" memberName="txt_osc_out_ip"
              virtualName="" explicitFocusOrder="0" pos="127 82 99 24" tooltip="send OSC to this IP address"
              hilitecol="ff2b1d69" initialText="127.0.0.1" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <LABEL name="new label" id="51caa0b36a1744d" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="42 82 82 24" textCol="ffffffff" edTextCol="ff000000"
         edBkgCol="0" labelText="send to ip" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="328e5776e35144cf" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="34 111 90 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="send to port" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <TEXTEDITOR name="new text editor" id="4b9e0fe6c57925d" memberName="txt_osc_out_ip2"
              virtualName="" explicitFocusOrder="0" pos="127 112 48 24" tooltip="send OSC to this IP address"
              hilitecol="ff2b1d69" initialText="8000" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="new text editor" id="d25dce86311a344" memberName="txt_osc_out_ip3"
              virtualName="" explicitFocusOrder="0" pos="127 149 48 23" tooltip="send OSC to this IP address"
              hilitecol="ff2b1d69" initialText="8000" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <LABEL name="new label" id="1385fa14ab06c6ea" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="35 148 90 23" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="receive port" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="84e53a1ed2877481" memberName="label4" virtualName=""
         explicitFocusOrder="0" pos="26 213 264 45" textCol="fff0ffff"
         edTextCol="ff000000" edBkgCol="0" labelText="AMBIX plug-in suite by&#10;Matthias Kronlachner"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
  <HYPERLINKBUTTON name="new hyperlink" id="d10872c1ab0bea3d" memberName="hyperlinkButton"
                   virtualName="" explicitFocusOrder="0" pos="33 262 255 19" tooltip="http://www.matthiaskronlachner.com"
                   textCol="fff0ffff" buttonText="www.matthiaskronlachner.com" connectedEdges="0"
                   needsCallback="0" radioGroupId="0" url="http://www.matthiaskronlachner.com"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



//[EndFile] You can add extra defines here...
//[/EndFile]
