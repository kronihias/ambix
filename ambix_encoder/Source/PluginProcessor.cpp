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
                        myProperties(),
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
    
    
    PropertiesFile::Options prop_options;
    prop_options.applicationName = "settings";
    prop_options.commonToAllUsers = false;
    prop_options.filenameSuffix = "xml";
    prop_options.folderName = "ambix/settings";
    prop_options.storageFormat = PropertiesFile::storeAsXML;
    // options.storageFormat = PropertiesFile::storeAsBinary;
    prop_options.ignoreCaseOfKeyNames = true;
    prop_options.osxLibrarySubFolder = "Application Support";
    
    myProperties.setStorageParameters(prop_options);
    
#if WITH_OSC

    osc_in = false;
	osc_out = false;
	
	osc_in_port = "0";
	
	osc_out_ip = myProperties.getUserSettings()->getValue("osc_out_ip", "localhost");
	osc_out_port = myProperties.getUserSettings()->getValue("osc_out_port", "7130");
    
    osc_interval = myProperties.getUserSettings()->getIntValue("osc_out_interval", 50);
    
    osc_out = myProperties.getUserSettings()->getBoolValue("osc_out", true);
    osc_in = myProperties.getUserSettings()->getBoolValue("osc_in", true);
    
    oscReceiver = new OSCReceiver;
    
    oscOut(osc_out);
    oscIn(osc_in);

#endif
    
}

Ambix_encoderAudioProcessor::~Ambix_encoderAudioProcessor()
{
    Ambix_encoderAudioProcessor::s_ID--; // instance counter
    
#if WITH_OSC
    oscIn(false);
    oscOut(false);
#endif
    
    myProperties.closeFiles();
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
        OSCMessage mymsg = OSCMessage("/ambi_enc");
        mymsg.addInt32(m_id); // source id
        mymsg.addString("test"); // name... currently unused
        mymsg.addFloat32(2.0f); // distance... currently unused
        mymsg.addFloat32(360.f*(azimuth_param-0.5f)); // azimuth -180....180°
        mymsg.addFloat32(360.f*(elevation_param-0.5f)); // elevation -180....180°
        mymsg.addFloat32(size_param); // size param 0.0 ... 1.0
        mymsg.addFloat32(dpk); // digital peak value linear 0.0 ... 1.0 (=0dBFS)
        mymsg.addFloat32(rms); // rms value linear 0.0 ... 1.0 (=0dBFS)
        
        if(osc_in)
        {
            mymsg.addInt32(osc_in_port.getIntValue()); // osc receiver udp port
        }
        
        for (int i = 0; i < oscSenders.size(); i++) {
            oscSenders.getUnchecked(i)->send(mymsg);
        }
        
        _azimuth_param = azimuth_param; // change buffers
        _elevation_param = elevation_param;
        _size_param = size_param;
        _rms = rms;
        _dpk = dpk;
    }
    
}


// this is called if an OSC message is received
void Ambix_encoderAudioProcessor::oscMessageReceived (const OSCMessage& message)
{
    // /ambi_enc_set <id> <distance> <azimuth> <elevation> <size>
    
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
    
    setParameterNotifyingHost(Ambix_encoderAudioProcessor::AzimuthParam, jlimit(0.f, 1.f, (val[2]+180.f)/360.f) );
    setParameterNotifyingHost(Ambix_encoderAudioProcessor::ElevationParam, jlimit(0.f, 1.f, (val[3]+180.f)/360.f) );
	setParameterNotifyingHost(Ambix_encoderAudioProcessor::SizeParam, jlimit(0.f, 1.f, val[4]));
    
}

void Ambix_encoderAudioProcessor::oscOut(bool arg)
{
	if (osc_out) {
        stopTimer();
        
        oscSenders.clear();
        
        osc_out = false;
        
    }
    
    if (arg)
    {
        bool suc = false;
        
        // parse all ip/port configurations
        
        String tmp_out_ips = osc_out_ip.trim();
        String tmp_out_ports = osc_out_port.trim();
        
        String tmp_ip;
        String tmp_port;
        
        while (tmp_out_ips.length() > 0 || tmp_out_ports.length() > 0) {
            
            if (tmp_out_ips.length() > 0)
                tmp_ip = tmp_out_ips.upToFirstOccurrenceOf(";", false, false);
            
            if (tmp_out_ports.length() > 0)
                tmp_port = tmp_out_ports.upToFirstOccurrenceOf(";", false, false);
            
            // create new sender
            
            if (tmp_ip.equalsIgnoreCase("localhost"))
                tmp_ip = "127.0.0.1";
            
            oscSenders.add(new OSCSender());
            suc = oscSenders.getLast()->connect(tmp_ip, tmp_port.getIntValue()) ? true : suc;
            
            
            // std::cout << "add sender: " << tmp_ip << " " << tmp_port << " success: " << suc << std::endl;
            
            // trim
            tmp_out_ips = tmp_out_ips.fromFirstOccurrenceOf(";", false, false).trim();
            
            tmp_out_ports = tmp_out_ports.fromFirstOccurrenceOf(";", false, false).trim();
            
        }
        
        if (suc)
        {
            osc_out = true;
            startTimer(osc_interval); // osc send rate
        }
        
        
	}
    
}

void Ambix_encoderAudioProcessor::oscIn(bool arg)
{
	if (arg) {
        
        int port = 7200+m_id;
        
        
        Random rand(Time::currentTimeMillis());
        
        int trials = 0;
        
        while (trials++ < 10)
        {
            // std::cout << "try to connect to port " << port << std::endl;

            
            if (oscReceiver->connect(port))
            {
                oscReceiver->addListener (this, "/ambi_enc_set");
                
                osc_in_port = String(port);
                osc_in = true;
                
                break;
            }
            
            port += rand.nextInt(999);
            
        }
        
	} else { // turn off osc
        
        osc_in = false;
        
        oscReceiver->removeListener(this);
        
        oscReceiver->disconnect();
        
	}
}

void Ambix_encoderAudioProcessor::changeTimer(int time)
{
    if (osc_out)
    {
        stopTimer();
        osc_interval = time;
        startTimer(time);
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
    // Create an outer XML element..
    
    XmlElement xml ("MYPLUGINSETTINGS");
    
    // add some attributes to it..
    for (int i=0; i < getNumParameters(); i++)
    {
        xml.setAttribute (String(i), getParameter(i));
    }
  
    xml.setAttribute ("mID", m_id);
  
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_encoderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
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
        }
        
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_encoderAudioProcessor();
}
