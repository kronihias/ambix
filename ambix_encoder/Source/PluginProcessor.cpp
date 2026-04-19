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

#include "PluginProcessor.h"
#include "../../common/JuceCompat.h"
#include "PluginEditor.h"

// Must be emitted exactly once per binary — this is the one TU that owns it.
// `DECLARE_CLASS_IID` in ReaperVST3Integration.h declared the FUID; the VST3
// SDK requires a matching DEF_CLASS_IID somewhere in a .cpp for linkage.
namespace ambix_reaper
{
    DEF_CLASS_IID (IReaperHostApplication)
}

//==============================================================================
int Ambix_encoderAudioProcessor::s_ID = 0; // for counting id!

Ambix_encoderAudioProcessor::Ambix_encoderAudioProcessor():
#ifdef UNIVERSAL_AMBISONIC
    // Both buses default to the same ambisonic layout so that the host
    // negotiates them together based on track channel count.
    // processBlock only reads INPUT_CHANNELS from the input buffer.
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET(AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
#else
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::discreteChannels(INPUT_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
#endif
    azimuth_param(0.5f),
    elevation_param(0.5f),
    size_param(0.f),
    width_param(0.125f),
    _azimuth_param(0.5f), // buffers
    _elevation_param(0.5f),
    _size_param(0.0f),
    _rms(0.f),
    _dpk(0.f),
    speed_param(0.25f),
    azimuth_set_param(0.5f),
    azimuth_set_rel_param(0.5f),
    azimuth_mv_param(0.5f),
    elevation_set_param(0.5f),
    elevation_set_rel_param(0.5f),
    elevation_mv_param(0.5f),
    InputBuffer(INPUT_CHANNELS, 512),
    rms(0.0f),
    dpk(0.0f)

{
    // create encoders
    for (int i =0; i < INPUT_CHANNELS; i++) {
        AmbiEnc.add(new AmbixEncoder());

        // call twice to set buffers zero
        AmbiEnc.getLast()->calcParams();
        AmbiEnc.getLast()->calcParams();
    }


    // azimuth, elevation, size
    // if more than one channel add width parameter: all sources are aligned along with equal distance
    NumParameters = 3;

#if INPUT_CHANNELS > 1
    NumParameters += 1;
#endif

    // advanced control add set x2, setrelative x2, move x2 and movespeed
#if WITH_ADVANCED_CONTROL
    NumParameters += 7;
#endif

    //set encoder id
    Ambix_encoderAudioProcessor::s_ID++;
    m_id = Ambix_encoderAudioProcessor::s_ID;


#if WITH_OSC

    // Defaults for a fresh plugin instance. These are the values a newly
    // inserted encoder starts with. Persistence is now per-instance via
    // get/setStateInformation (the host saves them in the project), so the
    // old shared `~/Library/Application Support/ambix/settings.xml` file is
    // no longer read or written — two encoders in the same session no longer
    // step on each other's settings, and opening a saved project restores
    // exactly what was in it.
    osc_in        = false;
    osc_out       = false;
    osc_in_port   = "0";              // oscIn() picks a real free port
    osc_out_ip    = "localhost";
    osc_out_port  = "7130";
    osc_interval  = 30;
    discoverable  = true;

    networkAdvertiser = std::make_unique<NetworkAdvertiser>();
    networkAdvertiser->addChangeListener (this);

    // Bind the UDP socket (needed for subscribe handling even if manual RX is
    // off) and spin up the send timer if the manual list is configured. With
    // no subscribers yet and osc_out=false the timer stays idle — it only
    // starts once a visualizer subscribes or the user enables manual send.
    refreshOscReceiverBinding();
    refreshOscOutput();

    // Start the REAPER project-name poll BEFORE refreshAdvertiser so that the
    // first NSD broadcast (gated by NetworkAdvertiser::advertiseAfter ~2 s)
    // already carries the right `proj=` value. The poll is a cheap host-API
    // call throttled to 1 Hz — it runs regardless of whether any OSC timer
    // is active.
    reaperPollTimer.startTimerHz (1);
    pollReaperProject();   // try once synchronously; host may or may not be ready

    refreshAdvertiser();

#endif

}

Ambix_encoderAudioProcessor::~Ambix_encoderAudioProcessor()
{
    Ambix_encoderAudioProcessor::s_ID--; // instance counter

#if WITH_OSC
    if (networkAdvertiser != nullptr)
        networkAdvertiser->removeChangeListener (this);

    // Stop the send path and tear down the receive socket. We can't just call
    // oscIn(false)/oscOut(false) any more — those now defer to refresh
    // helpers that inspect the flags' combined state, so instead clear both
    // flags and force a full refresh.
    stopTimer();
    reaperPollTimer.stopTimer();
    osc_in = false;
    osc_out = false;
    discoverable = false;
    refreshOscReceiverBinding();          // unbinds the socket
    {
        const ScopedLock sl (oscSenders_lock);
        oscSenders.clear();
    }
#endif
}

#if WITH_OSC

void Ambix_encoderAudioProcessor::timerCallback() // check if new values and call send osc
{

    // Timer only runs when refreshOscOutput decided we have destinations
    // (manual enabled OR at least one subscriber). We still diff here to avoid
    // spamming identical packets.
    if (_azimuth_param != azimuth_param ||
        _elevation_param != elevation_param ||
        _size_param != size_param ||
        _rms != rms ||
        _dpk != dpk)
    {
        sendOSC();
    }

    // Project-name polling lives on the standalone reaperPollTimer now so it
    // runs even when there are no OSC destinations (e.g. before the first
    // visualizer subscribes). See pollReaperProject().
}

void Ambix_encoderAudioProcessor::pollReaperProject()
{
    const auto projName = reaperIntegration.getProjectName();
    if (projName != currentReaperProject)
    {
        currentReaperProject = projName;
        refreshAdvertiser();
    }
}

void Ambix_encoderAudioProcessor::sendOSC() // send osc data
{
    // No gate on osc_out here: the destination list is what gates us. Manual
    // send contributes entries only when osc_out is true; subscribers
    // contribute only when discoverable is true (see rebuildOscSenders). If
    // both are off, oscSenders is empty and this is a no-op.

    OSCMessage mymsg = OSCMessage("/ambi_enc");
    mymsg.addInt32(m_id); // source id
    mymsg.addString(getTrackName());
    mymsg.addFloat32(2.0f); // distance... currently unused
    mymsg.addFloat32(360.f*(azimuth_param-0.5f)); // azimuth -180....180°
    mymsg.addFloat32(360.f*(elevation_param-0.5f)); // elevation -180....180°
    mymsg.addFloat32(size_param); // size param 0.0 ... 1.0
    mymsg.addFloat32(dpk); // digital peak value linear 0.0 ... 1.0 (=0dBFS)
    mymsg.addFloat32(rms); // rms value linear 0.0 ... 1.0 (=0dBFS)

    // Reply port is advertised whenever we're actually willing to act on
    // /ambi_enc_set — that's either explicit manual-receive (osc_in) or the
    // subscription path (discoverable, where subscribed visualizers drive us
    // back). Without one of those, tell the receiver we're send-only.
    if (receiverBound && (osc_in || discoverable))
        mymsg.addInt32 (osc_in_port.getIntValue());

    {
        const ScopedLock sl (oscSenders_lock);
        for (int i = 0; i < oscSenders.size(); i++)
            oscSenders.getUnchecked(i)->send (mymsg);
    }

    _azimuth_param  = azimuth_param; // change buffers
    _elevation_param = elevation_param;
    _size_param      = size_param;
    _rms             = rms;
    _dpk             = dpk;
}


// this is called if an OSC message is received
void Ambix_encoderAudioProcessor::oscMessageReceived (const OSCMessage& message)
{
    const auto address = message.getAddressPattern().toString();

    if (address == "/ambi_enc_subscribe")
    {
        if (! discoverable) return; // silently ignore while opted-out
        dispatchSubscribe (message);
        return;
    }
    if (address == "/ambi_enc_unsubscribe")
    {
        if (! discoverable) return;
        dispatchUnsubscribe (message);
        return;
    }

    // Default: /ambi_enc_set <id> <distance> <azimuth> <elevation> <size>
    // Accepted when either:
    //   * the user opted in explicitly via "manual receive" (osc_in), or
    //   * we're discoverable — subscribed visualizers need a back-channel to
    //     drag pucks around and have that reflected here.
    // If neither is true, the UDP socket isn't even bound and we never get
    // here; the check below is defensive.
    if (! (osc_in || discoverable)) return;

    // parse the message for int and float
    float val[5];

    for (int i=0; i < jmin(5,message.size()); i++) {

        val[i] = 0.5f;

        // get the value wheter it is a int or float value
        if (message[i].getType() == OSCTypes::float32)
        {
            val[i] = (float)message[i].getFloat32();
        }
        else if (message[i].getType() == OSCTypes::int32)
        {
            val[i] = (float)message[i].getInt32();
        }

    }

    setParameterNotifyingHost(this, Ambix_encoderAudioProcessor::AzimuthParam, jlimit(0.f, 1.f, (val[2]+180.f)/360.f) );
    setParameterNotifyingHost(this, Ambix_encoderAudioProcessor::ElevationParam, jlimit(0.f, 1.f, (val[3]+180.f)/360.f) );
	setParameterNotifyingHost(this, Ambix_encoderAudioProcessor::SizeParam, jlimit(0.f, 1.f, val[4]));

}

void Ambix_encoderAudioProcessor::dispatchSubscribe (const OSCMessage& m)
{
    // /ambi_enc_subscribe <string uuid> <int32 reply_port> <string friendly_name> [<string visualizer_ip>]
    if (m.size() < 2) return;
    if (m[0].getType() != OSCTypes::string)  return;
    if (m[1].getType() != OSCTypes::int32)   return;

    const juce::String uuid = m[0].getString();
    const int replyPort     = m[1].getInt32();
    juce::String friendlyName;
    juce::String visualizerIp;
    if (m.size() >= 3 && m[2].getType() == OSCTypes::string)
        friendlyName = m[2].getString();
    if (m.size() >= 4 && m[3].getType() == OSCTypes::string)
        visualizerIp = m[3].getString();

    DBG ("ambix_encoder: subscribe uuid=" << uuid << " port=" << replyPort
         << " name=" << friendlyName << " ip=" << visualizerIp);

    // If the visualizer included its IP in the message, use that directly; if
    // not, addSubscriber falls back to correlating the UUID against the NSD
    // visualizer list (may be empty until NSD catches up).
    if (networkAdvertiser != nullptr)
    {
        networkAdvertiser->addSubscriber (uuid, visualizerIp, replyPort, friendlyName);

        // Rebuild senders + (re)start the timer if this was the first
        // subscriber and manual send is off.
        refreshOscOutput();

        // Immediate snapshot to the freshly added subscriber so the visualizer
        // doesn't have to wait for the next parameter change before its puck
        // appears. We pushed the timer to life above; push one packet now too
        // so the very first frame lands without waiting a full tick.
        // sendOSC() iterates oscSenders under lock — if it's empty (e.g. the
        // subscriber had an empty IP that NSD hasn't resolved yet) it's a no-op.
        sendOSC();
    }
}

void Ambix_encoderAudioProcessor::dispatchUnsubscribe (const OSCMessage& m)
{
    if (m.size() < 1) return;
    if (m[0].getType() != OSCTypes::string) return;
    const juce::String uuid = m[0].getString();
    if (networkAdvertiser != nullptr)
        networkAdvertiser->removeSubscriber (uuid);
    // Rebuild senders and stop the timer if this emptied the combined list.
    refreshOscOutput();
}

void Ambix_encoderAudioProcessor::oscOut (bool arg)
{
    // Manual-send UI toggle. Adds/removes the manual ip:port list from the
    // outgoing sender set but does NOT gate subscriber-driven sending — if
    // the plugin is discoverable and a visualizer is subscribed, /ambi_enc
    // still streams regardless of this flag.
    if (osc_out == arg) return;
    osc_out = arg;
    refreshOscOutput();
}

void Ambix_encoderAudioProcessor::oscIn (bool arg)
{
    // Manual-receive UI toggle. Purely flips whether /ambi_enc_set (remote
    // parameter control) is honoured. The UDP socket itself stays bound as
    // long as either osc_in or discoverable is true, because /ambi_enc_subscribe
    // must keep working even when the user doesn't want remote control.
    if (osc_in == arg) return;
    osc_in = arg;
    refreshOscReceiverBinding();
    // osc_in also decides whether we append a reply-port to outgoing
    // /ambi_enc packets, and whether the receiver is bound at all (which in
    // turn affects the advertiser). Both are handled by the receiver helper.
}

void Ambix_encoderAudioProcessor::setDiscoverable (bool arg)
{
    if (discoverable == arg) return;
    discoverable = arg;
    refreshAdvertiser();             // start/stop LAN announcement
    refreshOscReceiverBinding();     // receiver may no longer be needed (if osc_in=false)
    refreshOscOutput();              // subscribers may have just been evicted
}

void Ambix_encoderAudioProcessor::refreshOscReceiverBinding()
{
    // Socket stays up whenever either the manual RX toggle or discovery wants
    // a listener. If both are off, we unbind — no point burning a local
    // network permission prompt or a UDP port when the plugin has nothing to
    // receive.
    const bool wantBound = osc_in || discoverable;

    if (wantBound && ! receiverBound)
    {
        int port = 7200 + m_id;
        Random rand (Time::currentTimeMillis());

        for (int trials = 0; trials < 10; ++trials)
        {
            if (oscReceiver.connect (port))
            {
                oscReceiver.addListener (this, "/ambi_enc_set");
                oscReceiver.addListener (this, "/ambi_enc_subscribe");
                oscReceiver.addListener (this, "/ambi_enc_unsubscribe");
                osc_in_port = String (port);
                receiverBound = true;
                refreshAdvertiser(); // advertiser needs the (possibly new) port
                return;
            }
            port += rand.nextInt (999);
        }
        // If we get here, all 10 attempts failed — leave receiverBound=false.
    }
    else if (! wantBound && receiverBound)
    {
        oscReceiver.removeListener (this);
        oscReceiver.disconnect();
        receiverBound = false;
        osc_in_port = "0";
        refreshAdvertiser(); // connectionPort=0 → NetworkAdvertiser stops broadcasting
    }
}

void Ambix_encoderAudioProcessor::refreshOscOutput()
{
    // Rebuild the sender list and then make the timer state track whether we
    // actually have anywhere to send. This is the single authority on timer
    // start/stop post-refactor.
    rebuildOscSenders();

    bool haveDestinations;
    {
        const ScopedLock sl (oscSenders_lock);
        haveDestinations = ! oscSenders.isEmpty();
    }

    if (haveDestinations)
    {
        if (! isTimerRunning())
            startTimer (osc_interval);
    }
    else if (isTimerRunning())
    {
        stopTimer();
    }
}

void Ambix_encoderAudioProcessor::rebuildOscSenders()
{
    // Rebuilds the oscSenders array by combining:
    //   1. The user's manual semicolon-list in osc_out_ip / osc_out_port
    //      (only when osc_out is true).
    //   2. Subscribers added via /ambi_enc_subscribe (only when discoverable).
    // Called on settings change, on subscribe/unsubscribe, and on NSD expiry.
    //
    // Build the new list *outside* the lock so we never hold the lock while
    // doing UDP connects, which can block. Then swap it in under the lock so
    // the Timer thread's sendOSC never sees a half-torn array.

    OwnedArray<OSCSender> newSenders;

    // --- Manual list (only when user enabled manual send) ---
    if (osc_out)
    {
        String tmp_out_ips   = osc_out_ip.trim();
        String tmp_out_ports = osc_out_port.trim();
        String tmp_ip, tmp_port;

        while (tmp_out_ips.length() > 0 || tmp_out_ports.length() > 0)
        {
            if (tmp_out_ips.length() > 0)
                tmp_ip = tmp_out_ips.upToFirstOccurrenceOf (";", false, false);
            if (tmp_out_ports.length() > 0)
                tmp_port = tmp_out_ports.upToFirstOccurrenceOf (";", false, false);

            if (tmp_ip.equalsIgnoreCase ("localhost"))
                tmp_ip = "127.0.0.1";

            if (tmp_ip.isNotEmpty() && tmp_port.getIntValue() > 0)
            {
                newSenders.add (new OSCSender());
                newSenders.getLast()->connect (tmp_ip, tmp_port.getIntValue());
            }

            tmp_out_ips   = tmp_out_ips.fromFirstOccurrenceOf   (";", false, false).trim();
            tmp_out_ports = tmp_out_ports.fromFirstOccurrenceOf (";", false, false).trim();
        }
    }

    // --- Subscribers from the network advertiser (only when discoverable) ---
    if (discoverable && networkAdvertiser != nullptr)
    {
        for (const auto& sub : networkAdvertiser->getSubscribers())
        {
            if (sub.ip.isEmpty() || sub.port <= 0)
                continue;
            newSenders.add (new OSCSender());
            newSenders.getLast()->connect (sub.ip, sub.port);
        }
    }

    {
        const ScopedLock sl (oscSenders_lock);
        oscSenders.swapWith (newSenders);
    }
    // newSenders now holds the old objects and will destroy them on scope exit,
    // safely outside the lock.

    sendChangeMessage(); // editor can refresh subscriber info
}

void Ambix_encoderAudioProcessor::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (networkAdvertiser != nullptr && source == networkAdvertiser.get())
    {
        // NSD expired a subscriber, or one was just added / removed — keep the
        // OSC sender array + timer in sync. Also forwards the signal to the
        // editor so it can redraw the Subscribers panel.
        refreshOscOutput();
    }
}

void Ambix_encoderAudioProcessor::refreshAdvertiser()
{
    if (networkAdvertiser == nullptr) return;

    const auto host = juce::SystemStats::getComputerName();
    const auto daw  = juce::PluginHostType().getHostDescription();
    const auto connectionPort = osc_in_port.getIntValue();

    networkAdvertiser->setAdvertising (discoverable,
                                       connectionPort,
                                       instanceUuid,
                                       getTrackName(),
                                       m_id,
                                       host,
                                       daw,
                                       currentReaperProject /* REAPER-only; empty elsewhere */);
}

void Ambix_encoderAudioProcessor::changeTimer (int time)
{
    osc_interval = time;
    if (isTimerRunning())
    {
        stopTimer();
        startTimer (time);
    }
}
#endif

//==============================================================================
const String Ambix_encoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}



const String Ambix_encoderAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_encoderAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_encoderAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_encoderAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_encoderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_encoderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_encoderAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_encoderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_encoderAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_encoderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_encoderAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_encoderAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_encoderAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_encoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    SampleRate = sampleRate;

    // init meter dsp
    _my_meter_dsp.setAudioParams((int)SampleRate, samplesPerBlock);
    _my_meter_dsp.setParams(0.5f, 20.0f);
#if WITH_OSC
    sendOSC();
#endif
}

void Ambix_encoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool Ambix_encoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == INPUT_CHANNELS));
#endif
}

void Ambix_encoderAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    sendChangeMessage();
#endif
}

void Ambix_encoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    int NumSamples = buffer.getNumSamples();

    // std::cout << "buffer size: " << NumSamples << " channels: " << buffer.getNumChannels() << std::endl;

#if WITH_ADVANCED_CONTROL
    // calculate new azimuth and elevation parameters if move
    calcNewParameters(SampleRate, NumSamples);
#endif

    // resize input buffer if necessary
    if (InputBuffer.getNumSamples() != NumSamples || InputBuffer.getNumChannels() != getTotalNumInputChannels()) {
        InputBuffer.setSize(getTotalNumInputChannels(), NumSamples);
        // std::cout << "input buffer resized: " << InputBuffer.getNumSamples() << " channels: " << InputBuffer.getNumChannels() << std::endl;
    }

    // clear input buffer and copy input samples
    InputBuffer.clear();

    for (int i=0; i < std::min(getTotalNumInputChannels(), INPUT_CHANNELS); i++) {
        InputBuffer.copyFrom(i, 0, buffer, i, 0, NumSamples);
        // std::cout << "copied buffer channel " << i << std::endl;
    }

    // clear output buffer
    buffer.clear();

    // calculate new parameters
    for (int i=0; i < INPUT_CHANNELS; i++) {
        AmbiEnc.getUnchecked(i)->calcParams();
    }

    for (int in_ch=0; in_ch < std::min(getTotalNumInputChannels(), INPUT_CHANNELS); in_ch++) {

				// String debug_output = "Gains: ";
				const float* in_channel_data = InputBuffer.getReadPointer(in_ch);

        for (int out_ch = 0; out_ch < getTotalNumOutputChannels(); out_ch++)
        {

					// std::cout << "copying channel: " << out_ch << std::endl;
					// buffer.copyFrom(out_ch,0,buffer,0,0,NumSamples);

          // copy data from in_ch to output channels and scale with ambi factors
					if (AmbiEnc.getUnchecked(in_ch)->_ambi_gain[out_ch] == AmbiEnc.getUnchecked(in_ch)->ambi_gain[out_ch])
					{
						buffer.addFrom(out_ch, 0, InputBuffer, in_ch,  0, NumSamples, (float)AmbiEnc.getUnchecked(in_ch)->_ambi_gain[out_ch]);
						// buffer.addFrom(out_ch, 0, in_channel_data, NumSamples, AmbiEnc.getUnchecked(in_ch)->_ambi_gain[out_ch]);
					} else {
						buffer.addFromWithRamp(out_ch, 0, in_channel_data, NumSamples, (float)AmbiEnc.getUnchecked(in_ch)->_ambi_gain[out_ch], (float)AmbiEnc.getUnchecked(in_ch)->ambi_gain[out_ch]);
					}

				// debug_output << AmbiEnc.getUnchecked(in_ch)->ambi_gain[out_ch] << " ";

        }
		// std::cout << debug_output << std::endl;
    }

#if WITH_OSC
    // level of W channel for OSC output

    _my_meter_dsp.calc((float*)buffer.getReadPointer(0), NumSamples);

    dpk = _my_meter_dsp.getPeak();
    rms = _my_meter_dsp.getRMS();
#endif

}

//==============================================================================
bool Ambix_encoderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_encoderAudioProcessor::createEditor()
{
    return new Ambix_encoderAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_encoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // Create an outer XML element..

    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    for (int i=0; i < getNumParameters(); i++)
    {
        xml.setAttribute (String(i), getParameter(i));
    }

    xml.setAttribute ("mID", m_id);

#if WITH_OSC
    // Per-instance OSC + discovery settings: stored in plugin state so every
    // encoder remembers its own configuration with the project, not a
    // process-wide settings file.
    xml.setAttribute ("osc_out",          osc_out);
    xml.setAttribute ("osc_in",           osc_in);
    xml.setAttribute ("osc_out_ip",       osc_out_ip);
    xml.setAttribute ("osc_out_port",     osc_out_port);
    xml.setAttribute ("osc_out_interval", osc_interval);
    xml.setAttribute ("discoverable",     discoverable);
#endif

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_encoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            for (int i=0; i < getNumParameters(); i++) {
                setParameter(i, xmlState->getDoubleAttribute(String(i)));
            }
            if (xmlState->hasAttribute("mID"))
              m_id  = xmlState->getIntAttribute("mID", m_id);

#if WITH_OSC
            // Restore per-instance OSC + discovery settings. Fall back to
            // whatever the ctor set if an attribute is missing (older session
            // that pre-dates this migration).
            const bool new_osc_out      = xmlState->getBoolAttribute   ("osc_out",          osc_out);
            const bool new_osc_in       = xmlState->getBoolAttribute   ("osc_in",           osc_in);
            const String new_ip         = xmlState->getStringAttribute ("osc_out_ip",       osc_out_ip);
            const String new_port       = xmlState->getStringAttribute ("osc_out_port",     osc_out_port);
            const int  new_interval     = xmlState->getIntAttribute    ("osc_out_interval", osc_interval);
            const bool new_discoverable = xmlState->getBoolAttribute   ("discoverable",     discoverable);

            // Apply the simple fields first.
            osc_out_ip   = new_ip;
            osc_out_port = new_port;
            if (new_interval > 0)
                changeTimer (new_interval);  // updates osc_interval + retimes if running

            // Then re-run the refresh helpers — they're idempotent and tolerant
            // of already-matching state. Doing it via the setters keeps a
            // single code path responsible for binding the socket, rebuilding
            // senders, (re)starting the timer, and toggling the advertiser.
            oscIn            (new_osc_in);
            setDiscoverable  (new_discoverable);
            oscOut           (new_osc_out);

            // If only ip/port changed but osc_out stayed true, oscOut() was a
            // no-op above — force a sender rebuild so the new destination is
            // actually picked up.
            if (osc_out)
                refreshOscOutput();
#endif
        }

    }
}

//==============================================================================
void Ambix_encoderAudioProcessor::updateTrackProperties (const TrackProperties& properties)
{
    bool trackChanged = false;
    if (properties.name.has_value())
    {
        const ScopedLock sl (track_name_lock);
        if (track_name != *properties.name)
        {
            track_name = *properties.name;
            trackChanged = true;
        }
    }
#if WITH_OSC
    if (trackChanged)
        refreshAdvertiser();
#endif
}

String Ambix_encoderAudioProcessor::getTrackName() const
{
    const ScopedLock sl (track_name_lock);
    return track_name;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_encoderAudioProcessor();
}
