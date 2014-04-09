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
                                            public ComboBoxListener,
                                            public ButtonListener
{
public:
    //==============================================================================
    Ambix_binauralAudioProcessorEditor (Ambix_binauralAudioProcessor* ownerFilter);
    ~Ambix_binauralAudioProcessorEditor();

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
    ScopedPointer<HyperlinkButton> hyperlinkButton;
    ScopedPointer<Label> label;
    ScopedPointer<ComboBox> box_presets;
    ScopedPointer<Label> label5;
    ScopedPointer<TextEditor> txt_debug;
    ScopedPointer<TextButton> btn_open;
    ScopedPointer<Label> label2;
    ScopedPointer<Label> label3;
    ScopedPointer<Label> label4;
    ScopedPointer<Label> num_ch;
    ScopedPointer<Label> num_spk;
    ScopedPointer<Label> num_hrtf;
    ScopedPointer<TextButton> btn_preset_folder;
    ScopedPointer<ToggleButton> tgl_load_irs;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_binauralAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_4A9AC0213502B6A5__
