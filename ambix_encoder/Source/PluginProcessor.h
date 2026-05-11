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
#include "ambix_buses.h"

#include "AmbixEncoder.h"

#include "MyMeterDsp/MyMeterDsp.h"

#if WITH_OSC
 #include "NetworkAdvertiser.h"
#endif

#include "ReaperVST3Integration.h"

#define _2PI 6.2831853071795

// Maximum number of input source channels supported by the unified binary.
// The plugin allocates this many panners/encoders at construction; the active
// count is exposed at runtime via NumActiveSourcesParam.
#ifndef MAX_INPUT_CHANNELS
 #define MAX_INPUT_CHANNELS 8
#endif

//==============================================================================
/**
*/
class Ambix_encoderAudioProcessor  : public AudioProcessor,
#if WITH_OSC
                                    public Timer,
                                    private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
                                    private juce::ChangeListener,
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

    void updateTrackProperties (const TrackProperties& properties) override;

    String getTrackName() const;  // thread-safe accessor used by sendOSC()

    juce::VST3ClientExtensions* getVST3ClientExtensions() override
    {
        return &reaperIntegration;
    }

#if WITH_ADVANCED_CONTROL
    void calcNewParameters(double SampleRate, int BufferLength);
#endif

    //==============================================================================
    // Parameter layout. Indices are stable so DAW automation survives revisions.
    // Per-source params start at SourceParamsBase and pack 4 floats per source
    // in (az, el, size, gain) order. They are always exposed to the host so
    // automation works regardless of the linked toggle, but only consulted when
    // unlinked.
    static constexpr int kMaxSources         = MAX_INPUT_CHANNELS;
    static constexpr int kPerSourceParams    = 4;  // az, el, size, gain
    static constexpr int kBaseParams         = 13;
    static constexpr int kTotalParams        = kBaseParams + kMaxSources * kPerSourceParams;

    enum Parameters
    {
        AzimuthParam = 0,
        ElevationParam,
        SizeParam,
        WidthParam,
        AzimuthSetParam,    // advanced control
        AzimuthSetRelParam,
        AzimuthMvParam,
        ElevationSetParam,
        ElevationSetRelParam,
        ElevationMvParam,
        SpeedParam,
        LinkedParam,            // 0 = unlinked, 1 = linked
        NumActiveSourcesParam,  // 0..1 mapped to 1..kMaxSources (round)
        SourceParamsBase = 13   // per-source params follow
    };

    enum SourceSubParam { SrcAz = 0, SrcEl, SrcSize, SrcGain };

    static constexpr int sourceParamIndex (int srcIdx, int sub)
    {
        return SourceParamsBase + srcIdx * kPerSourceParams + sub;
    }

    // Decode the active source count from NumActiveSourcesParam (0..1 → 1..kMaxSources).
    int getActiveSources() const;

    bool isLinked() const { return linked_param >= 0.5f; }

    // Snapshot of per-source positions for the editor / Hammer-Aitoff view.
    // Returns degrees in (-180..180, -90..90) for convenience.
    struct SourcePos { float azDeg, elDeg, size, gain, meter; };
    SourcePos getSourceDisplayPos (int idx) const;

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

    void oscMessageReceived (const OSCMessage& message) override;

    void sendOSC(); // send osc data

    void oscOut(bool arg);
    void oscIn (bool arg);
    void setDiscoverable (bool arg);
    void setExtendedOscOut (bool arg);

    void changeTimer(int time);

    bool osc_in;
    bool osc_out;
    bool osc_extended_out; // emit per-source /ambix_encoder/... (incl. meters)
    int osc_interval;

    String osc_in_port, osc_out_ip, osc_out_port;

    bool discoverable;
    std::unique_ptr<NetworkAdvertiser> networkAdvertiser;
    void refreshAdvertiser();
    void rebuildOscSenders();
    void refreshOscReceiverBinding();
    void refreshOscOutput();

    void dispatchSubscribe   (const juce::OSCMessage& m);
    void dispatchUnsubscribe (const juce::OSCMessage& m);

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
#endif

    ReaperVST3Integration reaperIntegration;

    const juce::String instanceUuid { juce::Uuid().toDashedString() };

    //==========================================================================
    // Per-source per-block RMS (W-channel encoded contribution magnitude is too
    // entangled across sources, so we measure each input pre-encoding instead).
    // Read by the editor for VU display + sent over OSC.
    float getSourceMeter (int idx) const;

private:
    struct ReaperPollTimer : public juce::Timer
    {
        explicit ReaperPollTimer (Ambix_encoderAudioProcessor& p) : owner (p) {}
        void timerCallback() override { owner.pollReaperProject(); }
        Ambix_encoderAudioProcessor& owner;
    };
    ReaperPollTimer reaperPollTimer { *this };
    void pollReaperProject();

    CriticalSection track_name_lock;
    String          track_name;

    String          currentReaperProject;

    OwnedArray<AmbixEncoder> AmbiEnc;

    // Recompute per-source target azimuth/elevation/size from the active
    // parameter model (linked vs unlinked). Called whenever a relevant param
    // changes, and once per processBlock() before the gain ramp.
    void applyParamsToEncoders();

    // Migrate global → per-source positions when switching modes so the user
    // doesn't lose context between linked and unlinked.
    void linkedToUnlinkedSnapshot();
    void unlinkedToLinkedSnapshot();

    double SampleRate;

    // global (linked-mode) params
    float azimuth_param;
    float elevation_param;
    float size_param;
    float width_param;
    float linked_param;
    float num_active_sources_param;

    // per-source params (azimuth, elevation, size, gain), 0..1 normalised
    struct SourceParams { float az, el, size, gain; };
    std::array<SourceParams, kMaxSources> source_params;

    // last osc value sent (deltas only)
    float _azimuth_param;
    float _elevation_param;
    float _size_param;
    float _rms;
    float _dpk;

    // advanced parameter control
    float speed_param;
    float azimuth_set_param, azimuth_set_rel_param, azimuth_mv_param;
    float elevation_set_param, elevation_set_rel_param, elevation_mv_param;

    AudioSampleBuffer InputBuffer;

    MyMeterDsp _my_meter_dsp; // overall W-channel meter (legacy /ambi_enc)
    OwnedArray<MyMeterDsp> sourceMeterDsp;

    float rms;
    float dpk;

    // per-source meter snapshot (lock-free single-writer)
    std::array<std::atomic<float>, kMaxSources> source_meter {};

#if WITH_OSC
    OSCReceiver oscReceiver;
    bool receiverBound { false };

    CriticalSection oscSenders_lock;
    OwnedArray<OSCSender> oscSenders;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_encoderAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_58CE358E__
