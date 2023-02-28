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

#ifndef __JUCER_HEADER_AMBIXBINAURALAUDIOPROCESSOREDITOR_PLUGINEDITOR_378A6A50__
#define __JUCER_HEADER_AMBIXBINAURALAUDIOPROCESSOREDITOR_PLUGINEDITOR_378A6A50__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "MyMeter/MyMeter.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_binauralAudioProcessorEditor  : public AudioProcessorEditor,
                                            public Button::Listener,
                                            public Timer,
                                            public ChangeListener,
                                            public ComboBox::Listener,
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
    void buttonClicked (Button* buttonThatWasClicked) override;

    void changeListenerCallback (ChangeBroadcaster *source) override;

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    static void menuItemChosenCallback (int result, Ambix_binauralAudioProcessorEditor* demoComponent);

    void UpdatePresets();

    void timerCallback() override;

    void UpdateText();

    void DrawMeters();
    void UpdateMeters();

private:

    Ambix_binauralAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_binauralAudioProcessor*> (getAudioProcessor());
    }
    LookAndFeel_V3 globalLaF;
    TooltipWindow tooltipWindow;

    //==============================================================================
    Label label;
    TextEditor txt_preset;
    PopupMenu popup_presets;
    OwnedArray<PopupMenu> popup_submenu;
    Label label5;
    TextEditor txt_debug;
    TextButton btn_open;
    Label label2;
    Label label3;
    Label label4;
    Label num_ch;
    Label num_spk;
    Label num_hrtf;
    TextButton btn_preset_folder;
    Slider sld_gain;
    ToggleButton tgl_save_preset;

#if BINAURAL_DECODER
    ToggleButton tgl_load_irs;
    ComboBox box_conv_buffer;
#endif

    OwnedArray<MyMeter> _meters;

    OwnedArray<Label> _labels;

    OwnedArray<MyMeterScale> _scales;

    int _width;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_binauralAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AMBIXBINAURALAUDIOPROCESSOREDITOR_PLUGINEDITOR_378A6A50__
