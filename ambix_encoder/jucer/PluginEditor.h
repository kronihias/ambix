/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.2.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_647002131C011257__
#define __JUCE_HEADER_647002131C011257__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_encoderAudioProcessorEditor  : public AudioProcessorEditor,
                                           public Slider::Listener
{
public:
    //==============================================================================
    Ambix_encoderAudioProcessorEditor (Ambix_encoderAudioProcessor* ownerFilter);
    ~Ambix_encoderAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<Slider> sld_el;
    std::unique_ptr<Component> opengl_component;
    std::unique_ptr<Slider> sld_az;
    std::unique_ptr<Slider> sld_size;
    std::unique_ptr<Slider> sld_width;
    std::unique_ptr<Slider> sld_speed;
    std::unique_ptr<Slider> sld_el_move;
    std::unique_ptr<Slider> sld_az_move;
    std::unique_ptr<TextEditor> txt_az_move;
    std::unique_ptr<TextEditor> txt_el_move;
    std::unique_ptr<Label> lbl_id;
    std::unique_ptr<TextEditor> txt_id;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_encoderAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_647002131C011257__
