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

//==============================================================================
/**
*/
class Ambix_encoderAudioProcessor  : public AudioProcessor,
#if WITH_OSC
                                    public Timer,
                                    private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::RealtimeCallback>,
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

    void calcAzimuth();

    void updateTrackProperties (const TrackProperties& properties) override;

    String getTrackName() const;  // thread-safe accessor used by sendOSC()

    // Expose the VST3 extensions object so the JUCE VST3 wrapper can pass
    // host-side interfaces (e.g. REAPER's IReaperHostApplication) to it.
    juce::VST3ClientExtensions* getVST3ClientExtensions() override
    {
        return &reaperIntegration;
    }

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

    // UI toggles. Semantics (decoupled now):
    //   osc_out       — enable sending to the manual ip:port list.
    //   osc_in        — honour /ambi_enc_set for remote parameter control.
    //   discoverable  — advertise on the LAN + accept /ambi_enc_subscribe and
    //                   stream /ambi_enc to every subscriber, regardless of
    //                   osc_out. Subscriber traffic is independent of the
    //                   manual send/receive toggles.
    void oscOut(bool arg);
    void oscIn (bool arg);
    void setDiscoverable (bool arg);

    void changeTimer(int time);

    // osc stuff
    bool osc_in;
	bool osc_out;
    int osc_interval;

	String osc_in_port, osc_out_ip, osc_out_port;

    // zeroconf discovery
    bool discoverable;
    std::unique_ptr<NetworkAdvertiser> networkAdvertiser;
    void refreshAdvertiser();
    void rebuildOscSenders(); // combines manual + subscribers

    // Decides whether the UDP receive socket should be bound, and whether the
    // send timer should be running, based on the current flags + subscriber
    // count. Called from every setter that affects those conditions.
    void refreshOscReceiverBinding();
    void refreshOscOutput();

    void dispatchSubscribe   (const juce::OSCMessage& m);
    void dispatchUnsubscribe (const juce::OSCMessage& m);

    // Rebuilds sender list when NSD expires subscribers.
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

#endif

    // Fully constructed by the member-initializer list so the VST3 wrapper
    // can call setIHostApplication on it even before our ctor body runs.
    ReaperVST3Integration reaperIntegration;

    // Stable per-plugin-load identifier broadcast in the NSD description as
    // `euid=...`. The visualizer keys its subscription list on this — it is
    // invariant across Advertiser rebuilds, unlike juce's `Service::instanceID`
    // which mints a fresh random value every time the Advertiser is recreated
    // (e.g. when the description changes after a project-name poll).
    const juce::String instanceUuid { juce::Uuid().toDashedString() };

private:
    // Lightweight timer that polls REAPER's project name for this plugin
    // instance. Runs regardless of whether the OSC-send timer is active —
    // without it, the project name would only resolve AFTER the first
    // subscriber arrived, which triggered a description change and (before we
    // added the stable `euid`) would have invalidated that very subscription.
    // Decoupling project-poll from OSC-send keeps the first-ever broadcast
    // carrying the right `proj=` value.
    struct ReaperPollTimer : public juce::Timer
    {
        explicit ReaperPollTimer (Ambix_encoderAudioProcessor& p) : owner (p) {}
        void timerCallback() override { owner.pollReaperProject(); }
        Ambix_encoderAudioProcessor& owner;
    };
    ReaperPollTimer reaperPollTimer { *this };
    void pollReaperProject();

    CriticalSection track_name_lock;
    String          track_name;   // updated by the host via updateTrackProperties()

    // Cached "last seen" REAPER project name; compared each poll tick so we
    // only rebuild the NSD advertiser when the project is saved/renamed/changed.
    String          currentReaperProject;

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
    bool receiverBound { false }; // tracks whether oscReceiver is connected so
                                  // refreshOscReceiverBinding() can idempotently
                                  // bind/unbind.

    CriticalSection oscSenders_lock; // protects oscSenders against concurrent
                                     // access from Timer thread (sendOSC) and
                                     // OSC receiver / NSD threads.
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
