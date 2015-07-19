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

#ifndef __PLUGINPROCESSOR_H_B809653C__
#define __PLUGINPROCESSOR_H_B809653C__

#include "JuceHeader.h"
#include "AmbiSpeaker.h"

#if BINAURAL_DECODER

    #include "MtxConv.h"
    #include "ConvolverData.h"
#endif

//==============================================================================
/**
*/
class Ambix_binauralAudioProcessor  : public AudioProcessor,
                                      public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_binauralAudioProcessor();
    ~Ambix_binauralAudioProcessor();

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

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    
    void LoadConfiguration(File configFile); // do the loading
    
    void UnloadConfiguration();
    void ReloadConfiguration();
    
    unsigned int getBufferSize();
    unsigned int getConvBufferSize();
    void setConvBufferSize(unsigned int bufsize);
    
    OwnedArray<AmbiSpeaker> _AmbiSpeakers;

#if BINAURAL_DECODER
    // OwnedArray<SpkConv> _SpkConv;
    int num_conv;
#endif
    
    int _AmbiChannels;
    
    bool configLoaded; // is a configuration successfully loaded?
    
    void DebugPrint(String debugText);
    
    String _DebugText;
    
    // for gui
    
    void SearchPresets(File SearchFolder);
    
    void LoadPreset(unsigned int preset);
    
    void LoadPresetByName(String presetName);
    
    File presetDir; // where to search for presets
    File lastDir; // for open file dialog...
    
    String activePreset; // store filename
    
    Array<File> _presetFiles;
    
    String box_preset_str;

    bool _load_ir; // normally false, true if you don't want to load new impulse resonses (for fast decoder matrix switching)
    
private:
    
    AudioSampleBuffer ambi_spk_buffer_;
    
    File _configFile;
    
#if BINAURAL_DECODER
    bool loadIr(AudioSampleBuffer* IRBuffer, const File& audioFile, double &samplerate, float gain=1.f, int offset=0, int length=0);
    
    ConvolverData conv_data;
    
    MtxConvMaster mtxconv_;
    
#endif
    
    double SampleRate;
    int BufferSize;
    
    int ConvBufferSize;
    
    bool isProcessing;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_binauralAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_B809653C__
