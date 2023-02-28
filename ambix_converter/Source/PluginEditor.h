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

#include "JuceHeader.h"
#include "PluginProcessor.h"



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_converterAudioProcessorEditor  : public AudioProcessorEditor,
                                            public ComboBox::Listener,
                                            public Button::Listener,
                                            public ChangeListener
{
public:
    //==============================================================================
    Ambix_converterAudioProcessorEditor (Ambix_converterAudioProcessor* ownerFilter);
    ~Ambix_converterAudioProcessorEditor();

    //==============================================================================

    void paint (Graphics& g) override;
    void resized() override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    void getParamsFromHost();

    void changeListenerCallback (ChangeBroadcaster *source) override;

private:
    Ambix_converterAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_converterAudioProcessor*> (getAudioProcessor());
    }
    LookAndFeel_V3 globalLaF;
    TooltipWindow tooltipWindow;


    //==============================================================================
    HyperlinkButton hyperlinkButton;
    ComboBox box_in_ch_seq;
    Label label;
    Label label2;
    Label label3;
    ComboBox box_out_ch_seq;
    Label label4;
    ComboBox box_in_norm;
    ComboBox box_out_norm;
    ToggleButton tgl_invert_cs;
    ComboBox box_presets;
    Label label5;
    ToggleButton tgl_flip;
    ToggleButton tgl_flop;
    ToggleButton tgl_flap;
    Label label6;

    ToggleButton tgl_in_2d;
    ToggleButton tgl_out_2d;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_converterAudioProcessorEditor)
};

#endif   // __JUCER_HEADER_AMBIXCONVERTERAUDIOPROCESSOREDITOR_PLUGINEDITOR_910E8126__
