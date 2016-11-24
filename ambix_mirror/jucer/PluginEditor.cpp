/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  11 Jun 2013 11:33:03am

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
#include "PluginProcessor.h"
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Ambix_mirrorAudioProcessorEditor::Ambix_mirrorAudioProcessorEditor (Ambix_mirrorAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      box_presets (nullptr),
      label5 (nullptr),
      sld_x_even (nullptr),
      label (nullptr),
      label2 (nullptr),
      tgl_x_even_inv (nullptr),
      sld_x_odd (nullptr),
      label3 (nullptr),
      tgl_x_odd_inv (nullptr),
      sld_y_even (nullptr),
      label6 (nullptr),
      tgl_y_even_inv (nullptr),
      sld_y_odd (nullptr),
      label7 (nullptr),
      tgl_y_odd_inv (nullptr),
      sld_z_even (nullptr),
      label9 (nullptr),
      tgl_z_even_inv (nullptr),
      sld_z_odd (nullptr),
      label10 (nullptr),
      tgl_z_odd_inv (nullptr),
      lbl_x_even (nullptr),
      lbl_x_odd (nullptr),
      lbl_y_even (nullptr),
      lbl_y_odd (nullptr),
      lbl_z_even (nullptr),
      lbl_z_odd (nullptr),
      label4 (nullptr),
      label8 (nullptr),
      sld_circular (nullptr),
      label11 (nullptr),
      tgl_circular_inv (nullptr),
      lbl_circular (nullptr),
      cachedImage_coordinate_system_png (nullptr)
{
    addAndMakeVisible (box_presets = new ComboBox ("new combo box"));
    box_presets->setTooltip ("choose conversion scheme from several presets");
    box_presets->setEditableText (false);
    box_presets->setJustificationType (Justification::centredLeft);
    box_presets->setTextWhenNothingSelected ("no change");
    box_presets->setTextWhenNoChoicesAvailable ("(no presets)");
    box_presets->addItem ("no change", 1);
    box_presets->addItem ("flip left <> right", 2);
    box_presets->addItem ("flop front <> back", 3);
    box_presets->addItem ("flap top <> bottom", 4);
    box_presets->addItem ("merge left + right", 5);
    box_presets->addItem ("merge front + back", 6);
    box_presets->addItem ("merge top + bottom", 7);
    box_presets->addListener (this);

    addAndMakeVisible (label5 = new Label ("new label",
                                           "Presets"));
    label5->setFont (Font (15.0000f, Font::plain));
    label5->setJustificationType (Justification::centredRight);
    label5->setEditable (false, false, false);
    label5->setColour (Label::textColourId, Colours::azure);
    label5->setColour (TextEditor::textColourId, Colours::black);
    label5->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (sld_x_even = new Slider ("new slider"));
    sld_x_even->setRange (0, 1, 0);
    sld_x_even->setSliderStyle (Slider::LinearHorizontal);
    sld_x_even->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_x_even->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_x_even->addListener (this);

    addAndMakeVisible (label = new Label ("new label",
                                          "x axis symmetries"));
    label->setFont (Font (15.0000f, Font::italic));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colour (0x66ffffff));
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label2 = new Label ("new label",
                                           "X even"));
    label2->setFont (Font (15.0000f, Font::plain));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (Label::textColourId, Colours::white);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_x_even_inv = new ToggleButton ("new toggle button"));
    tgl_x_even_inv->setButtonText ("invert");
    tgl_x_even_inv->addListener (this);
    tgl_x_even_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (sld_x_odd = new Slider ("new slider"));
    sld_x_odd->setRange (0, 1, 0);
    sld_x_odd->setSliderStyle (Slider::LinearHorizontal);
    sld_x_odd->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_x_odd->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_x_odd->addListener (this);

    addAndMakeVisible (label3 = new Label ("new label",
                                           "X odd"));
    label3->setFont (Font (15.0000f, Font::plain));
    label3->setJustificationType (Justification::centredLeft);
    label3->setEditable (false, false, false);
    label3->setColour (Label::textColourId, Colours::white);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_x_odd_inv = new ToggleButton ("new toggle button"));
    tgl_x_odd_inv->setButtonText ("invert");
    tgl_x_odd_inv->addListener (this);
    tgl_x_odd_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (sld_y_even = new Slider ("new slider"));
    sld_y_even->setRange (0, 1, 0);
    sld_y_even->setSliderStyle (Slider::LinearHorizontal);
    sld_y_even->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_y_even->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_y_even->addListener (this);

    addAndMakeVisible (label6 = new Label ("new label",
                                           "Y even"));
    label6->setFont (Font (15.0000f, Font::plain));
    label6->setJustificationType (Justification::centredLeft);
    label6->setEditable (false, false, false);
    label6->setColour (Label::textColourId, Colours::white);
    label6->setColour (TextEditor::textColourId, Colours::black);
    label6->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_y_even_inv = new ToggleButton ("new toggle button"));
    tgl_y_even_inv->setButtonText ("invert");
    tgl_y_even_inv->addListener (this);
    tgl_y_even_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (sld_y_odd = new Slider ("new slider"));
    sld_y_odd->setRange (0, 1, 0);
    sld_y_odd->setSliderStyle (Slider::LinearHorizontal);
    sld_y_odd->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_y_odd->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_y_odd->addListener (this);

    addAndMakeVisible (label7 = new Label ("new label",
                                           "Y odd"));
    label7->setFont (Font (15.0000f, Font::plain));
    label7->setJustificationType (Justification::centredLeft);
    label7->setEditable (false, false, false);
    label7->setColour (Label::textColourId, Colours::white);
    label7->setColour (TextEditor::textColourId, Colours::black);
    label7->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_y_odd_inv = new ToggleButton ("new toggle button"));
    tgl_y_odd_inv->setButtonText ("invert");
    tgl_y_odd_inv->addListener (this);
    tgl_y_odd_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (sld_z_even = new Slider ("new slider"));
    sld_z_even->setRange (0, 1, 0);
    sld_z_even->setSliderStyle (Slider::LinearHorizontal);
    sld_z_even->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_z_even->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_z_even->addListener (this);

    addAndMakeVisible (label9 = new Label ("new label",
                                           "Z even"));
    label9->setFont (Font (15.0000f, Font::plain));
    label9->setJustificationType (Justification::centredLeft);
    label9->setEditable (false, false, false);
    label9->setColour (Label::textColourId, Colours::white);
    label9->setColour (TextEditor::textColourId, Colours::black);
    label9->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_z_even_inv = new ToggleButton ("new toggle button"));
    tgl_z_even_inv->setButtonText ("invert");
    tgl_z_even_inv->addListener (this);
    tgl_z_even_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (sld_z_odd = new Slider ("new slider"));
    sld_z_odd->setRange (0, 1, 0);
    sld_z_odd->setSliderStyle (Slider::LinearHorizontal);
    sld_z_odd->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_z_odd->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_z_odd->addListener (this);

    addAndMakeVisible (label10 = new Label ("new label",
                                            "Z odd"));
    label10->setFont (Font (15.0000f, Font::plain));
    label10->setJustificationType (Justification::centredLeft);
    label10->setEditable (false, false, false);
    label10->setColour (Label::textColourId, Colours::white);
    label10->setColour (TextEditor::textColourId, Colours::black);
    label10->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_z_odd_inv = new ToggleButton ("new toggle button"));
    tgl_z_odd_inv->setButtonText ("invert");
    tgl_z_odd_inv->addListener (this);
    tgl_z_odd_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (lbl_x_even = new Label ("new label",
                                               "-99 dB"));
    lbl_x_even->setFont (Font (15.0000f, Font::plain));
    lbl_x_even->setJustificationType (Justification::centredRight);
    lbl_x_even->setEditable (false, false, false);
    lbl_x_even->setColour (Label::textColourId, Colours::yellow);
    lbl_x_even->setColour (TextEditor::textColourId, Colours::black);
    lbl_x_even->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (lbl_x_odd = new Label ("new label",
                                              "-99 dB"));
    lbl_x_odd->setFont (Font (15.0000f, Font::plain));
    lbl_x_odd->setJustificationType (Justification::centredRight);
    lbl_x_odd->setEditable (false, false, false);
    lbl_x_odd->setColour (Label::textColourId, Colours::yellow);
    lbl_x_odd->setColour (TextEditor::textColourId, Colours::black);
    lbl_x_odd->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (lbl_y_even = new Label ("new label",
                                               "-99 dB"));
    lbl_y_even->setFont (Font (15.0000f, Font::plain));
    lbl_y_even->setJustificationType (Justification::centredRight);
    lbl_y_even->setEditable (false, false, false);
    lbl_y_even->setColour (Label::textColourId, Colours::yellow);
    lbl_y_even->setColour (TextEditor::textColourId, Colours::black);
    lbl_y_even->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (lbl_y_odd = new Label ("new label",
                                              "-99 dB"));
    lbl_y_odd->setFont (Font (15.0000f, Font::plain));
    lbl_y_odd->setJustificationType (Justification::centredRight);
    lbl_y_odd->setEditable (false, false, false);
    lbl_y_odd->setColour (Label::textColourId, Colours::yellow);
    lbl_y_odd->setColour (TextEditor::textColourId, Colours::black);
    lbl_y_odd->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (lbl_z_even = new Label ("new label",
                                               "-99 dB"));
    lbl_z_even->setFont (Font (15.0000f, Font::plain));
    lbl_z_even->setJustificationType (Justification::centredRight);
    lbl_z_even->setEditable (false, false, false);
    lbl_z_even->setColour (Label::textColourId, Colours::yellow);
    lbl_z_even->setColour (TextEditor::textColourId, Colours::black);
    lbl_z_even->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (lbl_z_odd = new Label ("new label",
                                              "-99 dB"));
    lbl_z_odd->setFont (Font (15.0000f, Font::plain));
    lbl_z_odd->setJustificationType (Justification::centredRight);
    lbl_z_odd->setEditable (false, false, false);
    lbl_z_odd->setColour (Label::textColourId, Colours::yellow);
    lbl_z_odd->setColour (TextEditor::textColourId, Colours::black);
    lbl_z_odd->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label4 = new Label ("new label",
                                           "y axis symmetries"));
    label4->setFont (Font (15.0000f, Font::italic));
    label4->setJustificationType (Justification::centred);
    label4->setEditable (false, false, false);
    label4->setColour (Label::textColourId, Colour (0x66ffffff));
    label4->setColour (TextEditor::textColourId, Colours::black);
    label4->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (label8 = new Label ("new label",
                                           "z axis symmetries"));
    label8->setFont (Font (15.0000f, Font::italic));
    label8->setJustificationType (Justification::centred);
    label8->setEditable (false, false, false);
    label8->setColour (Label::textColourId, Colour (0x66ffffff));
    label8->setColour (TextEditor::textColourId, Colours::black);
    label8->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (sld_circular = new Slider ("new slider"));
    sld_circular->setRange (0, 1, 0);
    sld_circular->setSliderStyle (Slider::LinearHorizontal);
    sld_circular->setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sld_circular->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_circular->addListener (this);

    addAndMakeVisible (label11 = new Label ("new label",
                                            "Circular"));
    label11->setFont (Font (15.0000f, Font::plain));
    label11->setJustificationType (Justification::centredLeft);
    label11->setEditable (false, false, false);
    label11->setColour (Label::textColourId, Colours::white);
    label11->setColour (TextEditor::textColourId, Colours::black);
    label11->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (tgl_circular_inv = new ToggleButton ("new toggle button"));
    tgl_circular_inv->setButtonText ("invert");
    tgl_circular_inv->addListener (this);
    tgl_circular_inv->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (lbl_circular = new Label ("new label",
                                                 "-99 dB"));
    lbl_circular->setFont (Font (15.0000f, Font::plain));
    lbl_circular->setJustificationType (Justification::centredRight);
    lbl_circular->setEditable (false, false, false);
    lbl_circular->setColour (Label::textColourId, Colours::yellow);
    lbl_circular->setColour (TextEditor::textColourId, Colours::black);
    lbl_circular->setColour (TextEditor::backgroundColourId, Colour (0x0));

    cachedImage_coordinate_system_png = ImageCache::getFromMemory (coordinate_system_png, coordinate_system_pngSize);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (410, 410);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

Ambix_mirrorAudioProcessorEditor::~Ambix_mirrorAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (box_presets);
    deleteAndZero (label5);
    deleteAndZero (sld_x_even);
    deleteAndZero (label);
    deleteAndZero (label2);
    deleteAndZero (tgl_x_even_inv);
    deleteAndZero (sld_x_odd);
    deleteAndZero (label3);
    deleteAndZero (tgl_x_odd_inv);
    deleteAndZero (sld_y_even);
    deleteAndZero (label6);
    deleteAndZero (tgl_y_even_inv);
    deleteAndZero (sld_y_odd);
    deleteAndZero (label7);
    deleteAndZero (tgl_y_odd_inv);
    deleteAndZero (sld_z_even);
    deleteAndZero (label9);
    deleteAndZero (tgl_z_even_inv);
    deleteAndZero (sld_z_odd);
    deleteAndZero (label10);
    deleteAndZero (tgl_z_odd_inv);
    deleteAndZero (lbl_x_even);
    deleteAndZero (lbl_x_odd);
    deleteAndZero (lbl_y_even);
    deleteAndZero (lbl_y_odd);
    deleteAndZero (lbl_z_even);
    deleteAndZero (lbl_z_odd);
    deleteAndZero (label4);
    deleteAndZero (label8);
    deleteAndZero (sld_circular);
    deleteAndZero (label11);
    deleteAndZero (tgl_circular_inv);
    deleteAndZero (lbl_circular);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Ambix_mirrorAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       (float) (proportionOfWidth (0.6400f)), (float) (proportionOfHeight (0.6933f)),
                                       Colours::black,
                                       (float) (proportionOfWidth (0.1143f)), (float) (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 410, 410);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 410, 410, 1);

    g.setColour (Colour (0x95808080));
    g.fillRoundedRectangle (8.0f, 100.0f, 392.0f, 76.0f, 10.0000f);

    g.setColour (Colours::azure);
    g.setFont (Font (17.2000f, Font::bold));
    g.drawText ("AMBIX-MIRROR",
                18, 2, 380, 30,
                Justification::centred, true);

    g.setColour (Colours::azure);
    g.setFont (Font (12.4000f, Font::plain));
    g.drawText ("adjusts symmetric components in Ambisonics streams",
                -27, 25, 380, 30,
                Justification::centred, true);

    g.setColour (Colour (0x932b1d69));
    g.fillRoundedRectangle (11.0f, 188.0f, 392.0f, 76.0f, 10.0000f);

    g.setColour (Colour (0x84ff1a30));
    g.fillRoundedRectangle (11.0f, 272.0f, 392.0f, 76.0f, 10.0000f);

    g.setColour (Colour (0x84fffefe));
    g.fillRoundedRectangle (11.0f, 360.0f, 392.0f, 38.0f, 10.0000f);

    g.setColour (Colours::black);
    g.drawImageWithin (cachedImage_coordinate_system_png,
                       312, 5, 91, 89,
                       RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                       false);

    
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

void Ambix_mirrorAudioProcessorEditor::resized()
{
    box_presets->setBounds (64, 64, 232, 24);
    label5->setBounds (8, 64, 56, 24);
    sld_x_even->setBounds (64, 104, 199, 24);
    label->setBounds (120, 128, 150, 16);
    label2->setBounds (10, 104, 54, 24);
    tgl_x_even_inv->setBounds (336, 104, 64, 24);
    sld_x_odd->setBounds (64, 144, 199, 24);
    label3->setBounds (10, 144, 54, 24);
    tgl_x_odd_inv->setBounds (336, 144, 64, 24);
    sld_y_even->setBounds (64, 192, 199, 24);
    label6->setBounds (10, 192, 54, 24);
    tgl_y_even_inv->setBounds (336, 192, 64, 24);
    sld_y_odd->setBounds (64, 232, 199, 24);
    label7->setBounds (10, 232, 54, 24);
    tgl_y_odd_inv->setBounds (336, 232, 64, 24);
    sld_z_even->setBounds (64, 280, 199, 24);
    label9->setBounds (10, 280, 54, 24);
    tgl_z_even_inv->setBounds (336, 280, 64, 24);
    sld_z_odd->setBounds (64, 320, 199, 24);
    label10->setBounds (10, 320, 54, 24);
    tgl_z_odd_inv->setBounds (336, 320, 64, 24);
    lbl_x_even->setBounds (264, 104, 64, 24);
    lbl_x_odd->setBounds (264, 144, 64, 24);
    lbl_y_even->setBounds (264, 192, 64, 24);
    lbl_y_odd->setBounds (264, 232, 64, 24);
    lbl_z_even->setBounds (264, 280, 64, 24);
    lbl_z_odd->setBounds (264, 320, 64, 24);
    label4->setBounds (120, 216, 150, 16);
    label8->setBounds (112, 304, 150, 16);
    sld_circular->setBounds (64, 368, 199, 24);
    label11->setBounds (10, 368, 62, 24);
    tgl_circular_inv->setBounds (336, 368, 64, 24);
    lbl_circular->setBounds (264, 368, 64, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Ambix_mirrorAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
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

void Ambix_mirrorAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sld_x_even)
    {
        //[UserSliderCode_sld_x_even] -- add your slider handling code here..
        //[/UserSliderCode_sld_x_even]
    }
    else if (sliderThatWasMoved == sld_x_odd)
    {
        //[UserSliderCode_sld_x_odd] -- add your slider handling code here..
        //[/UserSliderCode_sld_x_odd]
    }
    else if (sliderThatWasMoved == sld_y_even)
    {
        //[UserSliderCode_sld_y_even] -- add your slider handling code here..
        //[/UserSliderCode_sld_y_even]
    }
    else if (sliderThatWasMoved == sld_y_odd)
    {
        //[UserSliderCode_sld_y_odd] -- add your slider handling code here..
        //[/UserSliderCode_sld_y_odd]
    }
    else if (sliderThatWasMoved == sld_z_even)
    {
        //[UserSliderCode_sld_z_even] -- add your slider handling code here..
        //[/UserSliderCode_sld_z_even]
    }
    else if (sliderThatWasMoved == sld_z_odd)
    {
        //[UserSliderCode_sld_z_odd] -- add your slider handling code here..
        //[/UserSliderCode_sld_z_odd]
    }
    else if (sliderThatWasMoved == sld_circular)
    {
        //[UserSliderCode_sld_circular] -- add your slider handling code here..
        //[/UserSliderCode_sld_circular]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void Ambix_mirrorAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == tgl_x_even_inv)
    {
        //[UserButtonCode_tgl_x_even_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_x_even_inv]
    }
    else if (buttonThatWasClicked == tgl_x_odd_inv)
    {
        //[UserButtonCode_tgl_x_odd_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_x_odd_inv]
    }
    else if (buttonThatWasClicked == tgl_y_even_inv)
    {
        //[UserButtonCode_tgl_y_even_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_y_even_inv]
    }
    else if (buttonThatWasClicked == tgl_y_odd_inv)
    {
        //[UserButtonCode_tgl_y_odd_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_y_odd_inv]
    }
    else if (buttonThatWasClicked == tgl_z_even_inv)
    {
        //[UserButtonCode_tgl_z_even_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_z_even_inv]
    }
    else if (buttonThatWasClicked == tgl_z_odd_inv)
    {
        //[UserButtonCode_tgl_z_odd_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_z_odd_inv]
    }
    else if (buttonThatWasClicked == tgl_circular_inv)
    {
        //[UserButtonCode_tgl_circular_inv] -- add your button handler code here..
        //[/UserButtonCode_tgl_circular_inv]
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

<JUCER_COMPONENT documentType="Component" className="Ambix_mirrorAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="Ambix_mirrorAudioProcessor* ownerFilter" variableInitialisers="AudioProcessorEditor (ownerFilter)"
                 snapPixels="8" snapActive="0" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="410" initialHeight="410">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 410 410" fill=" radial: 64% 69.333%, 11.429% 8%, 0=ff4e4e4e, 1=ff000000"
          hasStroke="1" stroke="1, mitered, butt" strokeColour="solid: ff000000"/>
    <ROUNDRECT pos="8 100 392 76" cornerSize="10" fill="solid: 95808080" hasStroke="0"/>
    <TEXT pos="18 2 380 30" fill="solid: fff0ffff" hasStroke="0" text="AMBIX-MIRROR"
          fontname="Default font" fontsize="17.2" bold="1" italic="0" justification="36"/>
    <TEXT pos="-27 25 380 30" fill="solid: fff0ffff" hasStroke="0" text="adjusts symmetric components in Ambisonics streams"
          fontname="Default font" fontsize="12.4" bold="0" italic="0" justification="36"/>
    <ROUNDRECT pos="11 188 392 76" cornerSize="10" fill="solid: 932b1d69" hasStroke="0"/>
    <ROUNDRECT pos="11 272 392 76" cornerSize="10" fill="solid: 84ff1a30" hasStroke="0"/>
    <ROUNDRECT pos="11 360 392 38" cornerSize="10" fill="solid: 84fffefe" hasStroke="0"/>
    <IMAGE pos="312 5 91 89" resource="coordinate_system_png" opacity="1"
           mode="2"/>
  </BACKGROUND>
  <COMBOBOX name="new combo box" id="d106da7f451db4f8" memberName="box_presets"
            virtualName="" explicitFocusOrder="0" pos="64 64 232 24" tooltip="choose conversion scheme from several presets"
            editable="0" layout="33" items="no change&#10;flip left &lt;&gt; right&#10;flop front &lt;&gt; back&#10;flap top &lt;&gt; bottom&#10;merge left + right&#10;merge front + back&#10;merge top + bottom"
            textWhenNonSelected="no change" textWhenNoItems="(no presets)"/>
  <LABEL name="new label" id="2071080ca9cd2158" memberName="label5" virtualName=""
         explicitFocusOrder="0" pos="8 64 56 24" textCol="fff0ffff" edTextCol="ff000000"
         edBkgCol="0" labelText="Presets" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <SLIDER name="new slider" id="6a7f4df769cb226c" memberName="sld_x_even"
          virtualName="" explicitFocusOrder="0" pos="64 104 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="d7252feac27a4b67" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="120 128 150 16" textCol="66ffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="x axis symmetries"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="1" justification="36"/>
  <LABEL name="new label" id="6a4664ad5ca45ea7" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="10 104 54 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="X even" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="9ddbe77a5950b0f3" memberName="tgl_x_even_inv"
                virtualName="" explicitFocusOrder="0" pos="336 104 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <SLIDER name="new slider" id="bf057070f712bac9" memberName="sld_x_odd"
          virtualName="" explicitFocusOrder="0" pos="64 144 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="aac6b2a05f4e6a91" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="10 144 54 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="X odd" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="9e11996776bd1af0" memberName="tgl_x_odd_inv"
                virtualName="" explicitFocusOrder="0" pos="336 144 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <SLIDER name="new slider" id="d8801ebaaaec7bbc" memberName="sld_y_even"
          virtualName="" explicitFocusOrder="0" pos="64 192 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="8ebd37954ba22bc2" memberName="label6" virtualName=""
         explicitFocusOrder="0" pos="10 192 54 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Y even" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="d70398f0fc7044d0" memberName="tgl_y_even_inv"
                virtualName="" explicitFocusOrder="0" pos="336 192 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <SLIDER name="new slider" id="174bed208f6e461f" memberName="sld_y_odd"
          virtualName="" explicitFocusOrder="0" pos="64 232 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="40ba7236494ca55f" memberName="label7" virtualName=""
         explicitFocusOrder="0" pos="10 232 54 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Y odd" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="93ebc5924ec81dfc" memberName="tgl_y_odd_inv"
                virtualName="" explicitFocusOrder="0" pos="336 232 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <SLIDER name="new slider" id="34570a72b7d551b6" memberName="sld_z_even"
          virtualName="" explicitFocusOrder="0" pos="64 280 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="5088f43b501c44b" memberName="label9" virtualName=""
         explicitFocusOrder="0" pos="10 280 54 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Z even" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="f192c3674f0348a0" memberName="tgl_z_even_inv"
                virtualName="" explicitFocusOrder="0" pos="336 280 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <SLIDER name="new slider" id="c79e8b717463f079" memberName="sld_z_odd"
          virtualName="" explicitFocusOrder="0" pos="64 320 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="6aaab5fd1e845595" memberName="label10" virtualName=""
         explicitFocusOrder="0" pos="10 320 54 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Z odd" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="9f951ad3bbab11ce" memberName="tgl_z_odd_inv"
                virtualName="" explicitFocusOrder="0" pos="336 320 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <LABEL name="new label" id="877d9a99df99a0f9" memberName="lbl_x_even"
         virtualName="" explicitFocusOrder="0" pos="264 104 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="8a9e37c765dd06a2" memberName="lbl_x_odd"
         virtualName="" explicitFocusOrder="0" pos="264 144 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="f168057b0c6d7bd" memberName="lbl_y_even"
         virtualName="" explicitFocusOrder="0" pos="264 192 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="18b42f3fda9f229f" memberName="lbl_y_odd"
         virtualName="" explicitFocusOrder="0" pos="264 232 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="78e1ab2ed3535ef8" memberName="lbl_z_even"
         virtualName="" explicitFocusOrder="0" pos="264 280 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="335d020086febb86" memberName="lbl_z_odd"
         virtualName="" explicitFocusOrder="0" pos="264 320 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="ca9727202dec9caa" memberName="label4" virtualName=""
         explicitFocusOrder="0" pos="120 216 150 16" textCol="66ffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="y axis symmetries"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="1" justification="36"/>
  <LABEL name="new label" id="36697fe551f38270" memberName="label8" virtualName=""
         explicitFocusOrder="0" pos="112 304 150 16" textCol="66ffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="z axis symmetries"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="1" justification="36"/>
  <SLIDER name="new slider" id="7621651aa91962b3" memberName="sld_circular"
          virtualName="" explicitFocusOrder="0" pos="64 368 199 24" thumbcol="ff2b1d69"
          min="0" max="1" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="0" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <LABEL name="new label" id="4c6d88de7c1461a8" memberName="label11" virtualName=""
         explicitFocusOrder="0" pos="10 368 62 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Circular" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="c53ff9a65af41d40" memberName="tgl_circular_inv"
                virtualName="" explicitFocusOrder="0" pos="336 368 64 24" txtcol="ffffffff"
                buttonText="invert" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <LABEL name="new label" id="47ba2858da86bddc" memberName="lbl_circular"
         virtualName="" explicitFocusOrder="0" pos="264 368 64 24" textCol="ffffff00"
         edTextCol="ff000000" edBkgCol="0" labelText="-99 dB" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: coordinate_system_png, 6120, "../Graphics/coordinate_system.png"
static const unsigned char resource_Ambix_mirrorAudioProcessorEditor_coordinate_system_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,91,0,0,0,89,8,6,0,0,0,81,254,88,146,0,0,4,36,105,67,
67,80,73,67,67,32,80,114,111,102,105,108,101,0,0,56,17,133,85,223,111,219,84,20,62,137,111,82,164,22,63,32,88,71,135,138,197,175,85,83,91,185,27,26,173,198,6,73,147,165,237,74,22,165,233,216,42,36,228,
58,55,137,169,27,7,219,233,182,170,79,123,129,55,6,252,1,64,217,3,15,72,60,33,13,6,98,123,217,246,192,180,73,83,135,42,170,73,72,123,232,196,15,33,38,237,5,85,225,187,118,98,39,83,196,92,245,250,203,57,
223,57,231,59,231,94,219,68,61,95,105,181,154,25,85,136,150,171,174,157,207,36,149,147,167,22,148,158,77,138,210,179,212,75,3,212,171,233,78,45,145,203,205,18,46,193,21,247,206,235,225,29,138,8,203,237,
145,238,254,78,246,35,191,122,139,220,209,137,34,79,192,110,21,29,125,25,248,12,81,204,212,107,182,75,20,191,7,251,248,105,183,6,220,243,28,240,211,54,4,2,171,2,151,125,156,18,120,209,199,39,61,78,33,
63,9,142,208,42,235,21,173,8,188,6,60,188,216,102,47,183,97,95,3,24,200,147,225,85,110,27,186,34,102,145,179,173,146,97,114,207,225,47,143,113,183,49,255,15,46,155,117,244,236,93,131,88,251,156,165,185,
99,184,15,137,222,43,246,84,190,137,63,215,181,244,28,240,75,176,95,175,185,73,97,127,5,248,143,250,210,124,2,120,31,81,244,169,146,125,116,222,231,71,95,95,173,20,222,6,222,5,123,209,112,167,11,77,251,
106,117,49,123,28,24,177,209,245,37,235,152,200,35,56,215,117,103,18,179,164,23,128,239,86,248,180,216,99,232,145,168,200,83,105,224,97,224,193,74,125,170,153,95,154,113,86,230,132,221,203,179,90,153,
204,2,35,143,100,191,167,205,228,128,7,128,63,180,173,188,168,5,205,210,58,55,51,162,22,242,75,87,107,110,174,169,65,218,174,154,89,81,11,253,50,153,59,94,143,200,41,109,187,149,194,148,31,203,14,186,
118,161,25,203,22,74,198,209,233,38,127,173,102,122,103,17,218,216,5,187,158,23,218,144,159,221,210,236,116,6,24,121,216,63,188,58,47,230,6,28,219,93,212,82,98,182,163,192,71,232,68,68,35,78,22,45,98,
213,169,74,59,164,80,158,50,148,196,189,70,54,60,37,50,200,132,133,195,203,97,49,34,79,210,18,108,221,121,57,143,227,199,132,140,178,23,189,45,162,81,165,59,199,175,112,191,201,177,88,63,83,217,1,252,
31,98,179,236,48,27,103,19,164,176,55,216,155,236,8,75,193,58,193,14,5,10,114,109,58,21,42,211,125,40,245,243,188,79,117,84,22,58,78,80,242,252,64,125,40,136,81,206,217,239,26,250,205,143,254,134,154,
176,75,43,244,35,79,208,49,52,91,237,19,32,7,19,104,117,55,210,62,167,107,107,63,236,14,60,202,6,187,248,206,237,190,107,107,116,252,113,243,141,221,139,109,199,54,176,110,198,182,194,248,216,175,177,
45,252,109,82,2,59,96,122,138,150,161,202,240,118,194,9,120,35,29,61,92,1,211,37,13,235,111,224,89,208,218,82,218,177,163,165,234,249,129,208,35,38,193,63,200,62,204,210,185,225,208,170,254,162,254,169,
110,168,95,168,23,212,223,59,106,132,25,59,166,36,125,42,125,43,253,40,125,39,125,47,253,76,138,116,89,186,34,253,36,93,149,190,145,46,5,57,187,239,189,159,37,216,123,175,95,97,19,221,138,93,104,213,107,
159,53,39,83,78,202,123,228,23,229,148,252,188,252,178,60,27,176,20,185,95,30,147,167,228,189,240,236,9,246,205,12,253,29,189,24,116,10,179,106,77,181,123,45,241,4,24,52,15,37,6,157,6,215,198,84,197,132,
171,116,22,89,219,159,147,166,82,54,200,198,216,244,35,167,118,92,156,229,150,138,120,58,158,138,39,72,137,239,139,79,196,199,226,51,2,183,158,188,248,94,248,38,176,166,3,245,254,147,19,48,58,58,224,109,
44,76,37,200,51,226,157,58,113,86,69,244,10,116,155,208,205,93,126,6,223,14,162,73,171,118,214,54,202,21,87,217,175,170,175,41,9,124,202,184,50,93,213,71,135,21,205,52,21,207,229,40,54,119,184,189,194,
139,163,36,190,131,34,142,232,65,222,251,190,69,118,221,12,109,238,91,68,135,255,194,59,235,86,104,91,168,19,125,237,16,245,191,26,218,134,240,78,124,230,51,162,139,7,245,186,189,226,231,163,72,228,6,
145,83,58,176,223,251,29,233,75,226,221,116,183,209,120,128,247,85,207,39,68,59,31,55,26,255,174,55,26,59,95,34,255,22,209,101,243,63,89,0,113,120,108,27,43,19,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,
1,0,154,156,24,0,0,19,106,73,68,65,84,120,1,237,157,121,116,92,213,125,199,191,247,109,179,105,70,146,101,201,174,204,98,27,98,22,67,32,134,20,47,49,198,44,54,32,115,104,32,144,52,33,132,83,32,33,205,
57,61,77,8,41,105,75,255,41,161,228,20,154,115,90,18,154,147,22,10,9,24,142,9,14,198,53,24,92,179,217,46,139,56,174,113,12,198,24,188,225,93,150,165,209,104,214,247,222,237,247,55,146,140,52,122,146,23,
45,72,246,92,159,241,204,188,229,222,247,62,191,223,253,221,223,239,119,239,27,41,173,53,202,101,120,8,24,195,211,76,185,21,33,112,212,176,85,103,41,227,59,122,2,86,247,83,148,250,222,15,129,159,222,6,
44,91,6,100,92,32,205,151,75,59,19,53,129,154,40,48,243,34,192,159,33,231,40,53,115,154,214,107,214,118,63,191,252,185,127,2,61,96,3,251,182,3,91,55,1,55,125,7,136,215,126,118,170,246,129,109,235,129,
137,231,117,108,91,180,16,216,177,241,179,253,229,79,71,68,64,6,200,160,23,79,86,61,95,175,190,78,173,166,150,255,228,142,160,227,203,219,130,57,118,231,18,8,186,199,1,69,232,43,87,116,128,190,249,107,
221,247,149,63,31,30,112,119,70,253,194,166,102,211,86,175,120,185,3,244,61,63,234,126,98,249,243,209,129,22,94,125,194,38,104,122,42,107,86,117,128,190,247,238,50,220,163,135,91,202,44,16,54,7,194,48,
240,214,234,14,208,223,253,102,175,147,0,14,172,83,198,150,110,47,127,239,95,32,189,96,19,244,120,224,189,198,14,208,191,248,41,80,79,151,79,94,98,82,96,119,104,252,131,247,3,217,118,126,86,101,192,253,
3,238,206,167,8,139,208,138,69,169,83,234,129,77,31,2,161,138,174,109,125,191,191,254,10,112,201,101,172,140,198,168,92,142,132,64,137,159,61,46,14,56,17,224,190,251,128,205,91,129,124,1,168,155,72,133,
166,27,24,163,13,119,248,242,8,87,94,15,223,95,6,125,36,136,63,59,166,135,102,203,102,137,198,203,16,63,3,52,152,159,168,169,61,139,128,38,111,67,169,139,166,40,245,133,110,81,100,207,227,202,223,142,
158,64,15,51,34,144,129,249,231,3,79,126,29,248,242,28,96,249,83,220,246,16,249,51,71,82,46,3,37,208,3,54,65,79,6,238,249,9,141,9,161,231,211,192,120,14,152,39,137,7,82,134,61,80,210,60,191,4,246,238,
36,208,248,10,7,66,7,136,112,176,108,222,3,132,5,118,102,16,218,58,225,171,40,177,217,123,233,59,103,82,244,62,8,187,121,63,80,145,160,102,243,85,46,131,65,160,20,118,14,104,106,162,213,160,203,23,34,
240,108,150,26,30,26,140,134,202,117,244,50,35,160,255,172,104,90,12,70,139,109,212,240,2,97,219,22,7,201,178,59,56,8,218,82,98,179,139,53,82,219,199,50,100,79,140,1,90,169,229,162,225,229,50,24,4,74,
204,136,84,233,209,132,184,157,222,135,132,226,102,144,64,6,163,237,19,174,142,82,216,156,254,106,227,32,233,123,124,17,120,158,102,196,17,111,68,102,109,202,101,128,4,2,180,54,151,167,54,83,8,22,205,
135,162,237,62,62,52,91,198,29,97,85,154,138,80,39,171,8,124,102,47,119,106,198,21,67,91,74,53,155,173,181,211,140,104,110,23,237,182,169,213,21,146,94,29,245,154,93,255,154,189,112,98,58,180,75,213,169,
98,70,83,224,215,254,210,126,96,242,142,80,186,238,95,236,7,134,22,115,71,237,1,176,115,12,96,196,11,241,104,70,60,2,183,68,251,71,61,236,236,107,94,163,17,193,248,234,31,91,119,147,179,93,191,202,121,
49,241,3,243,206,194,86,189,174,237,23,133,39,135,3,118,64,214,111,214,169,192,237,223,160,173,230,108,77,203,1,96,203,22,224,129,151,216,253,168,241,163,183,16,176,121,234,94,231,125,21,87,137,252,255,
249,31,133,103,24,179,115,107,253,53,59,175,43,92,166,183,104,42,215,208,151,0,205,46,112,144,12,209,116,68,59,35,71,177,223,83,71,189,102,83,89,188,230,191,246,254,70,180,91,64,167,95,240,126,191,115,
90,97,246,112,129,22,81,6,192,54,232,238,217,140,26,35,156,68,168,102,138,53,94,180,113,67,47,247,161,111,193,58,77,79,232,106,37,249,128,186,143,2,160,98,13,95,9,128,45,46,159,132,235,50,27,35,131,100,
199,40,62,124,151,52,240,150,104,50,172,216,52,85,175,190,160,14,165,26,156,11,213,180,234,127,180,31,42,108,211,239,177,133,252,216,199,212,227,106,146,162,169,28,190,18,0,59,77,215,47,197,236,95,142,
174,80,129,185,18,139,238,95,123,192,113,195,119,145,71,211,146,128,174,250,145,121,245,216,165,206,210,250,71,236,71,195,179,213,20,85,171,226,245,203,156,133,180,133,238,142,171,242,87,54,221,225,222,
106,157,164,166,214,221,111,255,250,104,234,30,232,177,65,126,182,4,51,244,72,52,181,90,114,36,22,221,63,119,196,219,108,66,86,56,9,225,216,13,56,61,116,169,49,215,136,171,49,206,12,204,27,247,140,115,
142,223,140,237,102,157,154,178,247,54,247,70,253,190,222,205,67,159,136,204,55,26,42,190,110,220,92,247,59,167,176,255,219,133,239,14,135,73,9,208,216,54,49,29,20,130,197,46,40,176,139,62,247,136,134,
45,160,43,190,136,218,202,219,205,171,237,179,204,201,249,55,252,21,238,14,189,17,5,149,211,237,248,216,57,75,53,36,31,115,31,110,251,15,119,145,104,39,193,234,189,215,21,190,149,107,244,87,199,111,50,
110,61,229,19,231,221,49,255,108,255,140,213,72,180,60,100,37,64,179,109,14,26,18,204,132,233,145,180,211,156,200,128,41,175,145,89,8,200,8,205,199,228,208,117,246,124,35,166,19,126,74,181,102,87,21,86,
235,167,241,48,135,63,219,153,164,102,23,222,87,139,154,110,241,126,128,239,124,118,15,162,201,60,119,118,244,171,102,67,205,189,198,221,102,21,232,16,128,138,54,116,37,0,182,79,45,22,141,54,69,171,9,
89,60,19,217,54,178,10,65,41,76,228,2,151,239,91,23,69,102,97,134,17,83,9,191,93,39,141,10,157,136,204,180,103,228,63,214,91,146,15,186,203,162,215,152,133,244,7,238,139,162,205,165,119,208,185,109,41,
183,47,197,217,252,255,246,210,35,6,247,123,0,108,105,64,162,71,89,30,28,34,104,241,70,188,17,5,91,64,199,207,196,24,231,47,204,6,107,138,113,22,179,240,12,185,252,130,114,224,248,251,212,174,236,203,
133,53,169,205,216,76,152,18,136,61,55,184,200,142,189,182,0,216,98,50,242,244,72,68,19,10,132,94,224,5,87,5,216,246,99,111,116,32,103,146,179,25,187,22,103,135,111,176,22,152,227,213,73,200,234,12,180,
162,233,51,224,109,215,59,218,127,95,88,158,121,29,159,6,105,242,64,218,29,140,115,3,96,135,121,225,146,207,150,229,11,226,254,21,125,238,207,93,179,69,155,113,58,156,170,191,180,102,134,175,86,13,204,
213,197,168,14,190,182,96,170,118,63,157,223,164,63,116,23,123,47,102,222,70,243,72,4,45,194,10,128,189,153,176,37,25,37,233,85,41,50,153,240,249,218,236,34,232,169,168,174,186,217,108,8,127,73,93,100,
68,9,186,128,172,193,212,168,151,85,217,212,75,122,69,234,121,175,17,187,144,25,169,160,133,100,0,108,102,119,209,206,196,76,136,209,149,36,163,34,12,215,35,157,224,229,148,225,45,98,54,66,87,96,114,252,
207,237,5,214,4,76,162,83,106,139,141,230,63,163,176,91,111,77,47,42,44,109,127,14,239,19,242,144,122,18,131,113,215,65,176,89,175,71,224,82,100,112,148,20,235,240,187,126,69,109,22,111,227,14,107,122,
244,50,204,49,34,204,67,251,28,6,83,42,109,88,58,92,248,8,239,103,126,231,46,105,127,7,123,71,178,54,119,112,236,248,63,8,118,167,139,36,90,45,46,224,240,151,46,179,49,230,70,243,50,103,154,113,33,179,
233,134,206,251,57,101,26,142,110,245,155,211,141,120,183,245,89,119,13,62,229,240,200,50,252,87,120,108,45,6,193,102,77,162,217,226,137,20,129,51,192,41,12,155,55,82,52,27,243,49,41,113,189,221,96,212,
169,63,225,197,116,12,206,134,97,185,205,254,158,236,179,238,242,212,243,216,56,26,204,70,169,72,130,96,83,83,14,54,119,28,24,98,84,21,229,171,82,162,171,33,47,4,109,39,110,193,5,161,203,173,185,102,92,
85,195,245,179,52,205,14,245,57,156,255,196,223,152,91,232,253,247,104,50,27,165,192,130,96,243,152,182,182,14,215,207,163,191,45,118,59,118,40,85,89,90,193,96,124,47,154,13,38,145,42,255,138,110,221,
92,227,178,226,240,71,208,12,171,180,78,249,45,153,181,88,155,124,218,163,247,60,186,204,70,41,155,62,96,183,208,245,43,38,162,184,95,102,217,101,34,97,104,138,128,150,36,146,115,163,57,215,62,27,231,
42,91,59,12,187,91,153,254,50,188,102,213,148,91,90,120,149,102,227,67,154,13,154,181,209,93,122,193,150,1,71,169,57,116,253,36,152,145,100,148,4,54,209,94,199,13,198,109,147,179,21,186,18,167,69,174,
179,175,52,199,169,9,126,206,207,40,91,209,133,211,112,55,234,13,249,69,238,235,169,247,176,95,174,105,48,218,251,188,235,232,3,98,129,128,37,176,137,16,118,140,115,145,177,65,157,209,232,50,27,140,6,
167,219,146,68,170,194,88,191,221,111,99,160,146,243,149,239,103,27,241,78,242,9,154,141,17,30,164,28,173,240,250,128,157,165,173,22,205,22,133,146,233,177,193,91,239,39,160,227,23,160,134,102,227,42,
235,60,53,77,21,84,94,32,211,135,246,116,139,58,208,190,186,208,152,126,18,239,81,153,217,254,241,85,250,128,45,55,154,227,148,88,188,211,229,19,251,61,240,66,206,70,248,114,76,12,93,107,207,179,206,192,
57,204,30,231,64,255,89,252,104,111,135,254,36,243,7,119,101,122,53,246,16,116,103,80,53,240,54,71,82,13,125,192,222,77,216,226,137,72,80,19,161,9,137,13,104,128,44,154,13,38,145,18,55,227,188,208,60,
235,82,163,26,117,2,90,103,84,86,183,233,100,118,189,222,144,92,234,173,194,135,72,29,47,246,57,72,200,125,192,142,112,144,202,210,102,75,80,35,147,7,177,184,0,59,22,16,114,158,152,13,235,107,230,21,161,
115,205,105,114,17,126,218,79,106,197,245,117,7,176,63,179,204,93,153,122,14,31,29,143,102,163,20,120,31,176,99,236,198,162,213,226,141,24,52,37,69,215,79,34,185,163,242,10,200,185,56,101,21,190,206,190,
218,56,5,19,105,151,53,135,193,86,100,249,19,61,219,244,150,220,50,119,85,251,91,216,119,44,66,44,189,145,209,240,189,15,216,27,8,91,6,72,121,218,87,0,39,170,248,222,17,54,31,193,93,137,54,99,42,108,154,
141,243,195,11,172,6,179,14,245,186,77,53,139,125,246,219,85,50,247,102,225,237,182,23,176,86,239,96,226,255,4,42,125,192,150,52,107,51,67,118,137,30,77,6,53,149,99,200,68,146,82,135,77,99,22,65,159,129,
138,234,5,230,37,161,139,141,75,24,156,168,46,208,121,106,115,251,114,247,141,220,114,108,161,54,31,182,174,227,77,14,125,193,166,54,167,25,178,75,244,24,142,49,168,225,99,122,19,15,155,140,18,179,17,
157,133,241,145,175,154,87,216,83,141,47,50,91,238,160,77,181,66,251,126,174,81,191,121,112,137,247,166,254,163,238,204,187,28,111,40,15,127,63,129,176,197,134,42,117,87,107,135,205,14,19,116,5,205,72,
190,95,216,4,109,71,191,141,243,162,115,237,217,246,4,76,212,244,159,149,163,45,114,70,242,89,247,57,250,206,235,89,45,61,156,19,183,4,194,238,192,145,97,20,41,69,124,94,153,105,15,158,97,47,154,141,122,
68,42,239,180,102,133,190,140,25,42,172,34,244,210,243,176,117,136,190,243,150,214,39,220,197,185,151,241,49,65,179,158,19,187,244,3,91,166,198,100,80,148,71,171,197,190,246,158,135,20,208,226,214,153,
215,152,23,135,46,82,95,81,174,114,57,8,102,185,6,186,42,183,202,127,181,121,161,183,2,27,71,238,4,236,112,139,190,31,216,178,132,65,60,17,153,248,173,228,210,225,72,143,165,89,228,108,113,110,112,146,
115,173,125,185,125,178,154,84,212,230,136,78,192,55,208,182,216,127,58,181,148,246,121,152,22,153,15,55,180,99,109,175,31,216,73,14,144,121,134,236,9,6,54,98,179,157,67,199,42,46,181,141,223,129,11,34,
211,173,25,198,88,53,78,26,231,74,164,26,127,191,222,211,254,100,225,233,212,98,124,64,179,113,194,121,27,135,19,194,33,128,189,15,108,229,58,191,162,173,150,65,141,185,228,234,112,209,62,159,143,202,
170,155,104,54,46,52,103,80,231,13,101,104,27,9,84,231,215,225,173,236,127,185,207,142,230,153,148,222,12,6,119,75,63,176,219,25,112,136,251,87,195,22,229,177,143,68,60,116,41,38,87,52,152,179,237,51,
140,115,56,12,198,84,20,113,101,42,59,243,130,247,76,243,67,222,11,122,159,230,35,216,229,210,23,129,126,96,203,143,4,164,232,254,69,43,105,70,106,156,63,77,76,173,188,221,57,83,37,116,181,49,70,141,51,
198,232,241,238,86,124,152,124,52,255,72,250,41,172,163,217,16,27,95,46,253,16,232,7,246,134,156,129,79,183,250,144,220,81,184,50,52,121,220,213,206,105,102,28,33,47,100,36,80,147,123,75,175,76,254,202,
253,245,72,93,87,215,207,61,127,110,187,250,129,13,119,22,158,217,88,135,15,222,205,32,170,118,126,156,211,21,107,149,83,72,248,78,250,29,255,183,45,75,253,37,19,54,161,105,51,195,120,218,114,241,161,
169,220,199,199,244,213,80,73,163,215,115,144,93,13,137,107,183,24,53,191,153,132,200,156,131,136,230,254,117,252,244,247,220,134,231,183,143,107,76,174,139,172,83,251,56,98,230,218,160,248,152,2,189,
107,62,157,144,227,106,87,26,108,153,113,200,50,30,207,203,103,62,4,239,210,111,212,140,249,125,74,85,83,48,34,148,162,96,186,218,225,59,221,203,142,50,210,133,117,178,82,17,46,100,153,252,182,214,27,
186,174,185,244,93,184,113,155,23,116,47,253,105,182,247,46,242,171,206,128,49,39,10,211,174,221,147,108,217,188,194,92,104,111,83,241,40,116,21,137,57,25,168,116,8,218,138,65,199,108,230,81,40,0,122,
217,18,5,105,147,107,30,101,101,143,176,54,8,219,150,237,60,71,209,181,225,195,241,74,156,120,73,43,22,248,217,77,67,101,50,252,81,130,43,148,114,185,95,158,154,18,97,21,215,43,115,132,246,56,58,123,221,
4,37,194,42,118,163,210,27,29,234,239,95,129,217,240,91,88,139,46,80,234,236,119,181,254,160,180,189,83,149,170,206,32,180,243,55,240,30,230,190,59,75,247,247,9,91,36,115,113,92,253,239,55,82,126,91,11,
194,123,106,176,105,67,102,91,218,168,129,174,141,195,136,103,224,183,167,161,35,213,208,99,171,192,33,147,32,219,185,141,36,92,190,100,209,180,60,98,102,50,18,42,130,38,245,226,154,8,137,251,101,57,72,
152,139,111,196,17,151,122,8,56,111,242,199,9,36,76,149,227,41,28,73,237,202,51,130,20,6,92,110,47,214,217,202,158,148,165,128,249,158,154,171,84,11,127,87,41,205,222,147,222,202,253,114,60,95,197,94,
18,164,85,220,55,224,114,22,20,127,25,14,152,11,227,74,190,245,130,61,30,168,37,208,200,55,97,222,68,13,255,113,233,117,244,9,91,42,125,35,133,143,126,137,220,157,14,118,155,187,176,103,175,108,59,0,181,
255,0,244,126,154,15,143,209,78,215,146,169,152,69,13,143,50,180,145,99,104,86,24,183,43,151,176,57,3,161,77,78,50,114,176,133,193,229,167,54,161,203,207,60,240,167,47,53,87,252,202,58,183,162,193,23,
79,134,89,42,149,231,54,166,174,138,14,190,12,0,18,25,241,1,122,63,23,130,17,174,134,95,105,67,133,234,224,79,8,51,215,158,130,223,202,94,65,161,11,116,191,157,174,19,21,67,181,95,171,156,131,251,80,144,
101,93,169,131,52,107,117,20,38,251,189,8,68,238,95,126,183,240,152,102,157,154,216,94,177,179,194,8,156,147,237,84,18,52,65,111,47,5,45,92,250,133,205,19,10,188,174,255,225,113,93,19,7,242,126,232,115,
61,181,144,55,226,196,216,211,171,57,119,86,5,59,158,128,174,140,67,39,104,106,162,14,140,136,131,142,39,176,58,96,67,96,203,178,88,106,175,225,72,111,144,250,68,163,29,232,16,183,75,79,8,9,100,233,21,
124,151,168,138,230,199,144,69,226,121,46,69,147,4,13,205,146,10,137,16,9,62,92,219,217,115,184,166,199,226,49,57,17,14,5,208,150,128,61,70,206,231,241,22,205,29,127,131,201,79,137,162,252,131,178,146,
119,193,204,220,161,172,76,19,140,3,251,81,104,38,65,249,227,14,252,142,220,88,246,38,10,70,132,223,203,84,177,62,17,24,167,74,241,142,188,119,47,34,192,167,96,255,157,108,171,128,146,252,127,175,210,
47,108,57,186,83,66,108,39,176,200,69,73,118,176,69,26,227,53,22,225,77,37,40,118,111,35,15,79,76,131,226,13,40,218,97,209,100,62,246,2,218,120,56,84,13,153,6,10,197,56,207,89,73,115,20,134,102,226,92,
203,3,243,21,236,49,116,238,141,42,142,7,178,72,60,70,200,146,160,225,26,120,45,207,118,39,44,194,22,225,177,30,25,59,104,255,181,43,2,96,227,137,28,116,86,4,193,27,46,206,46,81,248,99,217,78,148,227,
78,165,207,12,102,24,86,177,247,241,186,92,154,186,148,193,78,198,58,56,205,175,146,76,188,55,31,132,222,67,205,220,187,19,106,199,45,202,222,180,133,207,169,238,98,135,230,36,105,178,134,230,82,40,236,
134,251,145,188,119,21,185,247,251,96,221,115,61,140,27,100,27,15,26,47,219,74,181,251,176,176,187,42,60,220,123,137,80,120,47,71,86,228,162,74,142,148,239,198,233,240,76,14,142,150,244,28,134,178,22,
147,234,54,123,15,127,108,208,137,86,193,79,112,236,168,164,96,18,132,110,85,195,168,225,182,26,142,3,178,170,72,126,32,37,33,230,74,186,181,46,14,198,74,60,165,102,30,203,65,92,231,121,211,242,11,190,
210,176,14,65,177,7,34,194,239,227,198,115,147,201,196,27,21,33,66,21,150,222,148,105,131,209,244,43,120,15,242,154,30,61,5,170,154,90,231,209,158,238,239,186,102,185,254,37,112,22,95,5,117,237,43,240,
87,242,152,202,177,80,39,119,237,239,254,62,104,176,187,87,122,52,159,75,165,207,115,165,23,137,178,73,151,101,135,0,59,73,239,210,41,36,17,12,95,158,89,75,128,34,24,170,109,148,166,200,161,112,216,99,
108,246,20,29,175,130,174,150,158,194,193,188,150,21,251,99,96,84,83,251,29,10,133,130,67,173,244,26,214,67,15,138,21,81,168,49,2,227,248,18,162,230,39,215,65,191,179,12,222,138,159,243,128,153,80,211,
15,0,187,62,229,56,32,87,36,222,199,115,112,158,36,232,43,223,128,255,218,60,20,230,61,15,251,241,83,161,206,150,253,165,229,115,135,93,122,65,71,250,61,160,39,209,113,41,22,142,139,61,75,135,96,138,147,
31,100,9,76,36,88,74,82,49,15,33,223,139,139,208,41,85,25,47,68,200,54,5,151,160,160,98,155,81,216,190,150,235,167,217,86,209,86,211,166,239,166,112,40,83,168,235,149,245,103,235,17,122,148,2,171,92,67,
208,151,18,52,143,243,30,87,78,11,187,181,212,211,171,140,90,216,189,238,164,159,13,221,4,211,5,65,198,154,254,10,205,116,239,178,26,254,31,167,195,92,80,96,206,162,107,239,15,225,222,246,16,188,71,58,
219,224,224,136,232,167,208,178,88,191,215,56,39,221,167,92,142,144,192,99,112,255,157,227,71,146,18,115,255,9,222,253,245,200,197,255,77,187,255,89,10,150,80,3,185,158,16,154,125,132,44,15,123,24,31,
73,219,70,147,36,94,78,209,75,187,39,224,140,22,10,131,54,167,203,164,245,56,34,80,2,61,142,40,127,57,68,128,160,173,191,135,245,183,175,192,89,50,83,169,47,29,218,209,237,131,184,180,221,190,246,248,
88,214,236,30,56,250,254,66,208,246,6,56,111,79,97,200,254,50,244,243,59,248,199,214,130,142,62,25,170,190,14,138,142,74,111,63,91,186,67,249,117,4,12,110,133,241,61,14,140,250,46,24,223,39,228,98,182,
180,148,157,108,111,66,104,239,122,56,171,75,247,201,247,178,25,9,82,207,126,182,125,12,21,152,247,144,83,110,131,249,45,186,147,117,139,224,191,20,88,69,144,4,202,219,122,247,246,179,24,195,180,33,212,
150,67,40,63,31,214,229,165,140,174,129,121,149,104,190,236,31,39,41,134,128,222,210,231,228,65,160,100,78,240,141,23,242,79,156,44,65,104,33,163,154,73,159,64,175,127,2,254,31,152,50,8,205,135,113,233,
153,80,23,74,40,191,0,222,188,229,186,176,50,16,85,144,4,202,219,122,107,118,23,147,137,76,50,221,11,235,103,155,224,172,23,77,238,122,53,194,121,115,22,172,217,93,199,5,189,151,53,59,80,5,15,191,81,188,
141,115,153,117,100,184,46,137,178,20,151,23,4,230,112,186,215,84,134,221,157,198,16,127,46,123,35,67,12,184,123,245,255,15,131,72,51,37,124,117,39,14,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* Ambix_mirrorAudioProcessorEditor::coordinate_system_png = (const char*) resource_Ambix_mirrorAudioProcessorEditor_coordinate_system_png;
const int Ambix_mirrorAudioProcessorEditor::coordinate_system_pngSize = 6120;



//[EndFile] You can add extra defines here...
//[/EndFile]
