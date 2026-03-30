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

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "WideningVisualizer.h"
#include "ImpulseResponseDisplay.h"

//==============================================================================
class Ambix_wideningAudioProcessorEditor : public AudioProcessorEditor,
                                            public Slider::Listener,
                                            public Button::Listener,
                                            public ChangeListener,
                                            public Timer
{
public:
    Ambix_wideningAudioProcessorEditor (Ambix_wideningAudioProcessor* ownerFilter);
    ~Ambix_wideningAudioProcessorEditor();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void changeListenerCallback (ChangeBroadcaster* source) override;
    void timerCallback() override;

private:
    LookAndFeel_V3 globalLaF;

    WideningVisualizer visualizer;
    ImpulseResponseDisplay irDisplay;

    Slider sld_mod_depth;
    Slider sld_mod_t;
    Slider sld_rot_offset;
    ToggleButton btn_single_sided;

    Label lbl_mod_depth;
    Label lbl_mod_t;
    Label lbl_rot_offset;

    TooltipWindow tooltipWindow;

    bool _changed;

    Ambix_wideningAudioProcessor* getProcessor() const
    {
        return static_cast<Ambix_wideningAudioProcessor*> (getAudioProcessor());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_wideningAudioProcessorEditor)
};

#endif  // PLUGINEDITOR_H_INCLUDED
