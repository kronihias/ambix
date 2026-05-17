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

namespace
{
    // Per-block sum-of-squares. Tight float loop that modern Clang/GCC
    // happily autovectorise (4–8x speedup over scalar via SSE/NEON SIMD
    // reductions). Keeping float (not double) is intentional — audio in
    // ±1 doesn't need double precision, and float maps 1:1 to SIMD lanes
    // while double halves throughput.
    // (Apple's Accelerate vDSP_svesq would be ~equivalent but pulls in
    // CarbonCore via <Accelerate/Accelerate.h>, which collides with
    // juce::Point. Not worth the build mess for an autovec-friendly loop.)
    // See "Improve your root mean calculations"
    // (https://www.embedded.com/improve-your-root-mean-calculations/) for
    // the broader algorithmic context.
    inline float sumOfSquares (const float* buf, int n) noexcept
    {
        float s = 0.f;
        for (int i = 0; i < n; ++i)
            s += buf[i] * buf[i];
        return s;
    }
}

namespace ambix_reaper
{
    DEF_CLASS_IID (IReaperHostApplication)
}

//==============================================================================
int Ambix_encoderAudioProcessor::s_ID = 0;

namespace
{
    // Map normalised 0..1 → integer 1..maxN (round half away from zero).
    inline int normToCount (float v, int maxN)
    {
        v = juce::jlimit (0.f, 1.f, v);
        int n = (int) std::round (v * (float)(maxN - 1)) + 1;
        return juce::jlimit (1, maxN, n);
    }

    inline float countToNorm (int n, int maxN)
    {
        if (maxN <= 1) return 0.f;
        return (float)(n - 1) / (float)(maxN - 1);
    }

    // Wrap (az, el) — both in degrees — so that elevation stays in
    // [-90°, +90°]. Going past a pole is equivalent to flipping the
    // azimuth by 180° and reflecting the elevation about ±90°. Azimuth
    // is then normalised to (-180°, +180°].
    inline void wrapAzEl (float& azDeg, float& elDeg) noexcept
    {
        // First fold elevation into [-180°, +180°].
        while (elDeg >  180.f) elDeg -= 360.f;
        while (elDeg < -180.f) elDeg += 360.f;
        // Past the poles → flip to the other side of the sphere.
        if (elDeg > 90.f)       { elDeg =  180.f - elDeg; azDeg += 180.f; }
        else if (elDeg < -90.f) { elDeg = -180.f - elDeg; azDeg += 180.f; }
        while (azDeg >  180.f) azDeg -= 360.f;
        while (azDeg < -180.f) azDeg += 360.f;
    }
}

Ambix_encoderAudioProcessor::Ambix_encoderAudioProcessor():
#ifdef UNIVERSAL_AMBISONIC
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::discreteChannels (MAX_INPUT_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
#else
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::discreteChannels (MAX_INPUT_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
#endif
    azimuth_param(0.5f),
    elevation_param(0.5f),
    size_param(0.f),
    width_param(0.125f),
    linked_param(1.f),
    num_active_sources_param(0.f),       // default: 1 active source
    _azimuth_param(0.5f),
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
    InputBuffer(MAX_INPUT_CHANNELS, 512),
    rms(0.0f),
    dpk(0.0f)
{
    // create encoders + per-source meter dsp
    for (int i = 0; i < kMaxSources; ++i) {
        AmbiEnc.add (new AmbixEncoder());
        AmbiEnc.getLast()->calcParams();
        AmbiEnc.getLast()->calcParams();
        sourceMeterDsp.add (new MyMeterDsp());
        source_rms[i].store (0.f);
        source_peak[i].store (0.f);
    }

    // initial per-source positions: at front, on the equator, full sharpness,
    // no mute, no solo.
    for (int i = 0; i < kMaxSources; ++i)
    {
        source_params[i].az   = 0.5f;
        source_params[i].el   = 0.5f;
        source_params[i].size = 0.f;
        source_params[i].mute = 0.f;
        source_params[i].solo = 0.f;
    }

    Ambix_encoderAudioProcessor::s_ID++;
    m_id = Ambix_encoderAudioProcessor::s_ID;

    applyParamsToEncoders();

#if WITH_OSC
    osc_in        = false;
    osc_out       = false;
    osc_in_port   = "0";
    osc_out_ip    = "localhost";
    osc_out_port  = "7130";
    osc_interval  = 30;
    discoverable  = true;

    networkAdvertiser = std::make_unique<NetworkAdvertiser>();
    networkAdvertiser->addChangeListener (this);

    refreshOscReceiverBinding();
    refreshOscOutput();

    reaperPollTimer.startTimerHz (1);
    pollReaperProject();

    refreshAdvertiser();
#endif
}

Ambix_encoderAudioProcessor::~Ambix_encoderAudioProcessor()
{
    Ambix_encoderAudioProcessor::s_ID--;

#if WITH_OSC
    if (networkAdvertiser != nullptr)
        networkAdvertiser->removeChangeListener (this);

    stopTimer();
    reaperPollTimer.stopTimer();
    osc_in = false;
    osc_out = false;
    discoverable = false;
    refreshOscReceiverBinding();
    {
        const ScopedLock sl (oscSenders_lock);
        oscSenders.clear();
    }
#endif
}

int Ambix_encoderAudioProcessor::getActiveSources() const
{
    return normToCount (num_active_sources_param, kMaxSources);
}

float Ambix_encoderAudioProcessor::getSourceMeter (int idx) const
{
    if (idx < 0 || idx >= kMaxSources) return 0.f;
    return source_rms[idx].load();
}

Ambix_encoderAudioProcessor::SourcePos
Ambix_encoderAudioProcessor::getSourceDisplayPos (int idx) const
{
    SourcePos p { 0.f, 0.f, 0.f, 0.f, 0.f };
    if (idx < 0 || idx >= kMaxSources) return p;

    // Compute the *logical* per-source position from the param store using
    // the same math `applyParamsToEncoders` runs on the audio thread. We
    // deliberately do NOT read AmbiEnc[i].azimuth/elevation here — those are
    // populated only when the audio thread actually processes a block, so a
    // freshly-loaded plugin with playback stopped, or a paint() call before
    // the first processBlock(), would otherwise see uninitialised positions.
    const int active = getActiveSources();
    const bool linked = isLinked();
    float azDeg = 0.f, elDeg = 0.f;

    if (linked)
    {
        const float centreAzDeg = (azimuth_param  - 0.5f) * 360.f;
        const float widthDeg    = width_param * 360.f;
        elDeg = (elevation_param - 0.5f) * 360.f;
        if (active <= 1)
            azDeg = centreAzDeg;
        else
            azDeg = centreAzDeg - widthDeg / 2.f
                  + (float) idx * widthDeg / (float)(active - 1);
    }
    else
    {
        azDeg = (source_params[idx].az - 0.5f) * 360.f;
        elDeg = (source_params[idx].el - 0.5f) * 360.f;
    }
    wrapAzEl (azDeg, elDeg);

    p.azDeg = azDeg;
    p.elDeg = elDeg;
    p.size  = linked ? size_param : source_params[idx].size;
    p.rms   = source_rms[idx].load();
    p.peak  = source_peak[idx].load();
    return p;
}

//==============================================================================
// Source-layout JSON import / export — format-compatible with SPARTA
// AmbiENC and IEM MultiEncoder so configurations move freely between the
// three tools.

juce::Result Ambix_encoderAudioProcessor::saveConfigurationToFile (const juce::File& file)
{
    juce::DynamicObject::Ptr layout (new juce::DynamicObject());
    layout->setProperty ("Name", juce::var ("Source Directions"));

    juce::Array<juce::var> elements;
    const int active = getActiveSources();
    for (int i = 0; i < active; ++i)
    {
        // Read effective positions via getSourceDisplayPos so the export
        // captures the auto-spread layout in linked mode just as well as
        // explicit per-source positions in unlinked mode.
        const auto pos = getSourceDisplayPos (i);
        const bool muted = source_params[i].mute >= 0.5f;

        juce::DynamicObject::Ptr el (new juce::DynamicObject());
        // SPARTA / IEM use math-positive azimuth (counter-clockwise from
        // front, looking down). ambix uses clockwise-positive. Negate on the
        // way out so the file is interoperable.
        el->setProperty ("Azimuth",     -pos.azDeg);
        el->setProperty ("Elevation",   pos.elDeg);
        el->setProperty ("Radius",      1.0);
        el->setProperty ("IsImaginary", false);
        el->setProperty ("Channel",     i + 1);
        // Encode mute as Gain=0; otherwise full unity. Per-source size isn't
        // part of the SPARTA/IEM format so it's not round-tripped.
        el->setProperty ("Gain",        muted ? 0.0 : 1.0);
        elements.add (juce::var (el.get()));
    }
    layout->setProperty ("Elements", elements);

    juce::DynamicObject::Ptr root (new juce::DynamicObject());
    root->setProperty ("Name", juce::var ("ambix_encoder source directions"));
    root->setProperty ("Description",
        juce::var ("Created by ambix_encoder. Format compatible with SPARTA "
                   "AmbiENC and IEM MultiEncoder.  Saved " +
                   juce::Time::getCurrentTime().toString (true, true)));
    root->setProperty ("GenericLayout", juce::var (layout.get()));

    const auto jsonStr = juce::JSON::toString (juce::var (root.get()), false);
    if (! file.replaceWithText (jsonStr))
        return juce::Result::fail ("Failed to write " + file.getFullPathName());

    lastConfigDir = file.getParentDirectory();
    return juce::Result::ok();
}

juce::Result Ambix_encoderAudioProcessor::loadConfigurationFromFile (const juce::File& file)
{
    if (! file.exists())
        return juce::Result::fail ("File does not exist: " + file.getFullPathName());
    lastConfigDir = file.getParentDirectory();
    return loadConfigurationFromString (file.loadFileAsString());
}

juce::Result Ambix_encoderAudioProcessor::loadConfigurationFromString (const juce::String& jsonText)
{
    if (jsonText.isEmpty())
        return juce::Result::fail ("Empty configuration string");

    juce::var parsed;
    auto parseResult = juce::JSON::parse (jsonText, parsed);
    if (parseResult.failed())
        return parseResult;

    // SPARTA writes a "GenericLayout" with "Elements"; IEM MultiEncoder
    // accepts loudspeaker layouts via "LoudspeakerLayout" / "Loudspeakers"
    // too. Try both so we can ingest either tool's exports.
    juce::var elementsVar;
    if (parsed.hasProperty ("GenericLayout"))
        elementsVar = parsed["GenericLayout"]["Elements"];
    if (! elementsVar.isArray() && parsed.hasProperty ("LoudspeakerLayout"))
        elementsVar = parsed["LoudspeakerLayout"]["Loudspeakers"];

    if (! elementsVar.isArray())
        return juce::Result::fail ("No 'Elements' array found "
                                    "(expected SPARTA/IEM JSON format)");

    auto* elements = elementsVar.getArray();

    // First pass: highest non-imaginary Channel index → active source count.
    auto isImaginary = [] (const juce::var& el) -> bool {
        // SPARTA uses "IsImaginary"; IEM also reads it back as "Imaginary".
        return (bool) el.getProperty ("IsImaginary", el.getProperty ("Imaginary", false));
    };

    int maxCh = 0;
    for (int i = 0; i < elements->size(); ++i)
    {
        const auto& el = (*elements)[i];
        if (isImaginary (el)) continue;
        const int ch = (int) el.getProperty ("Channel", 0);
        if (ch > maxCh) maxCh = ch;
    }
    if (maxCh < 1)
        return juce::Result::fail ("Configuration has no real (non-imaginary) channels");

    const int newActiveSources = juce::jmin (maxCh, kMaxSources);

    // Switch to unlinked mode — per-source positions only mean something
    // when each source has its own params — and set the active count via
    // the host so DAW automation lanes stay in sync.
    setParameterNotifyingHost (this, LinkedParam,           0.f);
    setParameterNotifyingHost (this, NumActiveSourcesParam, countToNorm (newActiveSources, kMaxSources));

    // Default-mute every active source first; channels referenced in the
    // file unmute themselves below. Channels absent from the file (gaps)
    // stay muted, matching IEM MultiEncoder's behaviour.
    for (int i = 0; i < newActiveSources; ++i)
        setParameterNotifyingHost (this, sourceParamIndex (i, SrcMute), 1.f);

    for (int i = 0; i < elements->size(); ++i)
    {
        const auto& el = (*elements)[i];
        if (isImaginary (el)) continue;
        const int ch = (int) el.getProperty ("Channel", 0) - 1;
        if (ch < 0 || ch >= newActiveSources) continue;

        // Flip sign: SPARTA/IEM store math-positive (counter-clockwise)
        // azimuth; ambix is clockwise-positive internally.
        float az = -(float) (double) el.getProperty ("Azimuth",   0.0);
        float ele = (float) (double) el.getProperty ("Elevation", 0.0);
        wrapAzEl (az, ele); // tolerate out-of-range values like IEM does

        const float gain = (float) (double) el.getProperty ("Gain", 1.0);
        const bool muteFromGain = gain <= 0.001f;

        setParameterNotifyingHost (this, sourceParamIndex (ch, SrcAz),
                                   juce::jlimit (0.f, 1.f, (az  + 180.f) / 360.f));
        setParameterNotifyingHost (this, sourceParamIndex (ch, SrcEl),
                                   juce::jlimit (0.f, 1.f, (ele + 180.f) / 360.f));
        setParameterNotifyingHost (this, sourceParamIndex (ch, SrcMute),
                                   muteFromGain ? 1.f : 0.f);
    }

    return juce::Result::ok();
}

void Ambix_encoderAudioProcessor::applyParamsToEncoders()
{
    const int active = getActiveSources();
    const bool linked = isLinked();

    // Per-source state passed to AmbiEnc[] always has elevation in the
    // physical range [-90°, +90°]. The param store itself may carry
    // out-of-range values (the user's slider goes past ±90, or DAW
    // automation curves do); we wrap on the boundary so the GUI/OSC/visual
    // path sees the equivalent in-range (az + 180°, 180° - el) position.
    // AmbixEncoder::calcParams' SH math is invariant to this wrap so the
    // rendered ambisonic signal is the same either way.
    auto assignWrapped = [this] (int i, float azDeg, float elDeg, float sizeVal)
    {
        wrapAzEl (azDeg, elDeg);
        AmbiEnc.getUnchecked(i)->azimuth   = azDeg / 360.f + 0.5f;
        AmbiEnc.getUnchecked(i)->elevation = elDeg / 360.f + 0.5f;
        AmbiEnc.getUnchecked(i)->size      = sizeVal;
    };

    if (linked)
    {
        const float centreAzDeg = (azimuth_param  - 0.5f) * 360.f;
        const float elDegRaw    = (elevation_param - 0.5f) * 360.f;
        const float widthDeg    = width_param * 360.f;

        if (active == 1)
        {
            assignWrapped (0, centreAzDeg, elDegRaw, size_param);
        }
        else
        {
            for (int i = 0; i < active; ++i)
            {
                const float azDeg = centreAzDeg - widthDeg / 2.f
                                  + (float)i * widthDeg / (float)(active - 1);
                assignWrapped (i, azDeg, elDegRaw, size_param);
            }
        }
        // Inactive encoders: keep last positions.
    }
    else
    {
        for (int i = 0; i < active; ++i)
        {
            const float azDeg = (source_params[i].az - 0.5f) * 360.f;
            const float elDeg = (source_params[i].el - 0.5f) * 360.f;
            assignWrapped (i, azDeg, elDeg, source_params[i].size);
        }
    }
}

void Ambix_encoderAudioProcessor::linkedToUnlinkedSnapshot()
{
    // Capture the linked-mode auto-spread into each source's per-source params
    // so the user's first frame in unlinked mode shows the same layout.
    const int active = getActiveSources();
    if (active == 1)
    {
        source_params[0].az   = azimuth_param;
        source_params[0].el   = elevation_param;
        source_params[0].size = size_param;
    }
    else
    {
        for (int i = 0; i < active; ++i)
        {
            float angle = azimuth_param - width_param / 2.f
                        + (float)i * width_param / (float)(active - 1);
            if (angle < 0.f)  angle += 1.f;
            if (angle > 1.f)  angle -= 1.f;
            source_params[i].az   = angle;
            source_params[i].el   = elevation_param;
            source_params[i].size = size_param;
        }
    }
}

void Ambix_encoderAudioProcessor::unlinkedToLinkedSnapshot()
{
    // Going back to linked mode snaps to the auto-spread pattern. We don't
    // try to derive width from the unlinked layout — the user's prior linked
    // params are preserved as-is.
    // (Nothing to do; applyParamsToEncoders() recomputes from globals.)
}

#if WITH_OSC

void Ambix_encoderAudioProcessor::timerCallback()
{
    const bool posChanged   = (_azimuth_param   != azimuth_param   ||
                                _elevation_param != elevation_param ||
                                _size_param      != size_param);

    constexpr float kMeterThreshold = 0.002f;
    const bool meterChanged = (std::abs (_rms - rms) > kMeterThreshold ||
                                std::abs (_dpk - dpk) > kMeterThreshold);

    // Always tick — per-source meters fluctuate and the extended OSC path
    // streams them continuously to subscribed visualizers.
    (void) posChanged; (void) meterChanged;
    sendOSC();
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

void Ambix_encoderAudioProcessor::sendOSC()
{
    // Backwards-compat /ambi_enc message — single source representation
    // matches the legacy semantics (centre azimuth in linked mode, source 1
    // position in unlinked mode).
    {
        OSCMessage mymsg = OSCMessage("/ambi_enc");
        mymsg.addInt32(m_id);
        mymsg.addString(getTrackName());
        mymsg.addFloat32(2.0f);
        mymsg.addFloat32(360.f * (azimuth_param-0.5f));
        mymsg.addFloat32(360.f * (elevation_param-0.5f));
        mymsg.addFloat32(size_param);
        mymsg.addFloat32(dpk);
        mymsg.addFloat32(rms);

        if (receiverBound && (osc_in || discoverable))
            mymsg.addInt32 (osc_in_port.getIntValue());

        const ScopedLock sl (oscSenders_lock);
        for (int i = 0; i < oscSenders.size(); i++)
            oscSenders.getUnchecked(i)->sender->send (mymsg);
    }

    // Extended OSC: per-source positions + meters under /ambix_encoder/...
    // Sent to every entry in oscSenders (manual + NSD subscribers) so a
    // visualizer that auto-subscribed sees the individual sources, not just
    // the legacy /ambi_enc centre puck. The destination list itself is the
    // on/off switch.
    {
        const int active = getActiveSources();

        const ScopedLock sl (oscSenders_lock);
        if (! oscSenders.isEmpty())
        {
            // global state
            {
                OSCMessage m ("/ambix_encoder/linked");
                m.addInt32 (isLinked() ? 1 : 0);
                for (int i = 0; i < oscSenders.size(); i++)
                    oscSenders.getUnchecked(i)->sender->send (m);
            }
            {
                OSCMessage m ("/ambix_encoder/active_sources");
                m.addInt32 (active);
                for (int i = 0; i < oscSenders.size(); i++)
                    oscSenders.getUnchecked(i)->sender->send (m);
            }
            if (isLinked())
            {
                OSCMessage a ("/ambix_encoder/azimuth");
                a.addFloat32 (360.f * (azimuth_param - 0.5f));
                OSCMessage e ("/ambix_encoder/elevation");
                e.addFloat32 (360.f * (elevation_param - 0.5f));
                OSCMessage w ("/ambix_encoder/width");
                w.addFloat32 (360.f * width_param);
                OSCMessage s ("/ambix_encoder/size");
                s.addFloat32 (size_param);
                for (int i = 0; i < oscSenders.size(); i++)
                {
                    auto* snd = oscSenders.getUnchecked(i)->sender.get();
                    snd->send (a); snd->send (e); snd->send (w); snd->send (s);
                }
            }

            // per-source
            for (int s = 0; s < active; ++s)
            {
                const auto pos = getSourceDisplayPos (s);
                const String base = String("/ambix_encoder/source/") + String(s + 1);
                OSCMessage az (OSCAddressPattern (base + "/azimuth"));
                az.addFloat32 (pos.azDeg);
                OSCMessage el (OSCAddressPattern (base + "/elevation"));
                el.addFloat32 (pos.elDeg);
                OSCMessage sz (OSCAddressPattern (base + "/size"));
                sz.addFloat32 (pos.size);
                OSCMessage rm (OSCAddressPattern (base + "/rms"));
                rm.addFloat32 (pos.rms);
                OSCMessage pk (OSCAddressPattern (base + "/peak"));
                pk.addFloat32 (pos.peak);
                OSCMessage mu (OSCAddressPattern (base + "/mute"));
                mu.addInt32 (source_params[s].mute >= 0.5f ? 1 : 0);
                OSCMessage so (OSCAddressPattern (base + "/solo"));
                so.addInt32 (source_params[s].solo >= 0.5f ? 1 : 0);

                for (int i = 0; i < oscSenders.size(); i++)
                {
                    auto* snd = oscSenders.getUnchecked(i)->sender.get();
                    snd->send (az); snd->send (el); snd->send (sz);
                    snd->send (rm); snd->send (pk);
                    snd->send (mu); snd->send (so);
                }
            }
        }
    }

    _azimuth_param   = azimuth_param;
    _elevation_param = elevation_param;
    _size_param      = size_param;
    _rms             = rms;
    _dpk             = dpk;
}


void Ambix_encoderAudioProcessor::oscMessageReceived (const OSCMessage& message)
{
    const auto address = message.getAddressPattern().toString();

    if (address == "/ambi_enc_subscribe")
    {
        if (! discoverable) return;
        dispatchSubscribe (message);
        return;
    }
    if (address == "/ambi_enc_unsubscribe")
    {
        if (! discoverable) return;
        dispatchUnsubscribe (message);
        return;
    }

    if (! (osc_in || discoverable)) return;

    // Helper to extract a single float/int value from a message.
    auto firstFloat = [&] (float fallback) -> float {
        if (message.size() < 1) return fallback;
        if (message[0].getType() == OSCTypes::float32) return message[0].getFloat32();
        if (message[0].getType() == OSCTypes::int32)   return (float) message[0].getInt32();
        return fallback;
    };
    auto firstInt = [&] (int fallback) -> int {
        if (message.size() < 1) return fallback;
        if (message[0].getType() == OSCTypes::int32)   return message[0].getInt32();
        if (message[0].getType() == OSCTypes::float32) return (int) message[0].getFloat32();
        return fallback;
    };

    // Legacy: /ambi_enc_set <id> <distance> <azimuth> <elevation> <size>
    if (address == "/ambi_enc_set")
    {
        float val[5];
        for (int i=0; i < jmin(5,message.size()); i++) {
            val[i] = 0.5f;
            if (message[i].getType() == OSCTypes::float32)
                val[i] = (float)message[i].getFloat32();
            else if (message[i].getType() == OSCTypes::int32)
                val[i] = (float)message[i].getInt32();
        }
        setParameterNotifyingHost (this, AzimuthParam,   jlimit(0.f, 1.f, (val[2]+180.f)/360.f));
        setParameterNotifyingHost (this, ElevationParam, jlimit(0.f, 1.f, (val[3]+180.f)/360.f));
        setParameterNotifyingHost (this, SizeParam,      jlimit(0.f, 1.f, val[4]));
        return;
    }

    // Extended: /ambix_encoder/linked <int|float>
    if (address == "/ambix_encoder/linked")
    {
        const int v = firstInt (1);
        setParameterNotifyingHost (this, LinkedParam, v ? 1.f : 0.f);
        return;
    }
    if (address == "/ambix_encoder/active_sources")
    {
        const int n = juce::jlimit (1, kMaxSources, firstInt (1));
        setParameterNotifyingHost (this, NumActiveSourcesParam, countToNorm (n, kMaxSources));
        return;
    }
    if (address == "/ambix_encoder/azimuth")
    {
        setParameterNotifyingHost (this, AzimuthParam, jlimit(0.f, 1.f, (firstFloat(0.f)+180.f)/360.f));
        return;
    }
    if (address == "/ambix_encoder/elevation")
    {
        setParameterNotifyingHost (this, ElevationParam, jlimit(0.f, 1.f, (firstFloat(0.f)+180.f)/360.f));
        return;
    }
    if (address == "/ambix_encoder/width")
    {
        setParameterNotifyingHost (this, WidthParam, jlimit(0.f, 1.f, firstFloat(0.f)/360.f));
        return;
    }
    if (address == "/ambix_encoder/size")
    {
        setParameterNotifyingHost (this, SizeParam, jlimit(0.f, 1.f, firstFloat(0.f)));
        return;
    }

    // Per-source: /ambix_encoder/source/<n>/{azimuth|elevation|size}
    if (address.startsWith ("/ambix_encoder/source/"))
    {
        const String tail = address.substring (juce::String("/ambix_encoder/source/").length());
        const int slashPos = tail.indexOfChar ('/');
        if (slashPos <= 0) return;

        const int oneBased = tail.substring (0, slashPos).getIntValue();
        if (oneBased < 1 || oneBased > kMaxSources) return;
        const int idx = oneBased - 1;
        const String sub = tail.substring (slashPos + 1);
        const float v = firstFloat (0.f);

        // Azimuth wraps modulo 360° at any boundary — clamping would freeze
        // the constellation as soon as the centre crosses ±180°. Elevation
        // doesn't wrap (it's bounded by the poles), so we just clamp it.
        auto wrapAz = [] (float deg)
        {
            while (deg >   180.f) deg -= 360.f;
            while (deg <= -180.f) deg += 360.f;
            return deg;
        };

        if (isLinked())
        {
            // The visualizer sends per-source OSC for every puck it draws,
            // but in linked mode there's no such thing as a "per-source" az
            // or el — every source lives at globalAz + offset(idx). Translate
            // the per-source write into a global one so the whole constellation
            // shifts and the dragged source lands where the user expects.
            const int active = getActiveSources();
            const float widthDeg = width_param * 360.f;
            float offsetDeg = 0.f;
            if (active > 1)
                offsetDeg = (float) idx * widthDeg / (float)(active - 1)
                          - widthDeg / 2.f;

            if (sub == "azimuth")
            {
                const float newCentreAz = wrapAz (v - offsetDeg);
                setParameterNotifyingHost (this, AzimuthParam,
                                           jlimit (0.f, 1.f, (newCentreAz + 180.f) / 360.f));
            }
            else if (sub == "elevation")
            {
                // All linked sources share elevation, so the dragged value
                // *is* the new global elevation.
                setParameterNotifyingHost (this, ElevationParam,
                                           jlimit (0.f, 1.f, (v + 180.f) / 360.f));
            }
            else if (sub == "size")
            {
                setParameterNotifyingHost (this, SizeParam, jlimit (0.f, 1.f, v));
            }
            // mute/solo are per-source even in linked mode — fall through
            // to the unlinked path so the rest of the cases (mute, solo)
            // are handled uniformly.
            else if (sub != "mute" && sub != "solo")
            {
                return;
            }
        }

        // Per-source params (always applied regardless of linked state for
        // mute/solo; az/el/size only reach here in unlinked mode).
        if (sub == "azimuth")
            setParameterNotifyingHost (this, sourceParamIndex (idx, SrcAz),
                                       jlimit(0.f, 1.f, (wrapAz (v) + 180.f) / 360.f));
        else if (sub == "elevation")
            // Per-source elevation uses the same [0,1] → [-180°, +180°]
            // encoding as azimuth — the physical range is ±90° but the
            // param space is symmetric so mouse and OSC paths stay simple.
            setParameterNotifyingHost (this, sourceParamIndex (idx, SrcEl),
                                       jlimit(0.f, 1.f, (v + 180.f) / 360.f));
        else if (sub == "size")
            setParameterNotifyingHost (this, sourceParamIndex (idx, SrcSize), jlimit(0.f, 1.f, v));
        else if (sub == "mute")
            setParameterNotifyingHost (this, sourceParamIndex (idx, SrcMute), v >= 0.5f ? 1.f : 0.f);
        else if (sub == "solo")
            setParameterNotifyingHost (this, sourceParamIndex (idx, SrcSolo), v >= 0.5f ? 1.f : 0.f);
        return;
    }
}

void Ambix_encoderAudioProcessor::dispatchSubscribe (const OSCMessage& m)
{
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

    if (networkAdvertiser != nullptr)
    {
        networkAdvertiser->addSubscriber (uuid, visualizerIp, replyPort, friendlyName);
        refreshOscOutput();
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
    refreshOscOutput();
}

void Ambix_encoderAudioProcessor::oscOut (bool arg)
{
    if (osc_out == arg) return;
    osc_out = arg;
    refreshOscOutput();
}

void Ambix_encoderAudioProcessor::oscIn (bool arg)
{
    if (osc_in == arg) return;
    osc_in = arg;
    refreshOscReceiverBinding();
}

void Ambix_encoderAudioProcessor::setDiscoverable (bool arg)
{
    if (discoverable == arg) return;
    discoverable = arg;
    refreshAdvertiser();
    refreshOscReceiverBinding();
    refreshOscOutput();
}

void Ambix_encoderAudioProcessor::refreshOscReceiverBinding()
{
    const bool wantBound = osc_in || discoverable;

    if (wantBound && ! receiverBound)
    {
        int port = 7200 + m_id;
        Random rand (Time::currentTimeMillis());

        for (int trials = 0; trials < 10; ++trials)
        {
            if (oscReceiver.connect (port))
            {
                // Catch-all: dispatch by address inside oscMessageReceived.
                oscReceiver.addListener (this);
                osc_in_port = String (port);
                receiverBound = true;
                refreshAdvertiser();
                return;
            }
            port += rand.nextInt (999);
        }
    }
    else if (! wantBound && receiverBound)
    {
        oscReceiver.removeListener (this);
        oscReceiver.disconnect();
        receiverBound = false;
        osc_in_port = "0";
        refreshAdvertiser();
    }
}

void Ambix_encoderAudioProcessor::refreshOscOutput()
{
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
    // Build the desired (ip, port) list first, then reconcile against the
    // existing senders. Keeping an OSCSender alive across rebuilds preserves
    // its OS-assigned source port — the visualizer keys its AmbixSource
    // pucks on senderIp:senderPort, so a shifting port (e.g. due to
    // /ambi_enc_subscribe heartbeats triggering this function) would
    // duplicate the puck on every refresh.
    struct Wanted { juce::String ip; int port; };
    std::vector<Wanted> wanted;

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
                wanted.push_back ({ tmp_ip, tmp_port.getIntValue() });

            tmp_out_ips   = tmp_out_ips.fromFirstOccurrenceOf   (";", false, false).trim();
            tmp_out_ports = tmp_out_ports.fromFirstOccurrenceOf (";", false, false).trim();
        }
    }

    if (discoverable && networkAdvertiser != nullptr)
    {
        for (const auto& sub : networkAdvertiser->getSubscribers())
        {
            if (sub.ip.isEmpty() || sub.port <= 0)
                continue;
            wanted.push_back ({ sub.ip, sub.port });
        }
    }

    // Reconcile: for each desired destination either reuse the existing
    // sender (preserving its source port) or create a new one. Anything left
    // in the old list is dropped on scope exit.
    OwnedArray<OscDest> newSenders;
    {
        const ScopedLock sl (oscSenders_lock);
        for (const auto& w : wanted)
        {
            std::unique_ptr<juce::OSCSender> takenSender;
            for (int i = 0; i < oscSenders.size(); ++i)
            {
                auto* existing = oscSenders.getUnchecked (i);
                if (existing != nullptr && existing->ip == w.ip && existing->port == w.port)
                {
                    takenSender = std::move (existing->sender);
                    oscSenders.remove (i, true);
                    break;
                }
            }
            if (takenSender == nullptr)
            {
                takenSender = std::make_unique<juce::OSCSender>();
                takenSender->connect (w.ip, w.port);
            }
            auto* dest = new OscDest();
            dest->ip = w.ip;
            dest->port = w.port;
            dest->sender = std::move (takenSender);
            newSenders.add (dest);
        }
        oscSenders.swapWith (newSenders);
    }
    // newSenders now owns the now-unwanted destinations; destructed on exit.

    sendChangeMessage();
}

void Ambix_encoderAudioProcessor::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (networkAdvertiser != nullptr && source == networkAdvertiser.get())
        refreshOscOutput();
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
                                       currentReaperProject);
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

bool Ambix_encoderAudioProcessor::isInputChannelStereoPair (int index) const  { return true; }
bool Ambix_encoderAudioProcessor::isOutputChannelStereoPair (int index) const { return true; }

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

bool Ambix_encoderAudioProcessor::silenceInProducesSilenceOut() const { return false; }
double Ambix_encoderAudioProcessor::getTailLengthSeconds() const      { return 0.0; }

int  Ambix_encoderAudioProcessor::getNumPrograms()                      { return 0; }
int  Ambix_encoderAudioProcessor::getCurrentProgram()                   { return 0; }
void Ambix_encoderAudioProcessor::setCurrentProgram (int)               {}
const String Ambix_encoderAudioProcessor::getProgramName (int)          { return {}; }
void Ambix_encoderAudioProcessor::changeProgramName (int, const String&){}

//==============================================================================
void Ambix_encoderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    SampleRate = sampleRate;

    _my_meter_dsp.setAudioParams ((int) SampleRate, samplesPerBlock);
    _my_meter_dsp.setParams (0.5f, 20.0f);

    for (auto* m : sourceMeterDsp)
    {
        m->setAudioParams ((int) SampleRate, samplesPerBlock);
        m->setParams (0.5f, 20.0f);
    }
#if WITH_OSC
    sendOSC();
#endif
}

void Ambix_encoderAudioProcessor::releaseResources() {}

bool Ambix_encoderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    // Universal builds accept any input up to MAX_INPUT_CHANNELS (the runtime
    // active-source count clamps to the actual input width). Output stays
    // ambisonic.
    return layouts.getMainInputChannelSet().size()  >= 1
        && layouts.getMainInputChannelSet().size()  <= MAX_INPUT_CHANNELS
        && layouts.getMainOutputChannelSet().size() >= 1;
#else
    // Fixed-order build: take any input 1..MAX_INPUT_CHANNELS, output must
    // match the fixed ambisonic channel count.
    return layouts.getMainInputChannelSet().size()  >= 1
        && layouts.getMainInputChannelSet().size()  <= MAX_INPUT_CHANNELS
        && layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS;
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

#if WITH_ADVANCED_CONTROL
    calcNewParameters (SampleRate, NumSamples);
#endif

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int active = juce::jmin (getActiveSources(), numIn, kMaxSources);

    if (InputBuffer.getNumSamples() != NumSamples || InputBuffer.getNumChannels() != numIn)
        InputBuffer.setSize (juce::jmax (1, numIn), NumSamples);

    InputBuffer.clear();
    for (int i = 0; i < juce::jmin (numIn, kMaxSources); ++i)
        InputBuffer.copyFrom (i, 0, buffer, i, 0, NumSamples);

    buffer.clear();

    // recompute encoder targets from current params, then advance gain ramp
    applyParamsToEncoders();
    for (int i = 0; i < kMaxSources; ++i)
        AmbiEnc.getUnchecked(i)->calcParams();

    // Solo/mute gating: if any source within the active range is soloed, only
    // soloed sources contribute. Mute always silences the source regardless of
    // solo state. Both apply in linked AND unlinked mode — they're orthogonal
    // to the position auto-spread.
    bool anySolo = false;
    for (int i = 0; i < active; ++i)
        if (source_params[i].solo >= 0.5f) { anySolo = true; break; }

    for (int in_ch = 0; in_ch < active; ++in_ch)
    {
        const bool muted     = source_params[in_ch].mute >= 0.5f;
        const bool soloed    = source_params[in_ch].solo >= 0.5f;
        const bool effSilent = muted || (anySolo && ! soloed);
        if (effSilent) continue;

        const float* in_channel_data = InputBuffer.getReadPointer (in_ch);

        for (int out_ch = 0; out_ch < numOut; ++out_ch)
        {
            const float ngain  = (float) AmbiEnc.getUnchecked(in_ch)->ambi_gain[out_ch];
            const float pgain  = (float) AmbiEnc.getUnchecked(in_ch)->_ambi_gain[out_ch];
            if (pgain == ngain)
                buffer.addFrom (out_ch, 0, InputBuffer, in_ch, 0, NumSamples, ngain);
            else
                buffer.addFromWithRamp (out_ch, 0, in_channel_data, NumSamples, pgain, ngain);
        }
    }

    // Per-source meters (read pre-encoding source level). RMS uses the
    // recursive-Newton form from "Improve your root mean calculations"
    // (https://www.embedded.com/improve-your-root-mean-calculations/):
    //
    //   y(n) = y(n-1) + a · ( blockMs / y(n-1) - y(n-1) )
    //
    // y converges to sqrt(steady-state blockMs) = RMS, with a one-pole
    // IIR averager and one Newton sqrt iteration fused in a single step.
    // Compared to sqrt-of-EMA-on-MS the release is twice as fast in dB —
    // perceptually linear in dB matches the PPM/IEC meter spec.
    //
    // Per-sample as in the article would force a serial loop; we run the
    // block's SIMD sum-of-squares once (cheap) and apply one recursive
    // update per block, with the IIR coefficient scaled to block rate.
    constexpr float kMeterTauSec = 0.3f;
    const float blockDt = (float) NumSamples / (float) SampleRate;
    const float emaAlpha    = std::exp (-blockDt / kMeterTauSec);
    const float recCoef     = 1.f - emaAlpha;     // = 1 - exp(-Δt/τ); article's `a`
    constexpr float kRmsFloor = 1e-6f;
    for (int s = 0; s < kMaxSources; ++s)
    {
        // Reflect effective contribution to the output: a muted source — or
        // an unsoloed source while another source is soloed — produces no
        // signal in the encode loop above and therefore should read no
        // level. Reading the raw input level here would be confusing for
        // the user (mute toggle on → audio silenced → meter still bouncing).
        const bool muted     = (s < active) && source_params[s].mute >= 0.5f;
        const bool soloed    = (s < active) && source_params[s].solo >= 0.5f;
        const bool effSilent = muted || (anySolo && ! soloed);

        if (s < active && ! effSilent)
        {
            // Run MyMeterDsp for the peak: it does fast-attack + 500 ms
            // hold + 20 dB/s release — exactly what a peak meter wants.
            sourceMeterDsp.getUnchecked(s)
                ->calc ((float*) InputBuffer.getReadPointer(s), NumSamples);
            source_peak[s].store (sourceMeterDsp.getUnchecked(s)->getPeak());

            // RMS: recursive Newton-sqrt form (see header comment).
            const float sumSq = sumOfSquares (InputBuffer.getReadPointer (s), NumSamples);
            const float blockMs = sumSq / (float) juce::jmax (1, NumSamples);

            float y = source_rms_ema[s];
            if (y <= kRmsFloor)
            {
                // Startup or post-silence — Newton needs a seed close to the
                // true RMS, otherwise the blockMs/y term blows up. Direct
                // sqrt for the first non-trivial block; recursive thereafter.
                y = std::sqrt (blockMs);
                if (y < kRmsFloor) y = kRmsFloor;
            }
            else
            {
                y += recCoef * (blockMs / y - y);
                if (y < kRmsFloor) y = kRmsFloor;
            }
            source_rms_ema[s] = y;
            source_rms[s].store (y);
        }
        else
        {
            // Inactive slot OR muted/solo-suppressed: reset everything so
            // unmuting cleanly re-seeds via the direct-sqrt path on the
            // next active block (no overshoot from stale state).
            if (s < kMaxSources)
                sourceMeterDsp.getUnchecked(s)->reset();
            source_rms_ema[s] = 0.f;
            source_rms[s].store (0.f);
            source_peak[s].store (0.f);
        }
    }

#if WITH_OSC
    if (numOut > 0)
    {
        // Peak still comes from MyMeterDsp (fast-attack + peak-hold suits
        // a clip indicator). RMS uses the same 300 ms EMA the per-source
        // meters use so every meter the encoder emits has matching
        // ballistics — steady signal → steady reading on both legacy
        // /ambi_enc and /ambix_encoder/source/<n>/meter.
        _my_meter_dsp.calc ((float*) buffer.getReadPointer(0), NumSamples);
        dpk = _my_meter_dsp.getPeak();

        // Same recursive-Newton RMS as the per-source path.
        const float sumSq = sumOfSquares (buffer.getReadPointer (0), NumSamples);
        const float blockMs = sumSq / (float) juce::jmax (1, NumSamples);

        float y = overall_rms_ema;
        if (y <= kRmsFloor)
        {
            y = std::sqrt (blockMs);
            if (y < kRmsFloor) y = kRmsFloor;
        }
        else
        {
            y += recCoef * (blockMs / y - y);
            if (y < kRmsFloor) y = kRmsFloor;
        }
        overall_rms_ema = y;
        rms = y;
    }
#endif
}

//==============================================================================
bool Ambix_encoderAudioProcessor::hasEditor() const                  { return true; }
AudioProcessorEditor* Ambix_encoderAudioProcessor::createEditor()    { return new Ambix_encoderAudioProcessorEditor (this); }

//==============================================================================
void Ambix_encoderAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml ("MYPLUGINSETTINGS");

    for (int i = 0; i < getNumParameters(); ++i)
        xml.setAttribute (String(i), getParameter(i));

    xml.setAttribute ("mID", m_id);

#if WITH_OSC
    xml.setAttribute ("osc_out",          osc_out);
    xml.setAttribute ("osc_in",           osc_in);
    xml.setAttribute ("osc_out_ip",       osc_out_ip);
    xml.setAttribute ("osc_out_port",     osc_out_port);
    xml.setAttribute ("osc_out_interval", osc_interval);
    xml.setAttribute ("discoverable",     discoverable);
#endif

    // Editor UI state — per-view size + active view. Stored on the
    // processor so it persists across editor close/reopen and across
    // session saves. Each panner remembers its own layout so toggling
    // between Sphere (1:1) and H-A (2:1) restores the visually balanced
    // window shape the user set up for that view.
    xml.setAttribute ("editor_w",       editor_width);
    xml.setAttribute ("editor_h",       editor_height);
    xml.setAttribute ("editor_hammer",  editor_hammer_view);
    xml.setAttribute ("last_config_dir", lastConfigDir.getFullPathName());
    xml.setAttribute ("popout_open",    popout_open);
    xml.setAttribute ("popout_w",         popout_width);
    xml.setAttribute ("popout_h",         popout_height);
    xml.setAttribute ("popout_hammer",    popout_hammer_view);
    xml.setAttribute ("ha_upper_only",    ha_upper_hemisphere_only);

    copyXmlToBinary (xml, destData);
}

void Ambix_encoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName ("MYPLUGINSETTINGS"))
    {
        for (int i = 0; i < getNumParameters(); ++i)
        {
            if (xmlState->hasAttribute (String(i)))
                setParameter (i, xmlState->getDoubleAttribute (String(i)));
        }
        if (xmlState->hasAttribute ("mID"))
            m_id = xmlState->getIntAttribute ("mID", m_id);

#if WITH_OSC
        const bool new_osc_out      = xmlState->getBoolAttribute   ("osc_out",          osc_out);
        const bool new_osc_in       = xmlState->getBoolAttribute   ("osc_in",           osc_in);
        const String new_ip         = xmlState->getStringAttribute ("osc_out_ip",       osc_out_ip);
        const String new_port       = xmlState->getStringAttribute ("osc_out_port",     osc_out_port);
        const int  new_interval     = xmlState->getIntAttribute    ("osc_out_interval", osc_interval);
        const bool new_discoverable = xmlState->getBoolAttribute   ("discoverable",     discoverable);

        osc_out_ip   = new_ip;
        osc_out_port = new_port;
        if (new_interval > 0)
            changeTimer (new_interval);

        oscIn            (new_osc_in);
        setDiscoverable  (new_discoverable);
        oscOut           (new_osc_out);

        if (osc_out)
            refreshOscOutput();
#endif

        // Editor UI state — per-view sizes + active-view flag. Older
        // sessions wrote a single editor_w / editor_h pair before the
        // per-view split; honour either path so saved projects don't
        // wake up at the default 640×520.
        editor_width       = xmlState->getIntAttribute  ("editor_w",      editor_width);
        editor_height      = xmlState->getIntAttribute  ("editor_h",      editor_height);
        editor_hammer_view = xmlState->getBoolAttribute ("editor_hammer", editor_hammer_view);
        popout_open        = xmlState->getBoolAttribute ("popout_open",   popout_open);
        popout_width       = xmlState->getIntAttribute  ("popout_w",      popout_width);
        popout_height      = xmlState->getIntAttribute  ("popout_h",      popout_height);
        popout_hammer_view = xmlState->getBoolAttribute ("popout_hammer", popout_hammer_view);
        ha_upper_hemisphere_only = xmlState->getBoolAttribute ("ha_upper_only", ha_upper_hemisphere_only);
        const auto savedDir = xmlState->getStringAttribute ("last_config_dir", {});
        if (savedDir.isNotEmpty())
            lastConfigDir = juce::File (savedDir);
        // Older per-view-pair attributes — fold the active-view pair into
        // the single slot so saved projects don't reset to defaults.
        if (xmlState->hasAttribute ("editor_w_sphere") || xmlState->hasAttribute ("editor_w_hammer"))
        {
            const auto wTag = editor_hammer_view ? "editor_w_hammer" : "editor_w_sphere";
            const auto hTag = editor_hammer_view ? "editor_h_hammer" : "editor_h_sphere";
            editor_width  = xmlState->getIntAttribute (wTag, editor_width);
            editor_height = xmlState->getIntAttribute (hTag, editor_height);
        }
        applyParamsToEncoders();
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
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_encoderAudioProcessor();
}
