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

#define _USE_MATH_DEFINES
#include <cmath>

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
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    int getNumParameters() override;

    float getParameter (int index) override;
    void setParameter (int index, float newValue) override;

    const String getParameterName (int index) override;
    const String getParameterText (int index) override;

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    enum Parameters
	{
		YawParam,
        PitchParam,
        RollParam,
        RotOrderParam,
        Q0Param,
        Q1Param,
        Q2Param,
        Q3Param,
        Qinvert,
		totalNumParams
	};
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

#ifdef WITH_OSC
    
	String osc_in_port;
    
    // JUCE OSC
    void oscMessageReceived (const OSCMessage& message) override;
#endif
    
    bool isQuaternionActive(); // returns true if currently quaternions are used, false if euler angles are used

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
    
    float q0_param;
    float q1_param;
    float q2_param;
    float q3_param;
    float qinvert_param;

    bool _q_changed; // flag for telling calcParams which parameters to use for calculating rotationmatrix

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
