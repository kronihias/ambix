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
#include "HammerAitovGrid.h"
#include "WarpVisualizer.h"

//==============================================================================
class Ambix_warpAudioProcessorEditor  : public AudioProcessorEditor,
                                         public Slider::Listener,
                                         public Button::Listener,
                                         public ChangeListener,
                                         public Timer
{
public:
    Ambix_warpAudioProcessorEditor (Ambix_warpAudioProcessor* ownerFilter);
    ~Ambix_warpAudioProcessorEditor();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void sliderDragStarted (Slider* slider) override;
    void sliderDragEnded (Slider* slider) override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void changeListenerCallback (ChangeBroadcaster* source) override;
    void timerCallback() override;

private:
    void saveUndoState();

    LookAndFeel_V3 globalLaF;

    HammerAitovGrid grid;
    WarpVisualizer visualizer;

    Slider sld_phi;
    Slider sld_theta;
    Slider sld_in_order;
    Slider sld_out_order;

    ToggleButton btn_phi_curve;
    ToggleButton btn_theta_curve;
    ToggleButton btn_preemp;
    TextButton btn_undo;
    TextButton btn_redo;

    Label lbl_phi;
    Label lbl_theta;
    Label lbl_in_order;
    Label lbl_out_order;

    TooltipWindow tooltipWindow;
    ComponentBoundsConstrainer constrainer;

    bool _changed;

    // Undo/redo state
    struct ParamSnapshot
    {
        float params[Ambix_warpAudioProcessor::totalNumParams];
    };
    std::vector<ParamSnapshot> undoHistory;
    int undoIndex = -1;
    bool undoInProgress = false;

    Ambix_warpAudioProcessor* getProcessor() const
    {
        return static_cast<Ambix_warpAudioProcessor*> (getAudioProcessor());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_warpAudioProcessorEditor)
};

#endif  // PLUGINEDITOR_H_INCLUDED
