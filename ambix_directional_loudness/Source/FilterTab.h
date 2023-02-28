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

#ifndef __ambix_directional_loudness__FilterTab__
#define __ambix_directional_loudness__FilterTab__

#include "JuceHeader.h"
#include "PluginProcessor.h"

class FilterTab :   public Component,
                    public Slider::Listener,
                    public ComboBox::Listener,
                    public Button::Listener
{

public:
    FilterTab(int id, Ambix_directional_loudnessAudioProcessor* myProcessor);
    ~FilterTab();

    void paint (Graphics& g) override;

    void resized() override;

    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    void setFilter(float az, float el, bool shape, float width, float height, float gain, bool solo);

    // Binary resources:
    static const char* solo_symbol_png;
    static const int solo_symbol_pngSize;
    static const char* solo_symbol_over_png;
    static const int solo_symbol_over_pngSize;
    static const char* solo_symbol_act_png;
    static const int solo_symbol_act_pngSize;

private:
    int _id; // filter id

    Slider sld_az;
    Slider sld_el;
    ComboBox box_shape;
    Slider sld_w;
    Slider sld_h;
    ImageButton btn_solo;
    Slider sld_gain;

    TooltipWindow tooltipWindow;

    Ambix_directional_loudnessAudioProcessor* _myProcessor;
};

#endif /* defined(__ambix_directional_loudness__FilterTab__) */
