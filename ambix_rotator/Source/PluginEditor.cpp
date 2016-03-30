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

//[Headers] You can add your own extra header files here...
#include "PluginProcessor.h"
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
Ambix_rotatorAudioProcessorEditor::Ambix_rotatorAudioProcessorEditor (Ambix_rotatorAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
    _changed(true)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (sld_yaw = new Slider ("new slider"));
    sld_yaw->setTooltip (TRANS("yaw (left-right)"));
    sld_yaw->setRange (-180, 180, 0.1);
    sld_yaw->setSliderStyle (Slider::LinearHorizontal);
    sld_yaw->setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sld_yaw->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_yaw->addListener (this);
    sld_yaw->setDoubleClickReturnValue(true, 0.f);
    sld_yaw->setScrollWheelEndless(true);
    
    addAndMakeVisible (label2 = new Label ("new label",
                                           TRANS("Yaw")));
    label2->setFont (Font (15.00f, Font::plain));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (Label::textColourId, Colours::white);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label3 = new Label ("new label",
                                           TRANS("Pitch")));
    label3->setFont (Font (15.00f, Font::plain));
    label3->setJustificationType (Justification::centredLeft);
    label3->setEditable (false, false, false);
    label3->setColour (Label::textColourId, Colours::white);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sld_pitch = new Slider ("new slider"));
    sld_pitch->setTooltip (TRANS("pitch (up-down)"));
    sld_pitch->setRange (-180, 180, 0.1);
    sld_pitch->setSliderStyle (Slider::LinearHorizontal);
    sld_pitch->setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sld_pitch->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_pitch->addListener (this);
    sld_pitch->setDoubleClickReturnValue(true, 0.f);
    sld_pitch->setScrollWheelEndless(true);
    
    addAndMakeVisible (label4 = new Label ("new label",
                                           TRANS("Roll")));
    label4->setFont (Font (15.00f, Font::plain));
    label4->setJustificationType (Justification::centredLeft);
    label4->setEditable (false, false, false);
    label4->setColour (Label::textColourId, Colours::white);
    label4->setColour (TextEditor::textColourId, Colours::black);
    label4->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sld_roll = new Slider ("new slider"));
    sld_roll->setRange (-180, 180, 0.1);
    sld_roll->setSliderStyle (Slider::LinearHorizontal);
    sld_roll->setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sld_roll->setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_roll->addListener (this);
    sld_roll->setDoubleClickReturnValue(true, 0.f);
    sld_roll->setScrollWheelEndless(true);
    
    addAndMakeVisible (label5 = new Label ("new label",
                                           TRANS("Rotation order:")));
    label5->setFont (Font (15.00f, Font::plain));
    label5->setJustificationType (Justification::centredLeft);
    label5->setEditable (false, false, false);
    label5->setColour (Label::textColourId, Colours::white);
    label5->setColour (TextEditor::textColourId, Colours::black);
    label5->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (toggleButton = new ToggleButton ("new toggle button"));
    toggleButton->setButtonText (TRANS("yaw-pitch-roll"));
    toggleButton->setRadioGroupId (1);
    toggleButton->addListener (this);
    toggleButton->setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (toggleButton2 = new ToggleButton ("new toggle button"));
    toggleButton2->setButtonText (TRANS("roll-pitch-yaw"));
    toggleButton2->setRadioGroupId (1);
    toggleButton2->addListener (this);
    toggleButton2->setColour (ToggleButton::textColourId, Colours::white);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 240);

    timerCallback();
    
    ownerFilter->addChangeListener(this);
    
    startTimer(40); // update display rate
}

Ambix_rotatorAudioProcessorEditor::~Ambix_rotatorAudioProcessorEditor()
{
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();
    
    // remove me as listener for changes
    ourProcessor->removeChangeListener(this);
    stopTimer();
    
    sld_yaw = nullptr;
    label2 = nullptr;
    label3 = nullptr;
    sld_pitch = nullptr;
    label4 = nullptr;
    sld_roll = nullptr;
    label5 = nullptr;
    toggleButton = nullptr;
    toggleButton2 = nullptr;


}

//==============================================================================
void Ambix_rotatorAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                       static_cast<float> (proportionOfWidth (0.6400f)), static_cast<float> (proportionOfHeight (0.6933f)),
                                       Colours::black,
                                       static_cast<float> (proportionOfWidth (0.1143f)), static_cast<float> (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 410, 254);

    g.setColour (Colours::black);
    g.drawRect (0, 0, 410, 254, 1);

    g.setColour (Colours::azure);
    g.setFont (Font (17.20f, Font::bold));
    g.drawText (TRANS("AMBIX-ROTATOR"),
                10, 8, 380, 30,
                Justification::centred, true);

    g.setColour (Colour (0x932b1d69));
    g.fillRoundedRectangle (9.0f, 48.0f, 383.0f, 124.0f, 10.000f);

    g.setColour (Colour (0x93691d1d));
    g.fillRoundedRectangle (9.0f, 183.0f, 383.0f, 48.0f, 10.000f);
    
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

void Ambix_rotatorAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    sld_yaw->setBounds (78, 61, 301, 24);
    label2->setBounds (21, 61, 54, 24);
    label3->setBounds (21, 98, 54, 24);
    sld_pitch->setBounds (79, 98, 301, 24);
    label4->setBounds (21, 134, 54, 24);
    sld_roll->setBounds (79, 134, 301, 24);
    label5->setBounds (22, 182, 109, 24);
    toggleButton->setBounds (133, 183, 176, 24);
    toggleButton2->setBounds (133, 206, 176, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Ambix_rotatorAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    _changed = true;
}

void Ambix_rotatorAudioProcessorEditor::timerCallback()
{
    if (_changed)
    {
        _changed = false;
        
        Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();
        
        sld_yaw->setValue(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::YawParam)*360.f-180.f, dontSendNotification);
        
        sld_pitch->setValue(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::PitchParam)*360.f-180.f, dontSendNotification);
        
        sld_roll->setValue(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::RollParam)*360.f-180.f, dontSendNotification);
        
        if (ourProcessor->getParameter(Ambix_rotatorAudioProcessor::RotOrderParam) <= 0.5f)
        {
            toggleButton->setToggleState(true, dontSendNotification);
            toggleButton2->setToggleState(false, dontSendNotification);
        }
        else
        {
            toggleButton->setToggleState(false, dontSendNotification);
            toggleButton2->setToggleState(true, dontSendNotification);
        }
    }
}

void Ambix_rotatorAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();
    
    if (sliderThatWasMoved == sld_yaw)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::YawParam, (sld_yaw->getValue()+180.f)/360.f);
    }
    else if (sliderThatWasMoved == sld_pitch)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::PitchParam, (sld_pitch->getValue()+180.f)/360.f);
    }
    else if (sliderThatWasMoved == sld_roll)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RollParam, (sld_roll->getValue()+180.f)/360.f);
    }

}

void Ambix_rotatorAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();
    
    if (buttonThatWasClicked == toggleButton)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RotOrderParam, 0.f);
    }
    else if (buttonThatWasClicked == toggleButton2)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RotOrderParam, 1.f);
    }

}

