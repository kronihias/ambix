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
#include "ambix_buses.h"
#include "../../common/AmbixParameters.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include <Eigen/Eigen>

#include <array>
#include <atomic>
#include <vector>

#include "AmbisonicConfig.h"

//==============================================================================
/**
 ambix_move

 Translates *and rotates* the listener point of view through the soundfield.

 Translation range is ±5 m in x, y, z on a virtual sampling sphere of
 configurable reference radius (1..5 m). Rotation uses the same Yaw / Pitch /
 Roll + Quaternion convention as ambix_rotator / IEM SceneRotator.

 Re-encoding is performed as a single matrix operation in the same way as
 ambix_warp:

   1) Decode the input ambisonic signal at a fixed t-design sampling grid (the
      pseudo-inverse of the SH basis at those points).
   2) For each sampling point on the reference sphere, compute the new arrival
      direction *and* distance as seen from the translated listener, then apply
      the head rotation to that direction.
   3) Re-encode each sample using a SH basis evaluated at the new direction,
      scaled by R_ref / new_distance (1/r gain compensation).
   4) Multiply the two matrices to get the final transformation matrix — so at
      runtime the whole 6DoF operation is a single matrix multiply on the
      ambisonic signal.

 OSC control is available on a configurable UDP port (default 7130) using the
 same message patterns as ambix_rotator, plus new /xyz and /6dof messages for
 positional control.

 No delay compensation is applied (only gain compensation). The speed of sound
 c = 343 m/s is documented in the source as a constant for future use.
*/
class Ambix_moveAudioProcessor  : public AudioProcessor,
                                   public ambix::ParameterOwner,
#ifdef WITH_OSC
                                   private OSCReceiver,
                                   private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
#endif
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
        XParam,         // 0..1, mapped to -5..+5 m
        YParam,         // 0..1, mapped to -5..+5 m
        ZParam,         // 0..1, mapped to -5..+5 m
        RadiusParam,    // 0..1, mapped to  1..5  m

        YawParam,       // 0..1, mapped to -180..+180 deg (yaw around Z)
        PitchParam,     // 0..1, mapped to -180..+180 deg (pitch around Y)
        RollParam,      // 0..1, mapped to -180..+180 deg (roll around X)
        RotOrderParam,  // <0.5 = ypr, >=0.5 = rpy

        Q0Param,        // 0..1 mapped to -1..+1 (quaternion w)
        Q1Param,        // 0..1 mapped to -1..+1 (quaternion x)
        Q2Param,        // 0..1 mapped to -1..+1 (quaternion y)
        Q3Param,        // 0..1 mapped to -1..+1 (quaternion z)
        QinvertParam,   // <0.5 = forward, >=0.5 = inverse

        InOrderParam,   // 0..1, mapped to 0..AMBI_ORDER (active order)
        OutOrderParam,

        TranslEnabledParam, // 1 = translation active, 0 = bypass (listener at origin)
        RotEnabledParam,    // 1 = rotation active,    0 = bypass (identity rotation)

        totalNumParams
    };

    // Parameter -> physical units helpers (also used by the editor)
    static float xMetersFromParam   (float p) { return -5.f + 10.f * p; }
    static float xParamFromMeters   (float m) { return juce::jlimit (0.f, 1.f, (m + 5.f) * 0.1f); }
    static float radiusMetersFromParam (float p) { return 1.f + 4.f * p; }
    static float radiusParamFromMeters (float m) { return juce::jlimit (0.f, 1.f, (m - 1.f) * 0.25f); }

    // Rotation helpers: parameter is 0..1, displayed as -180..+180 deg.
    static float degFromParam   (float p) { return p * 360.f - 180.f; }
    static float paramFromDeg   (float d) { return juce::jlimit (0.f, 1.f, (d + 180.f) / 360.f); }

    // the parameter at `index` (one of Parameters)
    ambix::Parameter& param (int index) const { return *params[index]; }

#ifdef WITH_OSC
    String osc_in_port;
    bool   osc_enabled;
    void oscMessageReceived (const OSCMessage& message) override;

    // Called from the editor when the user types a new port number.
    // Returns true if the new port could be opened.
    bool setOscPort (const String& newPort);

    // Enable / disable the OSC receiver. Returns true if the request
    // succeeded (i.e. the listener is in the requested state afterwards).
    bool setOscEnabled (bool shouldBeEnabled);
#endif

    bool isQuaternionActive(); // true if quaternions drive the rotation, false for Euler angles

    // World-frame unit forward vector implied by the current rotation
    // parameters (ambix convention: x = front, y = left, z = up).
    // Used by the GUI to draw a "look direction" indicator on the listener dot.
    void getLookForward (float out[3]);

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

    // side effects of a set; runs on the thread that set the parameter
    void parameterValueSet (int index, float newValue) override;

    void calcParams();

    // the selected order, capped to the input bus as calcParams() does
    int orderForDisplay (float orderParam);

    // defaults: translation 0.5 = 0 m, reference radius 0 = 1 m,
    // rotation 0.5 = 0 deg / 0 quat
    ambix::Parameter* params[totalNumParams] {};

    // values the current transformation matrix was computed for
    float _x_param;
    float _y_param;
    float _z_param;
    float _radius_param;
    float _yaw_param;
    float _pitch_param;
    float _roll_param;
    float _rot_order_param;
    float _q0_param;
    float _q1_param;
    float _q2_param;
    float _q3_param;
    float _qinvert_param;

    std::atomic<bool> _q_changed; // true when quaternions were touched last (-> use them)

    float _transl_enabled_param;
    float _rot_enabled_param;

    // set from the parameters' side effects (any thread) and capped by
    // calcParams() (audio thread)
    int _in_order;  // orders the current matrices were built for (audio thread);
    int _out_order; // calcParams derives the wanted ones from the parameters

    std::atomic<bool> _initialized;

    // Speed of sound — kept for documentation / future delay compensation
    static constexpr float speedOfSound = 343.0f; // m/s

    AudioSampleBuffer output_buffer;

    // built once in the constructor for every order 0..AMBI_ORDER, so an
    // order change on the audio thread only picks one
    std::vector<Eigen::MatrixXd> Sh_matrix_inv_for_order; // pseudo-inverse of undistorted SH basis, per input order
    std::array<SphericalHarmonic, AMBI_ORDER + 1> sph_h_out_for_order;

    const Eigen::MatrixXd* Sh_matrix_inv = nullptr; // the one for the current input order
    SphericalHarmonic*     sph_h_out     = nullptr; // the one for the current output order

    // Eigen members (mirror ambix_warp layout)
    Eigen::MatrixXd Carth_coord;     // unit-sphere sampling points (numsamples x 3)
    Eigen::MatrixXd Sh_matrix_mod;   // moved/rotated SH basis (numsamples x AMBI_CHANNELS), sized in init
    Eigen::MatrixXd Sh_transf;       // current transformation matrix
    Eigen::MatrixXd _Sh_transf;      // previous transformation matrix (for ramping)

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_moveAudioProcessor)
};

#endif  // AMBIX_MOVE_PLUGINPROCESSOR_H_INCLUDED
