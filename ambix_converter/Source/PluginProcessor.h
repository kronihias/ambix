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
#include "ambix_buses.h"

#include "AmbisonicConfig.h"
#include "../../common/AmbixParameters.h"

//==============================================================================
/**
*/
class Ambix_converterAudioProcessor  : public AudioProcessor,
                                       public ambix::ParameterOwner,
                                       public ChangeBroadcaster // numChannelsChanged() only
{
public:
    //==============================================================================
    Ambix_converterAudioProcessor();
    ~Ambix_converterAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

   #ifdef UNIVERSAL_AMBISONIC
    AMBIX_APPLY_BUS_LAYOUTS_OVERRIDE
   #endif
    void numChannelsChanged() override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    // the parameter objects, indexed by the Parameters enum below
    ambix::Parameter& getParam (int index) const { return *params[index]; }

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
    // legacy setParameter() body: nothing left to do here, the conversion
    // tables are picked from the parameter values in processBlock()
    void parameterValueSet (int index, float newValue) override;

    // The parameter objects (owned by AudioProcessor). Their stored values
    // are what the legacy members held:
    //   seq/norm: 0, 0.5 or 1 (channel sequence acn, fuma, sid /
    //             normalization sn3d, fuma, n3d)
    //   switches: 0 or 1 (flip condon-shortley phase -> apply (-1)^m to
    //             output channels, mirror left-right, front-back,
    //             ceiling-floor, input is 2d (fill with empty channels),
    //             output is 2d (channels will be thrown away))
    ambix::Parameter* params[totalNumParams] {};

    AudioSampleBuffer output_buffer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_converterAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_9173E00A__
