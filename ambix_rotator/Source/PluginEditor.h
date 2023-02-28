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

#ifndef __JUCE_HEADER_545F3BF7D53A23F6__
#define __JUCE_HEADER_545F3BF7D53A23F6__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_rotatorAudioProcessorEditor  : public AudioProcessorEditor,
                                           public Slider::Listener,
                                           public Button::Listener,
                                           public ChangeListener,
                                           public TextEditor::Listener,
                                           public Timer
{
public:
    //==============================================================================
    Ambix_rotatorAudioProcessorEditor (Ambix_rotatorAudioProcessor* ownerFilter);
    ~Ambix_rotatorAudioProcessorEditor();

    //==============================================================================

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void textEditorFocusLost(TextEditor &) override;
    void textEditorReturnKeyPressed(TextEditor &) override;

    void changeListenerCallback (ChangeBroadcaster *source) override;

    void timerCallback() override;

private:
    LookAndFeel_V3 globalLaF;

    //==============================================================================
    Slider sld_yaw;
    Label label2;
    Label label3;
    Slider sld_pitch;
    Label label4;
    Slider sld_roll;
    Label label5;
    ToggleButton toggleButton;
    ToggleButton toggleButton2;
    TextEditor txt_q0;
    Label label7;
    Label label8;
    Label label6;
    TextEditor txt_q1;
    Label label9;
    TextEditor txt_q2;
    Label label10;
    TextEditor txt_q3;
    Label label11;
    ToggleButton tgl_qinvert;

    bool _changed;

    Ambix_rotatorAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_rotatorAudioProcessor*> (getAudioProcessor());
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_rotatorAudioProcessorEditor)
};

#endif   // __JUCE_HEADER_545F3BF7D53A23F6__
