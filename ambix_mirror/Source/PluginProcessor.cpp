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
    x_even_param(0.75f),
    x_even_inv_param(0.f),
    x_odd_param(0.75f),
    x_odd_inv_param(0.f),
    y_even_param(0.75f),
    y_even_inv_param(0.f),
    y_odd_param(0.75f),
    y_odd_inv_param(0.f),
    z_even_param(0.75f),
    z_even_inv_param(0.f),
    z_odd_param(0.75f),
    z_odd_inv_param(0.f),
    circular_param(0.75f),
    circular_inv_param(0.f),
    preset_param(0.f),
    preset_name("")
{
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

int Ambix_mirrorAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_mirrorAudioProcessor::getParameter (int index)
{
    switch (index)
	{
		case XEvenParam:    return x_even_param;
		case XEvenInvParam:    return x_even_inv_param;
        case XOddParam:     return x_odd_param;
		case XOddInvParam:    return x_odd_inv_param;
		case YEvenParam:    return y_even_param;
		case YEvenInvParam:    return y_even_inv_param;
        case YOddParam:     return y_odd_param;
		case YOddInvParam:    return y_odd_inv_param;
		case ZEvenParam:    return z_even_param;
		case ZEvenInvParam:    return z_even_inv_param;
        case ZOddParam:     return z_odd_param;
		case ZOddInvParam:    return z_odd_inv_param;
        case CircularParam: return circular_param;
		case CircularInvParam:    return circular_inv_param;
        case PresetParam:          return preset_param;
		default:            return 0.0f;
	}
}

void Ambix_mirrorAudioProcessor::setParameter (int index, float newValue)
{

    switch (index)
	{
        case XEvenParam:
            x_even_param = newValue;
            break;
        case XEvenInvParam:
            x_even_inv_param = newValue;
            break;
		case XOddParam:
            x_odd_param = newValue;
            break;
        case XOddInvParam:
            x_odd_inv_param = newValue;
            break;
        case YEvenParam:
            y_even_param = newValue;
            break;
        case YEvenInvParam:
            y_even_inv_param = newValue;
            break;
		case YOddParam:
            y_odd_param = newValue;
            break;
        case YOddInvParam:
            y_odd_inv_param = newValue;
            break;
        case ZEvenParam:
            z_even_param = newValue;
            break;
        case ZEvenInvParam:
            z_even_inv_param = newValue;
            break;
		case ZOddParam:
            z_odd_param = newValue;
            break;
        case ZOddInvParam:
            z_odd_inv_param = newValue;
            break;
		case CircularParam:
            circular_param = newValue;
            break;
        case CircularInvParam:
            circular_inv_param = newValue;
            break;
        case PresetParam:
            preset_param = newValue;
            SwitchPreset();
            break;
		default:
            break;
	}
    
    // notify gui
    sendChangeMessage();
}

const String Ambix_mirrorAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
        case XEvenParam:        return "X even gain";
        case XEvenInvParam:        return "X even invert";
        case XOddParam:        return "X odd gain";
        case XOddInvParam:        return "X odd invert";
        case YEvenParam:        return "Y even gain";
        case YEvenInvParam:        return "Y even invert";
        case YOddParam:        return "Y odd gain";
        case YOddInvParam:        return "Y odd invert";
        case ZEvenParam:        return "Z even gain";
        case ZEvenInvParam:        return "Z even invert";
        case ZOddParam:        return "Z odd gain";
        case ZOddInvParam:        return "Z odd invert";
        case CircularParam:        return "Circular gain";
        case CircularInvParam:        return "Circular invert";
        case PresetParam:           return "Preset";
		default:								break;
	}
	
	return String::empty;
}

const String Ambix_mirrorAudioProcessor::getParameterText (int index)
{
    
    String text;
    
    switch (index)
	{
        case XEvenParam:
            text = String(ParamToDB(x_even_param)).substring(0, 5);
            text << " dB";
            break;
        case XEvenInvParam:
            if (x_even_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
		case XOddParam:
            text = String(ParamToDB(x_odd_param)).substring(0, 5);
            text << " dB";
            break;
        case XOddInvParam:
            if (x_odd_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
        case YEvenParam:
            text = String(ParamToDB(y_even_param)).substring(0, 5);
            text << " dB";
            break;
        case YEvenInvParam:
            if (y_even_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
		case YOddParam:
            text = String(ParamToDB(y_odd_param)).substring(0, 5);
            text << " dB";
            break;
        case YOddInvParam:
            if (y_odd_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
        case ZEvenParam:
            text = String(ParamToDB(z_even_param)).substring(0, 5);
            text << " dB";
            break;
        case ZEvenInvParam:
            if (z_even_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
		case ZOddParam:
            text = String(ParamToDB(z_odd_param)).substring(0, 5);
            text << " dB";
            break;
        case ZOddInvParam:
            if (z_odd_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
		case CircularParam:
            text = String(ParamToDB(circular_param)).substring(0, 5);
            text << " dB";
            break;
        case CircularInvParam:
            if (circular_inv_param >= 0.5f)
                text = "Invert!";
            else
                text = "No";
            break;
            
        case PresetParam:
            text = preset_name;
            break;
		default:
            break;
	}
    
	return text;
}

void Ambix_mirrorAudioProcessor::SwitchPreset()
{
    
    int active_preset = (int)(preset_param * NUM_PRESETS);
    
    // 1 do nothing...
    preset_name = "";
    
    // first set all zero
    if (active_preset > 1)
    {
        setParameterNotifyingHost(XEvenParam, 0.75f);
        setParameterNotifyingHost(XEvenInvParam, 0.f);
        setParameterNotifyingHost(XOddParam, 0.75f);
        setParameterNotifyingHost(XOddInvParam, 0.f);
        
        setParameterNotifyingHost(YEvenParam, 0.75f);
        setParameterNotifyingHost(YEvenInvParam, 0.f);
        setParameterNotifyingHost(YOddParam, 0.75f);
        setParameterNotifyingHost(YOddInvParam, 0.f);
        
        setParameterNotifyingHost(ZEvenParam, 0.75f);
        setParameterNotifyingHost(ZEvenInvParam, 0.f);
        setParameterNotifyingHost(ZOddParam, 0.75f);
        setParameterNotifyingHost(ZOddInvParam, 0.f);
        
        setParameterNotifyingHost(CircularParam, 0.75f);
        setParameterNotifyingHost(CircularInvParam, 0.f);
    }
    
    switch (active_preset) {
        case 2:
            // all zero done...
            preset_name = "no change";
            break;
            
        case 3: // flip left right -> invert Y Odd
            setParameterNotifyingHost(YOddInvParam, 1.f);
            preset_name = "flip left <> right";
            break;
            
        case 4: // flop front back -> invert X Odd
            setParameterNotifyingHost(XOddInvParam, 1.f);
            preset_name = "flop front <> back";
            break;
            
        case 5: // flap top bottom -> invert Z Odd
            setParameterNotifyingHost(ZOddInvParam, 1.f);
            preset_name = "flap top <> bottom";
            break;
            
        case 6: // merge left+right
            setParameterNotifyingHost(YOddParam, 0.f); // zero
            setParameterNotifyingHost(YEvenParam, 1.f); // +6 dB
            setParameterNotifyingHost(CircularParam, 0.530959f); // -6 dB
            preset_name = "merge left + right";
            break;
            
        case 7: // merge front+back
            setParameterNotifyingHost(XOddParam, 0.f); // zero
            setParameterNotifyingHost(XEvenParam, 1.f); // +6 dB
            setParameterNotifyingHost(CircularParam, 0.530959f); // -6 dB
            preset_name = "merge front+back";
            break;
            
        case 8: // merge top+bottom
            setParameterNotifyingHost(ZOddParam, 0.f); // zero
            setParameterNotifyingHost(ZEvenParam, 1.f); // +6 dB
            setParameterNotifyingHost(CircularParam, 0.530959f); // -6 dB
            preset_name = "merge top+bottom";
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
    return String::empty;
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
    for (int i=0; i < gain_factors.size(); i++) {
        gain_factors.set(i, 1.f);
    }
    
    for (int acn = 0; acn < AMBI_CHANNELS; acn++)
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

void Ambix_mirrorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    
    int NumSamples = buffer.getNumSamples();
    
    
    // save old parameters for interpolation (start ramp)
    _gain_factors = gain_factors;
    
    calcParams();
    
    
    for (int acn = 0; acn < getNumInputChannels(); acn++)
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
}

void Ambix_mirrorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_mirrorAudioProcessor();
}
