/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  30 Apr 2013 10:30:32am

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_METERCOMPONENT_TEST_92438B50__
#define __JUCER_HEADER_METERCOMPONENT_TEST_92438B50__

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
class MeterComponent  : public Component
{
public:
    //==============================================================================
    MeterComponent ();
    ~MeterComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;

    // Binary resources:
    static const char* meter_gradient_png;
    static const int meter_gradient_pngSize;
    static const char* meter_gradient_off_png;
    static const int meter_gradient_off_pngSize;
    static const char* meter_scale_png;
    static const int meter_scale_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    Image cachedImage_meter_gradient_png;
    Image cachedImage_meter_gradient_off_png;
    Image cachedImage_meter_scale_png;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_METERCOMPONENT_TEST_92438B50__
