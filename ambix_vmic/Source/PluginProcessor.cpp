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

const float _180_PI = 180.f/(float)M_PI;
//==============================================================================
Ambix_vmicAudioProcessor::Ambix_vmicAudioProcessor() :
_initialized(false),
_param_changed(false),
output_buffer(AMBI_CHANNELS,256)
{
   // init parameters
    shape.setConstant(0.f);
    
    width.setConstant(0.f);
    
    height.setConstant(0.f);
    
    gain.setConstant(0.5f); // 0 dB
    
    window.setConstant(0.f);
    
    transition.setConstant(0.f);
    
    center_sph.setConstant(0.5f); // 0 deg
    
    // set transformation matrix to zero matrix
    Sh_transf = Eigen::MatrixXd::Zero(NUM_FILTERS_VMIC, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Zero(NUM_FILTERS_VMIC, AMBI_CHANNELS);
}

Ambix_vmicAudioProcessor::~Ambix_vmicAudioProcessor()
{
    
}

//==============================================================================
const String Ambix_vmicAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_vmicAudioProcessor::getNumParameters()
{
    return PARAMS_PER_FILTER*NUM_FILTERS_VMIC;
}

float Ambix_vmicAudioProcessor::getParameter (int index)
{
    int filter_id = (int)floor(index/PARAMS_PER_FILTER);
    
    if (filter_id >= NUM_FILTERS_VMIC) // safety..
        return 0.f;
    
    switch (index%PARAMS_PER_FILTER) {
            
        case 0:
            return (float)center_sph(filter_id, 0);
            break;
            
        case 1:
            return (float)center_sph(filter_id, 1);
            break;
            
        case 2:
            return (float)shape(filter_id);
            break;
            
        case 3:
            return (float)width(filter_id);
            
        case 4:
            return (float)height(filter_id);
            
            
        case 5:
            return (float)gain(filter_id);
            
            
        case 6:
            return (float)window(filter_id);
            
            
        case 7:
            return (float)transition(filter_id);


        default:
            return 0.0f;
    }
}

void Ambix_vmicAudioProcessor::setParameter (int index, float newValue)
{
    
    int filter_id = (int)floor(index/PARAMS_PER_FILTER);
    
    if (filter_id < NUM_FILTERS_VMIC) // safety..
    {
        _param_changed = true;
        
        switch (index%PARAMS_PER_FILTER) {
                
            case 0:
                center_sph(filter_id, 0) = newValue;
                break;
                
            case 1:
                center_sph(filter_id, 1) = newValue;
                break;
                
            case 2:
                shape(filter_id) = newValue;
                break;
                
            case 3:
                width(filter_id) = newValue;
                break;
                
            case 4:
                height(filter_id) = newValue;
                break;
                
            case 5:
                gain(filter_id) = newValue;
                break;
                
            case 6:
                window(filter_id) = newValue;
                break;
                
            case 7:
                transition(filter_id) = newValue;
                break;
                
            default:
                _param_changed = false;
                break;
        }
        
    }
    
}

const String Ambix_vmicAudioProcessor::getParameterName (int index)
{
    String text;
    
    int filter_id = (int)floor(index/PARAMS_PER_FILTER);
    
    if (filter_id >= NUM_FILTERS_VMIC) // safety..
        return String::empty;
    
    switch (index%PARAMS_PER_FILTER) {
            
        case 0:
            text = "center azimuth";
            break;
            
        case 1:
            text = "center elevation";
            break;
            
        case 2:
            text = "shape";
            break;
            
        case 3:
            text = "width";
            break;
            
        case 4:
            text = "height";
            break;
            
        case 5:
            text = "gain";
            break;
            
        case 6:
            text = "window";
            break;
            
        case 7:
            text = "transition";
            break;
            
        default:
            return "";
    }
    
    text << " ";
    text << filter_id+1;
    return text;
}

const String Ambix_vmicAudioProcessor::getParameterText (int index)
{
    String text;
    
    int filter_id = (int)floor(index/PARAMS_PER_FILTER);
    
    if (filter_id >= NUM_FILTERS_VMIC) // safety..
        return String::empty;
    
    switch (index%PARAMS_PER_FILTER) {
           
        case 0:
            text = String((center_sph(filter_id, 0) - 0.5)*360).substring(0, 5);
            text << " deg";
            break;
            
        case 1:
            text = String((center_sph(filter_id, 1) - 0.5)*360).substring(0, 5);
            text << " deg";
            break;
            
        case 2:
            if (shape(filter_id) <= 0.5)
                text = "circular";
            else
                text = "rectangular";
            break;
            
        case 3:
            text = String(width(filter_id)*180).substring(0, 5);
            text << " deg";
            break;
            
        case 4:
            text = String(height(filter_id)*180).substring(0, 5);
            text << " deg";
            break;
            
        case 5:
            text = String(ParamToDB(gain(filter_id))).substring(0, 5);
            text << " dB";
            break;
            
        case 6:
            if (window(filter_id) <= 0.5)
                text = "no";
            else
                text = "yes";
            break;
            
        case 7:
            text = String(transition(filter_id)).substring(0, 5);
            text << "";
            break;
            
        default:
            return "";
    }
    
    return text;
}

const String Ambix_vmicAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_vmicAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_vmicAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_vmicAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_vmicAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_vmicAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_vmicAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_vmicAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_vmicAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_vmicAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_vmicAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_vmicAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void Ambix_vmicAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_vmicAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Ambix_vmicAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_vmicAudioProcessor::calcParams()
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
        
        // std::cout << "Coordinate size: " << Carth_coord.rows() << " x " << Carth_coord.cols() << std::endl;
        // std::cout << Carth_coord << std::endl;
        
        Sph_coord.resize(numsamples,2); // positions in spherical coordinates
        
        
        Sh_matrix.setZero(numsamples,AMBI_CHANNELS);
        
        for (int i=0; i < numsamples; i++)
        {
            Eigen::VectorXd Ymn(AMBI_CHANNELS); // Ymn result
            
            Sph_coord(i,0) = atan2(Carth_coord(i,1),Carth_coord(i,0)); // azimuth
            Sph_coord(i,1) = atan2(Carth_coord(i,2),sqrt(Carth_coord(i,0)*Carth_coord(i,0) + Carth_coord(i,1)*Carth_coord(i,1))); // elevation
            
            sph_h.Calc(Sph_coord(i,0),Sph_coord(i,1)); // phi theta
            sph_h.Get(Ymn);
            
            Sh_matrix.block(i,0,1,AMBI_CHANNELS) = Ymn.transpose();
        }
        
        
        _initialized = true;
        
    }
    
    if (_param_changed)
    {
        // convert parameters to values for the filter
        // ArrayIntParam _shape = shape;
        
        ArrayParam _width = width * (float)M_PI; // 0...pi
        ArrayParam _height = height * (float)M_PI;
        
        ArrayParam _gain;
        
        for (int i=0; i < gain.rows();i++)
        {
            _gain(i) = ParamToRMS(gain(i));
        }
        
        SphCoordParam _center_sph = (center_sph - 0.5f)*2.f*M_PI;
        
        // std::cout << _center_sph << std::endl;
        
        CarthCoordParam _center_carth;
        
        // convert center spherical coordinates to carthesian
        for (int i=0; i < _center_sph.rows(); i++)
        {
            _center_carth(i,0) = cos(_center_sph(i,0))*cos(_center_sph(i,1)); // x
            _center_carth(i,1) = sin(_center_sph(i,0))*cos(_center_sph(i,1)); // y
            _center_carth(i,2) = sin(_center_sph(i,1)); // z
        }
        
        Sh_matrix_mod.setZero();
        
        // iterate over all filters
        for (int k=0; k < NUM_FILTERS_VMIC; k++)
        {
            // copy the SH_matrix to 
            Eigen::MatrixXd Sh_matrix_temp = Sh_matrix;
            
            // iterate over all sample points
            for (int i=0; i < Sh_matrix_temp.rows(); i++)
            {
                double multipl = 1.f;
                
                Eigen::Vector2d Sph_coord_vec = Sph_coord.row(i);
                Eigen::Vector2d _center_sph_vec = _center_sph.row(k);
                
                multipl *= sph_filter.GetWeight(&Sph_coord_vec, Carth_coord.row(i), &_center_sph_vec, _center_carth.row(k), (int)floor(shape(k)+0.5f), _width(k), _height(k), 1, true, transition(k)); // apply gain later
                
                Sh_matrix_temp.row(i) *= multipl;
            }
            
            // Sh_matrix_mod row is the sum over the columns
            Sh_matrix_mod.row(k) = Sh_matrix_temp.colwise().sum();
            
            // normalize and apply gain
            if (Sh_matrix_mod.row(k)(0) > 0.f)
                Sh_matrix_mod.row(k) *= 1/Sh_matrix_mod.row(k)(0)*_gain(k); 
        }
        
        // std::cout << "Size Sh_matrix_mod : " << Sh_matrix_mod.rows() << " x " << Sh_matrix_mod.cols() << std::endl;
        
        // std::cout << Sh_matrix_mod << std::endl;
        
        
        // this is the new transformation matrix
        Sh_transf = Sh_matrix_mod;
        
        // threshold coefficients
        for (int i = 0; i < Sh_transf.size(); i++)
        {
            if (abs(Sh_transf(i)) < 0.00001f)
                Sh_transf(i) = 0.f;
        }
        
        _param_changed = false;
    }
    
    
}

void Ambix_vmicAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    _Sh_transf = Sh_transf; // buffer old values
    
    calcParams(); // calc new transformation matrix
    
    int NumSamples = buffer.getNumSamples();
    
    output_buffer.setSize(buffer.getNumChannels(), NumSamples);
    output_buffer.clear();
    
    
    for (int out = 0; out < std::min(NUM_FILTERS_VMIC,getNumOutputChannels()); out++)
    {
        for (int in = 0; in < std::min(AMBI_CHANNELS,getNumInputChannels()); in++)
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
    
    // clear unused channels
    for (int out = std::min(NUM_FILTERS_VMIC,getNumOutputChannels()); out < output_buffer.getNumChannels(); out++)
    {
        output_buffer.clear(out, 0, NumSamples);
    }
    
    
    buffer = output_buffer;
}

//==============================================================================
bool Ambix_vmicAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_vmicAudioProcessor::createEditor()
{
    //return new Ambix_vmicAudioProcessorEditor (this);
    return nullptr;
}

//==============================================================================
void Ambix_vmicAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Ambix_vmicAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_vmicAudioProcessor();
}
