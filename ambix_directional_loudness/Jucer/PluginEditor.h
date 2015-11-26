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

#ifndef __JUCE_HEADER_CDA58EC13A3FDFE9__
#define __JUCE_HEADER_CDA58EC13A3FDFE9__

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
class Ambix_directional_loudnessAudioProcessorEditor  : public AudioProcessorEditor,
                                                        public ButtonListener,
                                                        public SliderListener,
                                                        public ComboBoxListener
{
public:
    //==============================================================================
    Ambix_directional_loudnessAudioProcessorEditor (Ambix_directional_loudnessAudioProcessor* ownerFilter);
    ~Ambix_directional_loudnessAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);

    // Binary resources:
    static const char* solo_symbol_png;
    static const int solo_symbol_pngSize;
    static const char* solo_symbol_over_png;
    static const int solo_symbol_over_pngSize;
    static const char* solo_symbol_act_png;
    static const int solo_symbol_act_pngSize;
    static const char* drag_off_png;
    static const int drag_off_pngSize;
    static const char* drag_over_png;
    static const int drag_over_pngSize;
    static const char* drag_on_png;
    static const int drag_on_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> lbl_gd;
    ScopedPointer<Component> filtergraph;
    ScopedPointer<ImageButton> btn_drag;
    ScopedPointer<Slider> sld_az_1;
    ScopedPointer<Slider> sld_el_1;
    ScopedPointer<ComboBox> box_shape_1;
    ScopedPointer<Slider> sld_w_1;
    ScopedPointer<Slider> sld_h_1;
    ScopedPointer<ImageButton> btn_solo_1;
    ScopedPointer<Slider> sld_gain_1;
    ScopedPointer<Slider> sld_az_2;
    ScopedPointer<Slider> sld_el_2;
    ScopedPointer<ComboBox> box_shape_2;
    ScopedPointer<Slider> sld_w_2;
    ScopedPointer<Slider> sld_h_2;
    ScopedPointer<ImageButton> btn_solo_2;
    ScopedPointer<Slider> sld_gain_2;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_directional_loudnessAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_CDA58EC13A3FDFE9__
