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
Ambix_converterAudioProcessor::Ambix_converterAudioProcessor() :
    active_preset(0),
    output_buffer(16, 256),
    in_seq_param(0.f),
    out_seq_param(0.f),
    in_norm_param(0.f),
    out_norm_param(0.f),
    flip_cs_phase(false),
    flip_param(false),
    flop_param(false),
    flap_param(false),
    in_2d(false),
    out_2d(false),
    ch_norm_flat(true)
{
    // constructor
    
    in_ch_seq = acn_ch_map;
    out_ch_seq = acn_ch_map;
    
    in_2d_ch_seq = acn_2d_ch_map;
    out_2d_ch_seq = acn_2d_ch_map;
    
    in_ch_norm = conv_norm_ones;
    
    acn_cs = acn_cs_phase;
    
}

Ambix_converterAudioProcessor::~Ambix_converterAudioProcessor()
{
}

//==============================================================================
const String Ambix_converterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_converterAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_converterAudioProcessor::getParameter (int index)
{
    switch (index)
	{
		case InSeqParam:    return in_seq_param;
        case OutSeqParam:    return out_seq_param;
        case InNormParam:    return in_norm_param;
        case OutNormParam:    return out_norm_param;
        case FlipCsParam:    return (float)flip_cs_phase;
        case FlipParam:      return (float)flip_param;
        case FlopParam:      return (float)flop_param;
        case FlapParam:      return (float)flap_param;
        case In2DParam:      return (float)in_2d;
        case Out2DParam:      return (float)out_2d;
            
		default:            return 0.0f;
	}
}

void Ambix_converterAudioProcessor::setParameter (int index, float newValue)
{
    switch (index)
	{
		case InSeqParam:
            if (newValue < 0.33f) {
                in_seq_param=0.f;
            } else if (newValue >= 0.33f && newValue < 0.66f) {
                in_seq_param=0.5f;
            } else if (newValue > 0.66f) {
                in_seq_param=1.f;
            }
            setChannelSequence();
            break;
            
        case OutSeqParam:
            if (newValue < 0.33f) {
                out_seq_param=0.0f;
            } else if (newValue >= 0.33f && newValue < 0.66f) {
                out_seq_param=0.5f;
            } else if (newValue > 0.66f) {
                out_seq_param=1.f;
            }
            setChannelSequence();
            break;
            
        case InNormParam:
            if (newValue < 0.33f) {
                in_norm_param=0.f;
            } else if (newValue >= 0.33f && newValue < 0.66f) {
                in_norm_param=0.5f;
            } else if (newValue > 0.66f) {
                in_norm_param=1.f;
            }
            setNormalizationScheme();
            break;
            
        case OutNormParam:
            if (newValue < 0.33f) {
                out_norm_param=0.f;
            } else if (newValue >= 0.33f && newValue < 0.66f) {
                out_norm_param=0.5f;
            } else if (newValue > 0.66f) {
                out_norm_param=1.f;
            }
            setNormalizationScheme();
            break;
            
        case FlipCsParam:
            if (newValue <= 0.5f)
            {
                flip_cs_phase = false;
            } else {
                flip_cs_phase = true;
            }
            break;
            
        case FlipParam:
            if (newValue <= 0.5f)
            {
                flip_param = false;
            } else {
                flip_param = true;
            }
            break;
            
        case FlopParam:
            if (newValue <= 0.5f)
            {
                flop_param = false;
            } else {
                flop_param = true;
            }
            break;
            
        case FlapParam:
            if (newValue <= 0.5f)
            {
                flap_param = false;
            } else {
                flap_param = true;
            }
            break;
            
        case In2DParam:
            if (newValue <= 0.5f)
            {
                in_2d = false;
            } else {
                in_2d = true;
            }
            break;

        case Out2DParam:
            if (newValue <= 0.5f)
            {
                out_2d = false;
            } else {
                out_2d = true;
            }
            break;
            
		default:
            break;
	}
    // notfiy editor
    sendChangeMessage();
}

void Ambix_converterAudioProcessor::setChannelSequence()
{
    // in ch sequence
    
    if (in_seq_param < 0.33f) {
        in_ch_seq = acn_ch_map;
        in_2d_ch_seq = acn_2d_ch_map;
        
    } else if (in_seq_param >= 0.33f && in_seq_param < 0.66f) {
        in_ch_seq = fuma_ch_map;
        in_2d_ch_seq = fuma_2d_ch_map;
        
    } else if (in_seq_param > 0.66f) {
        in_ch_seq = sid_ch_map;
        in_2d_ch_seq = sid_2d_ch_map;
        
    }
    
    // out ch sequence
    
    if (out_seq_param < 0.33f) {
        out_ch_seq = acn_ch_map;
        out_2d_ch_seq = acn_2d_ch_map;
        
    } else if (out_seq_param >= 0.33f && out_seq_param < 0.66f) {
        out_ch_seq = fuma_ch_map;
        out_2d_ch_seq = fuma_2d_ch_map;
        
    } else if (out_seq_param > 0.66f) {
        out_ch_seq = sid_ch_map;
        out_2d_ch_seq = sid_2d_ch_map;
        
    }
}

void Ambix_converterAudioProcessor::setNormalizationScheme()
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

const String Ambix_converterAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
		case InSeqParam:			return "InChannelSeq";
        case OutSeqParam:			return "OutChannelSeq";
		case InNormParam:			return "InNormalization";
        case OutNormParam:			return "OutNormalization";
        case FlipCsParam:           return "FlipCondonShortleyPhase";
        case FlipParam:           return "MirrorLeftRight";
        case FlopParam:           return "MirrorFrontBack";
        case FlapParam:           return "MirrorTopBottom";
        case In2DParam:           return "InputIs2D";
        case Out2DParam:           return "OutputIs2D";
		default:								break;
	}
	
	return String::empty;
}

const String Ambix_converterAudioProcessor::getParameterText (int index)
{
    switch (index)
	{
		case InSeqParam:
            if (in_seq_param < 0.33f) {
                return "ACN";
            } else if (in_seq_param >= 0.33f && in_seq_param < 0.66f) {
                return "FuMa";
            } else if (in_seq_param > 0.66f) {
                return "SID";
            }
            break;
            
        case OutSeqParam:
            if (out_seq_param < 0.33f) {
                return "ACN";
            } else if (out_seq_param >= 0.33f && out_seq_param < 0.66f) {
                return "FuMa";
            } else if (out_seq_param > 0.66f) {
                return "SID";
            }
            break;
		
        case InNormParam:
            if (in_norm_param < 0.33f) {
                return "SN3D";
            } else if (in_norm_param >= 0.33f && in_norm_param < 0.66f) {
                return "FuMa";
            } else if (in_norm_param > 0.66f) {
                return "N3D";
            }
            break;
            
        case OutNormParam:
            if (out_norm_param < 0.33f) {
                return "SN3D";
            } else if (out_norm_param >= 0.33f && out_norm_param < 0.66f) {
                return "FuMa";
            } else if (out_norm_param > 0.66f) {
                return "N3D";
            }
            break;
            
        case FlipCsParam:
            if (flip_cs_phase)
                return "On";
            else
                return "Off";
            break;
            
        case FlipParam:
            if (flip_param)
                return "On";
            else
                return "Off";
            break;
            
        case FlopParam:
            if (flop_param)
                return "On";
            else
                return "Off";
            break;
            
        case FlapParam:
            if (flap_param)
                return "On";
            else
                return "Off";
            break;
            
        case In2DParam:
            if (in_2d)
                return "Yes";
            else
                return "No";
            break;
            
        case Out2DParam:
            if (out_2d)
                return "Yes";
            else
                return "No";
            break;
            
		default:								break;
	}
	
	return String::empty;
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
    return String::empty;
}

void Ambix_converterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_converterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    output_buffer.setSize(std::max(getNumOutputChannels(), getNumInputChannels()), samplesPerBlock);
}

void Ambix_converterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_converterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // do the audio processing....
    // defines are until 10th order 3d (121 channels)!
    
    // resize output buffer if necessare
    int NumSamples = buffer.getNumSamples();
    
    output_buffer.setSize(std::max(getNumOutputChannels(), getNumInputChannels()), NumSamples);
  
    output_buffer.clear(); // in case of 2d where we might throw away some channels
    
    // std::cout << "NumInputChannels: " << getNumInputChannels() << " Buffersize: " << buffer.getNumChannels() << std::endl;
    
    for (int i = 0; i < getNumInputChannels(); i++) // iterate over acn channel numbering
    {
        
        int l = 0; // sometimes called n
        int m = 0; // -l...l
        
        ACNtoLM(i, l, m);
        
        
        int _in_ch_seq = in_ch_seq[i];
        int _out_ch_seq = out_ch_seq[i];
        
        if (in_2d)
        {
            _in_ch_seq = in_2d_ch_seq[ACN3DtoACN2D(i)];
            if (_in_ch_seq > getNumInputChannels())
                _in_ch_seq = -1;
        }
        
        if (out_2d)
        {
            _out_ch_seq = out_2d_ch_seq[ACN3DtoACN2D(i)];
            if (_out_ch_seq > getNumOutputChannels())
                _out_ch_seq = -1;
        }
        
        
        // std::cout << "InputCh: " << i << " IN_CHANNEL: " << _in_ch_seq << " OUT_CHANNEL: " << _out_ch_seq << std::endl;
        
        if (_in_ch_seq != -1 && _out_ch_seq != -1 && _in_ch_seq < getNumInputChannels())
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
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        output_buffer.clear (i, 0, buffer.getNumSamples());
    }
    */
    
    buffer = output_buffer;
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
    
    xml.setAttribute("in_seq_param", in_seq_param);
    xml.setAttribute("out_seq_param", out_seq_param);
    xml.setAttribute("in_norm_param", in_norm_param);
    xml.setAttribute("out_norm_param", out_norm_param);
    xml.setAttribute("flip_cs_phase", flip_cs_phase);
    xml.setAttribute("flip_param", flip_param);
    xml.setAttribute("flop_param", flop_param);
    xml.setAttribute("flap_param", flap_param);
    xml.setAttribute("in_2d_param", in_2d);
    xml.setAttribute("out_2d_param", out_2d);
    
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_converterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            
            box_presets_text = xmlState->getStringAttribute("box_presets", "");
            setParameterNotifyingHost(FlipCsParam, (float) xmlState->getDoubleAttribute("flip_cs_phase"));
            setParameterNotifyingHost(FlipParam, (float) xmlState->getDoubleAttribute("flip_param"));
            setParameterNotifyingHost(FlopParam, (float) xmlState->getDoubleAttribute("flop_param"));
            setParameterNotifyingHost(FlapParam, (float) xmlState->getDoubleAttribute("flap_param"));
            setParameterNotifyingHost(InSeqParam, (float) xmlState->getDoubleAttribute("in_seq_param"));
            setParameterNotifyingHost(OutSeqParam, (float) xmlState->getDoubleAttribute("out_seq_param"));
            setParameterNotifyingHost(InNormParam, (float) xmlState->getDoubleAttribute("in_norm_param"));
            setParameterNotifyingHost(OutNormParam, (float) xmlState->getDoubleAttribute("out_norm_param"));
            setParameterNotifyingHost(In2DParam, (float) xmlState->getDoubleAttribute("in_2d_param"));
            setParameterNotifyingHost(Out2DParam, (float) xmlState->getDoubleAttribute("out_2d_param"));
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
