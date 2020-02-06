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
    #if WITH_ZITA_CONVOLVER
        #include <zita-convolver.h>
        
        #define CONVPROC_SCHEDULER_PRIORITY 0
        #define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
        #define THREAD_SYNC_MODE true
    #else
        #include "MtxConv.h"
    #endif
    
    #include "ConvolverData.h"
#endif


#define LOGTEN 2.302585092994

inline float rmstodb(float rms)
{
    return (float) (20.f/LOGTEN * logf(rms));
}

inline float dbtorms(float db)
{
    return expf((float) (LOGTEN * 0.05f) * db);
}

// scale parameter for host between 0.0 and 1.0 to -inf dB ... +20dB
inline float ParamToRMS(float param)
{
    float rms = 0.f;
    
    if (param < 0.f )
    {
        rms = 0.f;
    }
    else if ((param >= 0.f) && (param <= 0.5f))
    {
        rms = (param * 2) * (param * 2); // normalize and square
    }
    else if ((param > 0.5f) && (param < 1.f))
    {
        rms = ((param - 0.5f) / 0.5f) * ((param - 0.5f) / 0.5f) * 9.f + 1.f;
    }
    else if (param >= 1.f)
    {
        rms = 10.f; // +20 dB
    }
    
    return rms;
}

// scale parameter for host between 0.0 and 1.0
inline float ParamToDB(float param)
{
    return rmstodb(ParamToRMS(param));
}


inline float DbToParam(float db)
{
    if (db <= -99.f) {
        return 0.f;
    }
    else if (db <= 0.f)
    {
        return sqrt(dbtorms(db))*0.5f;
    }
    else// if (db > 0.f)
    {
        // return    0.5f+sqrt(dbtorms(db)-1.f);
        
        return sqrt((dbtorms(db) - 1.f) / 9.f)*0.5f+0.5f;
    }
}

//==============================================================================
/**
*/
class Ambix_binauralAudioProcessor  : public AudioProcessor,
                                      public ChangeBroadcaster,
                                      public Thread
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

    // use a thread to load a configuration
    void run();

    // do the loading in a background thread
    void LoadConfigurationAsync(File configFile);

    void LoadConfiguration(File configFile); // do the loading
    
    void UnloadConfiguration();
    void ReloadConfiguration();

    // for gui
    bool SaveConfiguration(File zipFile);
    Atomic<int> _readyToSaveConfiguration;

    unsigned int getBufferSize();
    unsigned int getConvBufferSize();
    void setConvBufferSize(unsigned int bufsize);
    
    OwnedArray<AmbiSpeaker> _AmbiSpeakers;

#if BINAURAL_DECODER
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

    Atomic<int> _storeConfigDataInProject;

    File presetDir; // where to search for presets
    File lastDir; // for open file dialog...
    
    String activePreset; // store filename
    
    Array<File> _presetFiles;
    
    String box_preset_str;

    bool _load_ir; // normally false, true if you don't want to load new impulse resonses (for fast decoder matrix switching)
    
    File _configFile;
    
private:

    void DeleteTemporaryFiles();

    File _desConfigFile;

    File _tempConfigZipFile;
    Array<File> _cleanUpFilesOnExit;

    AudioSampleBuffer ambi_spk_buffer_;
    
    
#if BINAURAL_DECODER
    bool loadIr(AudioSampleBuffer* IRBuffer, const File& audioFile, double &samplerate, float gain=1.f, int offset=0, int length=0);

    ConvolverData conv_data;

    #if WITH_ZITA_CONVOLVER
        Convproc zita_conv; /* zita-convolver engine class instances */
        unsigned int _ConvBufferPos; // the position of the read/write head
    #else
        MtxConvMaster mtxconv_;
    #endif

    bool safemode_; // this will add some latency for hosts that might send partial blocks, done automatically based on host type
#endif

    double SampleRate;
    int BufferSize;
    
    int ConvBufferSize;
    
    bool isProcessing;
    
    float _gain; // output gain
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_binauralAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_B809653C__
