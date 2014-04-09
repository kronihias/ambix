/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 May 2013 7:19:37pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_AMBIXSETTINGS_AMBIXSETTINGS_EB1482B8__
#define __JUCER_HEADER_AMBIXSETTINGS_AMBIXSETTINGS_EB1482B8__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AmbixSettings  :
{
public:
    //==============================================================================
    AmbixSettings ();
    ~AmbixSettings();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    TextEditor* txt_osc_out_ip;
    Label* label;
    Label* label2;
    TextEditor* txt_osc_out_ip2;
    TextEditor* txt_osc_out_ip3;
    Label* label3;
    Label* label4;
    HyperlinkButton* hyperlinkButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbixSettings)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AMBIXSETTINGS_AMBIXSETTINGS_EB1482B8__
