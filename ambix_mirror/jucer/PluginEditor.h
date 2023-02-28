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

#ifndef __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_C80724AD__
#define __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_C80724AD__

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
class Ambix_mirrorAudioProcessorEditor  : public AudioProcessorEditor,
                                          public ComboBox::Listener,
                                          public Slider::Listener,
                                          public Button::Listener
{
public:
    //==============================================================================
    Ambix_mirrorAudioProcessorEditor (Ambix_mirrorAudioProcessor* ownerFilter);
    ~Ambix_mirrorAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    // Binary resources:
    static const char* coordinate_system_png;
    static const int coordinate_system_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ComboBox* box_presets;
    Label* label5;
    Slider* sld_x_even;
    Label* label;
    Label* label2;
    ToggleButton* tgl_x_even_inv;
    Slider* sld_x_odd;
    Label* label3;
    ToggleButton* tgl_x_odd_inv;
    Slider* sld_y_even;
    Label* label6;
    ToggleButton* tgl_y_even_inv;
    Slider* sld_y_odd;
    Label* label7;
    ToggleButton* tgl_y_odd_inv;
    Slider* sld_z_even;
    Label* label9;
    ToggleButton* tgl_z_even_inv;
    Slider* sld_z_odd;
    Label* label10;
    ToggleButton* tgl_z_odd_inv;
    Label* lbl_x_even;
    Label* lbl_x_odd;
    Label* lbl_y_even;
    Label* lbl_y_odd;
    Label* lbl_z_even;
    Label* lbl_z_odd;
    Label* label4;
    Label* label8;
    Slider* sld_circular;
    Label* label11;
    ToggleButton* tgl_circular_inv;
    Label* lbl_circular;
    Image cachedImage_coordinate_system_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_mirrorAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_C80724AD__
