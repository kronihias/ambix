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

#ifndef __PLUGINPROCESSOR_H_9173E00A__
#define __PLUGINPROCESSOR_H_9173E00A__

#include "JuceHeader.h"


//==============================================================================
/**
*/
class Ambix_converterAudioProcessor  : public AudioProcessor,
                                       public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_converterAudioProcessor();
    ~Ambix_converterAudioProcessor();

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
		InSeqParam,
        OutSeqParam,
        InNormParam,
		OutNormParam,
        FlipCsParam,
        FlipParam,
        FlopParam,
        FlapParam,
        In2DParam,
        Out2DParam,
		totalNumParams
	};

    /*
    // for later.... mixed order
     InHOrderParam,
     OutHOrderParam,
     InVOrderParam,
     OutVOrderParam,
     */

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    unsigned int active_preset; // for gui...

    String box_presets_text; // for save state...

private:
    void setNormalizationScheme();


    void setChannelSequence();

    AudioSampleBuffer output_buffer;
    const unsigned int* in_ch_seq;
    const unsigned int* out_ch_seq;

    const unsigned int* in_2d_ch_seq;
    const unsigned int* out_2d_ch_seq;

    const float* in_ch_norm; // holds normalization conversion array

    const int* acn_cs; // holds condon-shortley conversion map

    float in_seq_param; // channel sequence acn, fuma, sid
    float out_seq_param;
    float in_norm_param;
    float out_norm_param;

    bool flip_cs_phase; // flip condon-shortley phase -> apply (-1)^m to output channels

    bool flip_param; // mirror left-right
    bool flop_param; // mirror front-back
    bool flap_param; // mirror ceiling-floor

    bool in_2d; // input is 2d ambisonics (fill with empty channels)
    bool out_2d; // output is 2d ambisonics (channels will be thrown away)

    bool ch_norm_flat;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_converterAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_9173E00A__
