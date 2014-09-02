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


//[MiscUserDefs] You can add your own user definitions and misc code here...
#include "Graphics.h"
//[/MiscUserDefs]

//==============================================================================
Ambix_encoderAudioProcessorEditor::Ambix_encoderAudioProcessorEditor (Ambix_encoderAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
    sld_el (0),
    sld_az (0),
    sld_size (0),
#if INPUT_CHANNELS > 1
    sld_width (0),
#endif
    sld_speed (0),
    sld_el_move (0),
    sld_az_move (0),
    txt_az_move (0),
    txt_el_move (0),
	sphere_opengl(0)
{
    tooltipWindow.setMillisecondsBeforeTipAppears (700); // tooltip delay
    
    addAndMakeVisible (sld_el = new Slider ("new slider"));
    sld_el->setTooltip ("elevation");
    sld_el->setRange (-180, 180, 1);
    sld_el->setSliderStyle (Slider::LinearVertical);
    sld_el->setTextBoxStyle (Slider::TextBoxBelow, false, 41, 20);
    sld_el->setColour (Slider::thumbColourId, Colours::grey);
    sld_el->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_el->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_el->addListener (this);
    sld_el->setScrollWheelEndless(true); // added manually
    
    
    addAndMakeVisible (sld_az = new Slider ("new slider"));
    sld_az->setTooltip ("azimuth");
    sld_az->setRange (-180, 180, 1);
    sld_az->setSliderStyle (Slider::LinearHorizontal);
    sld_az->setTextBoxStyle (Slider::TextBoxRight, false, 40, 20);
    sld_az->setColour (Slider::thumbColourId, Colours::grey);
    sld_az->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_az->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_az->addListener (this);
    sld_az->setScrollWheelEndless(true); // added manually
    
    addAndMakeVisible (sld_size = new Slider ("new slider"));
    sld_size->setTooltip ("higher order scaling - decrease spatial sharpness");
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
    sld_size->setPopupDisplayEnabled(true, this);
    
    addAndMakeVisible (sld_speed = new Slider ("new slider"));
    sld_speed->setTooltip ("movement speed in deg/sec");
    sld_speed->setRange (0, 360, 1);
    sld_speed->setSliderStyle (Slider::LinearHorizontal);
    sld_speed->setTextBoxStyle (Slider::TextBoxRight, false, 40, 20);
    sld_speed->setColour (Slider::thumbColourId, Colour (0xffe98273));
    sld_speed->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_speed->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_speed->addListener (this);
    
    addAndMakeVisible (sld_el_move = new Slider ("new slider"));
    sld_el_move->setTooltip ("elevation movement speed");
    sld_el_move->setRange (0, 1, 0.01);
    sld_el_move->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_el_move->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    sld_el_move->setColour (Slider::trackColourId, Colour (0xff2d7dff));
    sld_el_move->setColour (Slider::rotarySliderFillColourId, Colours::white);
    sld_el_move->setColour (Slider::rotarySliderOutlineColourId, Colours::azure);
    sld_el_move->addListener (this);
    
    addAndMakeVisible (sld_az_move = new Slider ("new slider"));
    sld_az_move->setTooltip ("azimuth movement speed");
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
    txt_az_move->setColour (TextEditor::shadowColourId, Colour (0x0));
    txt_az_move->setText ("-180 deg/s");
    
    addAndMakeVisible (txt_el_move = new TextEditor ("new text editor"));
    txt_el_move->setTooltip ("elevation movement speed");
    txt_el_move->setMultiLine (false);
    txt_el_move->setReturnKeyStartsNewLine (false);
    txt_el_move->setReadOnly (true);
    txt_el_move->setScrollbarsShown (false);
    txt_el_move->setCaretVisible (false);
    txt_el_move->setPopupMenuEnabled (false);
    txt_el_move->setColour (TextEditor::outlineColourId, Colour (0x706884ff));
    txt_el_move->setColour (TextEditor::shadowColourId, Colour (0x0));
    txt_el_move->setText ("-180 deg/s");
    
    
    addAndMakeVisible (sphere_opengl = new SphereOpenGL());
    sphere_opengl->setName ("new OpenGl");
    // sphere_opengl->setBounds(10, 30, 240, 240);
    sphere_opengl->processor = ownerFilter;
    
    sld_az->setDoubleClickReturnValue(true, 0);
    sld_size->setDoubleClickReturnValue(true, 0);
    sld_el->setDoubleClickReturnValue(true, 0);
    sld_speed->setDoubleClickReturnValue(true, 90);
    sld_az_move->setDoubleClickReturnValue(true, 0.5f);
    sld_el_move->setDoubleClickReturnValue(true, 0.5f);
  
    addAndMakeVisible (lbl_id = new Label ("new label",
                                           TRANS("ID: 1")));
    lbl_id->setFont (Font (15.00f, Font::plain));
    lbl_id->setJustificationType (Justification::centredRight);
    lbl_id->setEditable (false, false, false);
    lbl_id->setColour (Label::textColourId, Colour (0xff888888));
    lbl_id->setColour (TextEditor::textColourId, Colours::black);
    lbl_id->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
  
#if INPUT_CHANNELS > 1
    addAndMakeVisible (sld_width = new Slider ("new slider"));
    sld_width->setTooltip ("multiple input sources are equally spread along this range");
    sld_width->setRange (0, 360, 1);
    sld_width->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sld_width->setTextBoxStyle (Slider::NoTextBox, false, 41, 20);
    sld_width->setColour (Slider::thumbColourId, Colour (0xffe982cd));
    sld_width->setColour (Slider::rotarySliderFillColourId, Colours::white);
    sld_width->setColour (Slider::rotarySliderOutlineColourId, Colours::white);
    sld_width->setColour (Slider::textBoxTextColourId, Colours::black);
    sld_width->setColour (Slider::textBoxBackgroundColourId, Colours::white);
    sld_width->addListener (this);
    sld_width->setPopupDisplayEnabled(true, this);
    sld_width->setDoubleClickReturnValue(true, 45);
#endif
    
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
#if INPUT_CHANNELS > 1
    setSize (330, 400);
#else
    setSize (330, 400);
#endif
    // register as change listener (gui/dsp sync)
    ownerFilter->addChangeListener(this);
    ownerFilter->sendChangeMessage(); // get status from dsp
    
    String str_id = "ID: ";
    str_id << ownerFilter->m_id;
    lbl_id->setText(str_id, dontSendNotification);
}

Ambix_encoderAudioProcessorEditor::~Ambix_encoderAudioProcessorEditor()
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();
    
    // remove me as listener for changes
    ourProcessor->removeChangeListener(this);
    
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    sld_el = nullptr;
    sld_az = nullptr;
    sld_size = nullptr;
    sld_speed = nullptr;
    sld_el_move = nullptr;
    sld_az_move = nullptr;
    txt_az_move = nullptr;
    txt_el_move = nullptr;
    lbl_id = nullptr;
    sphere_opengl = nullptr;
    
#if INPUT_CHANNELS > 1
    sld_width = nullptr;
#endif
    
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
                                       (float) (proportionOfWidth (0.6314f)), (float) (proportionOfHeight (0.5842f)),
                                       Colours::black,
                                       (float) (proportionOfWidth (0.1143f)), (float) (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, 330, 400);
    
    g.setColour (Colours::black);
    g.drawRect (0, 0, 330, 400, 1);
    
    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (165.0f, 310.0f, 154.0f, 77.0f, 4.0000f);
    
    g.setColour (Colours::white);
    g.setFont (Font (17.2000f, Font::bold));
    g.drawText ("AMBIX-ENCODER",
                -6, 2, 343, 30,
                Justification::centred, true);
    
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("elevation",
                266, 29, 48, 16,
                Justification::centred, true);
    
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("azimuth",
                216, 272, 48, 16,
                Justification::centredRight, true);
    
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("size",
                31, 382, 23, 16,
                Justification::centredRight, true);
    
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("max speed",
                206, 372, 57, 16,
                Justification::centredRight, true);
    
#if INPUT_CHANNELS > 1
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("multiple source width",
                61, 382, 95, 16,
                Justification::centredRight, true);
#endif
    
    g.setColour (Colour (0xff2b1d69));
    g.fillRoundedRectangle (24.0f, 310.0f, 226.0f, 43.0f, 4.0000f);
    
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("elevation move",
                219, 338, 81, 16,
                Justification::centredRight, true);
    
    g.setColour (Colours::white);
    g.setFont (Font (10.0000f, Font::plain));
    g.drawText ("azimuth move",
                59, 338, 81, 16,
                Justification::centredRight, true);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Ambix_encoderAudioProcessorEditor::resized()
{
    sphere_opengl->setBounds (23, 32, 240, 240);
    
    sld_el->setBounds (270, 38, 40, 232);
    sld_az->setBounds (27, 270, 282, 40);
    sld_size->setBounds (32, 355, 29, 29);
    
    sld_speed->setBounds (169, 347, 140, 40);
    sld_el_move->setBounds (275, 312, 29, 29);
    sld_az_move->setBounds (118, 312, 29, 29);
    txt_az_move->setBounds (37, 316, 78, 22);
    txt_el_move->setBounds (191, 316, 78, 22);
  
    lbl_id->setBounds (271, 0, 57, 24);
  
    btn_settings->setBounds (3, 3, 26, 25);
    
#if INPUT_CHANNELS > 1
    sld_width->setBounds (93, 355, 29, 29);
#endif
    
}

void Ambix_encoderAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();

    if (sliderThatWasMoved == sld_el)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::ElevationParam, ((float)sld_el->getValue() + 180) / 360.f);
    }
    else if (sliderThatWasMoved == sld_az)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::AzimuthParam, ((float)sld_az->getValue() + 180) / 360.f);
    }
    else if (sliderThatWasMoved == sld_size)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::SizeParam, (float)sld_size->getValue());
    }
#if INPUT_CHANNELS > 1
    // if multiple sources
    else if (sliderThatWasMoved == sld_width)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::WidthParam, (float)sld_width->getValue() / 360.f);
    }
#endif
    
    else if (sliderThatWasMoved == sld_az_move)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::AzimuthMvParam, (float)sld_az_move->getValue());
    }
    else if (sliderThatWasMoved == sld_el_move)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::ElevationMvParam, (float)sld_el_move->getValue());
    }
    else if (sliderThatWasMoved == sld_speed)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::SpeedParam, (float)sld_speed->getValue()/360.f);
    }
}


void Ambix_encoderAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();
    
    sld_az->setValue((ourProcessor->getParameter(Ambix_encoderAudioProcessor::AzimuthParam) - 0.5f) * 360.f, dontSendNotification);
    sld_el->setValue((ourProcessor->getParameter(Ambix_encoderAudioProcessor::ElevationParam) - 0.5f) * 360.f, dontSendNotification);
    sld_size->setValue(ourProcessor->getParameter(Ambix_encoderAudioProcessor::SizeParam), dontSendNotification);
    
    // set for gui!
    sphere_opengl->setSource((ourProcessor->getParameter(Ambix_encoderAudioProcessor::AzimuthParam) - 0.5f) * 360.f, (ourProcessor->getParameter(Ambix_encoderAudioProcessor::ElevationParam) - 0.5f) * 360.f);
    
#if INPUT_CHANNELS > 1
    sld_width->setValue(ourProcessor->getParameter(Ambix_encoderAudioProcessor::WidthParam)*360.f, dontSendNotification);
#endif
    
    sld_speed->setValue(ourProcessor->getParameter(Ambix_encoderAudioProcessor::SpeedParam)*360, dontSendNotification);
    
    String az_mv;
    
    float azimuth_mv_param = ourProcessor->getParameter(Ambix_encoderAudioProcessor::AzimuthMvParam);
    float speed_param = ourProcessor->getParameter(Ambix_encoderAudioProcessor::SpeedParam);
    
    sld_az_move->setValue(azimuth_mv_param, dontSendNotification);
    
    
    
    if (azimuth_mv_param <= 0.48f)
    {
        az_mv << "-" << String((int)(pow(speed_param*360.f, (0.45f - azimuth_mv_param)*2.22222f)+0.5f)).substring(0, 5) << " deg/s"; // from 0->90deg/sec
    } else if (azimuth_mv_param >= 0.52f) {
        az_mv << String((int)(pow(speed_param*360.f, (azimuth_mv_param - 0.55f)*2.22222f) + 0.5f)).substring(0, 5) << " deg/s";
    } else {
        az_mv << "0 deg/s";
    }
    
    txt_az_move->setText(az_mv);
    
    
    String el_mv;
    
    float elevation_mv_param = ourProcessor->getParameter(Ambix_encoderAudioProcessor::ElevationMvParam);
    
    sld_el_move->setValue(elevation_mv_param, dontSendNotification);
    
    
    if (elevation_mv_param <= 0.48f)
    {
        el_mv << "-" << String((int)(pow(speed_param*360.f, (0.45f - elevation_mv_param)*2.22222f) + 0.5f)).substring(0, 4) << " deg/s"; // from 0->90deg/sec
    } else if (elevation_mv_param >= 0.52f) {
        el_mv << String((int)(pow(speed_param*360.f, (elevation_mv_param - 0.55f)*2.22222f) + 0.5f)).substring(0, 4) << " deg/s";
    } else {
        el_mv << "0 deg/s";
    }
    
    
    
    txt_el_move->setText(el_mv);
}


void Ambix_encoderAudioProcessorEditor::modifierKeysChanged(const ModifierKeys &modifiers)
{
    
    if (modifiers.isShiftDown())
    {
        sld_el->setColour (Slider::thumbColourId, Colours::black);
        sld_el->setEnabled(false);
    }
    else
    {
        sld_el->setColour (Slider::thumbColourId, Colours::grey);
        sld_el->setEnabled(true);
    }
    
    
    if (modifiers.isCtrlDown())
    {
        sld_az->setColour (Slider::thumbColourId, Colours::black);
        sld_az->setEnabled(false);
    }
    else
    {
        sld_az->setColour (Slider::thumbColourId, Colours::grey);
        sld_az->setEnabled(true);
    }
    
}


void Ambix_encoderAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();
    
    if (buttonThatWasClicked == btn_settings)
    {
        if (!_settingsDialogWindow)
        {
            juce::DialogWindow::LaunchOptions launchOptions;
            launchOptions.dialogTitle = juce::String("Settings");
            launchOptions.content.setOwned(new Settings(*ourProcessor));
            launchOptions.componentToCentreAround = this;
            launchOptions.escapeKeyTriggersCloseButton = true;
            launchOptions.useNativeTitleBar = true;
            launchOptions.resizable = false;
            launchOptions.useBottomRightCornerResizer = false;
            _settingsDialogWindow = launchOptions.launchAsync();
        }
    }
    
}

