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
#include "Ressources/t_design.h"

#ifdef WITH_OSC
void Ambix_rotatorAudioProcessor::oscMessageReceived (const OSCMessage& message)
{

    if (message.getAddressPattern() == OSCAddressPattern("/rotation")) {
        // /rotation [pitch] [yaw] [roll]

        float val[3];

        for (int i=0; i < jmin(3,message.size()); i++) {

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

        setParameterNotifyingHost(Ambix_rotatorAudioProcessor::PitchParam, jlimit(0.f, 1.f, val[0]/360.f+0.5f));
        setParameterNotifyingHost(Ambix_rotatorAudioProcessor::YawParam, jlimit(0.f, 1.f, val[1]/360.f+0.5f));
        setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RollParam, jlimit(0.f, 1.f, val[2]/360.f+0.5f));

    } else if (message.getAddressPattern() == OSCAddressPattern("/head_pose")) {
        // /head_pose [User_ID] [x] [y] [z] [pitch] [yaw] [roll]

        float val[7];

        for (int i=4; i < jmin(7,message.size()); i++) {

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

        setParameterNotifyingHost(Ambix_rotatorAudioProcessor::PitchParam, jlimit(0.f, 1.f, val[4]/360.f+0.5f));
        setParameterNotifyingHost(Ambix_rotatorAudioProcessor::YawParam, jlimit(0.f, 1.f, val[5]/360.f+0.5f));
        setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RollParam, jlimit(0.f, 1.f, val[6]/360.f+0.5f));

    }
    else if (message.getAddressPattern() == OSCAddressPattern("/quaternion")) {
      // /quaternion [q0] [q1] [q2] [q3]
      float val[4];

      for (int i = 0; i < jmin(4, message.size()); i++) {

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

      setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q0Param, jlimit(0.f, 1.f, (val[0] + 1.f) / 2.f));
      setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q1Param, jlimit(0.f, 1.f, (val[1] + 1.f) / 2.f));
      setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q2Param, jlimit(0.f, 1.f, (val[2] + 1.f) / 2.f));
      setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q3Param, jlimit(0.f, 1.f, (val[3] + 1.f) / 2.f));
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
Ambix_rotatorAudioProcessor::Ambix_rotatorAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::discreteChannels(AMBI_CHANNELS), true)
        .withOutput ("Output", juce::AudioChannelSet::discreteChannels(AMBI_CHANNELS), true)
    ),
    yaw_param(0.5f),
    pitch_param(0.5f),
    roll_param(0.5f),
    rot_order_param(0.f),
    q0_param(0.5f),
    q1_param(0.5f),
    q2_param(0.5f),
    q3_param(0.5f),
    qinvert_param(0.f),
    _q_changed(false),
    _initialized(false),
    _new_params(true),
    output_buffer(AMBI_CHANNELS,256)
{
    // set transformation matrix to identity matrix
    Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);

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

Ambix_rotatorAudioProcessor::~Ambix_rotatorAudioProcessor()
{
}

//==============================================================================
const String Ambix_rotatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_rotatorAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_rotatorAudioProcessor::getParameter (int index)
{
    switch (index)
	{
		case YawParam:    return yaw_param;
		case PitchParam:    return pitch_param;
		case RollParam:    return roll_param;
        case RotOrderParam: return rot_order_param;
        case Q0Param:       return q0_param;
        case Q1Param:       return q1_param;
        case Q2Param:       return q2_param;
        case Q3Param:       return q3_param;
        case Qinvert:       return qinvert_param;

		default:            return 0.0f;
	}
}

void Ambix_rotatorAudioProcessor::setParameter (int index, float newValue)
{
  newValue = jlimit(0.f, 1.f, newValue);

    switch (index)
	{
        case YawParam:
            yaw_param = newValue;
            break;
        case PitchParam:
            pitch_param = newValue;
            break;
        case RollParam:
            roll_param = newValue;
            break;
        case RotOrderParam:
            rot_order_param = newValue;
            break;
        case Q0Param:
            q0_param = newValue;
            break;
        case Q1Param:
            q1_param = newValue;
            break;
        case Q2Param:
            q2_param = newValue;
            break;
        case Q3Param:
            q3_param = newValue;
            break;
        case Qinvert:
            qinvert_param = newValue;
            break;

		default:
            break;
	}

    if (index >= Q0Param)
      _q_changed = true;
    else
      _q_changed = false;

    _new_params = true;

    sendChangeMessage();
}

const String Ambix_rotatorAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
        case YawParam:        return "Yaw";
        case PitchParam:        return "Pitch";
        case RollParam:        return "Roll";
        case RotOrderParam:        return "Rotation order";
        case Q0Param:        return "Quaternion q0";
        case Q1Param:        return "Quaternion q1";
        case Q2Param:        return "Quaternion q2";
        case Q3Param:        return "Quaternion q3";
        case Qinvert:        return "Inverse Quaternion Rotation";

		default:								break;
	}

	return String();
}

const String Ambix_rotatorAudioProcessor::getParameterText (int index)
{
    String text;

    switch (index)
	{
        case YawParam:
            text = String(yaw_param*360-180).substring(0, 5);
            text << " deg";
            break;


        case PitchParam:
            text = String(pitch_param*360-180).substring(0, 5);
            text << " deg";
            break;

        case RollParam:
            text = String(roll_param*360-180).substring(0, 5);
            text << " deg";
            break;

        case RotOrderParam:
            if (rot_order_param <= 0.5f)
                text = "yaw-pitch-roll";
            else
                text = "roll-pitch-yaw";
            break;

        case Q0Param:
            text = String(q0_param * 2. - 1.).substring(0, 6);
            break;

        case Q1Param:
            text = String(q1_param * 2. - 1.).substring(0, 6);
            break;

        case Q2Param:
            text = String(q2_param * 2. - 1.).substring(0, 6);
            break;

        case Q3Param:
            text = String(q3_param * 2. - 1.).substring(0, 6);
            break;

        case Qinvert:
            if (qinvert_param <= 0.5f)
                text = "";
            else
                text = "inverse";
            break;

		default:
            break;
	}

	return text;
}

const String Ambix_rotatorAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_rotatorAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_rotatorAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_rotatorAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_rotatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_rotatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_rotatorAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_rotatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_rotatorAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_rotatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_rotatorAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_rotatorAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_rotatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_rotatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // calcParams();
}

void Ambix_rotatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


// functions to compute terms U, V, W of Eq.8.1 (Table II)
// function to compute term P of U,V,W (Table II)
double P(int i, int l, int mu, int m_, Eigen::Matrix3d& R_1, Eigen::MatrixXd& R_lm1)
{
  double ret = 0.;

  double ri1 = R_1(i+1,2);
  double rim1 = R_1(i+1,0);
  double ri0 = R_1(i+1,1);

  if (m_ == -l)
    ret = ri1*R_lm1(mu+l-1,0) + rim1*R_lm1(mu+l-1, 2*l-2);
  else
  {
    if (m_ == l)
      ret = ri1*R_lm1(mu+l-1,2*l-2) - rim1*R_lm1(mu+l-1, 0);
    else
      ret = ri0*R_lm1(mu+l-1,m_+l-1);
  }
  return ret;
}

double U(int l, int m, int n, Eigen::Matrix3d& R_1, Eigen::MatrixXd& R_lm1)
{
  return P(0, l, m, n, R_1, R_lm1);
}

double V(int l, int m, int n, Eigen::Matrix3d& R_1, Eigen::MatrixXd& R_lm1)
{
  double ret = 0.;
  if (m == 0)
  {
    double p0 = P(1,l,1,n,R_1,R_lm1);
    double p1 = P(-1,l,-1,n,R_1,R_lm1);
    ret = p0+p1;
  }
  else
  {
    if (m > 0)
    {
      int d = (m==1) ? 1 : 0; // delta function
      double p0 = P(1,l,m-1,n,R_1,R_lm1);
      double p1 = P(-1,l,-m+1,n,R_1,R_lm1);
      ret = p0*sqrt(1+d) - p1*(1-d);
    }
    else
    {
      int d = (m==-1) ? 1 : 0; // delta function
      double p0 = P(1,l,m+1,n,R_1,R_lm1);
      double p1 = P(-1,l,-m-1,n,R_1,R_lm1);
      ret = p0*(1-d) + p1*sqrt(1+d);
    }
  }
  return ret;
}

double W(int l, int m, int n, Eigen::Matrix3d& R_1, Eigen::MatrixXd& R_lm1)
{
  double ret = 0.;

  if (m != 0)
  {
    if (m>0)
		{
      double p0 = P(1,l,m+1,n,R_1,R_lm1);
      double p1 = P(-1,l,-m-1,n,R_1,R_lm1);
      ret = p0 + p1;
    }

    else
    {
      double p0 = P(1,l,m-1,n,R_1,R_lm1);
      double p1 = P(-1,l,-m+1,n,R_1,R_lm1);
      ret = p0 - p1;
    }
  }
  return ret;
}

void Ambix_rotatorAudioProcessor::calcParams()
{
// use old sampling method for generating rotation matrix...
#if 0
    if (!_initialized)
    {

        sph_h.Init(AMBI_ORDER);

        const String t_design_txt (t_design::des_3_240_21_txt);

        // std::cout << t_design_txt << std::endl;

        String::CharPointerType lineChar = t_design_txt.getCharPointer();

        int n = 0; // how many characters been read
        int numsamples = 0;
        int i = 0;

        int curr_n = 0;
        int max_n = lineChar.length();

        while (curr_n < max_n) { // check how many coordinates we have
            double value;
            sscanf(lineChar, "%lf\n%n", &value, &n);
            lineChar += n;
            curr_n += n;
            numsamples++;
        } // end parse numbers

        numsamples = numsamples/3; // xyz

        Carth_coord.resize(numsamples,3); // positions in cartesian coordinates

        curr_n = 0;
        lineChar = t_design_txt.getCharPointer();

        // parse line for numbers again and copy to carth coordinate matrix
        while (i < numsamples) {

            double x,y,z;

            sscanf(lineChar, "%lf%lf%lf%n", &x, &y, &z, &n);

            Carth_coord(i,0) = x;
            Carth_coord(i,1) = y;
            Carth_coord(i,2) = z;

            lineChar += n;

            curr_n += n;
            i++;

        } // end parse numbers

        Sph_coord.resize(numsamples,2); // positions in spherical coordinates

        Eigen::MatrixXd Sh_matrix(numsamples,AMBI_CHANNELS);

        for (int i=0; i < numsamples; i++)
        {
            Eigen::VectorXd Ymn(AMBI_CHANNELS); // Ymn result
            Sph_coord(i,0) = atan2(Carth_coord(i,1),Carth_coord(i,0)); // azimuth
            Sph_coord(i,1) = atan2(Carth_coord(i,2),sqrt(Carth_coord(i,0)*Carth_coord(i,0) + Carth_coord(i,1)*Carth_coord(i,1))); // elevation

            sph_h.Calc(Sph_coord(i,0),Sph_coord(i,1)); // phi theta
            sph_h.Get(Ymn);

            Sh_matrix.row(i) = Ymn;

        }

      // inversion would not be necessary because of t-design -> transpose is enough..

        Sh_matrix_inv = (Sh_matrix.transpose()*Sh_matrix).inverse()*Sh_matrix.transpose();

        _initialized = true;
    }

    Eigen::MatrixXd Sh_matrix_mod(Sph_coord.rows(),AMBI_CHANNELS);

    // rotation parameters in radiants
    // use mathematical negative angles for yaw

    double yaw = -((double)yaw_param*2*M_PI - M_PI); // z
    double pitch = (double)pitch_param*2*M_PI - M_PI; // y
    double roll = (double)roll_param*2*M_PI - M_PI; // x

    Eigen::Matrix3d RotX, RotY, RotZ, Rot;

    RotX = RotY = RotZ = Eigen::Matrix3d::Zero(3,3);

    RotX(0,0) = 1.f;
    RotX(1,1) = RotX(2,2) = cos(roll);
    RotX(1,2) = -sin(roll);
    RotX(2,1) = -RotX(1,2);

    RotY(0,0) = RotY(2,2) = cos(pitch);
    RotY(0,2) = sin(pitch);
    RotY(2,0) = -RotY(0,2);
    RotY(1,1) = 1.f;

    RotZ(0,0) = RotZ(1,1) = cos(yaw);
    RotZ(0,1) = -sin(yaw);
    RotZ(1,0) = -RotZ(0,1);
    RotZ(2,2) = 1.f;

    // multiply individual rotation matrices
    if (rot_order_param < 0.5f)
    {
        // ypr order zyx -> mutliply inverse!
        Rot = RotX * RotY * RotZ;
    } else {
        // rpy order xyz -> mutliply inverse!
        Rot = RotZ * RotY * RotX;
    }

    // combined roll-pitch-yaw rotation matrix would be here
    // http://planning.cs.uiuc.edu/node102.html

    for (int i=0; i < Carth_coord.rows(); i++)
    {
        // rotate carthesian coordinates
        Eigen::Vector3d Carth_coord_mod = Carth_coord.row(i)*Rot;

        Eigen::Vector2d Sph_coord_mod;

        // convert to spherical coordinates
        Sph_coord_mod(0) = atan2(Carth_coord_mod(1),Carth_coord_mod(0)); // azimuth
        Sph_coord_mod(1) = atan2(Carth_coord_mod(2),sqrt(Carth_coord_mod(0)*Carth_coord_mod(0) + Carth_coord_mod(1)*Carth_coord_mod(1))); // elevation

        Eigen::VectorXd Ymn(AMBI_CHANNELS); // Ymn result

        // calc spherical harmonic
        sph_h.Calc(Sph_coord_mod(0),Sph_coord_mod(1)); // phi theta
        sph_h.Get(Ymn);

        // save to sh matrix
        Sh_matrix_mod.row(i) = Ymn;
    }

    // calculate new transformation matrix

    Sh_transf = Sh_matrix_inv * Sh_matrix_mod;

#else
  // use
  // Ivanic, J., Ruedenberg, K. (1996). Rotation Matrices for Real
  // Spherical Harmonics. Direct Determination by Recursion.
  // The Journal of Physical Chemistry

  // rotation parameters in radiants
  // use mathematical negative angles for yaw

  Eigen::Matrix3d Rot = Eigen::Matrix3d::Identity(3,3);

  if (_q_changed)
  {
    // use quaternion input to calc rotation
    float q0 = q0_param*2. - 1.;
    float q1 = q1_param*2. - 1.;
    float q2 = q2_param*2. - 1.;
    float q3 = q3_param*2. - 1.;

    // normalize the quaternion just in case it isn't
    // ||q|| = 1, sqrt(q0^2+q1^2+q2^2+q3^2)
    float absq = sqrtf(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    if ((absq != 0.f) && absq != 1.0)
    {
      q0 = q0 / absq;
      q1 = q1 / absq;
      q2 = q2 / absq;
      q3 = q3 / absq;
    }

    if (qinvert_param > 0.5)
    {
      // do inverse rotation
      q1 = -q1;
      q2 = -q2;
      q3 = -q3;
    }
    float q0q0 = q0*q0;
    float q1q1 = q1*q1;
    float q2q2 = q2*q2;
    float q3q3 = q3*q3;

    // James Diebel -
    // Representing Attitude : Euler Angles, Unit Quaternions, and Rotation Vectors (Eq. 125)

    Rot(0, 0) = q0q0 + q1q1 - q2q2 - q3q3;
    Rot(0, 1) = 2 * q1*q2 + 2 * q0*q3;
    Rot(0, 2) = 2 * q1*q3 - 2 * q0*q2;
    Rot(1, 0) = 2 * q1*q2 - 2 * q0*q3;
    Rot(1, 1) = q0q0 - q1q1 + q2q2 - q3q3;
    Rot(1, 2) = 2 * q2*q3 + 2 * q0*q1;
    Rot(2, 0) = 2 * q1*q3 + 2 * q0*q2;
    Rot(2, 1) = 2 * q2*q3 - 2 * q0*q1;
    Rot(2, 2) = q0q0 - q1q1 - q2q2 + q3q3;

  } else {
    // use yaw, pitch, roll to calc rotation matrix
    Eigen::Matrix3d RotX, RotY, RotZ;

    double yaw = -((double)yaw_param * 2 * M_PI - M_PI); // z
    double pitch = (double)pitch_param * 2 * M_PI - M_PI; // y
    double roll = (double)roll_param * 2 * M_PI - M_PI; // x

    RotX = RotY = RotZ = Eigen::Matrix3d::Zero(3, 3);

    RotX(0, 0) = 1.f;
    RotX(1, 1) = RotX(2, 2) = cos(roll);
    RotX(1, 2) = sin(roll);
    RotX(2, 1) = -RotX(1, 2);

    RotY(0, 0) = RotY(2, 2) = cos(pitch);
    RotY(0, 2) = sin(pitch);
    RotY(2, 0) = -RotY(0, 2);
    RotY(1, 1) = 1.f;

    RotZ(0, 0) = RotZ(1, 1) = cos(yaw);
    RotZ(0, 1) = sin(yaw);
    RotZ(1, 0) = -RotZ(0, 1);
    RotZ(2, 2) = 1.f;

    // multiply individual rotation matrices
    if (rot_order_param < 0.5f)
    {
      // ypr order zyx -> mutliply inverse!
      Rot = RotX * RotY * RotZ;
    }
    else {
      // rpy order xyz -> mutliply inverse!
      Rot = RotZ * RotY * RotX;
    }
  }


  // first order initialization - prototype matrix
  Eigen::Matrix3d R_1;
  R_1(0,0) = Rot(1,1);
  R_1(0,1) = Rot(1,2);
  R_1(0,2) = Rot(1,0);
  R_1(1,0) = Rot(2,1);
  R_1(1,1) = Rot(2,2);
  R_1(1,2) = Rot(2,0);
  R_1(2,0) = Rot(0,1);
  R_1(2,1) = Rot(0,2);
  R_1(2,2) = Rot(0,0);
  // zeroth order is invariant
  Sh_transf(0,0) = 1.;
  // set first order
  Sh_transf.block(1, 1, 3, 3) = R_1;

  Eigen::MatrixXd R_lm1 = R_1;

  // recursivly generate higher orders
  for (int l=2; l<=AMBI_ORDER; l++)
  {
    Eigen::MatrixXd R_l = Eigen::MatrixXd::Zero(2*l+1, 2*l+1);
    for (int m=-l;m <= l; m++)
    {
      for (int n=-l;n <= l; n++)
      {
        // Table I
        int d = (m==0) ? 1 : 0;
        double denom = 0.;
        if (abs(n) == l)
          denom = (2*l)*(2*l-1);
        else
          denom = (l*l-n*n);

        double u = sqrt((l*l-m*m)/denom);
        double v = sqrt((1.+d)*(l+abs(m)-1.)*(l+abs(m))/denom)*(1.-2.*d)*0.5;
        double w = sqrt((l-abs(m)-1.)*(l-abs(m))/denom)*(1.-d)*(-0.5);

        if (u != 0.)
          u *= U(l,m,n,R_1,R_lm1);
        if (v != 0.)
          v *= V(l,m,n,R_1,R_lm1);
        if (w != 0.)
          w *= W(l,m,n,R_1,R_lm1);

        R_l(m+l,n+l) = u + v + w;
      }
    }
    Sh_transf.block(l*l, l*l, 2*l+1, 2*l+1) = R_l;
    R_lm1 = R_l;
  }

#endif

    // threshold coefficients
    // maybe not needed here...
    for (int i = 0; i < Sh_transf.size(); i++)
    {
        if (abs(Sh_transf(i)) < 0.00001f)
            Sh_transf(i) = 0.f;
    }

}

bool Ambix_rotatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
}

void Ambix_rotatorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    if (_new_params)
    {
        _Sh_transf = Sh_transf; // buffer old values
        calcParams(); // calc new transformation matrix
    }

    int NumSamples = buffer.getNumSamples();

    output_buffer.setSize(buffer.getNumChannels(), NumSamples);
    output_buffer.clear();

    int num_out_ch = jmin(AMBI_CHANNELS,getTotalNumOutputChannels());
    int num_in_ch = jmin(AMBI_CHANNELS,getTotalNumInputChannels());

    // 0th channel is invariant!
    output_buffer.addFrom(0, 0, buffer, 0, 0, NumSamples);

    for (int out = 1; out < num_out_ch; out++)
    {
        int n = (int)sqrtf(out);// order
        int in_start = n*n;
        int in_end = jmin((n+1)*(n+1), num_in_ch);
        for (int in = in_start; in < in_end; in++)
        {
            if (!_new_params)
            {
                if (Sh_transf(in, out) != 0.f)
                    output_buffer.addFrom(out, 0, buffer, in, 0, NumSamples, (float)Sh_transf(in, out));
            } else {
                if (_Sh_transf(in, out) != 0.f || Sh_transf(in, out) != 0.f)
                    output_buffer.addFromWithRamp(out, 0, buffer.getReadPointer(in), NumSamples, (float)_Sh_transf(in, out), (float)Sh_transf(in, out));
            }
        }
    }

    if (_new_params)
        _new_params = false;

    buffer = output_buffer;

}

bool Ambix_rotatorAudioProcessor::isQuaternionActive()
{
    return _q_changed;
}

//==============================================================================
bool Ambix_rotatorAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* Ambix_rotatorAudioProcessor::createEditor()
{
    return new Ambix_rotatorAudioProcessorEditor (this);
  //return nullptr;
}

//==============================================================================
void Ambix_rotatorAudioProcessor::getStateInformation (MemoryBlock& destData)
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
    xml.setAttribute("_q_changed", _q_changed);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_rotatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

            if (xmlState->hasAttribute("_q_changed"))
            {
              _q_changed = xmlState->getBoolAttribute("_q_changed");
              _new_params = true;
            }

        }

    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_rotatorAudioProcessor();
}
