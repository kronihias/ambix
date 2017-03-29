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
                                           public SliderListener,
                                           public ButtonListener,
                                           public ChangeListener,
                                           public TextEditorListener,
                                           public Timer
{
public:
    //==============================================================================
    Ambix_rotatorAudioProcessorEditor (Ambix_rotatorAudioProcessor* ownerFilter);
    ~Ambix_rotatorAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);
    void textEditorFocusLost(TextEditor &);
    void textEditorReturnKeyPressed(TextEditor &);

    void changeListenerCallback (ChangeBroadcaster *source);

    void timerCallback();
    
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Slider> sld_yaw;
    ScopedPointer<Label> label2;
    ScopedPointer<Label> label3;
    ScopedPointer<Slider> sld_pitch;
    ScopedPointer<Label> label4;
    ScopedPointer<Slider> sld_roll;
    ScopedPointer<Label> label5;
    ScopedPointer<ToggleButton> toggleButton;
    ScopedPointer<ToggleButton> toggleButton2;
    ScopedPointer<TextEditor> txt_q0;
    ScopedPointer<Label> label7;
    ScopedPointer<Label> label8;
    ScopedPointer<Label> label6;
    ScopedPointer<TextEditor> txt_q1;
    ScopedPointer<Label> label9;
    ScopedPointer<TextEditor> txt_q2;
    ScopedPointer<Label> label10;
    ScopedPointer<TextEditor> txt_q3;
    ScopedPointer<Label> label11;
    ScopedPointer<ToggleButton> tgl_qinvert;

    bool _changed;
    
    Ambix_rotatorAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_rotatorAudioProcessor*> (getAudioProcessor());
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_rotatorAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_545F3BF7D53A23F6__
