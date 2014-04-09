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


//==============================================================================
Ambix_converterAudioProcessorEditor::Ambix_converterAudioProcessorEditor (Ambix_converterAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      hyperlinkButton (0),
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
    tooltipWindow.setMillisecondsBeforeTipAppears (700); // tooltip delay
    
    addAndMakeVisible (hyperlinkButton = new HyperlinkButton ("(C) 2013 Matthias Kronlachner",
                                                              URL ("http://www.matthiaskronlachner.com")));
    hyperlinkButton->setTooltip ("http://www.matthiaskronlachner.com");
    hyperlinkButton->setButtonText ("(C) 2013 Matthias Kronlachner");
    hyperlinkButton->setColour (HyperlinkButton::textColourId, Colours::azure);
    
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
    tgl_invert_cs->setColour (ToggleButton::textColourId, Colours::azure);
    
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
    
   
    addAndMakeVisible (box_presets = new ComboBox ("new combo box"));
    box_presets->setTooltip ("choose conversion scheme from several presets");
    box_presets->setEditableText (false);
    box_presets->setJustificationType (Justification::centredLeft);
    box_presets->setTextWhenNothingSelected ("");
    box_presets->setTextWhenNoChoicesAvailable ("(no presets)");
    
    box_presets->addItem (".amb / AMB plugins (full periphonic) / Tetraproc -> ambix", 1);
    box_presets->addItem ("ambix -> .amb / AMB plugins (full periphonic) / Tetraproc", 2);
    box_presets->addSeparator();
    
    box_presets->addItem ("Universal Ambisonics (UA) -> ambix", 3);
    box_presets->addItem ("ambix -> Universal Ambisonics (UA)", 4);
    box_presets->addSeparator();

    
    box_presets->addItem ("Wigware / B2X (3D) -> ambix", 5);
    box_presets->addItem ("ambix -> Wigware / B2X (3D)", 6);
    box_presets->addSeparator();
    

    box_presets->addItem ("iem_ambi -> ambix", 7);
    box_presets->addItem ("ambix -> iem_ambi", 8);
    box_presets->addSeparator();
    
    box_presets->addItem ("ICST (may vary) -> ambix", 9);
    box_presets->addItem ("ambix -> ICST (may vary)", 10);
    box_presets->addSeparator();
    
    box_presets->addItem ("mtx_spherical_harmonics -> ambix", 11);
    box_presets->addItem ("ambix -> mtx_spherical_harmonics", 12);
    box_presets->addSeparator();
    
    
    box_presets->addItem ("flat - no change", 13);
    
    box_presets->addListener (this);
    
    box_presets->setText(ownerFilter->box_presets_text, dontSendNotification);
    
    addAndMakeVisible (label5 = new Label ("new label",
                                           "Presets"));
    label5->setFont (Font (15.0000f, Font::plain));
    label5->setJustificationType (Justification::centredRight);
    label5->setEditable (false, false, false);
    label5->setColour (Label::textColourId, Colours::azure);
    label5->setColour (TextEditor::textColourId, Colours::black);
    label5->setColour (TextEditor::backgroundColourId, Colour (0x0));
    
    addAndMakeVisible (tgl_in_2d = new ToggleButton ("new toggle button"));
    tgl_in_2d->setButtonText (TRANS("2D"));
    tgl_in_2d->setTooltip ("input is 2D Ambisonics");
    tgl_in_2d->addListener (this);
    
    addAndMakeVisible (tgl_out_2d = new ToggleButton ("new toggle button"));
    tgl_out_2d->setButtonText (TRANS("2D"));
    tgl_out_2d->setTooltip ("output is 2D Ambisonics (this could throw away channels!)");
    tgl_out_2d->addListener (this);

    //[UserPreSize]
    //[/UserPreSize]

    setSize (410, 305);


    //[Constructor] You can add your own custom stuff here..
    //startTimer (500); // timer is deprecated - use changelistener!
    //[/Constructor]
    ownerFilter->addChangeListener(this);
    getParamsFromHost();
}

Ambix_converterAudioProcessorEditor::~Ambix_converterAudioProcessorEditor()
{
    Ambix_converterAudioProcessor* ourProcessor = getProcessor();
    
    ourProcessor->removeChangeListener(this);

    hyperlinkButton = nullptr;
    box_in_ch_seq = nullptr;
    label = nullptr;
    label2 = nullptr;
    label3 = nullptr;
    box_out_ch_seq = nullptr;
    label4 = nullptr;
    box_in_norm = nullptr;
    box_out_norm = nullptr;
    tgl_invert_cs = nullptr;
    box_presets = nullptr;
    label5 = nullptr;
    tgl_flip = nullptr;
    tgl_flop = nullptr;
    tgl_flap = nullptr;
    label6 = nullptr;
    
    tgl_in_2d = nullptr;
    tgl_out_2d = nullptr;

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
    //getParamsFromHost();
    //[/UserPaint]
}

void Ambix_converterAudioProcessorEditor::resized()
{
    box_in_ch_seq->setBounds (136, 136, 120, 24);
    label->setBounds (0, 136, 127, 24);
    label2->setBounds (136, 112, 112, 24);
    label3->setBounds (16, 170, 111, 24);
    box_out_ch_seq->setBounds (280, 136, 120, 24);
    label4->setBounds (288, 112, 104, 24);
    box_in_norm->setBounds (136, 170, 120, 24);
    box_out_norm->setBounds (280, 170, 120, 24);
    tgl_in_2d->setBounds (136, 193, 112, 24);
    tgl_out_2d->setBounds (280, 193, 112, 24);
    tgl_invert_cs->setBounds (20, 271, 180, 24);
    box_presets->setBounds (112, 64, 288, 24);
    label5->setBounds (8, 64, 104, 24);
    tgl_flip->setBounds (280, 224, 120, 24);
    tgl_flop->setBounds (280, 248, 120, 24);
    tgl_flap->setBounds (280, 272, 120, 24);
    label6->setBounds (224, 224, 56, 24);
    
    //getParamsFromHost();
}

void Ambix_converterAudioProcessorEditor::getParamsFromHost()
{
	std::cout << "getParams" << std::endl;
    Ambix_converterAudioProcessor* ourProcessor = getProcessor();
	
    // active preset
    
    if (ourProcessor->active_preset)
        box_presets->setSelectedId(ourProcessor->active_preset, dontSendNotification);
        
    
	// input channel sequence box
    float value = ourProcessor->getParameter(Ambix_converterAudioProcessor::InSeqParam);
	if (value < 0.33f) {
		box_in_ch_seq->setSelectedId(1, dontSendNotification);
	} else if (value >= 0.33 && value < 0.66f) {
		box_in_ch_seq->setSelectedId(2, dontSendNotification);
	} else if (value >= 0.66f) {
		box_in_ch_seq->setSelectedId(3, dontSendNotification);
	}
  
	// output channel sequence box
    value = ourProcessor->getParameter(Ambix_converterAudioProcessor::OutSeqParam);
	if (value < 0.33f) {
		box_out_ch_seq->setSelectedId(1, dontSendNotification);
	} else if (value >= 0.33 && value < 0.66f) {
		box_out_ch_seq->setSelectedId(2, dontSendNotification);
	} else if (value >= 0.66f) {
		box_out_ch_seq->setSelectedId(3, dontSendNotification);
	}
    
	// input normalization scheme box
    value = ourProcessor->getParameter(Ambix_converterAudioProcessor::InNormParam);
	if (value < 0.33f) {
		box_in_norm->setSelectedId(1, dontSendNotification);
	} else if (value >= 0.33 && value < 0.66f) {
		box_in_norm->setSelectedId(2, dontSendNotification);
	} else if (value >= 0.66f) {
		box_in_norm->setSelectedId(3, dontSendNotification);
	}
    
	// output normalization scheme box
    value = ourProcessor->getParameter(Ambix_converterAudioProcessor::OutNormParam);
	if (value < 0.33f) {
		box_out_norm->setSelectedId(1, dontSendNotification);
	} else if (value >= 0.33 && value < 0.66f) {
		box_out_norm->setSelectedId(2, dontSendNotification);
	} else if (value >= 0.66f) {
		box_out_norm->setSelectedId(3, dontSendNotification);
	}
    
    // condon shortley phase
    tgl_invert_cs->setToggleState((ourProcessor->getParameter(Ambix_converterAudioProcessor::FlipCsParam) > 0.5f), dontSendNotification);

    // flip
    tgl_flip->setToggleState((ourProcessor->getParameter(Ambix_converterAudioProcessor::FlipParam) > 0.5f), dontSendNotification);
    
    // flop
    tgl_flop->setToggleState((ourProcessor->getParameter(Ambix_converterAudioProcessor::FlopParam) > 0.5f), dontSendNotification);
    
    // flip
    tgl_flap->setToggleState((ourProcessor->getParameter(Ambix_converterAudioProcessor::FlapParam) > 0.5f), dontSendNotification);
    
    // in_2d
    tgl_in_2d->setToggleState((ourProcessor->getParameter(Ambix_converterAudioProcessor::In2DParam) > 0.5f), dontSendNotification);
    
    // out_2d
    tgl_out_2d->setToggleState((ourProcessor->getParameter(Ambix_converterAudioProcessor::Out2DParam) > 0.5f), dontSendNotification);
    
}

void Ambix_converterAudioProcessorEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    
    Ambix_converterAudioProcessor* ourProcessor = getProcessor();
	
	//std::cout << "index: " << comboBoxThatHasChanged->getSelectedId() << std::endl;

    if (comboBoxThatHasChanged != box_presets)
    {
        ourProcessor->active_preset = 0;
        ourProcessor->box_presets_text = "";
        box_presets->setText("", dontSendNotification);
    }
    
    if (comboBoxThatHasChanged == box_in_ch_seq)
    {
        //[UserComboBoxCode_box_in_ch_seq] -- add your combo box handling code here..
	
        float value = (float)box_in_ch_seq->getSelectedItemIndex() * 0.333f;
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, value);
        //[/UserComboBoxCode_box_in_ch_seq]
    }
    else if (comboBoxThatHasChanged == box_out_ch_seq)
    {
        //[UserComboBoxCode_box_out_ch_seq] -- add your combo box handling code here..
        float value = (float)box_out_ch_seq->getSelectedItemIndex() * 0.333f;
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, value);
        //[/UserComboBoxCode_box_out_ch_seq]
    }
    else if (comboBoxThatHasChanged == box_in_norm)
    {
        //[UserComboBoxCode_box_in_norm] -- add your combo box handling code here..
        float value = (float)box_in_norm->getSelectedItemIndex() * 0.333f;
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, value);
        //[/UserComboBoxCode_box_in_norm]
    }
    else if (comboBoxThatHasChanged == box_out_norm)
    {
        //[UserComboBoxCode_box_out_norm] -- add your combo box handling code here..
        float value = (float)box_out_norm->getSelectedItemIndex() * 0.333f;
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, value);
        //[/UserComboBoxCode_box_out_norm]
    }
    else if (comboBoxThatHasChanged == box_presets)
    {
        float seq_acn = 0.f;
        float seq_fuma = 0.5f;
        float seq_sid = 1.f;
        
        float norm_sn3d = 0.f;
        float norm_fuma = 0.5f;
        float norm_n3d = 1.f;
        
        /////////////////////////////
        // presets are defined here!
        ourProcessor->active_preset = box_presets->getSelectedId();
        ourProcessor->box_presets_text = box_presets->getText();
        
        if (box_presets->getSelectedId() == 1) { // .amb -> ambix
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_fuma);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_fuma);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 2) { // ambix -> .amb
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_fuma);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_fuma);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 3) { // UA -> ambix
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_n3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_sid);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 4) { // ambix -> UA
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_n3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_sid);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 5) { // Wigware/B2X -> ambix
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_fuma);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_fuma);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 6) { // ambix -> Wigware/B2X
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_fuma);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_fuma);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 7) { // iem_ambi -> ambix
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_sid);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 8) { // ambix -> iem_ambi
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_sid);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 9) { // ICST -> ambix
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_fuma);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 10) { // ambix -> ICST
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_fuma);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 11) { // mtx_spherical_harmonics -> ambix
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_n3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f); // ! dont ! flip condon shortley phase
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 1.f); // flip negative m
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);

        }
        
        
        if (box_presets->getSelectedId() == 12) { // ambix -> mtx_spherical_harmonics
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_n3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f); // !dont! flip condon shortley phase
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, 1.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
        if (box_presets->getSelectedId() == 13) { // flat
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InNormParam, norm_sn3d);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutNormParam, norm_sn3d);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::InSeqParam, seq_acn);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::OutSeqParam, seq_acn);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, 0.f);
            
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, 0.f);
            ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, 0.f);
        }
        
        
    }
}

void Ambix_converterAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    
    Ambix_converterAudioProcessor* ourProcessor = getProcessor();
    
    // reset preset
    ourProcessor->active_preset = 0;
    ourProcessor->box_presets_text = "";
    box_presets->setText("", dontSendNotification);
    
    if (buttonThatWasClicked == tgl_invert_cs)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipCsParam, (float)tgl_invert_cs->getToggleState());
    }
    else if (buttonThatWasClicked == tgl_flip)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlipParam, (float)tgl_flip->getToggleState());
    }
    else if (buttonThatWasClicked == tgl_flop)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlopParam, (float)tgl_flop->getToggleState());
    }
    else if (buttonThatWasClicked == tgl_flap)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::FlapParam, (float)tgl_flap->getToggleState());
    }
    else if (buttonThatWasClicked == tgl_in_2d)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::In2DParam, (float)tgl_in_2d->getToggleState());
    }
    else if (buttonThatWasClicked == tgl_out_2d)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_converterAudioProcessor::Out2DParam, (float)tgl_out_2d->getToggleState());
    }
    
}

void Ambix_converterAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    getParamsFromHost();
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
                 initialWidth="410" initialHeight="200">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 0 410 200" fill="solid: ffdfe3e7" hasStroke="0"/>
    <ROUNDRECT pos="272 60 133 106" cornerSize="10" fill="solid: 41ffff00" hasStroke="0"/>
    <ROUNDRECT pos="131 62 133 106" cornerSize="10" fill="solid: 410000ff" hasStroke="0"/>
    <TEXT pos="11 6 380 30" fill="solid: ff000000" hasStroke="0" text="AMBIX-CONVERTER"
          fontname="Default font" fontsize="17.2" bold="1" italic="0" justification="36"/>
    <TEXT pos="8 28 380 30" fill="solid: ff000000" hasStroke="0" text="convert between Ambisonics formats"
          fontname="Default font" fontsize="12.4" bold="0" italic="0" justification="36"/>
  </BACKGROUND>
  <HYPERLINKBUTTON name="new hyperlink" id="529bec77f5596b3c" memberName="hyperlinkButton"
                   virtualName="" explicitFocusOrder="0" pos="216 176 192 20" tooltip="http://www.matthiaskronlachner.com"
                   textCol="cc000000" buttonText="(C) 2013 Matthias Kronlachner"
                   connectedEdges="0" needsCallback="0" radioGroupId="0" url="http://www.matthiaskronlachner.com"/>
  <COMBOBOX name="new combo box" id="f1000b0699c42d32" memberName="box_in_ch_seq"
            virtualName="" explicitFocusOrder="0" pos="136 88 120 24" tooltip="channel input sequence"
            editable="0" layout="33" items="ACN&#10;Furse-Malham&#10;SID"
            textWhenNonSelected="ACN" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="18d53e308ddb22b0" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0 88 127 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Channel sequence" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="new label" id="799520c015f0cd6c" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="120 64 112 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Input" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="1" italic="0" justification="36"/>
  <LABEL name="new label" id="31661e2aabfa68fd" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="16 136 111 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Normalization" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <COMBOBOX name="new combo box" id="248109d38bcbf793" memberName="box_out_ch_seq"
            virtualName="" explicitFocusOrder="0" pos="280 88 120 24" tooltip="channel output sequence"
            editable="0" layout="33" items="ACN&#10;Furse-Malham&#10;SID"
            textWhenNonSelected="ACN" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="b555a3e046bcdb5a" memberName="label4" virtualName=""
         explicitFocusOrder="0" pos="288 64 104 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Output" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="1" italic="0" justification="36"/>
  <COMBOBOX name="new combo box" id="fc31f754f89353ca" memberName="box_in_norm"
            virtualName="" explicitFocusOrder="0" pos="136 136 120 24" tooltip="channel input normalization scheme"
            editable="0" layout="33" items="SN3D&#10;Furse-Malham&#10;N3D"
            textWhenNonSelected="SN3D" textWhenNoItems="(no choices)"/>
  <COMBOBOX name="new combo box" id="370aea36432b7f84" memberName="box_out_norm"
            virtualName="" explicitFocusOrder="0" pos="280 136 120 24" tooltip="channel output normalization scheme"
            editable="0" layout="33" items="SN3D&#10;Furse-Malham&#10;N3D"
            textWhenNonSelected="SN3D" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



//[EndFile] You can add extra defines here...
//[/EndFile]
