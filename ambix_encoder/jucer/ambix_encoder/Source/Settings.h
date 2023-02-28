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

#ifndef __JUCE_HEADER_B57065F632DEFA52__
#define __JUCE_HEADER_B57065F632DEFA52__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Settings  : public Component,
                  public Button::Listener,
                  public Slider::Listener
{
public:
    //==============================================================================
    Settings (Ambix_encoderAudioProcessor& Processor);
    ~Settings();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    Ambix_encoderAudioProcessor& _processor;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<TextEditor> txt_snd_ip;
    std::unique_ptr<TextEditor> txt_snd_port;
    std::unique_ptr<ToggleButton> tgl_snd_active;
    std::unique_ptr<TextEditor> txt_rcv_port;
    std::unique_ptr<ToggleButton> tgl_rcv_active;
    std::unique_ptr<Label> lbl_id;
    std::unique_ptr<Slider> slider;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Settings)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_B57065F632DEFA52__
