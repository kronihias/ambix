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

#define NUM_PRESETS 8

// AMBISONICS ORDER IS DEFINED IN AppConfig.h!!!!!

#define _2PI 6.2831853071795

#define LOGTEN 2.302585092994

inline float rmstodb(float rms)
{
    return 20.f/(float)LOGTEN * logf(rms);
}

inline float dbtorms(float db)
{
    return expf(((float)LOGTEN * 0.05f) * db);
}

// scale parameter for host between 0.0 and 1.0
inline float ParamToRMS(float param)
{
    float rms = 0.f;

    if (param < 0.f )
    {
        rms = 0.f;
    }
    else if ((param >= 0.f) && (param <= 0.75f))
    {
        rms = (param / 0.75f) * (param / 0.75f); // normalize and square
    }
    else if ((param > 0.75f) && (param < 1.f))
    {
        rms = ((param - 0.75f) / 0.25f) * ((param - 0.75f) / 0.25f) + 1.f;
    }
    else if (param >= 1.f)
    {
        rms = 2.f;
    }


    return rms;
}

// scale parameter for host between 0.0 and 1.0
inline float ParamToDB(float param)
{
    return rmstodb(ParamToRMS(param));
}

//==============================================================================
/**
*/
class Ambix_mirrorAudioProcessor  : public AudioProcessor,
                                    public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_mirrorAudioProcessor();
    ~Ambix_mirrorAudioProcessor();

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
		XEvenParam,
        XEvenInvParam,
        XOddParam,
        XOddInvParam,
		YEvenParam,
		YEvenInvParam,
        YOddParam,
        YOddInvParam,
		ZEvenParam,
		ZEvenInvParam,
        ZOddParam,
        ZOddInvParam,
        CircularParam,
        CircularInvParam,
        PresetParam,
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

    void SwitchPreset();

private:

    void calcParams();

    // actual values
    float x_even_param;
    float x_even_inv_param;
    float x_odd_param;
    float x_odd_inv_param;
    float y_even_param;
    float y_even_inv_param;
    float y_odd_param;
    float y_odd_inv_param;
    float z_even_param;
    float z_even_inv_param;
    float z_odd_param;
    float z_odd_inv_param;
    float circular_param;
    float circular_inv_param;

    float preset_param;

    String preset_name;

    Array<float> gain_factors;

    // old values
    Array<float> _gain_factors;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_mirrorAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_3C9F642__
