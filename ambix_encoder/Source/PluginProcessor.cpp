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


//==============================================================================
int Ambix_encoderAudioProcessor::s_ID = 0; // for counting id!

Ambix_encoderAudioProcessor::Ambix_encoderAudioProcessor(): 
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
    osc_in = false;
	osc_out = false;
	
	osc_in_port = "7120";
	
	osc_out_ip = "127.0.0.1";
	osc_out_port = "7130";
    
    osc_error = "OSC: not receiving";
    
    osc_out = oscOut(true);
    osc_in = oscIn(true);
    
    startTimer(50); // osc send rate
#endif
    
}

Ambix_encoderAudioProcessor::~Ambix_encoderAudioProcessor()
{
    Ambix_encoderAudioProcessor::s_ID--; // instance counter
    
#if WITH_OSC
    oscIn(false);
    oscOut(false);
#endif
}

#if WITH_OSC

void Ambix_encoderAudioProcessor::timerCallback() // check if new values and call send osc
{

    if (osc_out)
    {
        if (_azimuth_param != azimuth_param ||
            _elevation_param != elevation_param ||
            _size_param != size_param ||
            _rms != rms ||
            _dpk != dpk)
            
            sendOSC();
        
    }
}

void Ambix_encoderAudioProcessor::sendOSC() // send osc data
{
    
    if (osc_out)
    {
        if(osc_in)
        {
            lo_send(addr,"/ambi_enc", "fsffffffi", (float)m_id, "test", 2.0f, 360.f*(azimuth_param-0.5f), 360.f*(elevation_param-0.5f), size_param, dpk, rms, osc_in_port.getIntValue());
        } else {
            lo_send(addr,"/ambi_enc", "fsffffff", (float)m_id, "test", 2.0f, 360.f*(azimuth_param-0.5f), 360.f*(elevation_param-0.5f), size_param, dpk, rms);
        }
        
        _azimuth_param = azimuth_param; // change buffers
        _elevation_param = elevation_param;
        _size_param = size_param;
        _rms = rms;
        _dpk = dpk;
    }
}


// /ambi_enc_set <id> <distance> <azimuth> <elevation> <size>
int ambi_enc_set_handler(const char *path, const char *types, lo_arg **argv, int argc,
                         void *data, void *user_data)
{
	Ambix_encoderAudioProcessor *me = (Ambix_encoderAudioProcessor*)user_data;
	
    me->setParameterNotifyingHost(Ambix_encoderAudioProcessor::AzimuthParam, (argv[2]->f+180.f)/360.f);
    me->setParameterNotifyingHost(Ambix_encoderAudioProcessor::ElevationParam, (argv[3]->f+180.f)/360.f);
	me->setParameterNotifyingHost(Ambix_encoderAudioProcessor::SizeParam, argv[4]->f);
    
	return 0;
}

void error(int num, const char *msg, const char *path)
{
    std::cout << "liblo server error " << num << "in path " << path << ": " << msg << std::endl;
}

bool Ambix_encoderAudioProcessor::oscOut(bool arg)
{
	if (arg) {
		addr = lo_address_new(osc_out_ip.toUTF8(), osc_out_port.toUTF8());
		arg = true;
	} else { // turn off osc out
		arg = false;
	}
	return arg;
}

bool Ambix_encoderAudioProcessor::oscIn(bool arg)
{
	if (arg) {
        st = lo_server_thread_new(NULL, error); //choose port by itself (free port)
		
		lo_server_thread_add_method(st, "/ambi_enc_set", "fffff", ambi_enc_set_handler, this);
		int res = lo_server_thread_start(st);
		
		if (res == 0)
		{
            osc_in_port = String(lo_server_thread_get_port(st));
			arg = true;
			osc_in = true;
			osc_error = "OSC: receiving on port ";
			osc_error += osc_in_port;
		} else {
			osc_error = "OSC: ERROR port is not free";
			//osc_error.formatted("OSC: ERROR %s", lo_address_errstr());
			arg = false;
			osc_in = false;
		}
	} else { // turn off osc
		if (st != NULL)
		{
			lo_server_thread_stop(st);
			#ifndef WIN32
			lo_server_thread_free(st); // this crashes in windows
			#endif
		}
        arg = false;
        osc_in = false;
        osc_error = "OSC: not receiving";
	}
	return arg;
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
    return String::empty;
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



void Ambix_encoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    int NumSamples = buffer.getNumSamples();
    
    // std::cout << "buffer size: " << NumSamples << " channels: " << buffer.getNumChannels() << std::endl;
    
#if WITH_ADVANCED_CONTROL
    // calculate new azimuth and elevation parameters if move
    calcNewParameters(SampleRate, NumSamples);
#endif

    // resize input buffer if necessary
    if (InputBuffer.getNumSamples() != NumSamples || InputBuffer.getNumChannels() != getNumInputChannels()) {
        InputBuffer.setSize(getNumInputChannels(), NumSamples);
        // std::cout << "input buffer resized: " << InputBuffer.getNumSamples() << " channels: " << InputBuffer.getNumChannels() << std::endl;
    }
    
    // clear input buffer and copy input samples
    InputBuffer.clear();
    
    for (int i=0; i < std::min(getNumInputChannels(), INPUT_CHANNELS); i++) {
        InputBuffer.copyFrom(i, 0, buffer, i, 0, NumSamples);
        // std::cout << "copied buffer channel " << i << std::endl;
    }
    
    // clear output buffer
    buffer.clear();
    
    // calculate new parameters
    for (int i=0; i < INPUT_CHANNELS; i++) {
        AmbiEnc.getUnchecked(i)->calcParams();
    }
    
    for (int in_ch=0; in_ch < std::min(getNumInputChannels(), INPUT_CHANNELS); in_ch++) {
				
				// String debug_output = "Gains: ";
				const float* in_channel_data = InputBuffer.getReadPointer(in_ch);
		
        for (int out_ch = 0; out_ch < getNumOutputChannels(); out_ch++)
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
}

void Ambix_encoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_encoderAudioProcessor();
}
