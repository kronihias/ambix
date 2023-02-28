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

#ifndef __PLUGINEDITOR_H_E3DADBB1__
#define __PLUGINEDITOR_H_E3DADBB1__

#include "JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class Ambix_rotator_zAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Ambix_rotator_zAudioProcessorEditor (Ambix_rotator_zAudioProcessor* ownerFilter);
    ~Ambix_rotator_zAudioProcessorEditor();

    //==============================================================================
    // This is just a standard Juce paint method...
    void paint (Graphics& g) override;
};


#endif  // __PLUGINEDITOR_H_E3DADBB1__
