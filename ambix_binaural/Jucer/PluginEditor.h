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

#ifndef __JUCE_HEADER_4A9AC0213502B6A5__
#define __JUCE_HEADER_4A9AC0213502B6A5__

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
class Ambix_binauralAudioProcessorEditor  : public AudioProcessorEditor,
                                            public ComboBox::Listener,
                                            public Button::Listener,
                                            public Slider::Listener
{
public:
    //==============================================================================
    Ambix_binauralAudioProcessorEditor (Ambix_binauralAudioProcessor* ownerFilter);
    ~Ambix_binauralAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<GroupComponent> gr_hp;
    std::unique_ptr<Label> label;
    std::unique_ptr<ComboBox> box_presets;
    std::unique_ptr<Label> label5;
    std::unique_ptr<TextEditor> txt_debug;
    std::unique_ptr<TextButton> btn_open;
    std::unique_ptr<Label> label2;
    std::unique_ptr<Label> label3;
    std::unique_ptr<Label> label4;
    std::unique_ptr<Label> num_ch;
    std::unique_ptr<Label> num_spk;
    std::unique_ptr<Label> num_hrtf;
    std::unique_ptr<TextButton> btn_preset_folder;
    std::unique_ptr<ToggleButton> tgl_load_irs;
    std::unique_ptr<Slider> sld_hpf;
    std::unique_ptr<ComboBox> box_hp_order;
    std::unique_ptr<GroupComponent> groupComponent2;
    std::unique_ptr<Slider> sld_lpf;
    std::unique_ptr<ComboBox> box_lp_order;
    std::unique_ptr<ComboBox> box_sub_output;
    std::unique_ptr<ComboBox> box_sub_output2;
    std::unique_ptr<Slider> sld_sub_vol;
    std::unique_ptr<Slider> sld_gain;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_binauralAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_4A9AC0213502B6A5__
