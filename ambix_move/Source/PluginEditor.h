/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ==============================================================================
 */

#ifndef AMBIX_MOVE_PLUGINEDITOR_H_INCLUDED
#define AMBIX_MOVE_PLUGINEDITOR_H_INCLUDED

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "MoveView.h"

//==============================================================================
class Ambix_moveAudioProcessorEditor  : public AudioProcessorEditor,
                                              public Slider::Listener,
                                              public Button::Listener,
                                              public TextEditor::Listener,
                                              public ChangeListener,
                                              public Timer
{
public:
    Ambix_moveAudioProcessorEditor (Ambix_moveAudioProcessor* ownerFilter);
    ~Ambix_moveAudioProcessorEditor() override;

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void sliderDragStarted (Slider*) override;
    void sliderDragEnded (Slider*) override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void textEditorFocusLost (TextEditor&) override;
    void textEditorReturnKeyPressed (TextEditor&) override;
    void changeListenerCallback (ChangeBroadcaster* source) override;
    void timerCallback() override;

private:
    void saveUndoState();

    LookAndFeel_V3 globalLaF;

    MoveView view;

    // Translation
    Slider sld_x, sld_y, sld_z, sld_radius;
    Slider sld_in_order, sld_out_order;

    Label  lbl_trans_title;
    Label  lbl_x, lbl_y, lbl_z, lbl_radius;
    Label  lbl_in_order, lbl_out_order;

    // Rotation
    Slider sld_yaw, sld_pitch, sld_roll;
    Label  lbl_yaw, lbl_pitch, lbl_roll;
    Label  lbl_rot_title, lbl_euler_title, lbl_quat_title;

    ToggleButton tgl_ypr;
    ToggleButton tgl_rpy;

    TextEditor txt_q0, txt_q1, txt_q2, txt_q3;
    Label  lbl_q0, lbl_q1, lbl_q2, lbl_q3;

    ToggleButton tgl_qinvert;

    Label        lbl_osc_title;
    Label        lbl_osc_port;
    ToggleButton tgl_osc_enable;
    TextEditor   txt_osc_port;
    Label        lbl_osc_syntax;

    TextButton btn_reset;
    TextButton btn_undo;
    TextButton btn_redo;

    TooltipWindow tooltipWindow;
    ComponentBoundsConstrainer constrainer;

    bool _changed;

    // Undo / redo state
    struct ParamSnapshot
    {
        float params[Ambix_moveAudioProcessor::totalNumParams];
    };
    std::vector<ParamSnapshot> undoHistory;
    int  undoIndex      = -1;
    bool undoInProgress = false;

    Ambix_moveAudioProcessor* getProcessor() const
    {
        return static_cast<Ambix_moveAudioProcessor*> (getAudioProcessor());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_moveAudioProcessorEditor)
};

#endif  // AMBIX_MOVE_PLUGINEDITOR_H_INCLUDED
