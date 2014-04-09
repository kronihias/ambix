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

#include "../JuceLibraryCode/JuceHeader.h"

// approximation order
#define BESSEL_APPR 8
// #define TRUNCATE 0.0316227766016838 // set coefficients to zero below -30dB
// #define TRUNCATE 0.01 // -40dB
#define TRUNCATE 0.001 // -60dB

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
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

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
