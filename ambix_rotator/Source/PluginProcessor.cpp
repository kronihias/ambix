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

//==============================================================================
Ambix_rotatorAudioProcessor::Ambix_rotatorAudioProcessor() :
yaw_param(0.5f),
_yaw_param(0.5f),
pitch_param(0.5f),
_pitch_param(0.5f),
roll_param(0.5f),
_roll_param(0.5f),
rot_order_param(0.f),
_rot_order_param(0.f),
_initialized(false),
output_buffer(AMBI_CHANNELS,256)
{
    // set transformation matrix to identity matrix
    Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
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
            
		default:            return 0.0f;
	}
}

void Ambix_rotatorAudioProcessor::setParameter (int index, float newValue)
{
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
            
		default:
            break;
	}
}

const String Ambix_rotatorAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
        case YawParam:        return "Yaw";
        case PitchParam:        return "Pitch";
        case RollParam:        return "Roll";
        case RotOrderParam:        return "Rotation order";
            
		default:								break;
	}
	
	return String::empty;
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
            if (rot_order_param < 0.5f)
                text = "yaw-pitch-roll";
            else
                text = "roll-pitch-yaw";
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
    return String::empty;
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

void Ambix_rotatorAudioProcessor::calcParams()
{
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
    
    if  (
        (yaw_param != _yaw_param) ||
        (pitch_param != _pitch_param) ||
         (roll_param != _roll_param) ||
         (rot_order_param != _rot_order_param)
        )
    {

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
        
        
        // threshold coefficients
        // maybe not needed here...
        for (int i = 0; i < Sh_transf.size(); i++)
        {
            if (abs(Sh_transf(i)) < 0.00001f)
                Sh_transf(i) = 0.f;
        }
        
        _yaw_param = yaw_param;
        _pitch_param = pitch_param;
        _roll_param = roll_param;
        _rot_order_param = rot_order_param;
    }
    
    
    
}

void Ambix_rotatorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    _Sh_transf = Sh_transf; // buffer old values
    
    calcParams(); // calc new transformation matrix
    
    int NumSamples = buffer.getNumSamples();
    
    output_buffer.setSize(buffer.getNumChannels(), NumSamples);
    output_buffer.clear();
    
    // this could be optimized because matrix is sparse!
    
    for (int out = 0; out < std::min(AMBI_CHANNELS,getNumOutputChannels()); out++)
    {
        for (int in = 0; in < std::min(AMBI_CHANNELS,getNumInputChannels()); in++)
        {
            if (_Sh_transf(in, out) != 0.f || Sh_transf(in, out) != 0.f)
            {
                if (_Sh_transf(in, out) == Sh_transf(in, out))
                {
                    output_buffer.addFrom(out, 0, buffer, in, 0, NumSamples, (float)Sh_transf(in, out));
                } else {
                    output_buffer.addFromWithRamp(out, 0, buffer.getReadPointer(in), NumSamples, (float)_Sh_transf(in, out), (float)Sh_transf(in, out));
                }
                
            }
        }
    }
    
    
    buffer = output_buffer;
    
}

//==============================================================================
bool Ambix_rotatorAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_rotatorAudioProcessor::createEditor()
{
    //return new Ambix_rotatorAudioProcessorEditor (this);
  return nullptr;
}

//==============================================================================
void Ambix_rotatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Ambix_rotatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_rotatorAudioProcessor();
}
