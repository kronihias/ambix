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
#include "PluginEditor.h"
#include "SphericalHarmonic/ch_sequence.h" // defines channel mappings
#include "SphericalHarmonic/normalization.h" // defines conversion between normalization schemes
#include "SphericalHarmonic/ch_cs.h" // defines condon shortley scheme in acn channel sequence
#include "SphericalHarmonic/tools.h"

//==============================================================================
namespace
{
    // the legacy tri-state switch: < 0.33 -> 0, [0.33, 0.66) -> 0.5,
    // > 0.66 -> 1. Exactly 0.66f matches no branch, so legacy kept the value
    // it had: the snap returns the stored value for it (all other stored
    // values, 0 / 0.5 / 1, map to themselves as JUCE requires).
    float snapTriState (float v, const ambix::Parameter* const* self)
    {
        if (v < 0.33f)
            return 0.f;
        else if (v >= 0.33f && v < 0.66f)
            return 0.5f;
        else if (v > 0.66f)
            return 1.f;

        return *self != nullptr ? (*self)->get() : 0.f;
    }

    float snapSwitch (float v) { return v <= 0.5f ? 0.f : 1.f; }

    // legacy getParameterText(), for the stored (snapped) value
    String seqText (float v, int)
    {
        if (v < 0.33f) {
            return "ACN";
        } else if (v >= 0.33f && v < 0.66f) {
            return "FuMa";
        } else if (v > 0.66f) {
            return "SID";
        }
        return String();
    }

    String normText (float v, int)
    {
        if (v < 0.33f) {
            return "SN3D";
        } else if (v >= 0.33f && v < 0.66f) {
            return "FuMa";
        } else if (v > 0.66f) {
            return "N3D";
        }
        return String();
    }

    String onOffText (float v, int)  { return v > 0.5f ? "On" : "Off"; }
    String yesNoText (float v, int)  { return v > 0.5f ? "Yes" : "No"; }

    // legacy setChannelSequence(), as a pure function of the stored values
    void channelSequence (float seq_param, const unsigned int*& ch_seq, const unsigned int*& ch_2d_seq)
    {
        if (seq_param < 0.33f) {
            ch_seq = acn_ch_map;
            ch_2d_seq = acn_2d_ch_map;

        } else if (seq_param >= 0.33f && seq_param < 0.66f) {
            ch_seq = fuma_ch_map;
            ch_2d_seq = fuma_2d_ch_map;

        } else if (seq_param > 0.66f) {
            ch_seq = sid_ch_map;
            ch_2d_seq = sid_2d_ch_map;

        }
    }

    // legacy setNormalizationScheme(), as a pure function of the stored values
    void normalizationScheme (float in_norm_param, float out_norm_param, const float*& in_ch_norm, bool& ch_norm_flat)
    {
        if (in_norm_param < 0.33f) { // in SN3D

            if (out_norm_param < 0.33f) // out SN3D
            {
                in_ch_norm = conv_norm_ones;
                ch_norm_flat = true;
            }
            else if (out_norm_param >= 0.33f && out_norm_param < 0.66f) // out FuMa
            {
                in_ch_norm = conv_norm_sn3d_fuma;
                ch_norm_flat = false;
            }
            else if (out_norm_param >= 0.66f)// out N3D
            {
                in_ch_norm = conv_norm_sn3d_n3d;
                ch_norm_flat = false;
            }

        } else if (in_norm_param >= 0.33f && in_norm_param < 0.66f) { // in FuMa

            if (out_norm_param < 0.33f) // out SN3D
            {
                in_ch_norm = conv_norm_fuma_sn3d;
                ch_norm_flat = false;
            }
            else if (out_norm_param >= 0.33f && out_norm_param < 0.66f) // out FuMa
            {
                in_ch_norm = conv_norm_ones;
                ch_norm_flat = true;
            }
            else if (out_norm_param >= 0.66f)// out N3D
            {
                in_ch_norm = conv_norm_fuma_n3d;
                ch_norm_flat = false;
            }

        } else if (in_norm_param > 0.66f) { // in N3D

            if (out_norm_param < 0.33f) // out SN3D
            {
                in_ch_norm = conv_norm_n3d_sn3d;
                ch_norm_flat = false;
            }
            else if (out_norm_param >= 0.33f && out_norm_param < 0.66f) // out FuMa
            {
                in_ch_norm = conv_norm_n3d_fuma;
                ch_norm_flat = false;
            }
            else if (out_norm_param >= 0.66f)// out N3D
            {
                in_ch_norm = conv_norm_ones;
                ch_norm_flat = true;
            }

        }
    }
}

//==============================================================================
Ambix_converterAudioProcessor::Ambix_converterAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET(AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
    active_preset(0),
    output_buffer(16, 256)
{
    // constructor

    // order, names and texts as the legacy index-based API had them; every
    // parameter starts at 0
    auto addTriState = [this] (int index, const char* name, ambix::Parameter::TextFunction text)
    {
        const ambix::Parameter* const* self = &params[index];
        addParameter (params[index] = new ambix::Parameter (*this, index, name, 0.f, std::move (text),
                                                            [self] (float v) { return snapTriState (v, self); }));
    };
    auto addSwitch = [this] (int index, const char* name, ambix::Parameter::TextFunction text)
    {
        addParameter (params[index] = new ambix::Parameter (*this, index, name, 0.f, std::move (text), snapSwitch));
    };

    addTriState (InSeqParam,   "InChannelSeq",     seqText);
    addTriState (OutSeqParam,  "OutChannelSeq",    seqText);
    addTriState (InNormParam,  "InNormalization",  normText);
    addTriState (OutNormParam, "OutNormalization", normText);
    addSwitch (FlipCsParam, "FlipCondonShortleyPhase", onOffText);
    addSwitch (FlipParam,   "MirrorLeftRight",         onOffText);
    addSwitch (FlopParam,   "MirrorFrontBack",         onOffText);
    addSwitch (FlapParam,   "MirrorTopBottom",         onOffText);
    addSwitch (In2DParam,   "InputIs2D",               yesNoText);
    addSwitch (Out2DParam,  "OutputIs2D",              yesNoText);
}

Ambix_converterAudioProcessor::~Ambix_converterAudioProcessor()
{
}

//==============================================================================
const String Ambix_converterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void Ambix_converterAudioProcessor::parameterValueSet (int, float)
{
    // Legacy setParameter() stored the (snapped) value - the parameter object
    // does that now - swapped the conversion table pointers, and posted a
    // change message. The tables are derived per block in processBlock(),
    // the editor polls.
}

const String Ambix_converterAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_converterAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_converterAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_converterAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_converterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_converterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_converterAudioProcessor::silenceInProducesSilenceOut() const
{
    return true;
}

double Ambix_converterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_converterAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_converterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_converterAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_converterAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_converterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_converterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    output_buffer.setSize(std::max(getTotalNumOutputChannels(), getTotalNumInputChannels()), samplesPerBlock);
}

void Ambix_converterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool Ambix_converterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
#endif
}

void Ambix_converterAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    sendChangeMessage();
#endif
}

void Ambix_converterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // do the audio processing....
    // defines are until 10th order 3d (121 channels)!

    // resize output buffer if necessare
    int NumSamples = buffer.getNumSamples();

    output_buffer.setSize(std::max(getTotalNumOutputChannels(), getTotalNumInputChannels()), NumSamples, false, false, true);

    output_buffer.clear(); // in case of 2d where we might throw away some channels

    // one read per parameter and block; the tables follow from the values
    // exactly as legacy setChannelSequence() / setNormalizationScheme() did
    const unsigned int* in_ch_seq = acn_ch_map;
    const unsigned int* out_ch_seq = acn_ch_map;
    const unsigned int* in_2d_ch_seq = acn_2d_ch_map;
    const unsigned int* out_2d_ch_seq = acn_2d_ch_map;
    const float* in_ch_norm = conv_norm_ones; // holds normalization conversion array
    bool ch_norm_flat = true;
    const int* acn_cs = acn_cs_phase; // holds condon-shortley conversion map

    channelSequence (params[InSeqParam]->get(), in_ch_seq, in_2d_ch_seq);
    channelSequence (params[OutSeqParam]->get(), out_ch_seq, out_2d_ch_seq);
    normalizationScheme (params[InNormParam]->get(), params[OutNormParam]->get(), in_ch_norm, ch_norm_flat);

    const bool flip_cs_phase = params[FlipCsParam]->get() > 0.5f;
    const bool flip_param    = params[FlipParam]->get() > 0.5f;
    const bool flop_param    = params[FlopParam]->get() > 0.5f;
    const bool flap_param    = params[FlapParam]->get() > 0.5f;
    const bool in_2d         = params[In2DParam]->get() > 0.5f;
    const bool out_2d        = params[Out2DParam]->get() > 0.5f;

    // std::cout << "NumInputChannels: " << getTotalNumInputChannels() << " Buffersize: " << buffer.getNumChannels() << std::endl;

    for (int i = 0; i < getTotalNumInputChannels(); i++) // iterate over acn channel numbering
    {

        int l = 0; // sometimes called n
        int m = 0; // -l...l

        ACNtoLM(i, l, m);


        // 3D-only channels (l != |m|) have no 2D equivalent; ACN3DtoACN2D
        // returns -1 for them. Skip — indexing the 2D map with -1 is UB.
        int acn_2d_idx = (in_2d || out_2d) ? ACN3DtoACN2D(i) : 0;
        if ((in_2d || out_2d) && acn_2d_idx < 0)
            continue;

        int _in_ch_seq = in_2d ? in_2d_ch_seq[acn_2d_idx] : in_ch_seq[i];
        int _out_ch_seq = out_2d ? out_2d_ch_seq[acn_2d_idx] : out_ch_seq[i];

        // std::cout << "InputCh: " << i << " IN_CHANNEL: " << _in_ch_seq << " OUT_CHANNEL: " << _out_ch_seq << std::endl;

        if (_in_ch_seq < getTotalNumInputChannels() && _out_ch_seq < getTotalNumOutputChannels())
        {
            // copy input channels to output channels!
            output_buffer.copyFrom(_out_ch_seq, 0, buffer, _in_ch_seq, 0, NumSamples);

            //  apply normalization conversion gain if different input/output scheme
            if (!ch_norm_flat) {
                output_buffer.applyGain(_out_ch_seq, 0, NumSamples, in_ch_norm[i]);
            }

            // do the inversions...
            if (flip_cs_phase || flip_param || flop_param || flap_param)
            {


                signed int flip, flop, flap, total;
                flip = flop = flap = total = 1;

                // taken from paper Symmetries of Spherical Harmonics by Michael Chapman (Ambi Symp 2009),

                // mirror left right
                if ( flip_param && (m < 0) ) // m < 0 -> invert
                    flip = -1;

                // mirror front back
                if ( flop_param && ( ((m < 0) && !(m % 2)) || ((m >= 0) && (m % 2)) ) ) // m < 0 and even || m >= 0 and odd
                    flop = -1;

                // mirror top bottom
                if ( flap_param && ( (l + m) % 2 ) ) // l+m odd   ( (odd, even) or (even, odd) )
                    flap = -1;

                // compute total multiplicator
                if (flip_cs_phase)
                    total = acn_cs[i] * flip * flop * flap;
                else
                    total = flip * flop * flap;

                output_buffer.applyGain(_out_ch_seq, 0, NumSamples, (float)total);
            }
        } // index not -1

    } // iterate over all input channels


    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    /*
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
    {
        output_buffer.clear (i, 0, buffer.getNumSamples());
    }
    */

    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        buffer.copyFrom(ch, 0, output_buffer, ch, 0, NumSamples);
}

//==============================================================================
bool Ambix_converterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_converterAudioProcessor::createEditor()
{
    return new Ambix_converterAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_converterAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    xml.setAttribute ("box_presets", box_presets_text);

    // same formatting as the legacy members: seq/norm as double, the
    // switches as bools (the int overload: "0" / "1")
    xml.setAttribute("in_seq_param", (double) params[InSeqParam]->get());
    xml.setAttribute("out_seq_param", (double) params[OutSeqParam]->get());
    xml.setAttribute("in_norm_param", (double) params[InNormParam]->get());
    xml.setAttribute("out_norm_param", (double) params[OutNormParam]->get());
    xml.setAttribute("flip_cs_phase", params[FlipCsParam]->get() > 0.5f);
    xml.setAttribute("flip_param", params[FlipParam]->get() > 0.5f);
    xml.setAttribute("flop_param", params[FlopParam]->get() > 0.5f);
    xml.setAttribute("flap_param", params[FlapParam]->get() > 0.5f);
    xml.setAttribute("in_2d_param", params[In2DParam]->get() > 0.5f);
    xml.setAttribute("out_2d_param", params[Out2DParam]->get() > 0.5f);


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_converterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {

        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..

            box_presets_text = xmlState->getStringAttribute("box_presets", "");

            // legacy restored through setParameterNotifyingHost(): the
            // side effects, then a host notification - keep both
            params[FlipCsParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("flip_cs_phase"));
            params[FlipParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("flip_param"));
            params[FlopParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("flop_param"));
            params[FlapParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("flap_param"));
            params[InSeqParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("in_seq_param"));
            params[OutSeqParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("out_seq_param"));
            params[InNormParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("in_norm_param"));
            params[OutNormParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("out_norm_param"));
            params[In2DParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("in_2d_param"));
            params[Out2DParam]->setValueNotifyingHost((float) xmlState->getDoubleAttribute("out_2d_param"));
        }


    }

    // reset arrays...
    //setNormalizationScheme();
    //setChannelSequence();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_converterAudioProcessor();
}
