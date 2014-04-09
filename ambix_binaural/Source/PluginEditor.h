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
                                            public ButtonListener,
                                            public Timer,
                                            public ChangeListener
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
    void buttonClicked (Button* buttonThatWasClicked);

    void changeListenerCallback (ChangeBroadcaster *source);
    
    static void menuItemChosenCallback (int result, Ambix_binauralAudioProcessorEditor* demoComponent);
    
    void UpdatePresets();
    
    void timerCallback();

    void UpdateText();
    
    void DrawMeters();
    void UpdateMeters();
    
private:
    
    Ambix_binauralAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_binauralAudioProcessor*> (getAudioProcessor());
    }
    TooltipWindow tooltipWindow;

    //==============================================================================
    ScopedPointer<Label> label;
    ScopedPointer<TextEditor> txt_preset;
    PopupMenu popup_presets;
    OwnedArray<PopupMenu> popup_submenu;
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
    
#if BINAURAL_DECODER
    ScopedPointer<ToggleButton> tgl_load_irs;
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
