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
Ambix_directional_loudnessAudioProcessorEditor::Ambix_directional_loudnessAudioProcessorEditor (Ambix_directional_loudnessAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (lbl_gd = new Label ("new label",
                                           TRANS("ambix_directional_loudness")));
    lbl_gd->setFont (Font (15.00f, Font::plain));
    lbl_gd->setJustificationType (Justification::centredLeft);
    lbl_gd->setEditable (false, false, false);
    lbl_gd->setColour (Label::textColourId, Colours::aquamarine);
    lbl_gd->setColour (TextEditor::textColourId, Colours::black);
    lbl_gd->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (filtergraph = new Component());
    filtergraph->setName ("new component");

    addAndMakeVisible (btn_drag = new ImageButton ("new button"));
    btn_drag->addListener (this);

    btn_drag->setImages (false, true, true,
                         ImageCache::getFromMemory (drag_off_png, drag_off_pngSize), 1.000f, Colour (0x00000000),
                         ImageCache::getFromMemory (drag_over_png, drag_over_pngSize), 1.000f, Colour (0x00000000),
                         ImageCache::getFromMemory (drag_on_png, drag_on_pngSize), 1.000f, Colour (0x00000000));
    addAndMakeVisible (sld_az_1 = new Slider ("new slider"));
    sld_az_1->setTooltip (TRANS("Azimuth Filter 1"));
    sld_az_1->setRange (-180, 180, 0.1);
    sld_az_1->setSliderStyle (Slider::Rotary);
    sld_az_1->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_az_1->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_az_1->setColour (Slider::trackColourId, Colours::aqua);
    sld_az_1->setColour (Slider::rotarySliderFillColourId, Colours::aqua);
    sld_az_1->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_az_1->addListener (this);

    addAndMakeVisible (sld_el_1 = new Slider ("new slider"));
    sld_el_1->setTooltip (TRANS("Elevation Filter 1"));
    sld_el_1->setRange (-180, 180, 0.1);
    sld_el_1->setSliderStyle (Slider::Rotary);
    sld_el_1->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_el_1->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_el_1->setColour (Slider::trackColourId, Colours::aqua);
    sld_el_1->setColour (Slider::rotarySliderFillColourId, Colours::aqua);
    sld_el_1->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_el_1->addListener (this);

    addAndMakeVisible (box_shape_1 = new ComboBox ("new combo box"));
    box_shape_1->setTooltip (TRANS("Shape of Filter 1"));
    box_shape_1->setEditableText (false);
    box_shape_1->setJustificationType (Justification::centredLeft);
    box_shape_1->setTextWhenNothingSelected (TRANS("circular"));
    box_shape_1->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    box_shape_1->addItem (TRANS("circular"), 1);
    box_shape_1->addItem (TRANS("rectangular"), 2);
    box_shape_1->addListener (this);

    addAndMakeVisible (sld_w_1 = new Slider ("new slider"));
    sld_w_1->setTooltip (TRANS("Width Filter 1"));
    sld_w_1->setRange (0, 180, 0.1);
    sld_w_1->setSliderStyle (Slider::Rotary);
    sld_w_1->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_w_1->setColour (Slider::thumbColourId, Colour (0xff000002));
    sld_w_1->setColour (Slider::trackColourId, Colour (0xff010202));
    sld_w_1->setColour (Slider::rotarySliderFillColourId, Colours::yellow);
    sld_w_1->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_w_1->addListener (this);
    sld_w_1->setSkewFactor (0.8);

    addAndMakeVisible (sld_h_1 = new Slider ("new slider"));
    sld_h_1->setTooltip (TRANS("Height Filter 1"));
    sld_h_1->setRange (0, 180, 0.1);
    sld_h_1->setSliderStyle (Slider::Rotary);
    sld_h_1->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_h_1->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_h_1->setColour (Slider::trackColourId, Colours::aqua);
    sld_h_1->setColour (Slider::rotarySliderFillColourId, Colours::yellow);
    sld_h_1->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_h_1->addListener (this);
    sld_h_1->setSkewFactor (0.8);

    addAndMakeVisible (btn_solo_1 = new ImageButton ("new button"));
    btn_solo_1->setTooltip (TRANS("Solo Filter Output 1"));
    btn_solo_1->addListener (this);

    btn_solo_1->setImages (false, true, true,
                           ImageCache::getFromMemory (solo_symbol_png, solo_symbol_pngSize), 1.000f, Colour (0x00000000),
                           ImageCache::getFromMemory (solo_symbol_over_png, solo_symbol_over_pngSize), 1.000f, Colour (0x00000000),
                           ImageCache::getFromMemory (solo_symbol_act_png, solo_symbol_act_pngSize), 1.000f, Colour (0x00000000));
    addAndMakeVisible (sld_gain_1 = new Slider ("new slider"));
    sld_gain_1->setTooltip (TRANS("Gain Filter 1"));
    sld_gain_1->setRange (-18, 18, 0.1);
    sld_gain_1->setSliderStyle (Slider::Rotary);
    sld_gain_1->setTextBoxStyle (Slider::TextBoxRight, false, 45, 18);
    sld_gain_1->setColour (Slider::thumbColourId, Colour (0xff000002));
    sld_gain_1->setColour (Slider::trackColourId, Colour (0xff010202));
    sld_gain_1->setColour (Slider::rotarySliderFillColourId, Colours::cornsilk);
    sld_gain_1->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_gain_1->addListener (this);

    addAndMakeVisible (sld_az_2 = new Slider ("new slider"));
    sld_az_2->setTooltip (TRANS("Azimuth Filter 2"));
    sld_az_2->setRange (-180, 180, 0.1);
    sld_az_2->setSliderStyle (Slider::Rotary);
    sld_az_2->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_az_2->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_az_2->setColour (Slider::trackColourId, Colours::aqua);
    sld_az_2->setColour (Slider::rotarySliderFillColourId, Colours::aqua);
    sld_az_2->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_az_2->addListener (this);

    addAndMakeVisible (sld_el_2 = new Slider ("new slider"));
    sld_el_2->setTooltip (TRANS("Elevation Filter 2"));
    sld_el_2->setRange (-180, 180, 0.1);
    sld_el_2->setSliderStyle (Slider::Rotary);
    sld_el_2->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_el_2->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_el_2->setColour (Slider::trackColourId, Colours::aqua);
    sld_el_2->setColour (Slider::rotarySliderFillColourId, Colours::aqua);
    sld_el_2->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_el_2->addListener (this);

    addAndMakeVisible (box_shape_2 = new ComboBox ("new combo box"));
    box_shape_2->setTooltip (TRANS("Shape of Filter 2"));
    box_shape_2->setEditableText (false);
    box_shape_2->setJustificationType (Justification::centredLeft);
    box_shape_2->setTextWhenNothingSelected (TRANS("circular"));
    box_shape_2->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    box_shape_2->addItem (TRANS("circular"), 1);
    box_shape_2->addItem (TRANS("rectangular"), 2);
    box_shape_2->addListener (this);

    addAndMakeVisible (sld_w_2 = new Slider ("new slider"));
    sld_w_2->setTooltip (TRANS("Width Filter 2"));
    sld_w_2->setRange (0, 180, 0.1);
    sld_w_2->setSliderStyle (Slider::Rotary);
    sld_w_2->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_w_2->setColour (Slider::thumbColourId, Colour (0xff000002));
    sld_w_2->setColour (Slider::trackColourId, Colour (0xff010202));
    sld_w_2->setColour (Slider::rotarySliderFillColourId, Colours::yellow);
    sld_w_2->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_w_2->addListener (this);
    sld_w_2->setSkewFactor (0.8);

    addAndMakeVisible (sld_h_2 = new Slider ("new slider"));
    sld_h_2->setTooltip (TRANS("Height Filter 2"));
    sld_h_2->setRange (0, 180, 0.1);
    sld_h_2->setSliderStyle (Slider::Rotary);
    sld_h_2->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_h_2->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_h_2->setColour (Slider::trackColourId, Colours::aqua);
    sld_h_2->setColour (Slider::rotarySliderFillColourId, Colours::yellow);
    sld_h_2->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_h_2->addListener (this);
    sld_h_2->setSkewFactor (0.8);

    addAndMakeVisible (btn_solo_2 = new ImageButton ("new button"));
    btn_solo_2->setTooltip (TRANS("Solo Filter Output 2"));
    btn_solo_2->addListener (this);

    btn_solo_2->setImages (false, true, true,
                           ImageCache::getFromMemory (solo_symbol_png, solo_symbol_pngSize), 1.000f, Colour (0x00000000),
                           ImageCache::getFromMemory (solo_symbol_over_png, solo_symbol_over_pngSize), 1.000f, Colour (0x00000000),
                           ImageCache::getFromMemory (solo_symbol_act_png, solo_symbol_act_pngSize), 1.000f, Colour (0x00000000));
    addAndMakeVisible (sld_gain_2 = new Slider ("new slider"));
    sld_gain_2->setTooltip (TRANS("Gain Filter 2"));
    sld_gain_2->setRange (-18, 18, 0.1);
    sld_gain_2->setSliderStyle (Slider::Rotary);
    sld_gain_2->setTextBoxStyle (Slider::TextBoxRight, false, 45, 18);
    sld_gain_2->setColour (Slider::thumbColourId, Colour (0xff000002));
    sld_gain_2->setColour (Slider::trackColourId, Colour (0xff010202));
    sld_gain_2->setColour (Slider::rotarySliderFillColourId, Colours::cornsilk);
    sld_gain_2->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_gain_2->addListener (this);

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (30);
    tabbedComponent->addTab (TRANS("Tab 0"), Colour (0xff7d0000), 0, false);
    tabbedComponent->setCurrentTabIndex (0);

    addAndMakeVisible (tabbedComponent2 = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent2->setTabBarDepth (30);
    tabbedComponent2->addTab (TRANS("Tab 0"), Colour (0xff7d0000), 0, false);
    tabbedComponent2->setCurrentTabIndex (0);

    addAndMakeVisible (lbl_drag = new Label ("new label",
                                             TRANS("1")));
    lbl_drag->setFont (Font (15.00f, Font::plain));
    lbl_drag->setJustificationType (Justification::centred);
    lbl_drag->setEditable (false, false, false);
    lbl_drag->setColour (Label::textColourId, Colours::white);
    lbl_drag->setColour (TextEditor::textColourId, Colours::black);
    lbl_drag->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btn_solo_reset = new ImageButton ("new button"));
    btn_solo_reset->setTooltip (TRANS("Reset all solos"));
    btn_solo_reset->addListener (this);

    btn_solo_reset->setImages (false, true, true,
                               ImageCache::getFromMemory (solo_symbol_png, solo_symbol_pngSize), 1.000f, Colour (0x00000000),
                               ImageCache::getFromMemory (solo_symbol_over_png, solo_symbol_over_pngSize), 1.000f, Colour (0x00000000),
                               ImageCache::getFromMemory (solo_symbol_act_png, solo_symbol_act_pngSize), 1.000f, Colour (0x00000000));

    //[UserPreSize]
    //[/UserPreSize]

    setSize (630, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Ambix_directional_loudnessAudioProcessorEditor::~Ambix_directional_loudnessAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    lbl_gd = nullptr;
    filtergraph = nullptr;
    btn_drag = nullptr;
    sld_az_1 = nullptr;
    sld_el_1 = nullptr;
    box_shape_1 = nullptr;
    sld_w_1 = nullptr;
    sld_h_1 = nullptr;
    btn_solo_1 = nullptr;
    sld_gain_1 = nullptr;
    sld_az_2 = nullptr;
    sld_el_2 = nullptr;
    box_shape_2 = nullptr;
    sld_w_2 = nullptr;
    sld_h_2 = nullptr;
    btn_solo_2 = nullptr;
    sld_gain_2 = nullptr;
    tabbedComponent = nullptr;
    tabbedComponent2 = nullptr;
    lbl_drag = nullptr;
    btn_solo_reset = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Ambix_directional_loudnessAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff1a1a1a));

    g.setColour (Colour (0xff2f2866));
    g.fillRoundedRectangle (47.0f, 275.0f, 234.0f, 110.0f, 10.000f);

    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("filter 1"),
                55, 274, 47, 23,
                Justification::centredLeft, true);

    g.setColour (Colours::aqua);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("elevation"),
                71, 337, 82, 23,
                Justification::centred, true);

    g.setColour (Colours::aqua);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("azimuth"),
                74, 295, 82, 23,
                Justification::centred, true);

    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("width"),
                156, 332, 36, 23,
                Justification::centredRight, true);

    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("height"),
                149, 357, 44, 23,
                Justification::centredRight, true);

    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("gain"),
                110, 275, 30, 23,
                Justification::centredLeft, true);

    g.setColour (Colour (0xff2f2866));
    g.fillRoundedRectangle (352.0f, 276.0f, 234.0f, 110.0f, 10.000f);

    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("filter 2"),
                360, 275, 47, 23,
                Justification::centredLeft, true);

    g.setColour (Colours::aqua);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("elevation"),
                376, 338, 82, 23,
                Justification::centred, true);

    g.setColour (Colours::aqua);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("azimuth"),
                379, 296, 82, 23,
                Justification::centred, true);

    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("width"),
                461, 333, 36, 23,
                Justification::centredRight, true);

    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("height"),
                454, 358, 44, 23,
                Justification::centredRight, true);

    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("[dB]"),
                216, 276, 30, 23,
                Justification::centredLeft, true);

    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("gain"),
                418, 275, 30, 23,
                Justification::centredLeft, true);

    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("[dB]"),
                524, 276, 30, 23,
                Justification::centredLeft, true);

    g.setColour (Colour (0xff2f2866));
    g.fillRoundedRectangle (0.0f, 0.0f, 234.0f, 110.0f, 10.000f);

    
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

void Ambix_directional_loudnessAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    lbl_gd->setBounds (0, 0, 204, 16);
    filtergraph->setBounds (26, 22, 580, 237);
    btn_drag->setBounds (310, 148, 9, 10);
    sld_az_1->setBounds (56, 314, 84, 22);
    sld_el_1->setBounds (56, 355, 84, 22);
    box_shape_1->setBounds (161, 307, 113, 22);
    sld_w_1->setBounds (190, 333, 84, 22);
    sld_h_1->setBounds (190, 358, 84, 22);
    btn_solo_1->setBounds (252, 278, 20, 20);
    sld_gain_1->setBounds (136, 277, 76, 22);
    sld_az_2->setBounds (360, 316, 84, 22);
    sld_el_2->setBounds (360, 357, 84, 22);
    box_shape_2->setBounds (465, 309, 113, 22);
    sld_w_2->setBounds (494, 335, 84, 22);
    sld_h_2->setBounds (494, 360, 84, 22);
    btn_solo_2->setBounds (557, 279, 20, 20);
    sld_gain_2->setBounds (441, 278, 76, 22);
    tabbedComponent->setBounds (20, 253, 282, 140);
    tabbedComponent2->setBounds (322, 253, 282, 140);
    lbl_drag->setBounds (230, 182, 26, 23);
    btn_solo_reset->setBounds (597, 254, 20, 20);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Ambix_directional_loudnessAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btn_drag)
    {
        //[UserButtonCode_btn_drag] -- add your button handler code here..
        //[/UserButtonCode_btn_drag]
    }
    else if (buttonThatWasClicked == btn_solo_1)
    {
        //[UserButtonCode_btn_solo_1] -- add your button handler code here..
        //[/UserButtonCode_btn_solo_1]
    }
    else if (buttonThatWasClicked == btn_solo_2)
    {
        //[UserButtonCode_btn_solo_2] -- add your button handler code here..
        //[/UserButtonCode_btn_solo_2]
    }
    else if (buttonThatWasClicked == btn_solo_reset)
    {
        //[UserButtonCode_btn_solo_reset] -- add your button handler code here..
        //[/UserButtonCode_btn_solo_reset]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void Ambix_directional_loudnessAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sld_az_1)
    {
        //[UserSliderCode_sld_az_1] -- add your slider handling code here..
        //[/UserSliderCode_sld_az_1]
    }
    else if (sliderThatWasMoved == sld_el_1)
    {
        //[UserSliderCode_sld_el_1] -- add your slider handling code here..
        //[/UserSliderCode_sld_el_1]
    }
    else if (sliderThatWasMoved == sld_w_1)
    {
        //[UserSliderCode_sld_w_1] -- add your slider handling code here..
        //[/UserSliderCode_sld_w_1]
    }
    else if (sliderThatWasMoved == sld_h_1)
    {
        //[UserSliderCode_sld_h_1] -- add your slider handling code here..
        //[/UserSliderCode_sld_h_1]
    }
    else if (sliderThatWasMoved == sld_gain_1)
    {
        //[UserSliderCode_sld_gain_1] -- add your slider handling code here..
        //[/UserSliderCode_sld_gain_1]
    }
    else if (sliderThatWasMoved == sld_az_2)
    {
        //[UserSliderCode_sld_az_2] -- add your slider handling code here..
        //[/UserSliderCode_sld_az_2]
    }
    else if (sliderThatWasMoved == sld_el_2)
    {
        //[UserSliderCode_sld_el_2] -- add your slider handling code here..
        //[/UserSliderCode_sld_el_2]
    }
    else if (sliderThatWasMoved == sld_w_2)
    {
        //[UserSliderCode_sld_w_2] -- add your slider handling code here..
        //[/UserSliderCode_sld_w_2]
    }
    else if (sliderThatWasMoved == sld_h_2)
    {
        //[UserSliderCode_sld_h_2] -- add your slider handling code here..
        //[/UserSliderCode_sld_h_2]
    }
    else if (sliderThatWasMoved == sld_gain_2)
    {
        //[UserSliderCode_sld_gain_2] -- add your slider handling code here..
        //[/UserSliderCode_sld_gain_2]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void Ambix_directional_loudnessAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == box_shape_1)
    {
        //[UserComboBoxCode_box_shape_1] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_shape_1]
    }
    else if (comboBoxThatHasChanged == box_shape_2)
    {
        //[UserComboBoxCode_box_shape_2] -- add your combo box handling code here..
        //[/UserComboBoxCode_box_shape_2]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Ambix_directional_loudnessAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="Ambix_directional_loudnessAudioProcessor* ownerFilter"
                 variableInitialisers="AudioProcessorEditor (ownerFilter)" snapPixels="8"
                 snapActive="0" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="630" initialHeight="400">
  <BACKGROUND backgroundColour="ff1a1a1a">
    <ROUNDRECT pos="47 275 234 110" cornerSize="10" fill="solid: ff2f2866" hasStroke="0"/>
    <TEXT pos="55 274 47 23" fill="solid: ffffffff" hasStroke="0" text="filter 1"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
    <TEXT pos="71 337 82 23" fill="solid: ff00ffff" hasStroke="0" text="elevation"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
    <TEXT pos="74 295 82 23" fill="solid: ff00ffff" hasStroke="0" text="azimuth"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
    <TEXT pos="156 332 36 23" fill="solid: ffffff00" hasStroke="0" text="width"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
    <TEXT pos="149 357 44 23" fill="solid: ffffff00" hasStroke="0" text="height"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
    <TEXT pos="110 275 30 23" fill="solid: ffffffff" hasStroke="0" text="gain"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
    <ROUNDRECT pos="352 276 234 110" cornerSize="10" fill="solid: ff2f2866"
               hasStroke="0"/>
    <TEXT pos="360 275 47 23" fill="solid: ffffffff" hasStroke="0" text="filter 2"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
    <TEXT pos="376 338 82 23" fill="solid: ff00ffff" hasStroke="0" text="elevation"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
    <TEXT pos="379 296 82 23" fill="solid: ff00ffff" hasStroke="0" text="azimuth"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="36"/>
    <TEXT pos="461 333 36 23" fill="solid: ffffff00" hasStroke="0" text="width"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
    <TEXT pos="454 358 44 23" fill="solid: ffffff00" hasStroke="0" text="height"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="34"/>
    <TEXT pos="216 276 30 23" fill="solid: ffffffff" hasStroke="0" text="[dB]"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
    <TEXT pos="418 275 30 23" fill="solid: ffffffff" hasStroke="0" text="gain"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
    <TEXT pos="524 276 30 23" fill="solid: ffffffff" hasStroke="0" text="[dB]"
          fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
    <ROUNDRECT pos="0 0 234 110" cornerSize="10" fill="solid: ff2f2866" hasStroke="0"/>
  </BACKGROUND>
  <LABEL name="new label" id="b45e45d811b90270" memberName="lbl_gd" virtualName=""
         explicitFocusOrder="0" pos="0 0 204 16" textCol="ff7fffd4" edTextCol="ff000000"
         edBkgCol="0" labelText="ambix_directional_loudness" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="new component" id="885d5198e7f04ca3" memberName="filtergraph"
                    virtualName="" explicitFocusOrder="0" pos="26 22 580 237" class="Component"
                    params=""/>
  <IMAGEBUTTON name="new button" id="40d434b127505653" memberName="btn_drag"
               virtualName="" explicitFocusOrder="0" pos="310 148 9 10" buttonText="new button"
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="drag_off_png" opacityNormal="1" colourNormal="0"
               resourceOver="drag_over_png" opacityOver="1" colourOver="0" resourceDown="drag_on_png"
               opacityDown="1" colourDown="0"/>
  <SLIDER name="new slider" id="18369e36ec28b233" memberName="sld_az_1"
          virtualName="" explicitFocusOrder="0" pos="56 314 84 22" tooltip="Azimuth Filter 1"
          thumbcol="ff5a5a90" trackcol="ff00ffff" rotarysliderfill="ff00ffff"
          rotaryslideroutline="ff00ffff" min="-180" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="1"/>
  <SLIDER name="new slider" id="617e5d5a85d20cd4" memberName="sld_el_1"
          virtualName="" explicitFocusOrder="0" pos="56 355 84 22" tooltip="Elevation Filter 1"
          thumbcol="ff5a5a90" trackcol="ff00ffff" rotarysliderfill="ff00ffff"
          rotaryslideroutline="ff00ffff" min="-180" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="1"/>
  <COMBOBOX name="new combo box" id="cd79431255d05e19" memberName="box_shape_1"
            virtualName="" explicitFocusOrder="0" pos="161 307 113 22" tooltip="Shape of Filter 1"
            editable="0" layout="33" items="circular&#10;rectangular" textWhenNonSelected="circular"
            textWhenNoItems="(no choices)"/>
  <SLIDER name="new slider" id="52b78e43d4486cdb" memberName="sld_w_1"
          virtualName="" explicitFocusOrder="0" pos="190 333 84 22" tooltip="Width Filter 1"
          thumbcol="ff000002" trackcol="ff010202" rotarysliderfill="ffffff00"
          rotaryslideroutline="ff00ffff" min="0" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="0.80000000000000004441"/>
  <SLIDER name="new slider" id="ea0e09fa6e38b5b5" memberName="sld_h_1"
          virtualName="" explicitFocusOrder="0" pos="190 358 84 22" tooltip="Height Filter 1"
          thumbcol="ff5a5a90" trackcol="ff00ffff" rotarysliderfill="ffffff00"
          rotaryslideroutline="ff00ffff" min="0" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="0.80000000000000004441"/>
  <IMAGEBUTTON name="new button" id="474d5d3efba18a44" memberName="btn_solo_1"
               virtualName="" explicitFocusOrder="0" pos="252 278 20 20" tooltip="Solo Filter Output 1"
               buttonText="new button" connectedEdges="0" needsCallback="1"
               radioGroupId="0" keepProportions="1" resourceNormal="solo_symbol_png"
               opacityNormal="1" colourNormal="0" resourceOver="solo_symbol_over_png"
               opacityOver="1" colourOver="0" resourceDown="solo_symbol_act_png"
               opacityDown="1" colourDown="0"/>
  <SLIDER name="new slider" id="30bf9cc16f094d43" memberName="sld_gain_1"
          virtualName="" explicitFocusOrder="0" pos="136 277 76 22" tooltip="Gain Filter 1"
          thumbcol="ff000002" trackcol="ff010202" rotarysliderfill="fffff8dc"
          rotaryslideroutline="ff00ffff" min="-18" max="18" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="45" textBoxHeight="18" skewFactor="1"/>
  <SLIDER name="new slider" id="eb7cdc45626135ad" memberName="sld_az_2"
          virtualName="" explicitFocusOrder="0" pos="360 316 84 22" tooltip="Azimuth Filter 2"
          thumbcol="ff5a5a90" trackcol="ff00ffff" rotarysliderfill="ff00ffff"
          rotaryslideroutline="ff00ffff" min="-180" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="1"/>
  <SLIDER name="new slider" id="bcb40193e123044b" memberName="sld_el_2"
          virtualName="" explicitFocusOrder="0" pos="360 357 84 22" tooltip="Elevation Filter 2"
          thumbcol="ff5a5a90" trackcol="ff00ffff" rotarysliderfill="ff00ffff"
          rotaryslideroutline="ff00ffff" min="-180" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="1"/>
  <COMBOBOX name="new combo box" id="5a3cb120bb4c6939" memberName="box_shape_2"
            virtualName="" explicitFocusOrder="0" pos="465 309 113 22" tooltip="Shape of Filter 2"
            editable="0" layout="33" items="circular&#10;rectangular" textWhenNonSelected="circular"
            textWhenNoItems="(no choices)"/>
  <SLIDER name="new slider" id="116367b233b878a0" memberName="sld_w_2"
          virtualName="" explicitFocusOrder="0" pos="494 335 84 22" tooltip="Width Filter 2"
          thumbcol="ff000002" trackcol="ff010202" rotarysliderfill="ffffff00"
          rotaryslideroutline="ff00ffff" min="0" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="0.80000000000000004441"/>
  <SLIDER name="new slider" id="3ec883a30828d4b3" memberName="sld_h_2"
          virtualName="" explicitFocusOrder="0" pos="494 360 84 22" tooltip="Height Filter 2"
          thumbcol="ff5a5a90" trackcol="ff00ffff" rotarysliderfill="ffffff00"
          rotaryslideroutline="ff00ffff" min="0" max="180" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="53" textBoxHeight="18" skewFactor="0.80000000000000004441"/>
  <IMAGEBUTTON name="new button" id="14f3aeeefa95cc16" memberName="btn_solo_2"
               virtualName="" explicitFocusOrder="0" pos="557 279 20 20" tooltip="Solo Filter Output 2"
               buttonText="new button" connectedEdges="0" needsCallback="1"
               radioGroupId="0" keepProportions="1" resourceNormal="solo_symbol_png"
               opacityNormal="1" colourNormal="0" resourceOver="solo_symbol_over_png"
               opacityOver="1" colourOver="0" resourceDown="solo_symbol_act_png"
               opacityDown="1" colourDown="0"/>
  <SLIDER name="new slider" id="40377f21e0eda329" memberName="sld_gain_2"
          virtualName="" explicitFocusOrder="0" pos="441 278 76 22" tooltip="Gain Filter 2"
          thumbcol="ff000002" trackcol="ff010202" rotarysliderfill="fffff8dc"
          rotaryslideroutline="ff00ffff" min="-18" max="18" int="0.10000000000000000555"
          style="Rotary" textBoxPos="TextBoxRight" textBoxEditable="1"
          textBoxWidth="45" textBoxHeight="18" skewFactor="1"/>
  <TABBEDCOMPONENT name="new tabbed component" id="2710a64906f39372" memberName="tabbedComponent"
                   virtualName="" explicitFocusOrder="0" pos="20 253 282 140" orientation="top"
                   tabBarDepth="30" initialTab="0">
    <TAB name="Tab 0" colour="ff7d0000" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
  </TABBEDCOMPONENT>
  <TABBEDCOMPONENT name="new tabbed component" id="2c3bf016a431a90f" memberName="tabbedComponent2"
                   virtualName="" explicitFocusOrder="0" pos="322 253 282 140" orientation="top"
                   tabBarDepth="30" initialTab="0">
    <TAB name="Tab 0" colour="ff7d0000" useJucerComp="0" contentClassName=""
         constructorParams="" jucerComponentFile=""/>
  </TABBEDCOMPONENT>
  <LABEL name="new label" id="1838d88f72a345e5" memberName="lbl_drag"
         virtualName="" explicitFocusOrder="0" pos="230 182 26 23" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="1" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <IMAGEBUTTON name="new button" id="a96b833c1358f969" memberName="btn_solo_reset"
               virtualName="" explicitFocusOrder="0" pos="597 254 20 20" tooltip="Reset all solos"
               buttonText="new button" connectedEdges="0" needsCallback="1"
               radioGroupId="0" keepProportions="1" resourceNormal="solo_symbol_png"
               opacityNormal="1" colourNormal="0" resourceOver="solo_symbol_over_png"
               opacityOver="1" colourOver="0" resourceDown="solo_symbol_act_png"
               opacityDown="1" colourDown="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: solo_symbol_png, 3173, "solo_symbol.png"
static const unsigned char resource_Ambix_directional_loudnessAudioProcessorEditor_solo_symbol_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,49,0,0,0,49,8,6,0,0,1,4,155,113,138,0,0,0,9,
112,72,89,115,0,0,14,197,0,0,14,197,1,71,108,236,255,0,0,12,23,73,68,65,84,120,218,205,89,123,80,83,103,22,63,132,11,18,94,193,32,4,17,65,80,131,136,162,40,176,65,170,168,104,113,128,21,80,68,11,62,43,
214,161,148,118,59,253,195,157,110,103,156,177,213,218,233,195,22,180,86,132,42,165,172,11,226,98,21,108,209,10,190,80,86,16,71,20,171,236,2,21,40,15,33,148,135,128,70,52,123,206,133,27,19,184,137,36,
98,119,207,204,133,36,223,227,60,190,243,248,157,239,50,74,165,18,56,138,136,136,40,59,126,252,184,15,247,157,161,63,177,177,177,74,91,91,91,112,118,118,134,183,223,126,155,157,157,148,148,100,196,14,
210,0,71,201,201,201,144,152,152,8,237,237,237,98,134,182,162,21,220,128,173,155,47,251,249,181,215,94,43,100,136,199,192,86,221,176,53,62,1,14,236,223,135,67,50,8,11,11,75,99,84,251,129,21,152,226,55,
218,146,8,255,39,51,28,115,226,65,91,209,10,26,96,165,229,84,41,41,41,137,184,115,231,142,235,198,141,27,247,168,212,224,68,39,42,47,47,255,130,85,129,251,113,64,244,4,252,36,96,39,170,132,72,140,127,
29,7,247,169,132,80,13,36,239,255,22,98,163,194,84,178,50,180,31,45,229,102,18,173,90,181,106,129,74,100,107,107,235,110,59,59,187,182,234,234,106,87,13,113,89,85,173,172,148,155,54,109,130,161,84,89,
89,9,103,207,158,53,210,56,17,110,178,250,130,150,166,38,48,179,180,6,145,149,5,120,122,122,178,90,60,124,248,16,82,82,82,140,24,50,200,208,221,185,211,26,74,102,102,102,64,162,50,100,193,161,131,234,
11,134,110,64,186,49,100,242,140,140,140,47,72,4,62,26,202,145,140,193,234,196,41,249,198,27,111,40,73,4,62,34,115,206,159,63,255,226,48,235,113,20,31,31,127,170,175,175,111,204,225,195,135,131,134,142,
169,78,238,234,213,171,43,81,196,28,35,35,35,48,49,49,97,31,206,15,208,220,151,81,146,0,213,2,212,231,58,90,99,54,77,230,35,212,115,30,30,71,111,119,119,183,57,187,128,38,171,79,120,218,175,128,86,249,
239,32,145,72,84,191,225,81,8,207,157,59,183,144,17,10,133,202,45,91,182,104,152,52,54,46,129,157,156,131,159,189,95,79,132,201,22,3,99,153,153,153,69,140,250,100,142,172,133,2,246,127,212,16,211,226,
230,192,240,217,255,124,126,14,84,212,52,241,158,7,195,167,100,96,104,20,4,170,68,252,1,23,133,63,91,144,159,159,15,161,161,161,26,58,120,202,150,192,98,95,119,54,114,252,194,55,106,114,192,35,55,82,15,
70,117,17,134,138,83,92,92,92,206,138,148,158,158,222,187,97,195,6,115,208,65,150,150,150,103,174,93,187,246,42,187,160,179,179,211,194,193,193,225,126,116,116,180,29,223,228,154,154,154,179,121,121,121,
175,106,40,221,220,220,108,79,255,49,171,228,97,134,113,29,51,102,140,226,238,221,187,146,198,198,70,71,13,29,248,156,143,163,29,59,118,100,125,252,241,199,209,50,153,12,102,206,156,57,108,188,163,163,
131,244,34,113,107,111,220,184,225,166,109,159,97,71,65,190,216,210,210,50,123,234,212,169,236,119,62,103,224,200,198,198,134,59,21,87,206,216,100,67,50,139,214,168,216,190,125,123,14,109,142,62,12,134,
18,29,18,49,204,206,206,110,229,204,201,112,102,105,109,109,141,230,164,31,70,138,22,72,62,144,205,59,100,59,125,9,196,4,121,12,251,157,14,119,220,184,113,29,109,109,109,54,44,147,162,162,162,104,62,155,
15,114,96,25,24,79,13,134,55,151,73,135,140,117,195,217,226,187,208,210,169,0,137,200,116,216,202,152,152,24,17,57,5,67,25,70,59,3,34,83,136,244,119,134,220,43,5,144,252,239,2,246,23,233,244,57,48,219,
219,11,36,98,43,8,10,240,209,105,62,242,58,134,82,24,101,37,93,228,228,19,14,137,67,246,234,110,111,192,144,59,60,96,50,239,112,136,121,197,153,119,45,185,53,67,57,114,253,250,245,74,242,20,62,186,117,
124,47,20,213,43,33,120,125,2,72,69,130,103,149,74,236,196,134,41,197,185,252,250,117,0,45,76,40,110,216,51,65,31,175,13,12,12,116,229,155,52,35,226,45,152,209,223,137,213,116,31,20,240,140,47,142,92,
15,158,78,34,94,6,88,221,126,196,192,12,225,152,184,5,5,5,21,83,62,230,143,38,17,111,181,211,69,131,41,34,68,35,78,184,234,128,5,65,169,239,134,234,132,41,254,148,183,183,247,190,144,144,144,83,90,35,
30,211,12,91,118,168,88,68,69,69,29,143,140,140,20,81,90,215,69,100,22,204,130,18,124,230,242,26,66,87,238,226,232,241,227,199,230,123,247,238,221,93,86,86,54,7,189,209,100,210,164,73,53,232,250,59,48,
120,127,121,222,90,70,219,0,110,36,242,240,240,104,100,24,198,60,32,32,0,68,34,145,10,63,246,247,247,251,161,71,173,169,168,168,160,84,244,244,200,145,35,171,194,195,195,255,57,98,6,238,238,238,221,46,
46,46,150,203,151,47,215,41,157,151,151,23,61,130,51,103,206,28,91,187,118,109,255,189,123,247,36,98,177,184,93,43,131,39,79,158,152,96,66,84,108,222,188,89,175,3,22,8,4,84,232,153,213,171,87,203,215,
172,89,179,5,215,167,242,50,64,115,40,94,196,147,208,164,128,218,28,68,134,79,144,225,33,13,6,4,21,95,100,115,142,236,237,237,1,207,36,5,61,48,23,51,70,7,203,96,231,206,157,71,176,59,120,238,226,238,206,
118,232,125,248,24,63,25,131,173,221,56,96,4,252,243,166,77,155,198,160,167,213,99,133,180,98,25,124,246,217,103,107,214,173,91,167,117,227,75,57,123,225,122,19,191,59,111,69,173,77,121,126,199,67,183,
196,32,14,98,208,205,86,224,225,104,221,188,163,170,144,221,92,224,22,0,9,161,115,158,253,222,80,9,25,185,133,112,32,57,19,83,74,44,239,193,163,235,158,96,80,242,111,131,131,131,181,231,151,95,110,15,
160,86,181,205,217,218,237,228,9,50,233,175,96,238,58,77,235,218,197,139,23,203,25,204,126,34,93,118,119,156,56,17,160,174,14,78,94,169,133,112,127,205,4,236,27,28,170,243,204,240,28,38,50,142,142,142,
58,39,57,204,65,32,88,156,12,117,101,121,144,92,70,191,140,135,192,224,185,48,93,234,170,61,13,168,187,254,243,146,25,135,9,155,170,43,33,231,84,33,126,107,130,243,5,121,248,12,2,128,184,68,144,232,216,
130,193,68,54,34,255,30,63,217,19,25,121,114,152,29,42,174,156,133,243,229,85,144,157,154,172,179,110,48,114,185,92,255,104,18,48,224,21,16,12,19,108,140,224,239,133,119,161,73,129,2,152,106,97,208,211,
211,163,96,97,5,47,245,99,29,222,207,11,157,137,30,245,245,233,148,3,129,64,37,147,144,144,144,214,219,219,27,175,45,155,251,140,53,130,178,223,149,200,40,19,162,98,151,195,120,132,49,253,138,62,184,126,
241,52,148,220,174,27,48,159,22,241,16,231,246,49,187,119,239,126,19,1,64,252,172,89,179,120,39,249,175,125,11,108,74,10,224,231,210,42,200,201,60,172,49,102,36,118,131,183,98,67,117,97,254,223,89,79,
171,171,171,107,70,6,14,90,179,164,44,152,125,244,33,172,148,39,78,159,62,29,206,50,168,173,173,29,63,99,198,140,39,24,121,2,24,5,194,180,223,219,208,208,240,72,35,93,127,254,249,231,33,135,14,29,250,
137,210,237,139,18,2,188,71,8,39,163,53,24,96,62,42,192,30,96,77,86,86,214,63,38,76,152,96,240,230,216,75,116,225,62,98,222,138,134,176,60,11,107,170,124,219,182,109,217,88,232,199,234,179,49,130,130,
42,220,92,136,155,59,235,44,250,75,150,44,249,25,49,142,24,139,249,77,212,68,130,208,196,78,215,198,88,195,171,15,30,60,104,149,155,155,187,233,195,15,63,188,60,98,216,130,144,132,197,251,95,126,249,229,
95,208,149,255,106,110,110,254,216,217,217,185,195,204,204,76,241,224,193,3,35,132,158,83,23,45,90,84,148,145,145,177,238,163,143,62,234,212,122,224,35,1,94,186,136,10,86,82,82,210,182,210,210,82,119,
244,28,17,21,26,11,11,11,22,71,161,55,177,23,25,196,3,177,20,1,56,232,234,234,2,20,144,160,170,98,202,148,41,77,120,246,167,222,123,239,189,237,118,118,118,173,6,123,173,62,147,81,16,211,79,62,249,36,
253,171,175,190,90,133,2,25,207,158,61,155,112,26,160,111,128,174,194,168,181,179,2,112,161,76,245,193,7,31,196,223,185,115,135,64,255,211,177,99,199,222,79,73,73,217,184,116,233,210,130,81,85,2,251,213,
163,159,126,250,105,148,171,171,43,204,155,55,15,16,195,193,104,18,230,94,160,140,136,15,229,19,135,156,156,156,159,222,125,247,221,167,216,52,119,103,102,102,174,166,72,53,72,9,2,172,8,54,111,28,61,122,
212,35,34,34,2,248,174,21,95,22,145,82,131,9,82,132,167,242,211,178,101,203,0,229,88,141,205,75,246,136,149,32,203,163,219,68,209,253,133,190,168,123,180,137,114,36,85,214,99,199,142,101,197,197,197,165,
225,41,69,82,230,209,170,4,249,60,54,161,114,180,132,229,255,90,248,161,132,129,15,216,242,91,82,94,70,197,46,157,56,113,98,249,48,37,168,75,195,70,74,142,193,106,140,128,220,64,86,79,17,38,255,7,110,
85,254,2,215,171,234,116,206,156,60,221,23,220,103,120,194,100,137,149,94,28,168,176,84,85,85,45,164,252,207,165,104,70,173,39,106,196,83,48,80,129,30,40,61,122,8,74,154,53,211,245,120,169,23,184,77,176,
7,145,208,20,20,136,127,250,122,58,161,254,110,57,212,97,239,88,125,187,148,125,88,212,237,19,14,43,253,157,71,204,77,42,149,90,97,138,150,96,220,20,34,45,102,6,187,148,127,81,159,172,251,174,73,59,53,
150,253,172,166,128,23,196,38,6,130,88,203,220,57,62,236,141,6,116,85,95,128,244,83,55,6,215,255,0,199,152,40,88,233,59,126,196,60,169,2,95,188,120,209,141,10,37,1,111,115,44,181,126,24,249,6,186,144,
2,218,91,234,159,1,65,137,57,8,71,176,202,122,242,2,12,216,5,47,20,39,8,243,36,84,233,25,186,2,193,14,154,173,176,134,145,41,76,245,89,0,69,53,231,7,0,101,75,9,164,38,151,176,168,86,54,107,26,76,114,113,
4,177,149,16,4,47,33,216,177,48,154,33,84,105,99,232,14,71,253,53,157,65,121,93,226,133,86,245,130,182,170,18,40,40,40,5,186,46,81,182,215,192,149,34,124,120,87,136,65,234,229,1,158,51,221,193,73,108,
241,66,188,9,107,209,149,168,9,97,156,209,160,113,82,25,196,226,243,236,21,77,31,116,96,27,215,214,214,12,245,191,254,6,183,107,184,140,213,14,85,21,197,236,51,24,170,16,181,53,88,107,179,163,139,8,44,
210,221,70,205,173,91,183,252,94,70,110,23,48,66,16,75,156,216,71,234,233,3,234,239,191,30,181,87,193,143,153,5,48,16,77,216,100,29,232,135,232,248,80,144,232,105,79,66,187,116,3,190,3,75,187,193,96,168,
187,173,1,154,90,238,195,253,134,122,144,155,121,64,104,160,116,68,128,108,140,88,10,161,209,15,224,155,236,98,67,176,40,75,136,144,123,9,174,51,116,143,137,207,61,44,28,46,116,35,168,47,25,61,184,7,5,
133,229,131,223,234,224,155,251,237,176,110,165,12,68,207,137,228,222,166,10,72,203,41,86,203,106,98,176,214,83,15,185,92,126,13,251,141,129,43,56,108,79,125,177,90,55,35,172,22,16,248,210,135,44,39,5,
64,98,156,11,20,164,230,162,83,96,64,55,151,194,119,251,74,245,218,195,69,22,9,203,125,157,244,90,131,25,181,6,155,92,111,236,97,156,152,65,92,210,74,23,190,27,54,108,56,138,133,79,64,141,141,94,36,116,
130,96,4,105,108,71,209,223,3,45,141,191,65,93,125,19,220,111,235,128,62,181,11,32,19,19,33,152,89,90,130,173,157,61,56,56,58,129,227,56,43,131,82,47,165,213,212,212,84,203,147,39,79,254,25,75,67,167,
234,0,233,198,26,225,110,200,138,21,43,242,98,98,98,24,125,79,228,25,164,180,0,137,179,148,125,94,6,81,147,141,181,205,62,63,63,63,20,123,155,203,195,162,137,186,41,186,25,71,108,82,227,237,237,77,215,
246,255,87,72,214,216,216,248,120,122,122,250,92,148,209,5,27,251,46,173,253,4,93,237,211,219,193,180,180,180,184,119,222,121,103,111,88,88,216,152,209,184,92,121,17,66,255,191,180,107,215,174,87,208,
229,179,246,236,217,19,57,226,206,142,94,33,208,131,193,179,9,91,197,36,63,63,63,51,186,51,255,163,4,167,43,43,165,82,121,26,45,31,248,245,215,95,239,195,207,243,13,238,177,233,85,5,61,29,29,29,54,216,
148,124,119,225,194,133,69,50,153,76,57,101,202,20,43,189,19,192,8,2,182,167,167,231,242,247,223,127,191,0,93,251,12,6,111,28,90,190,107,212,110,59,232,149,136,122,55,69,47,24,118,238,220,249,183,171,
87,175,254,9,235,76,235,196,137,19,5,24,116,118,4,202,70,82,164,208,0,181,173,173,173,237,216,224,8,111,222,188,57,205,223,223,255,202,251,239,191,191,107,225,194,133,231,232,197,171,62,244,95,103,164,
1,103,57,59,205,7,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_directional_loudnessAudioProcessorEditor::solo_symbol_png = (const char*) resource_Ambix_directional_loudnessAudioProcessorEditor_solo_symbol_png;
const int Ambix_directional_loudnessAudioProcessorEditor::solo_symbol_pngSize = 3173;

// JUCER_RESOURCE: solo_symbol_over_png, 3175, "solo_symbol_over.png"
static const unsigned char resource_Ambix_directional_loudnessAudioProcessorEditor_solo_symbol_over_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,49,0,0,0,49,8,6,0,0,1,4,155,113,138,0,
0,0,9,112,72,89,115,0,0,14,197,0,0,14,197,1,71,108,236,255,0,0,12,25,73,68,65,84,120,218,205,89,11,84,212,101,22,191,131,127,40,94,205,96,14,227,163,20,200,32,21,55,69,5,211,64,4,45,197,18,12,80,82,129,
53,95,41,184,6,190,16,203,20,141,4,54,4,20,246,44,34,134,120,52,18,212,82,30,137,8,130,226,250,0,20,100,87,176,16,16,89,229,161,152,130,167,49,152,189,247,207,204,48,3,255,25,103,0,107,239,57,159,14,243,
61,238,227,187,223,239,62,134,145,72,36,32,163,192,192,192,171,145,145,145,19,101,127,51,244,207,141,27,55,36,214,214,214,128,19,244,167,108,53,143,157,164,9,25,241,120,60,160,211,30,60,120,48,144,145,
30,37,159,24,57,107,35,251,121,221,186,117,103,25,41,15,60,170,26,30,182,138,193,196,80,15,255,12,3,27,27,155,253,140,252,60,48,3,129,1,128,76,192,53,107,214,236,145,77,242,136,7,29,69,59,104,130,149,
86,182,178,186,186,218,13,165,54,255,224,131,15,118,203,213,32,126,102,102,102,64,3,41,82,166,130,164,75,116,49,126,210,101,23,202,133,144,180,222,197,73,61,185,16,242,9,158,225,48,40,203,63,14,138,166,
226,1,43,109,151,61,11,10,10,28,228,34,243,249,252,199,131,7,15,110,170,168,168,48,183,183,183,239,18,151,200,212,212,84,210,208,208,0,221,41,62,62,30,86,172,88,193,83,186,17,217,98,197,13,151,10,10,192,
228,245,55,192,210,108,40,109,96,101,71,35,193,192,129,3,121,12,25,164,251,233,178,219,234,78,184,1,72,84,134,44,40,53,150,156,20,55,116,63,128,116,99,200,228,40,115,164,84,132,30,212,157,35,25,131,213,
73,166,36,202,44,33,17,184,136,204,137,150,43,80,186,108,69,138,136,136,200,120,252,248,241,75,33,33,33,206,221,231,228,55,215,216,216,232,62,104,208,160,84,146,127,195,134,13,114,201,232,31,20,191,240,
212,169,83,83,229,27,226,226,226,74,86,175,94,61,14,84,16,46,158,130,226,182,161,216,6,236,134,238,139,159,181,181,64,113,89,5,216,217,217,201,191,195,197,250,121,121,121,142,140,177,177,177,4,229,85,
50,233,141,6,49,187,216,6,63,111,190,43,1,207,161,157,115,248,210,114,25,197,197,50,50,23,178,62,11,197,221,12,130,58,2,195,101,255,21,115,38,192,190,140,98,206,251,96,184,148,140,79,47,130,120,185,136,
211,112,211,185,174,13,179,103,207,134,204,204,76,37,29,124,182,238,131,164,237,190,236,203,9,58,85,165,204,1,23,243,20,112,68,73,132,238,226,76,152,48,161,152,21,233,213,87,95,109,107,110,110,54,0,53,
132,7,103,23,21,21,189,199,110,192,197,134,195,134,13,107,184,123,247,174,144,107,177,167,167,103,206,209,163,71,223,83,82,26,23,155,74,221,224,84,71,71,135,185,190,190,190,248,226,197,139,162,250,250,
250,161,184,184,75,7,46,231,147,209,137,19,39,82,188,189,189,231,135,134,134,18,102,245,152,175,172,172,132,181,107,215,194,147,39,79,110,163,71,91,168,58,167,199,85,144,47,10,4,130,113,11,23,46,4,55,
55,55,224,114,6,25,89,90,90,202,110,197,92,102,108,169,13,13,85,190,138,236,236,236,84,244,95,232,11,73,47,73,130,182,111,148,153,147,145,153,197,213,213,117,62,73,207,73,143,46,1,79,48,153,115,106,228,
162,120,184,117,104,121,143,239,233,114,17,118,90,238,221,187,39,96,153,220,185,115,103,62,57,32,55,181,176,12,12,220,146,160,245,184,79,183,185,106,240,249,236,0,92,170,104,1,59,43,65,143,157,200,128,
79,78,193,16,194,40,128,6,7,9,32,251,203,105,48,147,245,126,95,246,27,247,197,107,97,67,208,103,96,55,198,12,14,70,109,87,107,62,242,58,122,229,47,61,207,206,51,182,229,129,100,155,242,119,183,203,115,
144,169,57,251,249,205,53,233,80,25,227,194,185,151,220,154,33,140,68,87,148,144,167,112,209,94,7,93,88,83,240,59,36,223,20,195,98,43,221,46,184,25,227,204,62,83,50,243,173,61,97,0,42,152,208,187,97,239,
100,233,210,165,228,231,230,92,139,252,243,159,129,255,211,74,224,25,232,129,55,199,124,236,153,155,176,218,217,138,147,193,206,157,59,51,241,97,186,48,210,208,96,129,23,116,129,240,152,91,103,75,80,247,
104,213,64,132,139,210,59,145,69,7,84,95,162,237,129,138,84,88,88,152,209,210,210,18,139,12,50,84,190,120,100,192,250,50,5,11,47,47,175,19,24,146,249,4,235,234,136,204,114,252,248,113,17,34,225,132,41,
83,122,26,131,209,80,106,131,31,126,248,97,215,213,171,87,109,16,167,116,241,53,87,121,120,120,132,96,62,240,159,231,109,100,84,77,32,99,254,212,169,83,235,17,135,12,118,239,222,13,136,6,236,144,146,45,
14,175,232,232,104,248,252,243,207,59,82,82,82,60,93,92,92,142,105,204,224,253,247,223,127,140,38,50,66,187,170,149,142,144,23,135,78,123,123,123,26,154,242,119,124,2,34,12,224,15,212,49,208,69,137,197,
8,108,90,93,240,128,1,3,160,169,169,137,73,76,76,164,141,203,63,249,228,147,4,78,6,232,73,226,190,120,18,30,12,120,79,251,14,28,56,208,190,100,201,146,3,74,12,68,34,81,159,220,84,70,19,39,78,132,107,215,
174,197,163,203,30,199,24,212,194,72,3,238,145,91,183,110,61,119,115,117,101,57,220,127,248,4,63,233,193,88,155,241,96,160,203,189,110,217,178,101,12,154,250,14,154,218,152,101,240,233,167,159,122,213,
212,212,168,190,76,155,1,16,83,210,193,57,247,16,181,22,112,124,127,255,254,125,163,156,156,28,103,6,49,253,163,178,178,50,149,135,87,36,251,176,135,235,207,10,133,182,204,205,93,223,159,141,133,183,156,
253,193,132,247,6,186,244,47,61,221,147,97,40,60,255,200,44,90,180,40,17,57,169,100,144,182,239,112,103,224,83,56,156,200,202,201,15,182,126,148,1,166,174,62,42,247,110,217,178,165,153,41,45,45,229,171,
179,187,211,236,119,17,25,207,193,204,141,39,32,47,220,77,105,110,123,90,186,218,59,11,10,10,122,157,161,50,69,29,77,222,156,7,16,204,131,115,17,243,128,23,65,223,216,64,100,210,23,176,210,199,13,12,52,
240,42,102,200,144,33,207,93,68,238,155,159,18,11,211,188,252,41,203,133,64,223,121,56,164,85,79,131,4,108,133,106,24,60,122,244,72,35,255,118,88,224,7,18,28,210,156,29,34,55,248,192,186,232,52,176,51,
229,169,141,27,140,58,15,82,73,186,6,16,24,149,10,239,89,122,194,88,191,84,56,223,2,240,174,64,5,131,186,186,58,49,251,114,56,169,13,225,195,144,51,117,102,11,141,134,70,181,114,20,23,23,151,51,27,55,
110,220,143,159,87,169,8,3,176,193,76,7,34,170,59,144,209,27,144,127,35,7,236,49,141,105,107,105,132,176,191,121,64,72,114,62,187,74,149,244,88,162,63,101,54,109,218,180,26,241,126,85,64,64,0,231,162,
240,219,237,240,230,38,15,88,17,158,6,14,214,202,121,130,142,249,108,104,175,202,80,109,73,93,221,135,44,84,36,36,36,220,67,6,131,85,45,92,30,150,138,67,187,107,66,161,127,60,125,250,180,43,203,160,188,
188,124,136,143,143,79,251,193,131,7,117,160,127,168,13,115,164,223,200,42,114,184,70,200,112,65,44,207,34,184,237,43,97,84,251,13,43,205,249,74,241,0,195,228,79,169,169,169,94,24,212,191,115,116,116,
236,245,225,24,3,126,197,88,48,144,51,162,97,166,144,114,230,204,153,230,197,139,23,127,127,232,208,33,19,109,14,198,58,164,18,171,62,125,60,124,184,218,160,63,99,198,140,51,56,6,190,253,246,219,101,31,
127,252,177,8,1,75,168,238,96,196,253,95,112,173,241,177,99,199,150,212,214,214,22,106,156,182,92,191,126,125,44,253,31,21,21,245,217,174,93,187,130,80,245,103,88,140,183,24,25,25,137,49,134,240,48,227,
120,19,77,153,155,156,156,236,141,127,171,196,27,166,175,113,152,2,214,254,253,251,55,157,61,123,214,170,164,164,132,79,25,6,38,102,96,97,97,1,134,134,134,240,202,43,175,80,253,192,214,133,173,173,173,
80,85,85,69,21,18,33,131,216,198,198,230,191,104,173,12,76,22,190,20,10,133,141,189,149,129,209,114,189,30,230,8,73,232,226,158,79,159,62,29,176,126,253,122,240,245,245,165,224,197,14,109,207,194,49,130,
144,10,125,99,21,37,144,223,124,243,77,7,159,207,111,136,137,137,249,171,179,179,243,79,253,170,4,38,250,71,177,116,241,152,59,119,46,49,130,155,55,111,66,127,18,186,17,208,219,199,65,120,50,24,149,202,
66,124,233,192,135,243,24,83,185,5,244,82,123,171,132,238,183,223,126,123,61,48,48,112,20,22,14,228,248,240,71,17,41,37,5,72,126,110,110,110,22,21,137,8,55,11,220,221,221,191,215,88,9,178,60,186,137,71,
122,122,58,181,202,224,207,164,233,211,167,179,145,21,35,101,10,166,166,251,177,46,153,71,200,163,78,9,61,188,186,230,225,195,135,27,105,91,50,188,104,66,16,160,254,140,17,225,242,158,61,123,206,99,133,
55,183,135,18,84,165,141,26,53,170,25,163,246,0,44,39,122,201,234,25,84,228,124,7,113,255,72,128,152,180,124,181,43,93,22,111,4,95,191,85,48,127,178,153,86,28,40,176,196,198,198,58,18,254,203,32,90,174,
4,149,153,126,126,126,189,84,160,30,182,142,31,14,59,174,181,43,247,34,221,87,128,251,244,73,96,49,200,24,33,182,9,26,235,110,65,102,114,12,20,84,73,32,227,80,56,59,22,80,169,180,62,29,174,68,184,104,
204,13,229,52,70,200,22,57,57,57,33,178,159,117,98,149,248,234,171,175,46,81,157,204,213,64,212,132,242,183,45,84,80,96,57,148,75,226,97,180,138,181,155,183,69,117,182,143,190,95,9,22,11,58,27,189,87,
255,62,7,38,233,159,131,43,33,14,26,243,164,8,156,157,157,109,65,129,146,148,48,64,216,180,189,124,249,114,47,93,168,5,74,47,95,232,130,180,191,12,1,161,6,187,204,231,255,19,36,56,250,66,40,183,104,214,
172,89,65,12,181,64,240,33,195,200,145,35,123,11,136,176,104,91,56,172,201,12,236,124,21,165,33,96,202,11,97,179,218,47,214,249,130,235,28,39,176,54,19,130,238,11,120,236,227,198,141,123,25,225,184,137,
161,30,206,216,177,99,251,116,152,137,109,0,2,67,0,20,39,111,2,79,159,112,160,150,126,199,237,76,216,238,79,131,107,135,5,184,47,95,6,171,214,250,130,243,152,161,125,226,77,185,22,67,77,40,76,184,250,
7,6,189,195,224,23,239,174,90,224,89,91,35,84,148,149,66,113,81,1,100,167,231,194,161,12,25,98,85,97,141,30,204,142,78,114,135,130,135,169,42,139,29,117,68,201,34,67,93,180,11,23,46,216,190,8,108,215,
53,16,130,181,157,51,59,124,86,111,131,100,197,86,200,191,147,193,117,140,47,20,176,191,3,164,129,189,137,11,92,106,205,0,91,3,237,120,80,182,203,80,27,16,211,96,175,222,10,122,187,36,7,10,10,47,67,81,
110,22,92,55,89,10,25,251,124,52,234,21,152,140,246,134,204,75,117,96,100,23,44,47,77,123,83,115,81,186,206,80,31,115,252,248,241,53,209,209,209,35,168,35,168,117,42,90,123,18,124,253,163,101,96,11,130,
203,229,112,227,106,24,88,62,231,37,55,156,223,13,34,251,96,5,84,27,3,230,90,234,17,26,26,90,132,245,70,103,11,238,240,225,195,147,48,255,191,135,249,146,14,37,95,218,208,48,215,40,144,52,126,8,238,194,
153,112,12,93,227,89,105,56,88,233,133,107,117,134,227,23,103,32,183,231,143,173,240,156,74,172,234,235,175,191,30,95,87,87,247,26,171,4,21,36,212,240,197,151,126,180,166,166,70,135,10,27,173,104,144,
51,164,73,164,109,192,182,122,248,87,94,14,100,158,62,15,87,74,110,194,253,39,79,186,30,161,225,96,16,154,189,6,214,227,38,129,189,227,12,112,176,49,235,21,244,138,197,226,38,76,59,140,78,158,60,249,33,
214,31,143,228,105,7,117,172,147,146,146,92,68,34,209,169,159,127,254,153,209,246,70,186,186,46,67,97,178,139,55,59,94,4,81,145,109,101,101,101,138,153,246,156,41,83,166,20,246,200,98,169,154,162,206,
248,232,209,163,171,118,238,220,201,167,30,243,255,19,237,221,187,247,68,120,120,248,132,234,234,234,17,88,246,254,170,178,158,160,214,126,125,125,189,32,49,49,113,25,222,198,94,172,174,94,234,143,230,
74,95,232,60,146,189,189,253,187,71,142,28,73,169,173,173,157,167,113,101,71,63,33,208,192,242,112,9,214,24,49,97,97,97,47,83,207,252,15,148,189,13,45,127,122,235,214,173,211,226,226,226,98,177,84,176,
239,117,141,77,63,85,208,192,186,87,224,234,234,122,16,141,50,61,36,36,68,178,114,229,74,99,106,161,247,39,209,131,197,90,161,112,199,142,29,14,88,189,101,39,36,36,44,243,247,247,255,181,223,186,29,244,
147,136,98,53,69,63,48,96,10,191,229,202,149,43,118,182,182,182,141,51,103,206,212,193,140,82,72,73,153,38,86,198,114,243,118,86,86,214,3,76,167,245,49,127,123,235,157,119,222,185,24,28,28,28,26,16,16,
144,167,170,141,170,138,254,7,127,25,105,77,112,230,42,29,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_directional_loudnessAudioProcessorEditor::solo_symbol_over_png = (const char*) resource_Ambix_directional_loudnessAudioProcessorEditor_solo_symbol_over_png;
const int Ambix_directional_loudnessAudioProcessorEditor::solo_symbol_over_pngSize = 3175;

// JUCER_RESOURCE: solo_symbol_act_png, 3257, "solo_symbol_act.png"
static const unsigned char resource_Ambix_directional_loudnessAudioProcessorEditor_solo_symbol_act_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,49,0,0,0,49,8,6,0,0,1,4,155,113,138,0,0,
0,9,112,72,89,115,0,0,14,197,0,0,14,197,1,71,108,236,255,0,0,12,107,73,68,65,84,120,218,205,89,9,88,83,87,22,62,47,121,9,100,129,132,29,7,91,4,171,168,31,84,5,171,117,129,34,210,98,41,20,149,77,139,130,
160,173,74,101,220,171,216,106,45,45,74,91,69,112,97,58,86,68,197,78,101,145,106,65,16,113,161,160,56,150,69,81,59,69,173,224,138,32,38,6,67,130,36,36,111,222,125,49,36,129,36,178,216,206,252,223,119,
9,112,183,115,206,61,123,112,130,32,64,141,149,171,199,85,37,111,173,26,167,254,27,71,63,174,213,113,8,215,17,116,72,222,138,254,50,127,190,250,41,70,77,162,9,53,48,76,12,4,97,6,66,161,208,18,87,29,165,
153,120,45,0,163,126,95,181,122,218,25,92,117,135,234,168,39,50,46,88,48,219,168,73,247,177,49,233,56,104,129,207,192,168,35,17,226,226,226,118,226,234,203,209,29,232,40,180,3,77,80,212,170,89,185,125,
231,232,140,107,215,26,156,2,222,91,177,189,139,13,116,223,16,71,0,52,0,62,79,126,206,130,138,8,53,233,234,133,93,68,16,50,51,157,201,174,9,140,41,134,171,231,217,160,37,170,167,24,218,170,57,6,160,188,
252,184,87,23,201,60,158,153,216,222,222,250,241,245,235,13,78,158,158,90,228,34,216,218,97,196,163,102,51,232,142,61,233,50,248,104,193,51,76,231,69,212,139,181,55,92,188,160,0,139,87,48,24,62,152,70,
110,96,82,92,8,197,4,88,154,137,49,28,9,164,251,233,186,44,107,96,105,134,1,34,21,71,18,84,9,75,3,237,13,221,15,64,188,225,72,228,123,210,227,147,85,36,244,68,247,27,145,48,40,158,212,76,10,197,102,4,
34,65,31,144,56,61,61,61,203,117,30,91,27,223,110,243,47,20,139,101,38,9,155,78,77,235,62,215,245,114,45,130,35,193,214,86,209,185,232,142,53,171,160,235,205,209,207,128,64,183,138,130,252,243,147,187,
54,164,253,211,253,82,236,162,63,198,128,1,20,228,95,157,100,105,201,150,10,133,82,54,181,161,251,98,185,28,160,230,138,2,38,120,104,12,69,40,196,89,165,165,165,222,184,153,57,70,136,159,234,138,244,218,
19,14,181,216,157,252,61,254,17,27,66,109,84,27,93,199,6,158,197,181,23,171,225,196,167,81,159,53,221,68,107,205,195,0,215,39,255,143,222,147,194,247,133,10,189,239,129,235,99,114,207,113,54,236,49,240,
234,248,187,254,18,40,42,228,232,240,16,153,128,195,129,13,44,234,247,117,39,77,117,111,40,42,84,96,26,63,162,75,66,119,114,60,60,134,213,80,36,89,89,41,164,2,1,157,13,70,80,116,114,98,73,117,117,241,
59,212,6,129,64,194,113,112,176,125,244,224,193,51,27,125,139,67,195,198,159,206,201,46,126,71,135,233,7,15,30,217,82,106,16,16,80,160,36,26,156,88,44,166,236,66,69,179,93,99,99,227,223,114,178,181,120,
208,167,124,106,28,205,15,207,154,23,145,29,182,57,201,4,226,98,123,154,193,141,122,37,44,91,218,14,109,109,142,13,229,101,245,206,134,206,233,241,20,72,23,249,252,186,49,31,132,227,48,35,16,64,159,50,
168,49,220,153,246,252,85,30,59,169,133,173,146,161,132,99,208,42,74,78,205,203,141,93,132,27,82,131,94,65,245,72,230,132,131,131,105,139,90,156,184,90,44,65,129,69,97,136,122,189,104,35,0,51,107,211,
59,245,90,20,13,110,238,231,244,248,63,122,92,251,65,124,81,211,67,17,159,58,245,222,189,99,97,24,48,13,82,135,46,96,147,43,36,89,220,30,115,145,43,219,225,98,61,105,169,206,244,30,115,77,15,149,60,164,
20,56,242,48,107,86,49,141,138,160,100,147,41,188,189,233,25,96,217,98,234,239,224,40,58,172,137,55,129,9,46,116,56,152,204,50,186,23,105,29,142,92,216,139,228,236,251,57,3,8,114,104,163,225,186,146,50,
57,132,97,43,233,112,99,155,126,123,65,106,141,35,31,121,163,158,75,32,77,209,135,93,94,237,16,87,222,9,153,245,108,152,235,164,17,137,147,11,141,50,83,116,209,205,100,210,139,108,211,79,32,178,27,234,
77,22,204,183,37,245,28,169,97,79,44,45,99,193,82,185,42,154,206,211,51,191,187,148,5,177,111,233,87,152,175,18,167,20,53,54,22,250,83,179,200,144,2,2,39,159,71,254,88,239,106,6,232,141,118,198,160,114,
17,133,254,58,118,162,142,14,24,134,17,125,61,80,27,21,191,122,21,138,30,199,238,206,201,246,47,52,104,241,228,13,84,36,68,193,98,246,156,25,71,175,213,41,120,200,173,27,3,18,203,79,121,205,118,213,213,
213,30,147,198,235,113,43,198,124,151,6,82,246,177,252,117,73,85,85,151,220,219,196,50,134,195,96,231,250,144,89,27,19,134,12,25,249,251,139,118,26,244,29,4,209,202,155,236,233,208,104,101,213,206,222,
158,202,130,160,64,26,57,212,179,191,147,180,30,159,157,186,75,6,159,173,151,43,179,14,231,132,250,251,207,202,235,245,5,126,211,205,197,179,63,144,115,43,206,33,155,224,24,164,110,217,82,38,26,52,5,204,
63,98,109,29,222,121,227,70,179,157,165,165,165,208,200,5,114,134,149,21,83,38,16,152,169,212,168,151,64,22,243,248,49,11,223,119,192,65,0,138,157,31,198,196,44,220,171,247,2,12,99,202,6,162,73,49,81,
12,168,186,188,236,251,140,12,186,34,58,58,58,67,231,2,59,251,129,169,169,26,227,198,208,225,242,165,165,123,68,162,153,63,241,249,124,17,174,10,184,115,126,188,217,192,125,225,230,219,100,196,107,126,
162,210,58,55,50,181,49,20,197,23,70,211,113,43,171,87,238,9,4,98,51,234,130,197,31,30,158,125,231,142,97,234,151,185,75,96,199,37,165,222,185,39,36,215,124,61,255,111,22,96,220,211,167,79,79,195,155,
30,229,205,186,90,103,152,250,235,135,20,212,225,44,178,114,145,230,107,214,93,47,85,192,136,169,82,176,48,144,64,35,202,103,204,120,255,103,60,34,34,102,223,233,18,195,214,122,100,79,7,245,217,154,175,
75,132,139,55,29,54,134,224,96,59,211,112,24,254,116,163,181,0,191,82,219,202,3,48,44,30,31,127,50,80,149,183,195,219,235,165,80,186,89,87,234,95,228,24,15,68,235,214,60,121,5,247,244,162,25,93,244,230,
58,146,194,120,128,95,182,40,0,219,162,10,60,201,63,152,192,162,15,152,192,238,133,86,225,131,6,209,94,184,8,201,184,44,183,19,222,10,109,87,21,169,17,29,212,160,170,30,50,89,30,207,55,124,6,222,218,74,
244,74,191,189,72,121,107,63,102,242,138,103,176,42,69,14,19,44,36,70,227,6,126,245,138,178,95,6,181,114,187,41,188,51,28,7,183,216,118,56,39,81,194,20,142,126,46,240,251,247,24,50,242,211,96,202,161,
14,248,250,168,20,54,27,231,190,166,118,200,111,248,39,107,23,164,3,28,90,98,104,209,154,33,52,248,246,182,42,179,40,171,99,131,39,153,198,72,201,100,237,235,143,159,65,194,193,78,106,141,33,234,87,45,
55,105,199,215,126,146,22,187,61,53,125,201,138,101,250,179,154,111,26,56,48,44,190,3,62,74,146,129,215,8,169,206,28,109,40,128,226,15,195,242,103,48,44,158,80,86,178,247,123,203,166,21,203,36,246,134,
22,126,184,197,132,26,125,193,246,20,159,159,79,158,60,26,68,93,240,219,181,135,131,34,231,51,21,7,247,155,210,224,165,0,151,94,168,96,118,172,88,174,229,174,35,230,228,251,87,93,158,117,2,185,219,129,
194,210,18,58,132,194,236,48,157,120,224,231,231,87,156,155,187,111,118,91,91,204,97,239,41,253,191,132,207,199,158,138,68,66,75,189,17,45,36,36,60,235,212,41,43,193,220,121,225,217,135,50,229,22,125,
57,184,229,137,195,13,143,209,18,150,72,116,247,85,163,65,223,215,215,247,148,175,175,192,114,244,104,183,171,115,230,182,216,173,91,35,181,49,118,112,179,112,208,173,209,163,68,102,121,121,233,209,119,
239,78,170,232,117,218,82,91,123,213,13,125,166,164,164,44,79,74,74,90,199,231,51,228,174,110,124,17,151,203,148,53,53,17,88,197,249,155,195,188,189,167,158,205,204,204,156,215,212,196,107,53,248,220,
189,75,188,12,3,5,172,244,244,111,214,158,57,83,231,114,169,186,149,71,39,73,118,112,192,192,121,40,6,28,14,6,230,230,24,40,73,111,36,22,19,32,145,16,80,127,139,128,7,247,9,210,51,48,101,238,30,131,30,
250,250,250,23,198,68,127,241,185,141,141,77,75,191,117,182,111,203,101,204,162,147,81,7,86,44,203,13,109,151,118,210,87,175,53,129,168,104,6,124,26,143,145,3,205,247,41,49,33,93,160,208,17,121,42,81,
91,230,146,237,169,50,216,182,181,83,201,227,217,62,218,145,186,127,254,180,105,126,197,47,149,137,130,194,176,156,5,49,57,33,239,7,209,97,91,10,11,234,126,103,193,203,4,159,139,1,242,136,228,32,253,137,
196,94,212,22,114,34,114,126,135,242,212,73,182,56,35,35,43,28,89,106,63,153,144,51,246,103,142,169,93,185,252,63,35,75,203,217,208,220,100,6,127,21,16,83,42,7,169,228,157,45,159,117,2,195,164,144,155,
155,21,30,28,28,150,221,107,38,144,228,163,230,229,132,28,63,193,6,161,224,175,35,94,31,166,122,210,169,200,90,115,37,58,203,202,106,65,122,86,214,79,51,145,231,49,194,132,140,233,55,221,90,240,170,163,
148,43,248,31,19,223,29,238,175,211,65,32,0,46,242,203,59,119,78,62,119,236,216,207,239,247,96,2,85,105,35,71,89,11,214,198,227,244,232,72,118,191,47,67,105,114,218,238,14,216,145,171,48,186,206,63,138,
14,81,113,38,16,230,209,183,104,132,2,203,238,239,202,188,145,255,87,187,232,46,38,80,153,249,113,28,157,100,128,209,47,226,55,142,149,194,151,151,117,9,247,8,165,65,176,15,3,156,109,104,32,126,74,64,
203,61,37,20,29,144,67,121,61,1,133,7,20,228,144,66,56,42,149,226,233,80,185,185,247,130,251,120,177,210,76,44,110,177,243,241,241,57,67,194,135,98,34,113,203,155,23,81,157,28,23,203,233,23,3,101,155,
228,58,12,252,70,112,97,20,232,175,69,226,55,170,210,162,134,28,5,56,135,169,242,175,42,50,163,127,131,37,133,202,13,189,103,4,69,224,146,226,6,103,20,40,113,212,65,216,246,237,197,241,191,214,112,250,
173,66,87,126,237,212,36,122,30,24,216,0,246,194,61,78,161,116,24,104,193,187,45,69,104,55,221,55,105,29,142,90,32,175,58,98,240,218,144,254,167,124,17,9,38,16,87,164,98,68,94,77,128,45,153,70,163,172,
118,195,106,38,4,5,48,192,117,48,13,24,127,130,177,143,113,37,76,201,84,229,49,142,122,56,110,175,15,44,103,181,24,167,106,250,213,252,208,9,161,115,219,161,30,181,42,111,145,85,228,18,25,53,244,33,120,
49,29,98,151,155,128,143,203,192,114,89,148,107,225,168,9,197,229,98,47,199,13,70,224,112,43,66,163,34,232,43,154,235,181,10,168,169,82,66,73,129,12,14,29,215,148,116,71,190,83,144,67,83,147,148,183,113,
12,22,59,198,128,146,69,28,117,209,206,159,175,26,255,103,248,118,6,169,67,174,227,232,212,136,92,204,128,76,237,86,72,157,18,130,70,74,161,28,84,9,168,39,87,2,23,229,100,229,140,247,141,17,148,237,226,
168,13,152,180,249,240,236,254,18,218,64,22,204,229,21,10,168,62,45,135,90,107,128,194,127,176,123,213,43,176,24,65,131,162,74,54,112,223,144,12,168,230,66,233,58,142,250,152,99,221,29,239,164,238,106,
114,68,29,193,190,130,121,135,128,168,37,207,52,121,79,101,27,92,171,226,192,240,23,120,168,71,36,227,118,147,165,58,94,205,169,143,175,176,57,201,181,218,219,251,121,11,238,95,63,84,190,225,60,212,190,
41,106,62,131,198,239,163,125,56,4,146,174,82,200,133,96,75,9,228,145,170,129,188,147,11,214,214,167,51,188,19,112,56,187,161,111,153,113,179,208,190,126,75,226,165,177,247,239,223,31,76,49,129,10,18,
212,240,117,29,25,154,115,231,30,135,214,103,127,97,129,193,17,226,121,19,141,52,230,127,159,81,64,81,177,28,42,107,20,208,220,70,104,25,33,121,23,233,202,93,221,233,224,57,21,7,47,55,122,191,92,175,140,
224,60,30,61,170,149,155,159,159,31,200,227,241,90,187,210,14,212,177,62,176,191,208,223,206,58,160,224,143,219,166,56,191,191,30,139,164,234,77,63,58,53,254,12,160,34,219,101,104,147,109,65,193,241,247,
38,77,82,213,194,58,89,44,170,166,80,103,124,212,112,231,250,175,182,116,240,80,143,249,255,9,187,210,124,143,126,147,84,229,113,251,246,29,71,115,115,243,167,6,235,9,212,218,111,108,20,241,247,237,219,
187,144,111,177,116,215,169,179,116,147,151,209,92,25,8,206,93,24,115,206,115,82,249,148,31,127,12,203,186,123,55,111,102,175,43,59,244,21,2,26,25,25,25,209,126,211,254,190,227,235,173,74,83,212,51,255,
235,72,199,165,187,210,188,79,110,252,172,244,173,180,180,216,221,4,81,230,217,239,26,27,125,85,129,134,72,36,226,7,5,69,30,60,119,238,236,212,132,68,58,177,136,76,135,95,54,71,200,96,119,167,78,168,248,
50,225,23,47,178,122,43,217,187,119,239,194,165,177,26,181,25,112,183,3,125,37,162,93,77,161,47,24,18,19,19,63,173,172,188,56,97,252,4,219,150,183,253,148,180,233,239,138,108,80,82,214,27,41,215,212,14,
110,56,81,196,23,150,20,183,179,170,170,234,70,76,156,56,241,194,250,245,235,55,175,88,238,93,138,26,134,125,193,127,1,140,113,41,29,111,6,190,237,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_directional_loudnessAudioProcessorEditor::solo_symbol_act_png = (const char*) resource_Ambix_directional_loudnessAudioProcessorEditor_solo_symbol_act_png;
const int Ambix_directional_loudnessAudioProcessorEditor::solo_symbol_act_pngSize = 3257;

// JUCER_RESOURCE: drag_off_png, 903, "drag_off.png"
static const unsigned char resource_Ambix_directional_loudnessAudioProcessorEditor_drag_off_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,31,0,0,0,31,8,6,0,0,1,104,169,38,175,0,0,0,9,112,
72,89,115,0,0,17,141,0,0,17,141,1,26,43,169,153,0,0,3,57,73,68,65,84,120,218,189,87,205,111,76,81,20,127,211,94,157,81,31,19,70,75,136,38,118,54,172,172,37,18,177,242,81,98,131,77,117,49,66,66,8,18,218,
68,137,175,96,65,124,196,103,106,65,125,68,50,58,17,75,44,252,7,42,93,10,42,40,90,25,58,163,58,211,142,243,155,252,78,221,60,111,94,223,155,118,252,146,95,59,239,190,123,207,61,239,220,223,57,231,61,83,
44,22,29,226,164,176,221,56,127,209,142,63,24,56,37,108,211,81,99,95,148,6,104,99,161,240,163,206,24,22,198,56,161,104,172,11,32,130,1,172,137,112,32,99,44,63,134,93,179,139,186,66,55,176,111,58,180,178,
213,232,238,101,208,106,92,38,183,9,239,9,159,9,87,8,227,182,15,192,97,97,175,48,109,63,185,99,121,126,154,215,8,207,43,225,3,76,120,110,61,150,2,225,154,167,22,86,150,113,240,155,112,20,19,134,124,158,
162,20,135,184,207,132,62,76,152,89,230,102,41,178,152,144,21,174,19,62,98,52,23,11,223,171,227,238,56,56,124,170,85,194,49,90,201,11,103,9,107,224,52,93,202,186,227,164,177,122,43,92,226,76,12,204,93,
47,124,226,14,116,80,68,232,221,69,213,104,206,9,143,152,158,67,175,135,212,130,96,17,226,4,3,221,150,62,218,66,24,248,80,82,35,47,106,133,59,25,139,6,202,164,28,6,133,115,198,143,209,186,113,149,4,206,
10,247,91,1,27,101,226,44,19,206,245,202,107,47,108,100,210,100,132,157,94,94,25,215,53,44,127,165,96,32,162,31,194,58,230,80,13,147,172,217,107,241,17,225,49,225,33,225,25,15,79,48,247,151,173,21,93,
220,194,133,126,2,42,8,167,9,191,216,101,6,184,237,147,69,110,52,210,80,10,139,111,82,117,217,144,138,203,99,113,43,19,32,12,10,170,54,60,231,211,10,228,58,190,184,18,140,24,42,104,54,207,52,12,162,134,
154,133,130,54,135,92,92,107,168,217,207,33,23,94,67,208,176,184,159,71,5,3,11,2,46,222,161,110,3,51,168,154,204,4,53,89,171,235,26,13,152,93,175,178,52,130,163,75,90,61,108,187,240,134,213,50,94,122,
101,21,60,72,8,123,132,239,88,44,52,103,79,8,59,130,230,179,13,180,217,163,60,145,56,13,214,186,230,140,210,251,1,225,125,150,184,126,63,163,198,231,30,50,245,58,235,129,195,144,60,22,62,20,190,240,200,
37,104,109,53,122,62,67,182,135,227,57,142,165,131,108,190,87,120,158,191,223,8,151,7,76,90,136,60,69,42,18,108,114,90,240,119,11,47,123,109,158,224,1,215,177,26,54,58,147,199,0,91,177,67,219,151,88,40,
209,44,114,198,234,73,125,60,179,114,149,116,178,128,110,206,9,15,8,127,226,225,116,243,30,110,124,167,74,27,43,14,10,151,10,215,10,95,99,243,77,20,203,24,123,104,181,145,100,148,27,12,219,35,210,230,
123,200,110,80,41,62,241,229,167,222,176,36,33,111,163,206,255,67,84,95,245,59,89,16,98,60,130,84,149,55,110,209,202,103,152,86,87,152,131,93,76,141,66,149,54,198,3,222,162,184,143,171,218,81,141,154,
132,27,120,30,243,153,163,83,137,38,22,156,8,163,221,97,23,153,102,110,222,205,247,178,65,22,158,169,192,16,155,142,227,215,149,210,244,12,95,88,119,41,68,28,193,5,190,7,231,3,110,54,157,37,58,105,141,
109,193,119,86,144,198,210,69,214,179,240,236,99,101,210,239,138,49,254,255,77,103,53,83,106,172,183,161,17,134,119,23,127,255,131,63,229,47,212,182,38,118,159,122,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_directional_loudnessAudioProcessorEditor::drag_off_png = (const char*) resource_Ambix_directional_loudnessAudioProcessorEditor_drag_off_png;
const int Ambix_directional_loudnessAudioProcessorEditor::drag_off_pngSize = 903;

// JUCER_RESOURCE: drag_over_png, 693, "drag_over.png"
static const unsigned char resource_Ambix_directional_loudnessAudioProcessorEditor_drag_over_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,31,0,0,0,31,8,6,0,0,1,104,169,38,175,0,0,0,9,
112,72,89,115,0,0,17,141,0,0,17,141,1,26,43,169,153,0,0,2,103,73,68,65,84,120,218,189,87,207,75,84,81,20,190,175,185,80,76,133,144,216,194,160,254,2,119,65,173,90,182,44,141,54,234,162,69,110,10,108,99,
155,12,146,168,148,114,169,20,20,133,144,98,171,208,252,19,92,216,62,145,246,253,32,75,23,253,178,8,199,219,247,13,231,200,157,231,157,55,239,221,169,247,193,199,48,119,238,185,231,220,243,243,142,117,
206,25,193,61,240,166,149,47,186,58,202,133,113,227,129,11,163,224,141,221,5,57,163,27,252,168,59,92,90,196,100,45,124,179,158,29,206,236,69,98,69,193,7,19,134,179,170,189,9,126,169,206,36,165,98,11,220,
6,59,124,27,140,92,112,13,92,76,91,29,50,176,31,124,193,13,175,155,232,159,215,13,167,50,140,172,89,147,141,164,213,134,119,220,112,8,252,17,248,113,7,60,193,13,63,193,115,224,171,212,134,138,31,79,31,
43,224,233,102,70,211,121,114,232,158,232,58,211,26,21,207,92,90,181,100,11,8,167,193,43,77,106,142,198,98,196,74,236,99,177,195,3,22,196,229,251,34,14,232,180,158,115,174,128,15,115,8,253,6,15,72,138,
55,68,225,145,144,120,192,251,121,181,80,147,194,233,97,29,54,20,105,32,15,20,125,224,73,240,43,248,12,220,104,85,225,71,192,205,192,65,247,229,147,69,214,27,18,158,4,175,183,184,247,121,201,149,134,123,
15,229,16,52,169,76,221,45,175,39,17,33,123,73,225,153,200,132,233,163,240,165,72,97,103,219,72,85,211,142,176,179,190,235,11,162,238,109,102,78,87,172,48,115,246,83,65,65,22,137,165,240,186,52,255,106,
1,97,86,215,126,117,216,193,130,45,234,44,248,199,247,118,34,93,179,154,149,146,224,25,112,57,20,42,90,208,9,190,17,39,86,60,139,238,130,99,121,235,217,71,183,8,95,4,15,103,88,102,36,122,243,242,66,88,
143,77,178,203,224,227,2,125,82,115,141,55,190,38,212,73,62,224,15,236,44,229,183,192,219,230,223,161,42,13,159,24,6,167,67,202,25,171,207,145,19,33,47,166,164,171,210,59,91,170,252,24,248,222,148,3,122,
226,59,120,84,149,191,53,229,130,222,93,165,242,11,242,22,42,27,93,86,198,99,217,168,191,54,172,204,229,178,145,232,60,224,131,96,34,114,38,180,5,42,255,34,181,55,92,146,78,237,209,119,52,219,217,141,
142,203,163,164,12,151,211,219,99,126,147,233,21,229,11,255,65,97,77,255,36,100,77,165,69,177,108,16,156,109,83,161,62,105,157,212,117,253,127,86,158,193,50,39,100,55,122,46,30,169,4,98,231,82,67,37,73,
53,146,167,224,85,14,238,144,146,191,151,1,140,114,231,143,189,71,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_directional_loudnessAudioProcessorEditor::drag_over_png = (const char*) resource_Ambix_directional_loudnessAudioProcessorEditor_drag_over_png;
const int Ambix_directional_loudnessAudioProcessorEditor::drag_over_pngSize = 693;

// JUCER_RESOURCE: drag_on_png, 1172, "drag_on.png"
static const unsigned char resource_Ambix_directional_loudnessAudioProcessorEditor_drag_on_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,31,0,0,0,31,8,6,0,0,1,104,169,38,175,0,0,0,9,112,
72,89,115,0,0,17,141,0,0,17,141,1,26,43,169,153,0,0,4,70,73,68,65,84,120,218,181,87,75,104,156,85,20,62,231,159,191,157,48,153,52,154,113,180,166,165,150,10,69,132,236,20,5,193,93,21,55,73,42,105,67,226,
66,68,177,180,144,108,18,10,137,144,250,168,138,134,118,211,16,37,98,81,90,169,148,210,38,233,162,224,86,80,119,34,106,45,210,46,74,147,210,188,52,111,19,51,153,227,119,238,189,127,250,103,94,157,63,38,
7,206,63,247,117,238,121,159,115,199,23,17,114,240,33,240,29,223,142,147,193,106,183,46,124,68,33,208,133,110,96,215,218,130,187,163,22,120,215,158,144,101,33,78,217,109,153,195,66,48,81,224,42,61,49,
103,6,22,230,103,253,53,57,194,164,142,156,152,217,183,12,146,163,196,180,30,204,45,77,226,7,220,11,67,203,63,78,179,121,38,229,197,111,96,124,9,120,101,145,232,64,6,131,106,63,100,11,114,10,94,7,14,133,
53,39,75,93,21,58,119,28,216,211,130,207,183,80,107,225,39,226,199,114,120,127,10,252,250,2,209,45,28,160,248,115,133,5,28,211,207,42,44,53,67,37,0,118,144,26,202,179,193,125,184,131,27,56,169,122,230,
109,201,116,22,132,79,168,22,11,216,175,135,169,135,173,169,247,97,243,23,53,115,44,236,207,16,229,194,143,68,137,231,33,31,38,203,56,8,148,106,140,61,35,52,8,171,237,165,97,59,57,82,162,63,64,240,108,
1,85,214,70,49,35,174,117,116,61,198,87,139,24,250,1,160,103,101,10,42,197,123,93,140,46,82,100,48,246,152,239,240,173,239,227,209,47,160,189,250,201,234,5,131,80,45,139,248,240,108,8,148,9,242,171,218,
38,101,109,96,92,210,115,20,216,79,244,100,16,38,69,8,71,151,136,171,43,64,204,185,94,248,204,226,173,32,26,59,66,62,88,117,137,83,135,217,236,186,68,207,139,131,251,112,16,248,12,80,131,245,44,112,50,
247,128,159,51,175,33,89,157,178,1,3,147,240,95,16,183,18,227,237,159,24,247,51,107,146,53,230,19,203,124,47,81,101,39,177,38,99,127,129,224,193,81,153,104,32,222,38,193,170,11,158,204,91,196,201,78,84,
140,18,166,70,218,243,195,248,253,19,231,31,151,160,204,104,68,126,65,180,171,76,127,237,7,78,43,223,203,200,205,149,175,76,130,208,76,132,136,123,4,220,255,62,8,206,222,235,182,44,69,129,140,81,214,183,
150,189,182,129,112,205,228,185,42,2,172,40,231,44,76,159,130,233,167,34,18,199,213,218,30,34,231,100,154,232,104,68,98,143,181,208,213,145,180,222,35,142,66,124,26,73,194,190,111,83,201,67,133,184,158,
32,122,186,76,226,183,43,84,238,32,53,17,192,123,132,100,4,227,221,165,11,130,205,233,151,48,249,215,95,87,2,249,161,5,20,190,4,241,69,76,223,7,222,14,250,21,136,62,71,72,198,52,155,95,196,194,247,133,
178,170,18,82,160,96,53,227,250,230,180,171,164,226,174,62,137,239,137,114,243,57,12,181,8,99,16,103,155,64,226,202,175,151,115,68,93,110,214,38,33,192,5,247,66,24,43,117,105,241,32,147,204,155,248,14,
64,120,199,69,27,140,222,249,29,240,135,2,185,164,177,246,2,126,95,73,147,28,106,135,63,218,221,198,34,132,105,13,55,236,226,204,101,182,135,120,199,123,182,248,252,142,133,151,203,76,90,13,242,97,139,
107,97,135,126,47,63,35,138,118,12,186,133,54,96,95,33,230,41,104,59,78,84,229,153,146,203,251,233,255,3,172,206,181,110,140,48,144,189,103,96,5,84,85,210,120,88,12,152,239,66,249,29,177,218,30,119,253,
96,179,161,14,119,163,231,74,23,162,153,209,203,233,81,87,137,151,111,160,206,99,115,96,139,24,7,128,24,100,180,72,57,236,65,128,223,148,249,171,176,126,210,52,28,147,91,91,13,202,163,73,179,48,237,219,
246,8,55,243,100,180,110,176,97,208,194,177,4,77,43,61,223,114,196,88,18,165,158,138,155,12,113,118,157,72,31,4,222,199,118,161,222,165,203,86,66,139,43,124,54,213,38,224,252,62,146,88,27,9,228,224,157,
65,143,220,2,72,184,26,205,170,249,7,65,170,181,99,97,15,73,188,1,15,19,8,240,212,131,42,227,198,158,152,166,163,24,198,250,252,58,17,46,50,141,216,0,115,127,144,228,38,14,161,222,152,151,227,38,128,140,
227,173,159,136,185,152,42,218,149,134,156,100,175,161,52,158,55,239,122,125,100,178,62,60,79,217,250,94,22,232,203,237,221,37,146,35,120,210,50,204,108,58,142,249,159,85,78,99,249,198,32,51,156,228,159,
195,191,1,88,164,43,22,234,96,174,139,173,184,150,184,157,109,170,120,236,154,187,109,123,76,95,226,247,152,54,238,66,76,254,3,189,72,60,250,162,136,174,242,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_directional_loudnessAudioProcessorEditor::drag_on_png = (const char*) resource_Ambix_directional_loudnessAudioProcessorEditor_drag_on_png;
const int Ambix_directional_loudnessAudioProcessorEditor::drag_on_pngSize = 1172;


//[EndFile] You can add extra defines here...
//[/EndFile]
