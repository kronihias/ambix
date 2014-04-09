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

#ifndef __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_10AAA45__
#define __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_10AAA45__


#include "JuceHeader.h"
#include "PluginProcessor.h"



//==============================================================================

class Ambix_mirrorAudioProcessorEditor  : public AudioProcessorEditor,
                                          public ComboBoxListener,
                                          public SliderListener,
                                          public ButtonListener,
                                          public ChangeListener
{
public:
    //==============================================================================
    Ambix_mirrorAudioProcessorEditor (Ambix_mirrorAudioProcessor* ownerFilter);
    ~Ambix_mirrorAudioProcessorEditor();

    //==============================================================================


    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);
    
    void getParamsFromHost();
    
    void changeListenerCallback (ChangeBroadcaster *source);

    // Binary resources:
    static const char* coordinate_system_png;
    static const int coordinate_system_pngSize;
    
private:
    
    Ambix_mirrorAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_mirrorAudioProcessor*> (getAudioProcessor());
    }
    TooltipWindow tooltipWindow;
    
    //==============================================================================
    ScopedPointer<ComboBox> box_presets;
    ScopedPointer<Label> label5;
    ScopedPointer<Slider> sld_x_even;
    ScopedPointer<Label> label;
    ScopedPointer<Label> label2;
    ScopedPointer<ToggleButton> tgl_x_even_inv;
    ScopedPointer<Slider> sld_x_odd;
    ScopedPointer<Label> label3;
    ScopedPointer<ToggleButton> tgl_x_odd_inv;
    ScopedPointer<Slider> sld_y_even;
    ScopedPointer<Label> label6;
    ScopedPointer<ToggleButton> tgl_y_even_inv;
    ScopedPointer<Slider> sld_y_odd;
    ScopedPointer<Label> label7;
    ScopedPointer<ToggleButton> tgl_y_odd_inv;
    ScopedPointer<Slider> sld_z_even;
    ScopedPointer<Label> label9;
    ScopedPointer<ToggleButton> tgl_z_even_inv;
    ScopedPointer<Slider> sld_z_odd;
    ScopedPointer<Label> label10;
    ScopedPointer<ToggleButton> tgl_z_odd_inv;
    ScopedPointer<Label> lbl_x_even;
    ScopedPointer<Label> lbl_x_odd;
    ScopedPointer<Label> lbl_y_even;
    ScopedPointer<Label> lbl_y_odd;
    ScopedPointer<Label> lbl_z_even;
    ScopedPointer<Label> lbl_z_odd;
    ScopedPointer<Label> label4;
    ScopedPointer<Label> label8;
    ScopedPointer<Slider> sld_circular;
    ScopedPointer<Label> label11;
    ScopedPointer<ToggleButton> tgl_circular_inv;
    ScopedPointer<Label> lbl_circular;
    Image cachedImage_coordinate_system_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_mirrorAudioProcessorEditor)
};


#endif   // __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_10AAA45__
