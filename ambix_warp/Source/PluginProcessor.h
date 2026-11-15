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
#include "ambix_buses.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include <Eigen/Eigen>
#include "../../common/AmbixParameters.h"

#include <array>
#include <atomic>
#include <vector>

#include "AmbisonicConfig.h"

//==============================================================================
/**
*/
class Ambix_warpAudioProcessor  : public AudioProcessor,
                                  public ambix::ParameterOwner,
                                  public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_warpAudioProcessor();
    ~Ambix_warpAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

   #ifdef UNIVERSAL_AMBISONIC
    AMBIX_APPLY_BUS_LAYOUTS_OVERRIDE
   #endif
    void numChannelsChanged() override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

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

    // the parameter at `index` (one of Parameters)
    ambix::Parameter& param (int index) const { return *params[index]; }

private:

    // side effects of a set; runs on the thread that set the parameter
    void parameterValueSet (int index, float newValue) override;

    void calcParams();

    // the selected order, capped to the input bus as calcParams() does
    int orderForDisplay (float orderParam);

    ambix::Parameter* params[totalNumParams] {};

    // values the current transformation matrix was computed for
    // warp azimuth
    float _phi_param;
    float _phi_curve_param;

    // warp elevation
    float _theta_param;
    float _theta_curve_param;

    // use pre emphasis
    float _preemp_param;

    // input / output ambisonics order: set from the parameters' side effects
    // (any thread) and capped by calcParams() (audio thread)
    int _in_order;  // orders the current matrices were built for (audio thread);
    int _out_order; // calcParams derives the wanted ones from the parameters

    std::atomic<bool> _initialized;


    AudioSampleBuffer output_buffer;


    // built once in the constructor for every order 0..AMBI_ORDER, so an
    // order change on the audio thread only picks one
    std::vector<Eigen::MatrixXd> Sh_matrix_inv_for_order; // undistorted SH Matrix inverse, per input order
    std::array<SphericalHarmonic, AMBI_ORDER + 1> sph_h_out_for_order;

    const Eigen::MatrixXd* Sh_matrix_inv = nullptr; // the one for the current input order
    SphericalHarmonic*     sph_h_out     = nullptr; // the one for the current output order

    // Eigen stuff

    Eigen::MatrixXd Carth_coord; // carthesian coordinates
    Eigen::MatrixXd Sph_coord; // spherical coordinates


    // Eigen::VectorXd Ymn; // Ymn result
    Eigen::MatrixXd Sh_matrix_mod; // distorted SH Matrix
    Eigen::VectorXd pre_emphasis; // per sampling point pre-emphasis gain
    Eigen::MatrixXd Sph_coord_mod; // warped spherical coordinates

    Eigen::MatrixXd Sh_transf; // Transformation Matrix

    Eigen::MatrixXd _Sh_transf; // Old Transformation Matrix

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_warpAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
