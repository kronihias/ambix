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
inline double round( double d )
{
    return floor( d + 0.5 );
}
#endif

//==============================================================================
Ambix_warpAudioProcessor::Ambix_warpAudioProcessor() :
phi_param(0.5f),
_phi_param(-10.f),
phi_curve_param(0.f),
_phi_curve_param(0.f),
theta_param(0.5f),
_theta_param(-10.f),
theta_curve_param(0.f),
_theta_curve_param(0.f),
preemp_param(1.f),
_preemp_param(1.f),
in_order_param(1.f),
out_order_param(1.f),
in_order(AMBI_ORDER),
_in_order(-1),
out_order(AMBI_ORDER),
_out_order(-1),
_initialized(false),
output_buffer(AMBI_CHANNELS,256)
{
    // set transformation matrix to identity matrix
	Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
	_Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
}

Ambix_warpAudioProcessor::~Ambix_warpAudioProcessor()
{
}

//==============================================================================
const String Ambix_warpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_warpAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_warpAudioProcessor::getParameter (int index)
{
    switch (index)
	{
		case PhiParam:  return phi_param;
		case PhiCurveParam: return phi_curve_param;
            
        case ThetaParam:    return theta_param;
		case ThetaCurveParam:   return theta_curve_param;
            
        case InOrderParam:  return in_order_param;
        case OutOrderParam:  return out_order_param;
            
        case PreEmpParam:  return preemp_param;

		default:            return 0.0f;
	}
}

void Ambix_warpAudioProcessor::setParameter (int index, float newValue)
{
    switch (index)
	{
        case PhiParam:
            phi_param = newValue;
            break;
        case PhiCurveParam:
            phi_curve_param = newValue;
            break;
            
        case ThetaParam:
            theta_param = newValue;
            break;
        case ThetaCurveParam:
            theta_curve_param = newValue;
            break;
            
        case InOrderParam:
            in_order_param = newValue;
            in_order = (int)round(in_order_param*AMBI_ORDER); // AMBI_ORDER is maximum
            _initialized = false; // reinit
            break;
        case OutOrderParam:
            out_order_param = newValue;
            out_order = (int)round(out_order_param*AMBI_ORDER);
            break;
            
        case PreEmpParam:
            preemp_param = newValue;
            
		default:
            break;
	}
}

const String Ambix_warpAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
        case PhiParam:        return "Az Warp factor";
        case PhiCurveParam:        return "Az Warp curve";
        case ThetaParam:        return "El Warp factor";
        case ThetaCurveParam:        return "El Warp curve";
            
        case InOrderParam:        return "Ambi In order";
        case OutOrderParam:        return "Ambi Out order";
            
        case PreEmpParam:          return "PreEmphasis";
		default:								break;
	}
	
	return String::empty;
}

const String Ambix_warpAudioProcessor::getParameterText (int index)
{
    String text;
    
    switch (index)
	{
        case PhiParam:
            text = String(phi_param*1.8f-0.9f).substring(0, 5);
            // text << "";
            break;
            
        case PhiCurveParam:
            if (phi_curve_param <= 0.5f)
                text = "-90 / 90 deg";
            else
                text = "180 / 0 deg";
            break;
            
            
        case ThetaParam:
            text = String(theta_param*1.8f-0.9f).substring(0, 5);
            // text << "";
            break;
            
        case ThetaCurveParam:
            if (theta_curve_param <= 0.5f)
                text = "northpole";
            else
                text = "equator";
            break;
            
        case InOrderParam:
            text = String(in_order);
            break;
            
        case OutOrderParam:
            text = String(out_order);
            break;
            
        case PreEmpParam:
            if (preemp_param <= 0.5f)
                text = "Off";
            else
                text = "On";
            break;
            
		default:
            break;
	}
    
	return text;
}

const String Ambix_warpAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_warpAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_warpAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_warpAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_warpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_warpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_warpAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_warpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_warpAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_warpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_warpAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_warpAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void Ambix_warpAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_warpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // calcParams();
}

void Ambix_warpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_warpAudioProcessor::calcParams()
{
    if (!_initialized)
    {
    
        sph_h_in.Init(in_order);
        
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
        
        // std::cout << "Coordinate size: " << Carth_coord.rows() << " x " << Carth_coord.cols() << std::endl;
        // std::cout << Carth_coord << std::endl;
        
        Sph_coord.resize(numsamples,2); // positions in spherical coordinates
        
        Eigen::MatrixXd Sh_matrix;
        Sh_matrix.setZero(numsamples,AMBI_CHANNELS);
        
        int in_ambi_channels = (in_order+1)*(in_order+1);
        for (int i=0; i < numsamples; i++)
        {
            Eigen::VectorXd Ymn(in_ambi_channels); // Ymn result
            
            Sph_coord(i,0) = atan2(Carth_coord(i,1),Carth_coord(i,0)); // azimuth
            Sph_coord(i,1) = atan2(Carth_coord(i,2),sqrt(Carth_coord(i,0)*Carth_coord(i,0) + Carth_coord(i,1)*Carth_coord(i,1))); // elevation
            
            sph_h_in.Calc(Sph_coord(i,0),Sph_coord(i,1)); // phi theta
            sph_h_in.Get(Ymn);
            
            // std::cout << "Ymn Size: " << Ymn.size() << std::endl;
            // std::cout << "Sh_matrix Size: " << Sh_matrix.size() << std::endl;
            
            //Sh_matrix.row(i) = Ymn;
            Sh_matrix.block(i,0,1,in_ambi_channels) = Ymn.transpose();
            // std::cout << "Size: " << Sh_matrix.block(i,0,1,in_ambi_channels).size() << ": " << Sh_matrix.block(i,0,1,in_ambi_channels) << std::endl;
        }
        
        // Sh_matrix_inv.setZero();
        
        //Sh_matrix_inv = (Sh_matrix.transpose()*Sh_matrix).inverse()*Sh_matrix.transpose(); // not working for dynamic input order
        // if input order is different a better solving has to be used for the inverse:
        Sh_matrix_inv = (Sh_matrix.transpose()*Sh_matrix).colPivHouseholderQr().inverse()*Sh_matrix.transpose();
        
        // std::cout << "Size Sh_matrix: " << Sh_matrix.rows() << " x " << Sh_matrix.cols() << std::endl;
        
        // std::cout << "Size Inverse: " << Sh_matrix_inv.rows() << " x " << Sh_matrix_inv.cols() << std::endl;
        
        // std::cout << Sh_matrix_inv << std::endl;
    
        _initialized = true;
    }
    
    if ( (phi_param != _phi_param) ||
        (floor(phi_curve_param+0.5f) != floor(_phi_curve_param+0.5f)) ||
        (theta_param != _theta_param) ||
        (floor(theta_curve_param+0.5f) != floor(_theta_curve_param+0.5f)) ||
        (in_order != _in_order) ||
        (out_order != _out_order) ||
        (floor(preemp_param+0.5f) != floor(_preemp_param+0.5f))
        )
    {
        
        if (_out_order != out_order)
        {
            // init or reinit spherical harmonics
            sph_h_out.Init(out_order);
        }
        
        Eigen::MatrixXd Sh_matrix_mod;
        Sh_matrix_mod.setZero(Sph_coord.rows(),AMBI_CHANNELS);
        
        Eigen::VectorXd pre_emphasis;
        pre_emphasis.setOnes(Sph_coord.rows());
        
        Eigen::MatrixXd Sph_coord_mod = Sph_coord;
        
        // warping parameters - use inverse parameters! (see paper)
        double phi_alpha = -((double)phi_param*1.8f - 0.9f);
        double theta_alpha = -((double)theta_param*1.8f - 0.9f);
        
        int out_ambi_channels = (out_order+1)*(out_order+1);
        
        for (int i=0; i < Sph_coord_mod.rows(); i++)
        {
            if (theta_alpha != 0.)
            {
                // warp elevation
                // as we are using elevation from -pi/2....pi/2
                double mu = sin(Sph_coord(i,1));
                
                if (theta_curve_param <= 0.5f)
                {
                    // warp towards a pole
                    
                    Sph_coord_mod(i,1) = asin((mu+theta_alpha)/(1+theta_alpha*mu));
                    
                    // pre emphasis
                    pre_emphasis(i) = (1+theta_alpha*mu)/sqrt(1-theta_alpha*theta_alpha);
                } else {
                    //warp towards equator (< 0.)or away from equator (> 0.)
                    // in the paper this parameter is called beta
                    
                    // pre emphasis
                    pre_emphasis(i) = sqrt((1-abs(theta_alpha))*(1+abs(theta_alpha)*mu*mu))/(1-abs(theta_alpha)*mu*mu);
                  
                    if (theta_alpha > 0.) {
                        
                        Sph_coord_mod(i,1) = asin((theta_alpha-1+sqrt(pow(theta_alpha-1,2)+4*theta_alpha*pow(mu,2)))/(2*theta_alpha*mu));
                        
                    } else {
                        Sph_coord_mod(i,1) = asin(((1+theta_alpha)*mu)/(1+theta_alpha*pow(mu,2)));
                        pre_emphasis(i) = 1/pre_emphasis(i); // this is ^sgn(beta)
                    }
                    
                }
            }
            /*
             // this is not fully working yet
            if (phi_alpha != 0.)
            {
                // warp azimuth
                // as we are using azimuth from -pi....pi
                double phi_temp = Sph_coord(i,0) * 0.5f;
                
                double mu = sin(phi_temp);
                
                if (phi_curve_param <= 0.5f)
                {
                    // warp towards a pole
                    
                    Sph_coord_mod(i,0) = asin((mu+phi_alpha)/(1+phi_alpha*mu));
                    
                    // pre emphasis
                    pre_emphasis(i) *= (1+phi_alpha*mu)/sqrt(1-phi_alpha*phi_alpha);
                } else {
                    //warp towards equator (< 0.)or away from equator (> 0.)
                    // in the paper this parameter is called beta
                    
                    // pre emphasis
                    double pre_emph_temp = sqrt((1-abs(phi_alpha))*(1+abs(phi_alpha)*mu*mu))/(1-abs(phi_alpha)*mu*mu);
                    
                    if (phi_alpha > 0.) {
                        
                        Sph_coord_mod(i,0) = asin((phi_alpha-1+sqrt(pow(phi_alpha-1,2)+4*phi_alpha*pow(mu,2)))/(2*phi_alpha*mu));
                        pre_emphasis(i) *= pre_emph_temp;
                        
                    } else {
                        Sph_coord_mod(i,0) = asin(((1+phi_alpha)*mu)/(1+phi_alpha*pow(mu,2)));
                        pre_emphasis(i) *= 1/pre_emph_temp; // this is ^sgn(beta)
                    }
                    
                }
                Sph_coord_mod(i,0) *= 2;
            }
            */
            
            if (phi_alpha != 0.f)
            {
                // warp azimuth
                // as we are using azimuth from -pi....pi
                double phi_temp = Sph_coord(i,0) * 0.5f;
                
                if (phi_curve_param <= 0.5f)
                {
                    // warp towards poles
                    Sph_coord_mod(i,0) = asin((sin(phi_temp)+phi_alpha)/(1.f+phi_alpha*sin(phi_temp)));
                } else {
                    //warp towards equator
                    
                    if (phi_temp > 0)
                    {
                        Sph_coord_mod(i,0) = acos((cos(phi_temp*2)+phi_alpha)/(1+phi_alpha*cos(phi_temp*2))) / 2;
                    } else {
                        Sph_coord_mod(i,0) = -acos((cos(M_PI-(phi_temp-M_PI/2)*2)+phi_alpha)/(1+phi_alpha*cos(M_PI-(phi_temp-M_PI/2)*2))) / 2;
                    }
                }
                
                Sph_coord_mod(i,0) *= 2;
                
            }
            
            // rotate
            // Sph_mod(i,0) = Sph(i,0);
            
            Eigen::VectorXd Ymn(out_ambi_channels); // Ymn result
            
            sph_h_out.Calc(Sph_coord_mod(i,0),Sph_coord_mod(i,1)); // phi theta
            sph_h_out.Get(Ymn);
            
            // Sh_matrix_mod.row(i) = Ymn;
            Sh_matrix_mod.block(i,0,1,out_ambi_channels) = Ymn.transpose();
        }
        
        // apply preemphasis if wanted
        if (preemp_param > 0.5f)
        {
            Sh_matrix_mod = Sh_matrix_mod * pre_emphasis.asDiagonal();
        }
        
        // calculate new transformation matrix
        
        Sh_transf = Sh_matrix_inv * Sh_matrix_mod;
        
        // threshold coefficients
        for (int i = 0; i < Sh_transf.size(); i++)
        {
            if (abs(Sh_transf(i)) < 0.00001f)
                Sh_transf(i) = 0.f;
        }
        
        _phi_param = phi_param;
        _phi_curve_param = phi_curve_param;
        _theta_param = theta_param;
        _theta_curve_param = theta_curve_param;
        _in_order = in_order;
        _out_order = out_order;
        _preemp_param = preemp_param;
    }
    
    
    
}

void Ambix_warpAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    _Sh_transf = Sh_transf; // buffer old values
    
    calcParams(); // calc new transformation matrix
    
    int NumSamples = buffer.getNumSamples();
    
    output_buffer.setSize(buffer.getNumChannels(), NumSamples);
    output_buffer.clear();
    
    
    for (int out = 0; out < std::min(AMBI_CHANNELS,getTotalNumOutputChannels()); out++)
    {
        for (int in = 0; in < std::min(AMBI_CHANNELS,getTotalNumInputChannels()); in++)
        {
            if (_Sh_transf(out, in) != 0.f || Sh_transf(out, in) != 0.f)
            {
                if (_Sh_transf(out, in) == Sh_transf(out, in))
                {
                    output_buffer.addFrom(out, 0, buffer, in, 0, NumSamples, (float)Sh_transf(out, in));
                } else {
                    output_buffer.addFromWithRamp(out, 0, buffer.getReadPointer(in), NumSamples, (float)_Sh_transf(out, in), (float)Sh_transf(out, in));
                }
                
            }
        }
    }
    
    
    buffer = output_buffer;
    
}

//==============================================================================
bool Ambix_warpAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_warpAudioProcessor::createEditor()
{
    //return new Ambix_warpAudioProcessorEditor (this);
    return nullptr;
}

//==============================================================================
void Ambix_warpAudioProcessor::getStateInformation (MemoryBlock& destData)
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
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_warpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            for (int i=0; i < getNumParameters(); i++) {
                setParameter(i, xmlState->getDoubleAttribute(String(i)));
            }
        }
        
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_warpAudioProcessor();
}
