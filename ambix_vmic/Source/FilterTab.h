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
                    public SliderListener,
                    public ComboBoxListener
{
    
public:
    FilterTab(int id, Ambix_vmicAudioProcessor* myProcessor);
    ~FilterTab();
    
    void paint (Graphics& g);
    
    void resized();
    
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    
    void setFilter(float az, float el, bool shape, float width, float height, float gain);
    
private:
    int _id; // filter id
    
    ScopedPointer<Slider> sld_az;
    ScopedPointer<Slider> sld_el;
    ScopedPointer<ComboBox> box_shape;
    ScopedPointer<Slider> sld_w;
    ScopedPointer<Slider> sld_h;
    ScopedPointer<Slider> sld_gain;
    
    TooltipWindow tooltipWindow;
    
    Ambix_vmicAudioProcessor* _myProcessor;
};

#endif /* defined(__ambix_directional_loudness__FilterTab__) */
