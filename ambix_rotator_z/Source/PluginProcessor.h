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

#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

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
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    int getNumParameters() override;

    float getParameter (int index) override;
    void setParameter (int index, float newValue) override;

    const String getParameterName (int index) override;
    const String getParameterText (int index) override;

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    enum Parameters
	{
		RotZParam,
		totalNumParams
	};
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

#ifdef WITH_OSC

	String osc_in_port;

    // JUCE OSC
    void oscMessageReceived (const OSCMessage& message) override;
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
