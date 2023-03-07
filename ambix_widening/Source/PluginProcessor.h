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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "JuceHeader.h"

// approximation order
#define BESSEL_APPR 8
// #define TRUNCATE 0.0316227766016838 // set coefficients to zero below -30dB
// #define TRUNCATE 0.01 // -40dB
#define TRUNCATE 0.001 // -60dB

#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

//==============================================================================
/**
*/
class Ambix_wideningAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Ambix_wideningAudioProcessor();
    ~Ambix_wideningAudioProcessor();

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

    int getLatencySamples() const;

    enum Parameters
	{
        ModDepthParam,
		ModTParam,
        RotOffsetParam,
		SingleSideParam,
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

private:

    void calcParams();
    void checkBuffer();

    float mod_depth_param; // 0.0 -> 0deg, 1.0 -> 360deg

    float mod_t_param, _mod_t_param; // 0.0 ->

    // t = Q/samplerate ;  Q = t*samplerate;
    // Q=10....3000 @ 44.1kHz -> t = 0.2 ms....70ms

    float rot_offset_param; // 0.0 -> -180deg, 1.0 -> 180deg

    bool single_sided;

    bool param_changed; // mod_depth or offset changed

    int Q;

    AudioSampleBuffer ring_buffer;

    int _buf_write_pos;
    int _buf_read_pos[2*BESSEL_APPR+1];
    int _buf_size;

    float cos_coeffs[AMBI_ORDER][BESSEL_APPR+1];
    float _cos_coeffs[AMBI_ORDER][BESSEL_APPR+1];

    float sin_coeffs[AMBI_ORDER][BESSEL_APPR+1];
    float _sin_coeffs[AMBI_ORDER][BESSEL_APPR+1];


    //OwnedArray<Array<float>> cos_coeffs;
    //OwnedArray<Array<float>> sin_coeffs;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_wideningAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
