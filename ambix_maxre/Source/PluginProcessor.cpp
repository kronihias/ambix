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

#define DEGTORAD M_PI/180.0

//==============================================================================
Ambix_maxreAudioProcessor::Ambix_maxreAudioProcessor() :
apply_param(0.5f),
_apply_param(0.5f),
order_param(1.f),
order(AMBI_ORDER),
_order(-1)
{
    weights.resize(AMBI_ORDER+1);
    
    // set all weights
    for (int i=0; i < weights.size(); i++) {
        weights.set(i, 1.f);
    }
    
    CalcParams();
    
}

Ambix_maxreAudioProcessor::~Ambix_maxreAudioProcessor()
{
}

//==============================================================================
const String Ambix_maxreAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_maxreAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_maxreAudioProcessor::getParameter (int index)
{
    switch (index)
	{
        case ApplyParam:
            return apply_param;
        case OrderParam:
            return order_param;
            
		default:
            return 0.f;
	}
}

void Ambix_maxreAudioProcessor::setParameter (int index, float newValue)
{
    switch (index)
	{
        case ApplyParam:
            if (newValue < 0.33f)
            {
                apply_param = 0.f;
            } else if ((newValue >= 0.33f) && (newValue < 0.66f))
            {
                apply_param = 0.5f;
            } else {
                apply_param = 1.f;
            }
            break;
        case OrderParam:
            order_param = newValue;
            order = (int)round(order_param*AMBI_ORDER); // AMBI_ORDER is maximum
            break;
            
		default:
            break;
	}
}

const String Ambix_maxreAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
        case ApplyParam:
            return "apply";
        case OrderParam:
            return "order";
            
		default:
            return String::empty;
	}
}

const String Ambix_maxreAudioProcessor::getParameterText (int index)
{
    switch (index)
	{
        case ApplyParam:
            if (apply_param < 0.33f)
            {
                return "inverse max_re weight";
            } else if ((apply_param >= 0.33f) && (apply_param < 0.66f))
            {
                return "no weighting";
            } else {
                return "max_re weight";
            }
        case OrderParam:
            return String(order);
            
		default:
            return String::empty;
	}
    
}

const String Ambix_maxreAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_maxreAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_maxreAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_maxreAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_maxreAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_maxreAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_maxreAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_maxreAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_maxreAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_maxreAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_maxreAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_maxreAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void Ambix_maxreAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_maxreAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
}

void Ambix_maxreAudioProcessor::releaseResources()
{
    
}

inline double calccostheta(int N)
{
    double deg = 137.9/((double)N+1.51);
    return cos(deg*DEGTORAD);
}

void Ambix_maxreAudioProcessor::CalcParams()
{
    if (_order != order || _apply_param != apply_param)
    {
        costheta = calccostheta(order); // 137.9°/(N+1.51)
        
        legendre_u.Calc(order, costheta);
        Pn = legendre_u.Get();
        
        if (apply_param < 0.33f) // apply reciprokal
        {
            for (int i=0; i < Pn.size(); i++) {
                weights.set(i, (float)1./Pn(i));
            }
        } else {
            for (int i=0; i < Pn.size(); i++) {
                weights.set(i, (float)Pn(i));
            }
        }
        
        for (int i=Pn.size(); i < weights.size(); i++)
        {
            weights.set(i, 0.f);
        }
        
        /*
        std::cout << "weights: ";
        for (int i=0; i < weights.size(); i++)
        {
            std::cout << weights.getUnchecked(i) << " ";
        }
        std::cout << std::endl;
        */
        
        _order = order;
        _apply_param = apply_param;
    }
    
}

void Ambix_maxreAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    CalcParams();
    
    // do scaling only if weighting wanted
    if (apply_param < 0.33f || apply_param > 0.66f) {
        
        int numSamples = buffer.getNumSamples();
        int l = 0; // 0, 1, 2, 3, 4, ......
        int m = 0;
        
        for (int i=0; i < buffer.getNumChannels(); i++) {
            
            ACNtoLM(i, l, m); // get order depending on ACN
            
            // weight the channel, no interpolation!
            
            buffer.applyGain(i, 0, numSamples, weights.getUnchecked(l));
            
        }
    }
}

//==============================================================================
bool Ambix_maxreAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_maxreAudioProcessor::createEditor()
{
    //return new Ambix_maxreAudioProcessorEditor (this);
    return nullptr;
}

//==============================================================================
void Ambix_maxreAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Ambix_maxreAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_maxreAudioProcessor();
}
