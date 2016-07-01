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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include <Eigen/Eigen>

//==============================================================================
/**
*/
class Ambix_rotatorAudioProcessor  : public AudioProcessor,
#ifdef WITH_OSC
                                    private OSCReceiver,
                                    private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
#endif
                                    public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_rotatorAudioProcessor();
    ~Ambix_rotatorAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    enum Parameters
	{
		YawParam,
        PitchParam,
        RollParam,
        RotOrderParam,
		totalNumParams
	};
    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

#ifdef WITH_OSC
    
	String osc_in_port;
    
    // JUCE OSC
    void oscMessageReceived (const OSCMessage& message);
#endif
    
private:
    
    void calcParams();
    
    // yaw (z)
    float yaw_param;
    
    // pitch (y)
    float pitch_param;
    
    // roll (x)
    float roll_param;
    
    // rotation order :  < 0.5: ypr; >=0.5 rpy
    float rot_order_param;
    
    bool _initialized;
    bool _new_params;
    
    AudioSampleBuffer output_buffer;
    
    
    SphericalHarmonic sph_h;
    
    // Eigen stuff
    
    Eigen::MatrixXd Carth_coord; // carthesian coordinates
    Eigen::MatrixXd Sph_coord; // spherical coordinates
    
    
    // Eigen::VectorXd Ymn; // Ymn result
    Eigen::MatrixXd Sh_matrix_inv; // not rotated SH Matrix
    Eigen::MatrixXd Sh_matrix_mod; // rotated SH Matrix
    
    Eigen::MatrixXd Sh_transf; // SH Rotation Matrix
    
    Eigen::MatrixXd _Sh_transf; // Old SH Rotation Matrix
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_rotatorAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
