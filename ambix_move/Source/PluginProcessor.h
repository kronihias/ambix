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

#ifndef AMBIX_MOVE_PLUGINPROCESSOR_H_INCLUDED
#define AMBIX_MOVE_PLUGINPROCESSOR_H_INCLUDED

#define _USE_MATH_DEFINES
#include <cmath>

#include "JuceHeader.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include <Eigen/Eigen>

#include "AmbisonicConfig.h"

//==============================================================================
/**
 ambix_move

 Translates the listener point of view through the soundfield in x, y, z metres
 (range -5..+5 m) on a virtual sampling sphere of configurable reference radius
 (1..5 m). Re-encoding is performed as a single matrix operation in the same way
 as ambix_warp:

   1) Decode the input ambisonic signal at a fixed t-design sampling grid (the
      pseudo-inverse of the SH basis at those points).
   2) For each sampling point on the reference sphere, compute the new arrival
      direction *and* distance as seen from the translated listener.
   3) Re-encode each sample using a SH basis evaluated at the new direction,
      scaled by R_ref / new_distance (1/r gain compensation).
   4) Multiply the two matrices to get the final transformation matrix.

 No delay compensation is applied (only gain compensation). The speed of sound
 c = 343 m/s is documented in the source as a constant for future use.
*/
class Ambix_moveAudioProcessor  : public AudioProcessor,
                                       public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_moveAudioProcessor();
    ~Ambix_moveAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void numChannelsChanged() override;

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
        XParam,         // 0..1, mapped to -5..+5 m
        YParam,         // 0..1, mapped to -5..+5 m
        ZParam,         // 0..1, mapped to -5..+5 m
        RadiusParam,    // 0..1, mapped to  1..5  m
        InOrderParam,   // 0..1, mapped to 0..AMBI_ORDER (active order)
        OutOrderParam,
        totalNumParams
    };

    // Parameter -> physical units helpers (also used by the editor)
    static float xMetersFromParam   (float p) { return -5.f + 10.f * p; }
    static float xParamFromMeters   (float m) { return juce::jlimit (0.f, 1.f, (m + 5.f) * 0.1f); }
    static float radiusMetersFromParam (float p) { return 1.f + 4.f * p; }
    static float radiusParamFromMeters (float m) { return juce::jlimit (0.f, 1.f, (m - 1.f) * 0.25f); }

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

    // Translation parameters (normalized 0..1, default 0.5 = 0 m)
    float x_param,      _x_param;
    float y_param,      _y_param;
    float z_param,      _z_param;

    // Reference radius (normalized 0..1, default 0 = 1 m)
    float radius_param, _radius_param;

    // Ambisonic order parameters
    float in_order_param;
    float out_order_param;

    int in_order, _in_order;
    int out_order, _out_order;

    bool _initialized;

    // Speed of sound — kept for documentation / future delay compensation
    static constexpr float speedOfSound = 343.0f; // m/s

    AudioSampleBuffer output_buffer;

    SphericalHarmonic sph_h_in;
    SphericalHarmonic sph_h_out;

    // Eigen members (mirror ambix_warp layout)
    Eigen::MatrixXd Carth_coord;     // unit-sphere sampling points (numsamples x 3)
    Eigen::MatrixXd Sh_matrix_inv;   // pseudo-inverse of undistorted SH basis
    Eigen::MatrixXd Sh_transf;       // current transformation matrix
    Eigen::MatrixXd _Sh_transf;      // previous transformation matrix (for ramping)

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_moveAudioProcessor)
};

#endif  // AMBIX_MOVE_PLUGINPROCESSOR_H_INCLUDED
