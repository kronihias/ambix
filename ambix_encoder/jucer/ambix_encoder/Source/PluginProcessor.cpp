/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Ambix_encoderAudioProcessor::Ambix_encoderAudioProcessor()
{
}

Ambix_encoderAudioProcessor::~Ambix_encoderAudioProcessor()
{
}

//==============================================================================
const String Ambix_encoderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_encoderAudioProcessor::getNumParameters()
{
    return 0;
}

float Ambix_encoderAudioProcessor::getParameter (int index)
{
    return 0.0f;
}

void Ambix_encoderAudioProcessor::setParameter (int index, float newValue)
{
}

const String Ambix_encoderAudioProcessor::getParameterName (int index)
{
    return String::empty;
}

const String Ambix_encoderAudioProcessor::getParameterText (int index)
{
    return String::empty;
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
}

void Ambix_encoderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_encoderAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
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
