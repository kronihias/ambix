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

#ifndef __PLUGINPROCESSOR_H_58CE358E__
#define __PLUGINPROCESSOR_H_58CE358E__

#include "JuceHeader.h"

#include "AmbixEncoder.h"

#include "MyMeterDsp/MyMeterDsp.h"

#define _2PI 6.2831853071795

//==============================================================================
/**
*/
class Ambix_encoderAudioProcessor  : public AudioProcessor,
#if WITH_OSC
                                    public Timer,
                                    private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::RealtimeCallback>,
#endif
                                    public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_encoderAudioProcessor();
    ~Ambix_encoderAudioProcessor();

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
    const String getParameterLabel(int index);

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    void calcAzimuth();

#if WITH_ADVANCED_CONTROL
    void calcNewParameters(double SampleRate, int BufferLength);
#endif

    enum Parameters
	{
		AzimuthParam,
        ElevationParam,
        SizeParam,
#if INPUT_CHANNELS > 1
        WidthParam, // if multiple sources
#endif
        AzimuthSetParam, // for advanced control...
        AzimuthSetRelParam,
        AzimuthMvParam,
        ElevationSetParam,
        ElevationSetRelParam,
        ElevationMvParam,
        SpeedParam
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


    int m_id; // id of this instance

    static int s_ID; // global instance counter

#if WITH_OSC
    void timerCallback() override; // call osc send in timer callback

    // JUCE OSC
    void oscMessageReceived (const OSCMessage& message) override;

    void sendOSC(); // send osc data

    void oscOut(bool arg); // activate osc out
    void oscIn(bool arg); // activate osc in

    void changeTimer(int time);

    // osc stuff
    bool osc_in;
	bool osc_out;
    int osc_interval;

	String osc_in_port, osc_out_ip, osc_out_port;

#endif

    ApplicationProperties myProperties;

private:
    OwnedArray<AmbixEncoder> AmbiEnc;

    double SampleRate;

    unsigned int NumParameters;

    float azimuth_param; // for multiple inputs this is the center
    float elevation_param;
    float size_param;
    float width_param;  // arrange sources with equal angular distance

    // last osc value sent...
    float _azimuth_param; // for multiple inputs this is the center
    float _elevation_param;
    float _size_param;
    float _rms;
    float _dpk;

    // advanced parameter control
    float speed_param;
    float azimuth_set_param, azimuth_set_rel_param, azimuth_mv_param;
    float elevation_set_param, elevation_set_rel_param, elevation_mv_param;

    AudioSampleBuffer InputBuffer;

    MyMeterDsp _my_meter_dsp;

    float rms; // rms of W channel
    float dpk; // peak value of W channel

#if WITH_OSC
    OSCReceiver oscReceiver;

    OwnedArray<OSCSender> oscSenders;

#endif
    /*
    void calcParams();

    Array<float> ambi_gain;
    Array<float> _ambi_gain; // buffer for gain ramp
    */
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_encoderAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_58CE358E__
