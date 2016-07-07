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

#ifndef __JUCER_HEADER_AMBIXENCODERAUDIOPROCESSOREDITOR_PLUGINEDITOR_EF118A61__
#define __JUCER_HEADER_AMBIXENCODERAUDIOPROCESSOREDITOR_PLUGINEDITOR_EF118A61__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "SphereOpenGL.h"
#include "Settings.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Ambix_encoderAudioProcessorEditor  : public AudioProcessorEditor,
                                           public SliderListener,
                                           public ChangeListener,
                                           public ButtonListener,
                                           public Timer,
                                           public TextEditorListener
{
public:
    //==============================================================================
    Ambix_encoderAudioProcessorEditor (Ambix_encoderAudioProcessor* ownerFilter);
    ~Ambix_encoderAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);
  
    void textEditorFocusLost (TextEditor& ed);
    void textEditorReturnKeyPressed (TextEditor& ed);
  
    void changeListenerCallback (ChangeBroadcaster *source);
    
    void modifierKeysChanged (const ModifierKeys &modifiers );
    
    void timerCallback();
  
    void updateID();
    // Binary resources:
    static const char* settings_png;
    static const int settings_pngSize;
    static const char* settings_white_png;
    static const int settings_white_pngSize;
    
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Slider> sld_el;
    ScopedPointer<Slider> sld_az;
    ScopedPointer<Slider> sld_size;
#if INPUT_CHANNELS > 1
    ScopedPointer<Slider> sld_width;
#endif
    ScopedPointer<Slider> sld_speed;
    ScopedPointer<Slider> sld_el_move;
    ScopedPointer<Slider> sld_az_move;
    ScopedPointer<TextEditor> txt_az_move;
    ScopedPointer<TextEditor> txt_el_move;
    ScopedPointer<Label> lbl_id;
    ScopedPointer<ImageButton> btn_settings;
    ScopedPointer<TextEditor> txt_id;
  
    ScopedPointer<SphereOpenGL> sphere_opengl;
    
    bool changed_; // this is true if parameters have changed
    
    CriticalSection     lock_;              // lock critical section
    
    juce::Component::SafePointer<juce::DialogWindow> _settingsDialogWindow;
    
    Ambix_encoderAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_encoderAudioProcessor*> (getAudioProcessor());
    }
    
    TooltipWindow tooltipWindow;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_encoderAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AMBIXENCODERAUDIOPROCESSOREDITOR_PLUGINEDITOR_EF118A61__
