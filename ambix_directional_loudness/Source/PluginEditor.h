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

#ifndef __JUCE_HEADER_CDA58EC13A3FDFE9__
#define __JUCE_HEADER_CDA58EC13A3FDFE9__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "PanningGraph.h"
#include "FilterTab.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_directional_loudnessAudioProcessorEditor  : public AudioProcessorEditor,
                                                        public ChangeListener,
                                                        public ButtonListener
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
    
    void changeListenerCallback (ChangeBroadcaster *source);
    
    void buttonClicked (Button* buttonThatWasClicked);

    void selectFilterTab(int id);

    // Binary resources:
    static const char* solo_symbol_png;
    static const int solo_symbol_pngSize;
    static const char* solo_symbol_over_png;
    static const int solo_symbol_over_pngSize;
    static const char* solo_symbol_act_png;
    static const int solo_symbol_act_pngSize;
    
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> lbl_gd;
    ScopedPointer<Component> filtergraph;
    
    ScopedPointer<TabbedComponent> tabbedComponent;
    ScopedPointer<TabbedComponent> tabbedComponent2;
    
    OwnedArray<FilterTab> _filtertabs;
    
    ScopedPointer<PanningGraph> panninggraph;
    
    ScopedPointer<ImageButton> btn_solo_reset;
    
    TooltipWindow tooltipWindow;
    
    Ambix_directional_loudnessAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_directional_loudnessAudioProcessor*> (getAudioProcessor());
    }
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_directional_loudnessAudioProcessorEditor)
};


#endif   // __JUCE_HEADER_CDA58EC13A3FDFE9__
