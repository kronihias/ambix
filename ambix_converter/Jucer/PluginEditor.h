/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  10 Jun 2013 5:40:13pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_FC1F6DD__
#define __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_FC1F6DD__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_converterAudioProcessorEditor  : public AudioProcessorEditor,
                                             public ComboBoxListener,
                                             public ButtonListener
{
public:
    //==============================================================================
    Ambix_converterAudioProcessorEditor (Ambix_converterAudioProcessor* ownerFilter);
    ~Ambix_converterAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ComboBox* box_in_ch_seq;
    Label* label;
    Label* label2;
    Label* label3;
    ComboBox* box_out_ch_seq;
    Label* label4;
    ComboBox* box_in_norm;
    ComboBox* box_out_norm;
    ToggleButton* tgl_invert_cs;
    ComboBox* box_presets;
    Label* label5;
    ToggleButton* tgl_flip;
    ToggleButton* tgl_flop;
    ToggleButton* tgl_flap;
    Label* label6;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_converterAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_FC1F6DD__
