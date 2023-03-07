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

#include "JuceHeader.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include <Eigen/Eigen>

#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

//==============================================================================
/**
*/
class Ambix_warpAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Ambix_warpAudioProcessor();
    ~Ambix_warpAudioProcessor();

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
		PhiParam,
        PhiCurveParam,
        ThetaParam,
        ThetaCurveParam,
        InOrderParam,
        OutOrderParam,
        PreEmpParam,
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

private:

    void calcParams();

    // warp azimuth
    float phi_param, _phi_param;
    float phi_curve_param, _phi_curve_param;

    // warp elevation
    float theta_param, _theta_param;
    float theta_curve_param, _theta_curve_param;

    // use pre emphasis
    float preemp_param, _preemp_param;

    // input / output ambisonics order

    float in_order_param;
    float out_order_param;

    int in_order, _in_order;
    int out_order, _out_order;

    bool _initialized;


    AudioSampleBuffer output_buffer;


    SphericalHarmonic sph_h_in;
    SphericalHarmonic sph_h_out;

    // Eigen stuff

    Eigen::MatrixXd Carth_coord; // carthesian coordinates
    Eigen::MatrixXd Sph_coord; // spherical coordinates


    // Eigen::VectorXd Ymn; // Ymn result
    Eigen::MatrixXd Sh_matrix_inv; // undistorted SH Matrix
    Eigen::MatrixXd Sh_matrix_mod; // distorted SH Matrix

    Eigen::MatrixXd Sh_transf; // Transformation Matrix

    Eigen::MatrixXd _Sh_transf; // Old Transformation Matrix

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_warpAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
