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


#ifdef WITH_OSC
void Ambix_rotator_zAudioProcessor::oscMessageReceived (const OSCMessage& message)
{

    if (message.getAddressPattern() == OSCAddressPattern("/rotation")) {
        // /rotation [pitch] [yaw] [roll]
        float val=0.f;

        if (message[1].getType() == OSCTypes::float32)
        {
            val = (float)message[1].getFloat32();
        }
        else if (message[1].getType() == OSCTypes::int32)
        {
            val = (float)message[1].getInt32();
        }

        setParameterNotifyingHost(Ambix_rotator_zAudioProcessor::RotZParam, jlimit(0.f, 1.f, val/360.f+0.5f));


    } else if (message.getAddressPattern() == OSCAddressPattern("/head_pose")) {
        // /head_pose [User_ID] [x] [y] [z] [pitch] [yaw] [roll]

        float val=0.f;

        if (message[5].getType() == OSCTypes::float32)
        {
            val = (float)message[5].getFloat32();
        }
        else if (message[5].getType() == OSCTypes::int32)
        {
            val = (float)message[5].getInt32();
        }

        setParameterNotifyingHost(Ambix_rotator_zAudioProcessor::RotZParam, jlimit(0.f, 1.f, val/360.f+0.5f));
    }

// debug the message
#if 0
    std::cout << "osc message received: " << message.getAddressPattern().toString() << " ";

    for (int i=0; i<message.size(); i++) {

        if (message[i].getType() == OSCTypes::float32)
        {
            std::cout << "[f] " << message[i].getFloat32() << " ";
        }
        else if (message[i].getType() == OSCTypes::int32)
        {
            std::cout << "[i] " << message[i].getInt32() << " ";
        }
        else if (message[i].getType() == OSCTypes::string)
            std::cout << "[s] " << message[i].getString() << " ";
    }
    std::cout << std::endl;
#endif

}
#endif


//==============================================================================
Ambix_rotator_zAudioProcessor::Ambix_rotator_zAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::discreteChannels(AMBI_CHANNELS), true)
        .withOutput ("Output", juce::AudioChannelSet::discreteChannels(AMBI_CHANNELS), true)
    ),
    rot_z_param(0.5f),
    output_buffer(16, 256)
{
    cos_z.resize(AMBI_ORDER+1);
    sin_z.resize(AMBI_ORDER+1);

    cos_z.set(0, 1.f);
    sin_z.set(0, 0.f);

    _cos_z.resize(AMBI_ORDER+1);
    _sin_z.resize(AMBI_ORDER+1);

    _cos_z.set(0, 1.f);
    _sin_z.set(0, 0.f);

#ifdef WITH_OSC
    osc_in_port ="7120";

    // specify here on which UDP port number to receive incoming OSC messages
    if (! connect (osc_in_port.getIntValue()))
    {
        std::cout << "Could not connect to port " << osc_in_port << std::endl;
    } else {
        OSCReceiver::addListener (this);
    }
#endif

}

Ambix_rotator_zAudioProcessor::~Ambix_rotator_zAudioProcessor()
{

}

//==============================================================================
const String Ambix_rotator_zAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_rotator_zAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_rotator_zAudioProcessor::getParameter (int index)
{
    switch (index)
	{
		case RotZParam:    return rot_z_param;
		default:            return 0.0f;
	}
}

void Ambix_rotator_zAudioProcessor::setParameter (int index, float newValue)
{
    switch (index)
	{
		case RotZParam:
            rot_z_param = newValue;
            break;
		default:
            break;
	}
}

const String Ambix_rotator_zAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
		case RotZParam:			return "Z-axis rot";
		default:								break;
	}

	return String();
}

const String Ambix_rotator_zAudioProcessor::getParameterText (int index)
{

    float value;

    switch (index)
	{
		case RotZParam:
            value = ((rot_z_param-0.5f) * 360.f);
            break;


		default:
            break;
	}


    String text;
    text << (int)value;
    text << " deg";
	return text;
}

const String Ambix_rotator_zAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_rotator_zAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_rotator_zAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_rotator_zAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_rotator_zAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_rotator_zAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_rotator_zAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_rotator_zAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_rotator_zAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_rotator_zAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_rotator_zAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_rotator_zAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_rotator_zAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================



void Ambix_rotator_zAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // resize buffer if necessary
    output_buffer.setSize((std::max)(getTotalNumOutputChannels(), getTotalNumInputChannels()), samplesPerBlock);

    /*
    int l=0;
    int m=0;
    for (int i=0; i <= 25; i++)
    {
        ACNtoLM(i, l, m);
        std::cout << "ACN: " << i << " l: " << l << " m: " << m << std::endl;
    }
     */

}

void Ambix_rotator_zAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_rotator_zAudioProcessor::calcParams()
{

    // use mathematical negative angles!

    cos_z.set(1, cosf(-(rot_z_param - 0.5f) * (float)_2PI)); // rot_z_param is from 0.0 -> 1.0
    sin_z.set(1, sinf(-(rot_z_param - 0.5f) * (float)_2PI)); // -> map it to -pi...+pi ( -180....+180deg )

    // chebyshev recursion
    for (int i = 2; i <= AMBI_ORDER; i++) {

        cos_z.set(i, 2 * cos_z[1] * cos_z[i-1] - cos_z[i-2]);

        sin_z.set(i, 2 * cos_z[1] * sin_z[i-1] - sin_z[i-2]);

    }
}

bool Ambix_rotator_zAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
}

void Ambix_rotator_zAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{

    int NumSamples = buffer.getNumSamples();

    // resize buffer if necessary
    output_buffer.setSize((std::max)(getTotalNumOutputChannels(), getTotalNumInputChannels()), NumSamples);

    // clear output buffer
    output_buffer.clear();

    // save old parameters for interpolation (start ramp)
    _cos_z = cos_z;
    _sin_z = sin_z;

    // calculate new parameters (end ramp)
    calcParams();


    // iterate over input and output channels for z axis rotation
    // this should work for arbitary ambisonics orders!

    for (int acn_out = 0; acn_out < getTotalNumOutputChannels(); acn_out++)
    {
        int l_out = 0;
        int m_out = 0;

        ACNtoLM(acn_out, l_out, m_out);

        for (int acn_in = 0; acn_in < getTotalNumInputChannels(); acn_in++)
        {
            int l_in=0; // degree 0, 1, 2, 3, 4, ......
            int m_in=0; // order ...., -2, -1, 0 , 1, 2, ...

            ACNtoLM(acn_in, l_in, m_in);

            if (abs(m_out) == abs (m_in) && l_in == l_out) { // if degree and order match  do something

                if (m_out == 0 && m_in == 0) {
                    // gain 1 -> no interpolation needed
                    output_buffer.copyFrom(acn_out, 0, buffer, acn_in, 0, NumSamples);
                }
                else  if (m_in < 0 && m_out < 0)
                {
                    output_buffer.addFromWithRamp(acn_out, 0, buffer.getReadPointer(acn_in), NumSamples, _cos_z[-m_out], cos_z[-m_out]); // interpolation with ramp done by juce
                }
                else  if (m_in < 0 && m_out > 0)
                {
                    output_buffer.addFromWithRamp(acn_out, 0, buffer.getReadPointer(acn_in), NumSamples, -_sin_z[m_out], -sin_z[m_out]);
                }
                else  if (m_in > 0 && m_out > 0)
                {
                    output_buffer.addFromWithRamp(acn_out, 0, buffer.getReadPointer(acn_in), NumSamples, _cos_z[m_out], cos_z[m_out]);
                }
                else  if (m_in > 0 && m_out < 0)
                {
                    output_buffer.addFromWithRamp(acn_out, 0, buffer.getReadPointer(acn_in), NumSamples, _sin_z[m_in], sin_z[m_in]);
                }

            }

        }

    }

    // return output buffer
    buffer = output_buffer;
}

//==============================================================================
bool Ambix_rotator_zAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_rotator_zAudioProcessor::createEditor()
{
    //return new Ambix_rotator_zAudioProcessorEditor (this);
    return nullptr;
}

//==============================================================================
void Ambix_rotator_zAudioProcessor::getStateInformation (MemoryBlock& destData)
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

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_rotator_zAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
        }

    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_rotator_zAudioProcessor();
}
