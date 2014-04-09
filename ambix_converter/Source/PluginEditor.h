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

#ifndef __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_910E8126__
#define __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_910E8126__

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
class Ambix_converterAudioProcessorEditor  : public AudioProcessorEditor,
                                            public ComboBoxListener,
                                            public ButtonListener,
                                            public ChangeListener
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
    
    void getParamsFromHost();
    
    void changeListenerCallback (ChangeBroadcaster *source);
    
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    Ambix_converterAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_converterAudioProcessor*> (getAudioProcessor());
    }
    TooltipWindow tooltipWindow;
        
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<HyperlinkButton> hyperlinkButton;
    ScopedPointer<ComboBox> box_in_ch_seq;
    ScopedPointer<Label> label;
    ScopedPointer<Label> label2;
    ScopedPointer<Label> label3;
    ScopedPointer<ComboBox> box_out_ch_seq;
    ScopedPointer<Label> label4;
    ScopedPointer<ComboBox> box_in_norm;
    ScopedPointer<ComboBox> box_out_norm;
    ScopedPointer<ToggleButton> tgl_invert_cs;
    ScopedPointer<ComboBox> box_presets;
    ScopedPointer<Label> label5;
    ScopedPointer<ToggleButton> tgl_flip;
    ScopedPointer<ToggleButton> tgl_flop;
    ScopedPointer<ToggleButton> tgl_flap;
    ScopedPointer<Label> label6;
    
    ScopedPointer<ToggleButton> tgl_in_2d;
    ScopedPointer<ToggleButton> tgl_out_2d;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_converterAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_910E8126__
