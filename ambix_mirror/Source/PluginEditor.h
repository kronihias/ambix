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
                                          public ComboBox::Listener,
                                          public Slider::Listener,
                                          public Button::Listener,
                                          public ChangeListener
{
public:
    //==============================================================================
    Ambix_mirrorAudioProcessorEditor (Ambix_mirrorAudioProcessor* ownerFilter);
    ~Ambix_mirrorAudioProcessorEditor();

    //==============================================================================


    void paint (Graphics& g) override;
    void resized() override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void buttonClicked (Button* buttonThatWasClicked) override;

    void getParamsFromHost();

    void changeListenerCallback (ChangeBroadcaster *source) override;

    // Binary resources:
    static const char* coordinate_system_png;
    static const int coordinate_system_pngSize;

private:
    LookAndFeel_V3 globalLaF;

    Ambix_mirrorAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_mirrorAudioProcessor*> (getAudioProcessor());
    }
    TooltipWindow tooltipWindow;

    //==============================================================================
    ComboBox box_presets;
    Label label5;
    Slider sld_x_even;
    Label label;
    Label label2;
    ToggleButton tgl_x_even_inv;
    Slider sld_x_odd;
    Label label3;
    ToggleButton tgl_x_odd_inv;
    Slider sld_y_even;
    Label label6;
    ToggleButton tgl_y_even_inv;
    Slider sld_y_odd;
    Label label7;
    ToggleButton tgl_y_odd_inv;
    Slider sld_z_even;
    Label label9;
    ToggleButton tgl_z_even_inv;
    Slider sld_z_odd;
    Label label10;
    ToggleButton tgl_z_odd_inv;
    Label lbl_x_even;
    Label lbl_x_odd;
    Label lbl_y_even;
    Label lbl_y_odd;
    Label lbl_z_even;
    Label lbl_z_odd;
    Label label4;
    Label label8;
    Slider sld_circular;
    Label label11;
    ToggleButton tgl_circular_inv;
    Label lbl_circular;
    Image cachedImage_coordinate_system_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_mirrorAudioProcessorEditor)
};


#endif   // __JUCER_HEADER_AMBIXMIRRORAUDIOPROCESSOREDITOR_PLUGINEDITOR_10AAA45__
