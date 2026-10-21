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
#include "SphericalHarmonic/tools.h"


//==============================================================================
Ambix_mirrorAudioProcessor::Ambix_mirrorAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET(AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    )
{
    // legacy display texts, as getParameterText() built them
    auto gainText = [] (float v, int) -> String
    {
        String text = String(ParamToDB(v)).substring(0, 5);
        text << " dB";
        return text;
    };
    auto invText    = [] (float v, int) -> String { return (v >= 0.5f) ? "Invert!" : "No"; };
    auto presetText = [] (float v, int) -> String { return presetName (v); };

    static const char* const names[totalNumParams] = {
        "X even gain", "X even invert", "X odd gain", "X odd invert",
        "Y even gain", "Y even invert", "Y odd gain", "Y odd invert",
        "Z even gain", "Z even invert", "Z odd gain", "Z odd invert",
        "Circular gain", "Circular invert", "Preset" };

    // even indices below PresetParam are gains (start at 0.75 = 0 dB), odd
    // ones their invert switches (start at 0)
    for (int i = 0; i < PresetParam; ++i)
    {
        const bool isGain = (i % 2) == 0;
        addParameter (params[i] = new ambix::Parameter (*this, i, names[i], isGain ? 0.75f : 0.f,
                                                        isGain ? ambix::Parameter::TextFunction (gainText)
                                                               : ambix::Parameter::TextFunction (invText)));
    }
    addParameter (params[PresetParam] = new ambix::Parameter (*this, PresetParam, names[PresetParam], 0.f, presetText));

    _gain_factors.resize(AMBI_CHANNELS);
    gain_factors.resize(AMBI_CHANNELS);

    for (int i=0; i<gain_factors.size(); i++) {
        gain_factors.set(i, 1.f);
        _gain_factors.set(i, 1.f);
    }
}

Ambix_mirrorAudioProcessor::~Ambix_mirrorAudioProcessor()
{

}

//==============================================================================
const String Ambix_mirrorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

const char* Ambix_mirrorAudioProcessor::presetName (float presetValue)
{
    switch ((int)(presetValue * NUM_PRESETS))
    {
        case 2: return "no change";
        case 3: return "flip left <> right";
        case 4: return "flop front <> back";
        case 5: return "flap top <> bottom";
        case 6: return "merge left + right";
        case 7: return "merge front+back";
        case 8: return "merge top+bottom";
        default: return ""; // 1 do nothing...
    }
}

void Ambix_mirrorAudioProcessor::parameterValueSet (int index, float newValue)
{
    // only the preset has a side effect: it rewrites the other parameters,
    // on every set (also when it selects the preset it already had)
    if (index == PresetParam)
        SwitchPreset (newValue);
}

void Ambix_mirrorAudioProcessor::SwitchPreset (float presetValue)
{

    int active_preset = (int)(presetValue * NUM_PRESETS);

    // the preset name (Preset's text) is presetName (presetValue)

    // first set all zero
    if (active_preset > 1)
    {
        params[XEvenParam]->setValueNotifyingHost (0.75f);
        params[XEvenInvParam]->setValueNotifyingHost (0.f);
        params[XOddParam]->setValueNotifyingHost (0.75f);
        params[XOddInvParam]->setValueNotifyingHost (0.f);

        params[YEvenParam]->setValueNotifyingHost (0.75f);
        params[YEvenInvParam]->setValueNotifyingHost (0.f);
        params[YOddParam]->setValueNotifyingHost (0.75f);
        params[YOddInvParam]->setValueNotifyingHost (0.f);

        params[ZEvenParam]->setValueNotifyingHost (0.75f);
        params[ZEvenInvParam]->setValueNotifyingHost (0.f);
        params[ZOddParam]->setValueNotifyingHost (0.75f);
        params[ZOddInvParam]->setValueNotifyingHost (0.f);

        params[CircularParam]->setValueNotifyingHost (0.75f);
        params[CircularInvParam]->setValueNotifyingHost (0.f);
    }

    switch (active_preset) {
        case 2:
            // all zero done...
            break;

        case 3: // flip left right -> invert Y Odd
            params[YOddInvParam]->setValueNotifyingHost (1.f);
            break;

        case 4: // flop front back -> invert X Odd
            params[XOddInvParam]->setValueNotifyingHost (1.f);
            break;

        case 5: // flap top bottom -> invert Z Odd
            params[ZOddInvParam]->setValueNotifyingHost (1.f);
            break;

        case 6: // merge left+right
            params[YOddParam]->setValueNotifyingHost (0.f); // zero
            params[YEvenParam]->setValueNotifyingHost (1.f); // +6 dB
            params[CircularParam]->setValueNotifyingHost (0.530959f); // -6 dB
            break;

        case 7: // merge front+back
            params[XOddParam]->setValueNotifyingHost (0.f); // zero
            params[XEvenParam]->setValueNotifyingHost (1.f); // +6 dB
            params[CircularParam]->setValueNotifyingHost (0.530959f); // -6 dB
            break;

        case 8: // merge top+bottom
            params[ZOddParam]->setValueNotifyingHost (0.f); // zero
            params[ZEvenParam]->setValueNotifyingHost (1.f); // +6 dB
            params[CircularParam]->setValueNotifyingHost (0.530959f); // -6 dB
            break;

        default:
            break;
    }

}

const String Ambix_mirrorAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_mirrorAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_mirrorAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_mirrorAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_mirrorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_mirrorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_mirrorAudioProcessor::silenceInProducesSilenceOut() const
{
    return true;
}

double Ambix_mirrorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_mirrorAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_mirrorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_mirrorAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_mirrorAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_mirrorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================



void Ambix_mirrorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..


}

void Ambix_mirrorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_mirrorAudioProcessor::calcParams()
{
    // one atomic read per parameter and block
    const float x_even_param       = params[XEvenParam]->get();
    const float x_even_inv_param   = params[XEvenInvParam]->get();
    const float x_odd_param        = params[XOddParam]->get();
    const float x_odd_inv_param    = params[XOddInvParam]->get();
    const float y_even_param       = params[YEvenParam]->get();
    const float y_even_inv_param   = params[YEvenInvParam]->get();
    const float y_odd_param        = params[YOddParam]->get();
    const float y_odd_inv_param    = params[YOddInvParam]->get();
    const float z_even_param       = params[ZEvenParam]->get();
    const float z_even_inv_param   = params[ZEvenInvParam]->get();
    const float z_odd_param        = params[ZOddParam]->get();
    const float z_odd_inv_param    = params[ZOddInvParam]->get();
    const float circular_param     = params[CircularParam]->get();
    const float circular_inv_param = params[CircularInvParam]->get();

    for (int i=0; i < gain_factors.size(); i++) {
        gain_factors.set(i, 1.f);
    }

    int numCh = juce::jmin ((int) AMBI_CHANNELS, getTotalNumInputChannels());
    for (int acn = 0; acn < numCh; acn++)
    {

        float* g = &gain_factors.getReference(acn);

        int l = 0; // manchmal auch n
        int m = 0;

        ACNtoLM(acn, l, m);

        ///////////////
        // Z SYMMETRY
        // z even symmetry
        if (acn == 0)
        {
            *g *= ParamToRMS(z_even_param);

            if (z_even_inv_param >= 0.5)
                *g *= -1;
        }
        // (l odd, m odd) || (l even, m even)
        // same as (l+m) even
        else if ( !((l + m) % 2) )
        //else if ( ((l % 2) && (m % 2)) || (!(l % 2) && !(m % 2)) )
        {
            *g *= ParamToRMS(z_even_param);

            if (z_even_inv_param >= 0.5)
                *g *= -1;
        }

        // z odd symmetry
        if (acn == 0)
        {
            // do nothing
        }
        // (l odd, m even) || (l even, m odd)
        // same as (l+m) odd
        else if ( ((l + m) % 2) )
        //else if ( ((l % 2) && !(m % 2)) ||  (!(l % 2) && (m % 2)))
        {
            *g *= ParamToRMS(z_odd_param);

            if (z_odd_inv_param >= 0.5)
                *g *= -1;
        }

        /////////////////
        // Y SYMMETRY
        // y even symmetry
        if (m >= 0)
        {
            *g *= ParamToRMS(y_even_param);

            if (y_even_inv_param >= 0.5)
                *g *= -1;
        }
        // y odd symmetry
        if (m < 0)
        {
            *g *= ParamToRMS(y_odd_param);

            if (y_odd_inv_param >= 0.5)
                *g *= -1;
        }


        /////////////////
        // X SYMMETRY
        // x even symmetry
        // m < 0 and odd || m >= 0 and even
        if ( ((m < 0) && (m % 2)) || ((m >= 0) && !(m % 2)) )
        {
            *g *= ParamToRMS(x_even_param);

            if (x_even_inv_param >= 0.5)
                *g *= -1;
        }
        // x odd symmetry
        // m < 0 and even || m >= 0 and odd
        if ( ((m < 0) && !(m % 2)) || ((m >= 0) && (m % 2)) )
        {
            *g *= ParamToRMS(x_odd_param);

            if (x_odd_inv_param >= 0.5)
                *g *= -1;
        }

        /////////////////
        // BOOST 2D (RING)

        if (l == m || l == -m)
        {
            *g *= ParamToRMS(circular_param);

            if (circular_inv_param >= 0.5)
                *g *= -1;
        }
    }
}

bool Ambix_mirrorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
#endif
}

void Ambix_mirrorAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    sendChangeMessage();
#endif
}

void Ambix_mirrorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{

    int NumSamples = buffer.getNumSamples();


    // save old parameters for interpolation (start ramp)
    // copy in place (Array::operator= reallocates)
    jassert (_gain_factors.size() == gain_factors.size());
    FloatVectorOperations::copy (_gain_factors.getRawDataPointer(), gain_factors.getRawDataPointer(), jmin (_gain_factors.size(), gain_factors.size()));

    calcParams();


    const int numCh = jmin ((int) AMBI_CHANNELS, getTotalNumInputChannels());

    for (int acn = 0; acn < numCh; acn++)
    {
        buffer.applyGainRamp(acn, 0, NumSamples, _gain_factors.getUnchecked(acn), gain_factors.getUnchecked(acn));

    }

}

//==============================================================================
bool Ambix_mirrorAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* Ambix_mirrorAudioProcessor::createEditor()
{
    return new Ambix_mirrorAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_mirrorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // Create an outer XML element..

    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it.. (same attributes and order as the legacy
    // index-based API wrote, so projects stay readable by older versions)
    for (int i=0; i < totalNumParams; i++)
    {
        xml.setAttribute (String(i), params[i]->get());
    }

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_mirrorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // restore() runs the side effects like the legacy setParameter()
            // did; the preset comes last and so overrides the others
            for (int i=0; i < totalNumParams; i++) {
                params[i]->restore ((float)xmlState->getDoubleAttribute(String(i)));
            }
        }

    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_mirrorAudioProcessor();
}
