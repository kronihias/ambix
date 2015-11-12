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

#ifndef __PLUGINPROCESSOR_H_3C9F642__
#define __PLUGINPROCESSOR_H_3C9F642__

#include "JuceHeader.h"

// AMBISONICS ORDER IS DEFINED IN AppConfig.h!!!!!

#define _2PI 6.2831853071795

//==============================================================================
/**
*/
class Ambix_rotator_zAudioProcessor  : public AudioProcessor
#ifdef WITH_OSC
                                       ,
                                       private OSCReceiver,
                                       private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
#endif
{
public:
    //==============================================================================
    Ambix_rotator_zAudioProcessor();
    ~Ambix_rotator_zAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;
    
    enum Parameters
	{
		RotZParam,
		totalNumParams
	};
    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

#ifdef WITH_OSC

	String osc_in_port;
    
    // JUCE OSC
    void oscMessageReceived (const OSCMessage& message);
#endif
    
private:
    
    float rot_z_param;
    
    // actual values
    Array<float> sin_z;
    Array<float> cos_z;
    
    // old values
    Array<float> _sin_z;
    Array<float> _cos_z;
    
    AudioSampleBuffer output_buffer;
    
    void calcParams();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_rotator_zAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_3C9F642__
