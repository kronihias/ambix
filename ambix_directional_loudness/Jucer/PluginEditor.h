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
                                                        public Button::Listener,
                                                        public Slider::Listener,
                                                        public ComboBox::Listener
{
public:
    //==============================================================================
    Ambix_directional_loudnessAudioProcessorEditor (Ambix_directional_loudnessAudioProcessor* ownerFilter);
    ~Ambix_directional_loudnessAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

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
    std::unique_ptr<Label> lbl_gd;
    std::unique_ptr<Component> filtergraph;
    std::unique_ptr<ImageButton> btn_drag;
    std::unique_ptr<Slider> sld_az_1;
    std::unique_ptr<Slider> sld_el_1;
    std::unique_ptr<ComboBox> box_shape_1;
    std::unique_ptr<Slider> sld_w_1;
    std::unique_ptr<Slider> sld_h_1;
    std::unique_ptr<ImageButton> btn_solo_1;
    std::unique_ptr<Slider> sld_gain_1;
    std::unique_ptr<Slider> sld_az_2;
    std::unique_ptr<Slider> sld_el_2;
    std::unique_ptr<ComboBox> box_shape_2;
    std::unique_ptr<Slider> sld_w_2;
    std::unique_ptr<Slider> sld_h_2;
    std::unique_ptr<ImageButton> btn_solo_2;
    std::unique_ptr<Slider> sld_gain_2;
    std::unique_ptr<TabbedComponent> tabbedComponent;
    std::unique_ptr<TabbedComponent> tabbedComponent2;
    std::unique_ptr<Label> lbl_drag;
    std::unique_ptr<ImageButton> btn_solo_reset;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_directional_loudnessAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_CDA58EC13A3FDFE9__
