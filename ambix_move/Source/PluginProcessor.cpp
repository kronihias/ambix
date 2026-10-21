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
#include "t_design.h"

#ifdef _WIN32
inline double round (double d)
{
    return floor (d + 0.5);
}
#endif

#ifdef WITH_OSC
//==============================================================================
// OSC: mirrors ambix_rotator's conventions and adds /xyz, /translation, /6dof.
//
//   /rotation    [pitch] [yaw] [roll]              (deg, each -180..+180)
//   /quaternion  [qw] [qx] [qy] [qz]               (each -1..+1)
//   /xyz         [x] [y] [z]                       (metres, each -5..+5)
//   /translation [x] [y] [z]                       (alias of /xyz)
//   /6dof        [x] [y] [z] [qw] [qx] [qy] [qz]
//                x,y,z in metres, quaternion -1..+1
//
// Values are sent both as int32 and float32 by different senders, so we handle
// both just like ambix_rotator does.
void Ambix_moveAudioProcessor::oscMessageReceived (const OSCMessage& message)
{
    auto readFloat = [] (const OSCArgument& a, float fallback)
    {
        if (a.getType() == OSCTypes::float32) return (float) a.getFloat32();
        if (a.getType() == OSCTypes::int32)   return (float) a.getInt32();
        return fallback;
    };

    const auto addr = message.getAddressPattern();

    if (addr == OSCAddressPattern ("/rotation"))
    {
        // /rotation [pitch] [yaw] [roll] (deg)
        float val[3] = { 0.f, 0.f, 0.f };
        for (int i = 0; i < jmin (3, message.size()); ++i)
            val[i] = readFloat (message[i], 0.f);

        params[PitchParam]->setValueNotifyingHost (paramFromDeg (val[0]));
        params[YawParam]->setValueNotifyingHost (paramFromDeg (val[1]));
        params[RollParam]->setValueNotifyingHost (paramFromDeg (val[2]));
    }
    else if (addr == OSCAddressPattern ("/quaternion"))
    {
        // /quaternion [q0] [q1] [q2] [q3]  (-1..+1)
        float val[4] = { 0.f, 0.f, 0.f, 0.f };
        for (int i = 0; i < jmin (4, message.size()); ++i)
            val[i] = readFloat (message[i], 0.f);

        params[Q0Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[0] + 1.f) * 0.5f));
        params[Q1Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[1] + 1.f) * 0.5f));
        params[Q2Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[2] + 1.f) * 0.5f));
        params[Q3Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[3] + 1.f) * 0.5f));
    }
    else if (addr == OSCAddressPattern ("/xyz")
          || addr == OSCAddressPattern ("/translation"))
    {
        // /xyz [x] [y] [z] (metres)
        float val[3] = { 0.f, 0.f, 0.f };
        for (int i = 0; i < jmin (3, message.size()); ++i)
            val[i] = readFloat (message[i], 0.f);

        params[XParam]->setValueNotifyingHost (xParamFromMeters (val[0]));
        params[YParam]->setValueNotifyingHost (xParamFromMeters (val[1]));
        params[ZParam]->setValueNotifyingHost (xParamFromMeters (val[2]));
    }
    else if (addr == OSCAddressPattern ("/6dof"))
    {
        // /6dof [x] [y] [z] [qw] [qx] [qy] [qz]
        float val[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
        for (int i = 0; i < jmin (7, message.size()); ++i)
            val[i] = readFloat (message[i], 0.f);

        params[XParam]->setValueNotifyingHost (xParamFromMeters (val[0]));
        params[YParam]->setValueNotifyingHost (xParamFromMeters (val[1]));
        params[ZParam]->setValueNotifyingHost (xParamFromMeters (val[2]));
        params[Q0Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[3] + 1.f) * 0.5f));
        params[Q1Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[4] + 1.f) * 0.5f));
        params[Q2Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[5] + 1.f) * 0.5f));
        params[Q3Param]->setValueNotifyingHost (jlimit (0.f, 1.f, (val[6] + 1.f) * 0.5f));
    }
}
#endif

//==============================================================================
Ambix_moveAudioProcessor::Ambix_moveAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET (AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET (AMBI_CHANNELS), true)
    ),
    _x_param (-10.f),
    _y_param (-10.f),
    _z_param (-10.f),
    _radius_param (-10.f),
    _yaw_param (-10.f),
    _pitch_param (-10.f),
    _roll_param (-10.f),
    _rot_order_param (-10.f),
    _q0_param (-10.f),
    _q1_param (-10.f),
    _q2_param (-10.f),
    _q3_param (-10.f),
    _qinvert_param (-10.f),
    _q_changed (false),
    _transl_enabled_param (-10.f),
    _rot_enabled_param    (-10.f),
    _in_order (-1),
    _out_order (-1),
    _initialized (false),
    output_buffer (AMBI_CHANNELS, 256)
{
    // legacy display texts, as getParameterText() built them
    auto metres      = [] (float v, int) -> String { return String (xMetersFromParam (v), 3) + " m"; };
    auto radiusText  = [] (float v, int) -> String { return String (radiusMetersFromParam (v), 2) + " m"; };
    auto degrees     = [] (float v, int) -> String { return String (degFromParam (v), 1) + " deg"; };
    auto rotOrder    = [] (float v, int) -> String { return (v < 0.5f) ? "yaw-pitch-roll" : "roll-pitch-yaw"; };
    auto quat        = [] (float v, int) -> String { return String (v * 2. - 1., 4); };
    auto inverse     = [] (float v, int) -> String { return (v < 0.5f) ? "" : "inverse"; };
    auto orderText   = [this] (float v, int) -> String { return String (orderForDisplay (v)); };
    auto onOff       = [] (float v, int) -> String { return (v > 0.5f) ? "on" : "bypass"; };

    auto add = [this] (int index, const char* name, float initial, ambix::Parameter::TextFunction text)
    {
        addParameter (params[index] = new ambix::Parameter (*this, index, name, initial, std::move (text)));
    };

    add (XParam,             "X",                           0.5f, metres);
    add (YParam,             "Y",                           0.5f, metres);
    add (ZParam,             "Z",                           0.5f, metres);
    add (RadiusParam,        "Reference radius",            0.f,  radiusText);
    add (YawParam,           "Yaw",                         0.5f, degrees);
    add (PitchParam,         "Pitch",                       0.5f, degrees);
    add (RollParam,          "Roll",                        0.5f, degrees);
    add (RotOrderParam,      "Rotation order",              0.f,  rotOrder);
    add (Q0Param,            "Quaternion q0",               0.5f, quat);
    add (Q1Param,            "Quaternion q1",               0.5f, quat);
    add (Q2Param,            "Quaternion q2",               0.5f, quat);
    add (Q3Param,            "Quaternion q3",               0.5f, quat);
    add (QinvertParam,       "Inverse Quaternion Rotation", 0.f,  inverse);
    add (InOrderParam,       "Ambi In order",               1.f,  orderText);
    add (OutOrderParam,      "Ambi Out order",              1.f,  orderText);
    add (TranslEnabledParam, "Translation enabled",         1.f,  onOff);
    add (RotEnabledParam,    "Rotation enabled",            1.f,  onOff);

    // Parse the t-design sampling points (240 points on the unit sphere)
    {
        const String t_design_txt (t_design::des_3_240_21_txt);

        String::CharPointerType lineChar = t_design_txt.getCharPointer();

        int n = 0;
        int numsamples = 0;
        int curr_n = 0;
        int max_n  = lineChar.length();

        while (curr_n < max_n)
        {
            double value;
            sscanf (lineChar, "%lf\n%n", &value, &n);
            lineChar += n;
            curr_n += n;
            numsamples++;
        }

        numsamples /= 3; // x,y,z triples

        Carth_coord.resize (numsamples, 3);

        lineChar = t_design_txt.getCharPointer();

        for (int i = 0; i < numsamples; ++i)
        {
            double x, y, z;
            sscanf (lineChar, "%lf%lf%lf%n", &x, &y, &z, &n);
            Carth_coord (i, 0) = x;
            Carth_coord (i, 1) = y;
            Carth_coord (i, 2) = z;
            lineChar += n;
        }
    }

    const int numsamples = (int) Carth_coord.rows();

    // For every possible input order, the undistorted SH basis at the
    // sampling points and its pseudo-inverse - this stays fixed regardless
    // of listener position.
    Sh_matrix_inv_for_order.resize (AMBI_ORDER + 1);

    for (int order = 0; order <= AMBI_ORDER; ++order)
    {
        SphericalHarmonic sph_h_in;
        sph_h_in.Init (order);

        Eigen::MatrixXd Sh_matrix;
        Sh_matrix.setZero (numsamples, AMBI_CHANNELS);

        const int in_ambi_channels = (order + 1) * (order + 1);
        for (int i = 0; i < numsamples; ++i)
        {
            const double az = atan2 (Carth_coord (i, 1), Carth_coord (i, 0));
            const double el = atan2 (Carth_coord (i, 2),
                                     sqrt (Carth_coord (i, 0) * Carth_coord (i, 0)
                                         + Carth_coord (i, 1) * Carth_coord (i, 1)));

            sph_h_in.Calc (az, el);

            Sh_matrix.block (i, 0, 1, in_ambi_channels) = sph_h_in.Ymn.transpose();
        }

        Sh_matrix_inv_for_order[(size_t) order] = (Sh_matrix.transpose() * Sh_matrix)
                                                      .colPivHouseholderQr().inverse() * Sh_matrix.transpose();

        sph_h_out_for_order[(size_t) order].Init (order);
    }

    // sized once here so calcParams() never allocates
    Sh_matrix_mod.setZero (numsamples, AMBI_CHANNELS);

    // start as identity (pass-through) until calcParams() runs
    Sh_transf  = Eigen::MatrixXd::Identity (AMBI_CHANNELS, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Identity (AMBI_CHANNELS, AMBI_CHANNELS);

#ifdef WITH_OSC
    osc_in_port = "7130";
    osc_enabled = true;

    if (! connect (osc_in_port.getIntValue()))
    {
        std::cout << "ambix_move: could not connect to OSC port " << osc_in_port << std::endl;
    }
    else
    {
        OSCReceiver::addListener (this);
    }
#endif
}

Ambix_moveAudioProcessor::~Ambix_moveAudioProcessor()
{
#ifdef WITH_OSC
    // Stop the OSC thread first: its callback would otherwise run into a
    // processor that is already being taken apart (pure virtual call).
    OSCReceiver::removeListener (this);
    OSCReceiver::disconnect();
#endif
}

#ifdef WITH_OSC
bool Ambix_moveAudioProcessor::setOscPort (const String& newPort)
{
    const int portInt = newPort.getIntValue();
    if (portInt <= 0 || portInt >= 65536)
        return false;

    OSCReceiver::disconnect();
    osc_in_port = String (portInt);

    if (! osc_enabled)
        return true; // stay disconnected while disabled, just remember the port

    if (! connect (portInt))
    {
        std::cout << "ambix_move: could not connect to OSC port " << osc_in_port << std::endl;
        return false;
    }

    OSCReceiver::addListener (this);
    return true;
}

bool Ambix_moveAudioProcessor::setOscEnabled (bool shouldBeEnabled)
{
    if (shouldBeEnabled == osc_enabled)
        return true;

    osc_enabled = shouldBeEnabled;

    if (! osc_enabled)
    {
        OSCReceiver::removeListener (this);
        OSCReceiver::disconnect();
        return true;
    }

    const int portInt = osc_in_port.getIntValue();
    if (portInt <= 0 || portInt >= 65536)
        return false;

    if (! connect (portInt))
    {
        std::cout << "ambix_move: could not connect to OSC port " << osc_in_port << std::endl;
        return false;
    }

    OSCReceiver::addListener (this);
    return true;
}
#endif

//==============================================================================
const String Ambix_moveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void Ambix_moveAudioProcessor::parameterValueSet (int index, float newValue)
{
    switch (index)
    {
        case YawParam:
        case PitchParam:
        case RollParam:
        case RotOrderParam:
            _q_changed = false;
            break;

        case Q0Param:
        case Q1Param:
        case Q2Param:
        case Q3Param:
        case QinvertParam:
            _q_changed = true;
            break;

        default:
            break;
    }
}

int Ambix_moveAudioProcessor::orderForDisplay (float orderParam)
{
    int order = (int) round (orderParam * AMBI_ORDER);
#ifdef UNIVERSAL_AMBISONIC
    int activeOrder = ambiOrderFromChannels (getTotalNumInputChannels());
    if (activeOrder > 0)
        order = juce::jmin (order, activeOrder);
#endif
    return order;
}

bool Ambix_moveAudioProcessor::isQuaternionActive()
{
    return _q_changed;
}

void Ambix_moveAudioProcessor::getLookForward (float out[3])
{
    // Build the same rotation matrix as calcParams() and return its first
    // column. In the row-vector convention  h = w * Rot  the world-frame
    // vector that maps onto head-frame +X (forward) is the first column of
    // Rot, since  h = (1,0,0) = w * Rot  =>  w = (1,0,0) * Rot^T  = col 0.
    Eigen::Matrix3d Rot = Eigen::Matrix3d::Identity();

    const float yaw_param       = params[YawParam]->get();
    const float pitch_param     = params[PitchParam]->get();
    const float roll_param      = params[RollParam]->get();
    const float rot_order_param = params[RotOrderParam]->get();
    const float q0_param        = params[Q0Param]->get();
    const float q1_param        = params[Q1Param]->get();
    const float q2_param        = params[Q2Param]->get();
    const float q3_param        = params[Q3Param]->get();
    const float qinvert_param   = params[QinvertParam]->get();

    if (_q_changed)
    {
        float q0 = q0_param * 2.f - 1.f;
        float q1 = q1_param * 2.f - 1.f;
        float q2 = q2_param * 2.f - 1.f;
        float q3 = q3_param * 2.f - 1.f;

        float absq = std::sqrt (q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
        if (absq > 0.f && absq != 1.f)
        {
            q0 /= absq; q1 /= absq; q2 /= absq; q3 /= absq;
        }

        if (qinvert_param > 0.5f)
        {
            q1 = -q1; q2 = -q2; q3 = -q3;
        }

        const float q0q0 = q0 * q0, q1q1 = q1 * q1, q2q2 = q2 * q2, q3q3 = q3 * q3;

        Rot(0, 0) = q0q0 + q1q1 - q2q2 - q3q3;
        Rot(0, 1) = 2 * q1 * q2 + 2 * q0 * q3;
        Rot(0, 2) = 2 * q1 * q3 - 2 * q0 * q2;
        Rot(1, 0) = 2 * q1 * q2 - 2 * q0 * q3;
        Rot(1, 1) = q0q0 - q1q1 + q2q2 - q3q3;
        Rot(1, 2) = 2 * q2 * q3 + 2 * q0 * q1;
        Rot(2, 0) = 2 * q1 * q3 + 2 * q0 * q2;
        Rot(2, 1) = 2 * q2 * q3 - 2 * q0 * q1;
        Rot(2, 2) = q0q0 - q1q1 - q2q2 + q3q3;
    }
    else
    {
        // Yaw and roll are inverted relative to the raw Euler-to-rotation
        // math used in ambix_rotator. In ambix_move the rotation is applied
        // to the listener-to-source vector (`d * Rot` ≡ rotating `d` by
        // Rot^T, i.e. we view the world from the listener's head frame),
        // so the sign of the yaw (Z) and roll (X) angles has to flip to
        // give the same perceptual direction of motion as ambix_rotator,
        // which rotates the sound field in the opposite sense. Pitch (Y)
        // does not need the flip because the ambix convention already
        // uses a left-handed pitch axis (see ambix_rotator's RotY).
        const double yaw   =  ((double) yaw_param   * 2.0 * M_PI - M_PI);
        const double pitch =  ((double) pitch_param * 2.0 * M_PI - M_PI);
        const double roll  = -((double) roll_param  * 2.0 * M_PI - M_PI);

        Eigen::Matrix3d RotX = Eigen::Matrix3d::Zero();
        Eigen::Matrix3d RotY = Eigen::Matrix3d::Zero();
        Eigen::Matrix3d RotZ = Eigen::Matrix3d::Zero();

        RotX(0, 0) = 1.0;
        RotX(1, 1) = RotX(2, 2) = std::cos (roll);
        RotX(1, 2) = std::sin (roll);
        RotX(2, 1) = -RotX(1, 2);

        RotY(0, 0) = RotY(2, 2) = std::cos (pitch);
        RotY(0, 2) = std::sin (pitch);
        RotY(2, 0) = -RotY(0, 2);
        RotY(1, 1) = 1.0;

        RotZ(0, 0) = RotZ(1, 1) = std::cos (yaw);
        RotZ(0, 1) = std::sin (yaw);
        RotZ(1, 0) = -RotZ(0, 1);
        RotZ(2, 2) = 1.0;

        if (rot_order_param < 0.5f)
            Rot = RotX * RotY * RotZ; // ypr
        else
            Rot = RotZ * RotY * RotX; // rpy
    }

    out[0] = (float) Rot (0, 0);
    out[1] = (float) Rot (1, 0);
    out[2] = (float) Rot (2, 0);
}

const String Ambix_moveAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_moveAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_moveAudioProcessor::isInputChannelStereoPair (int) const  { return true; }
bool Ambix_moveAudioProcessor::isOutputChannelStereoPair (int) const { return true; }

bool Ambix_moveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_moveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_moveAudioProcessor::silenceInProducesSilenceOut() const { return false; }
double Ambix_moveAudioProcessor::getTailLengthSeconds() const     { return 0.0; }

int Ambix_moveAudioProcessor::getNumPrograms()                    { return 0; }
int Ambix_moveAudioProcessor::getCurrentProgram()                 { return 0; }
void Ambix_moveAudioProcessor::setCurrentProgram (int)            {}
const String Ambix_moveAudioProcessor::getProgramName (int)       { return String(); }
void Ambix_moveAudioProcessor::changeProgramName (int, const String&) {}

//==============================================================================
void Ambix_moveAudioProcessor::prepareToPlay (double, int samplesPerBlock)
{
    output_buffer.setSize (getTotalNumOutputChannels(), samplesPerBlock, false, false, false);
    _initialized = false; // force SH basis rebuild before audio starts
    calcParams();
}

void Ambix_moveAudioProcessor::releaseResources()
{
}

void Ambix_moveAudioProcessor::calcParams()
{
    // one atomic read per parameter and block
    const float x_param              = params[XParam]->get();
    const float y_param              = params[YParam]->get();
    const float z_param              = params[ZParam]->get();
    const float radius_param         = params[RadiusParam]->get();
    const float yaw_param            = params[YawParam]->get();
    const float pitch_param          = params[PitchParam]->get();
    const float roll_param           = params[RollParam]->get();
    const float rot_order_param      = params[RotOrderParam]->get();
    const float q0_param             = params[Q0Param]->get();
    const float q1_param             = params[Q1Param]->get();
    const float q2_param             = params[Q2Param]->get();
    const float q3_param             = params[Q3Param]->get();
    const float qinvert_param        = params[QinvertParam]->get();
    const float transl_enabled_param = params[TranslEnabledParam]->get();
    const float rot_enabled_param    = params[RotEnabledParam]->get();

    // The orders this computation uses, from the parameters as they are now
    // (they may be set from another thread meanwhile), capped to the active
    // channel count from the host. Computed afresh each block rather than
    // written back, so a concurrent set can't be lost.
    int inOrder  = (int) round (params[InOrderParam]->get()  * AMBI_ORDER);
    int outOrder = (int) round (params[OutOrderParam]->get() * AMBI_ORDER);

#ifdef UNIVERSAL_AMBISONIC
    const int activeOrder = ambiOrderFromChannels (getTotalNumInputChannels());
    if (activeOrder > 0)
    {
        inOrder  = juce::jmin (inOrder,  activeOrder);
        outOrder = juce::jmin (outOrder, activeOrder);
    }
#endif

    if (! _initialized.exchange (true) || inOrder != _in_order)
    {
        // pick the pseudo-inverse built for this input order (constructor)
        Sh_matrix_inv = &Sh_matrix_inv_for_order[(size_t) inOrder];

        Sh_matrix_mod.setZero(); // keeps size (numsamples x AMBI_CHANNELS)
    }

    // Recompute the transformation matrix only if any relevant parameter changed
    const bool anyChanged =
        (x_param              != _x_param)              ||
        (y_param              != _y_param)              ||
        (z_param              != _z_param)              ||
        (radius_param         != _radius_param)         ||
        (yaw_param            != _yaw_param)            ||
        (pitch_param          != _pitch_param)          ||
        (roll_param           != _roll_param)           ||
        (rot_order_param      != _rot_order_param)      ||
        (q0_param             != _q0_param)             ||
        (q1_param             != _q1_param)             ||
        (q2_param             != _q2_param)             ||
        (q3_param             != _q3_param)             ||
        (qinvert_param        != _qinvert_param)        ||
        (transl_enabled_param != _transl_enabled_param) ||
        (rot_enabled_param    != _rot_enabled_param)    ||
        (inOrder              != _in_order)             ||
        (outOrder             != _out_order);

    if (anyChanged)
    {
        if (_out_order != outOrder)
            sph_h_out = &sph_h_out_for_order[(size_t) outOrder]; // initialised in the constructor

        const int numsamples       = (int) Carth_coord.rows();
        const int out_ambi_channels = (outOrder + 1) * (outOrder + 1);

        const double R  = radiusMetersFromParam (radius_param);
        // When translation is bypassed the listener stays at the origin.
        const bool translActive = (transl_enabled_param > 0.5f);
        const double lx = translActive ? xMetersFromParam (x_param) : 0.0;
        // Y is negated so that moving the listener "right" (as per the GUI
        // and DAW convention) actually translates toward ambix -Y. The SH
        // library used by the suite has Y_1,-1 = -sin(az)*cos(el) (i.e. the
        // standard ambix sign is flipped), which makes a mathematically
        // correct left/right translation read inverted in downstream tools.
        // Flipping the parameter here keeps the rest of the chain consistent
        // without touching the GUI labels or the shared SH code.
        const double ly = translActive ? -xMetersFromParam (y_param) : 0.0;
        const double lz = translActive ? xMetersFromParam (z_param) : 0.0;

        // --------------------------------------------------------------
        // Build the 3x3 head-rotation matrix (same math as ambix_rotator
        // so Yaw/Pitch/Roll behave identically to IEM SceneRotator).
        // --------------------------------------------------------------
        Eigen::Matrix3d Rot = Eigen::Matrix3d::Identity();

        if (rot_enabled_param > 0.5f && _q_changed)
        {
            float q0 = q0_param * 2.f - 1.f;
            float q1 = q1_param * 2.f - 1.f;
            float q2 = q2_param * 2.f - 1.f;
            float q3 = q3_param * 2.f - 1.f;

            float absq = std::sqrt (q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
            if (absq > 0.f && absq != 1.f)
            {
                q0 /= absq; q1 /= absq; q2 /= absq; q3 /= absq;
            }

            if (qinvert_param < 0.5f)
            {
                // apply conjugate (inverse) by default so that the quaternion
                // convention matches the Euler path and external trackers
                q1 = -q1; q2 = -q2; q3 = -q3;
            }

            const float q0q0 = q0 * q0, q1q1 = q1 * q1, q2q2 = q2 * q2, q3q3 = q3 * q3;

            // James Diebel - Representing Attitude (Eq. 125)
            Rot(0, 0) = q0q0 + q1q1 - q2q2 - q3q3;
            Rot(0, 1) = 2 * q1 * q2 + 2 * q0 * q3;
            Rot(0, 2) = 2 * q1 * q3 - 2 * q0 * q2;
            Rot(1, 0) = 2 * q1 * q2 - 2 * q0 * q3;
            Rot(1, 1) = q0q0 - q1q1 + q2q2 - q3q3;
            Rot(1, 2) = 2 * q2 * q3 + 2 * q0 * q1;
            Rot(2, 0) = 2 * q1 * q3 + 2 * q0 * q2;
            Rot(2, 1) = 2 * q2 * q3 - 2 * q0 * q1;
            Rot(2, 2) = q0q0 - q1q1 - q2q2 + q3q3;
        }
        else if (rot_enabled_param > 0.5f)
        {
            // Euler angles. Yaw is mathematically negated (matches ambix_rotator)
            // so that a positive yaw slider rotates the scene in the same
            // direction as IEM SceneRotator.
            const double yaw   = -((double) yaw_param   * 2.0 * M_PI - M_PI); // z
            const double pitch =  ((double) pitch_param * 2.0 * M_PI - M_PI); // y
            const double roll  =  ((double) roll_param  * 2.0 * M_PI - M_PI); // x

            Eigen::Matrix3d RotX = Eigen::Matrix3d::Zero();
            Eigen::Matrix3d RotY = Eigen::Matrix3d::Zero();
            Eigen::Matrix3d RotZ = Eigen::Matrix3d::Zero();

            RotX(0, 0) = 1.0;
            RotX(1, 1) = RotX(2, 2) = std::cos (roll);
            RotX(1, 2) = std::sin (roll);
            RotX(2, 1) = -RotX(1, 2);

            RotY(0, 0) = RotY(2, 2) = std::cos (pitch);
            RotY(0, 2) = std::sin (pitch);
            RotY(2, 0) = -RotY(0, 2);
            RotY(1, 1) = 1.0;

            RotZ(0, 0) = RotZ(1, 1) = std::cos (yaw);
            RotZ(0, 1) = std::sin (yaw);
            RotZ(1, 0) = -RotZ(0, 1);
            RotZ(2, 2) = 1.0;

            if (rot_order_param < 0.5f)
                Rot = RotX * RotY * RotZ; // ypr
            else
                Rot = RotZ * RotY * RotX; // rpy
        }

        Sh_matrix_mod.setZero(); // keeps size (numsamples x AMBI_CHANNELS) from init

        for (int i = 0; i < numsamples; ++i)
        {
            // Sampling point on the reference sphere (world frame).
            // Coordinate convention: x = front, y = left, z = up.
            const double sx = R * Carth_coord (i, 0);
            const double sy = R * Carth_coord (i, 1);
            const double sz = R * Carth_coord (i, 2);

            // Step 1 — translate: vector from listener to sample in world frame
            double dx = sx - lx;
            double dy = sy - ly;
            double dz = sz - lz;
            const double newDist = std::sqrt (dx * dx + dy * dy + dz * dz);

            // Avoid div-by-zero singularity if listener exactly hits a sample
            const double safeDist = (newDist < 1.0e-6) ? 1.0e-6 : newDist;

            // Step 2 — rotate: apply head rotation to the translated direction.
            // We use the row-vector convention row*Rot (same as ambix_rotator),
            // which is equivalent to Rot^T * d on the column vector.
            Eigen::RowVector3d dRot = Eigen::RowVector3d (dx, dy, dz) * Rot;
            dx = dRot (0); dy = dRot (1); dz = dRot (2);

            // New arrival direction in ambix convention
            //   azimuth = atan2(y, x)  -> CCW from front
            //   elevation = asin(z / r)
            const double newAz = std::atan2 (dy, dx);
            const double newEl = std::asin  (dz / safeDist);

            // 1/r gain compensation. Unity when listener is at the origin
            // (newDist == R). No delay compensation; c = speedOfSound is left
            // documented for future use.
            const double gain = R / safeDist;

            sph_h_out->Calc (newAz, newEl);

            Sh_matrix_mod.block (i, 0, 1, out_ambi_channels) = gain * sph_h_out->Ymn.transpose();
        }

        // Final transformation matrix.
        //
        // Sh_matrix_inv is (in_ch x N) and Sh_matrix_mod is (N x out_ch), so
        // the product is (in_ch x out_ch) — i.e. element [i, j] is the
        // contribution of input channel i to output channel j. processBlock
        // however indexes the matrix as Sh_transf(out, in), so we transpose
        // here to get an (out_ch x in_ch) matrix that lines up with that
        // indexing convention. At zero translation Sh_transf is the identity
        // (symmetric), which is why the missing transpose only manifests as a
        // bug once the listener is actually moved.
        // (A * B)^T == B^T * A^T, written with noalias() so no temporary is allocated.
        Sh_transf.noalias() = Sh_matrix_mod.transpose() * Sh_matrix_inv->transpose();

        // threshold tiny values to zero (lets the inner loop in processBlock
        // skip them entirely)
        for (int i = 0; i < Sh_transf.size(); ++i)
        {
            if (std::abs (Sh_transf (i)) < 0.00001)
                Sh_transf (i) = 0.0;
        }

        _x_param              = x_param;
        _y_param              = y_param;
        _z_param              = z_param;
        _radius_param         = radius_param;
        _yaw_param            = yaw_param;
        _pitch_param          = pitch_param;
        _roll_param           = roll_param;
        _rot_order_param      = rot_order_param;
        _q0_param             = q0_param;
        _q1_param             = q1_param;
        _q2_param             = q2_param;
        _q3_param             = q3_param;
        _qinvert_param        = qinvert_param;
        _transl_enabled_param = transl_enabled_param;
        _rot_enabled_param    = rot_enabled_param;
        _in_order             = inOrder;
        _out_order            = outOrder;
    }
}

bool Ambix_moveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size()  == AMBI_CHANNELS));
#endif
}

void Ambix_moveAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    _initialized = false;
    sendChangeMessage();
#endif
}

void Ambix_moveAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer&)
{
    _Sh_transf = Sh_transf; // buffer old values for ramping

    calcParams();

    const int NumSamples = buffer.getNumSamples();

    output_buffer.clear();

    for (int out = 0; out < std::min (AMBI_CHANNELS, getTotalNumOutputChannels()); ++out)
    {
        for (int in = 0; in < std::min (AMBI_CHANNELS, getTotalNumInputChannels()); ++in)
        {
            if (_Sh_transf (out, in) != 0.0 || Sh_transf (out, in) != 0.0)
            {
                if (_Sh_transf (out, in) == Sh_transf (out, in))
                {
                    output_buffer.addFrom (out, 0, buffer, in, 0, NumSamples,
                                           (float) Sh_transf (out, in));
                }
                else
                {
                    output_buffer.addFromWithRamp (out, 0, buffer.getReadPointer (in), NumSamples,
                                                   (float) _Sh_transf (out, in),
                                                   (float) Sh_transf  (out, in));
                }
            }
        }
    }

    for (int ch = 0; ch < std::min (AMBI_CHANNELS, getTotalNumOutputChannels()); ++ch)
        buffer.copyFrom (ch, 0, output_buffer, ch, 0, NumSamples);
}

//==============================================================================
bool Ambix_moveAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* Ambix_moveAudioProcessor::createEditor()
{
    return new Ambix_moveAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_moveAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml ("MYPLUGINSETTINGS");

    // same attributes and order as the legacy index-based API wrote, so
    // projects stay readable by older versions
    for (int i = 0; i < totalNumParams; ++i)
        xml.setAttribute (String (i), params[i]->get());

    xml.setAttribute ("_q_changed", _q_changed.load());
#ifdef WITH_OSC
    xml.setAttribute ("osc_in_port", osc_in_port);
    xml.setAttribute ("osc_enabled", osc_enabled);
#endif

    copyXmlToBinary (xml, destData);
}

void Ambix_moveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName ("MYPLUGINSETTINGS"))
    {
        // restore() runs the side effects like the legacy setParameter() did
        for (int i = 0; i < totalNumParams; ++i)
            params[i]->restore ((float) xmlState->getDoubleAttribute (String (i)));

        if (xmlState->hasAttribute ("_q_changed"))
            _q_changed = xmlState->getBoolAttribute ("_q_changed");

#ifdef WITH_OSC
        if (xmlState->hasAttribute ("osc_enabled"))
            setOscEnabled (xmlState->getBoolAttribute ("osc_enabled"));

        if (xmlState->hasAttribute ("osc_in_port"))
        {
            String newPort = xmlState->getStringAttribute ("osc_in_port");
            if (newPort != osc_in_port)
                setOscPort (newPort);
        }
#endif
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_moveAudioProcessor();
}
