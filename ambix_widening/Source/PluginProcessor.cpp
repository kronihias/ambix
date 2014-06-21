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
#include "tools.h"
#define _USE_MATH_DEFINES
#include <math.h>



//==============================================================================
Ambix_wideningAudioProcessor::Ambix_wideningAudioProcessor() :
mod_depth_param(0.0f),
mod_t_param(0.02f),
_mod_t_param(0.f),
rot_offset_param(0.5f),
single_sided(false),
param_changed(true),
Q(10),
ring_buffer(AMBI_CHANNELS,1000),
_buf_write_pos(0),
_buf_size(1000)
{
    
}

Ambix_wideningAudioProcessor::~Ambix_wideningAudioProcessor()
{
}

//==============================================================================
const String Ambix_wideningAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_wideningAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_wideningAudioProcessor::getParameter (int index)
{
    switch (index)
	{
		case ModDepthParam:     return mod_depth_param;
		case ModTParam:         return mod_t_param;
        case RotOffsetParam:    return rot_offset_param;
		case SingleSideParam:
            if (single_sided)
                return 1.f;
            else
                return 0.f;
		default:                return 0.0f;
	}
}

void Ambix_wideningAudioProcessor::setParameter (int index, float newValue)
{
    
    switch (index)
	{
		case ModDepthParam:
            mod_depth_param = newValue;
            param_changed = true;
            break;
		case ModTParam:
            mod_t_param = newValue;
            param_changed = true;
            break;
        case RotOffsetParam:
            rot_offset_param = newValue;
            param_changed = true;
            break;
		case SingleSideParam:
            if (newValue <= 0.5)
            {
                single_sided = false;
            } else {
                single_sided = true;
            }
            param_changed = true;
            break;
		default:
            break;
	}
}

const String Ambix_wideningAudioProcessor::getParameterName (int index)
{
    switch (index)
	{
		case ModDepthParam:     return "Mod Depth";
		case ModTParam:         return "Mod T";
        case RotOffsetParam:    return "Rotation Offset";
		case SingleSideParam:   return "Single Sided";
		default:                return String::empty;
	}
    
}

// convert param 0...1 to T = 0.2 ms....70ms
float paramModT2ms(float param)
{
    return (param*69.8f + 0.2f);
}

double paramModT2s(float param)
{
    return (param*0.0698 + 0.0002);
}

float paramRot2deg(float param)
{
    return (param*360 - 180);
}

const String Ambix_wideningAudioProcessor::getParameterText (int index)
{
    String text;
    
    switch (index)
	{
		case ModDepthParam:
            text << String(mod_depth_param*360).substring(0, 5);
            text << " deg";
            break;
		case ModTParam:
            text << String(paramModT2ms(mod_t_param)).substring(0, 5);
            text << " ms";
            break;
        case RotOffsetParam:
            text << String(paramRot2deg(rot_offset_param)).substring(0, 5);
            text << " deg";
            break;
		case SingleSideParam:
            if (single_sided)
            {
                text << "yes";
            } else {
                text << "no";
            }
            break;
            
		default:
            break;
	}
    
    
	return text;
}

const String Ambix_wideningAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_wideningAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_wideningAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_wideningAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_wideningAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_wideningAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_wideningAudioProcessor::silenceInProducesSilenceOut() const
{
    return true;
}

double Ambix_wideningAudioProcessor::getTailLengthSeconds() const
{
    return Q*BESSEL_APPR;
}

int Ambix_wideningAudioProcessor::getLatencySamples() const
{
    if (single_sided)
        return 0;
    else
        return Q*BESSEL_APPR;
}

int Ambix_wideningAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_wideningAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_wideningAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_wideningAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void Ambix_wideningAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_wideningAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    calcParams();
    
    ring_buffer.clear();
}

void Ambix_wideningAudioProcessor::releaseResources()
{
    ring_buffer.clear();
}

void Ambix_wideningAudioProcessor::checkBuffer()
{
    Q = (int)floor(paramModT2s(mod_t_param)*getSampleRate()+0.5f);
    
    // std::cout << "t: " << paramModT2ms(mod_t_param) << "ms; Q: " << Q << " samples; f: " << 1.f/paramModT2s(mod_t_param) << " Hz" << std::endl;
    
    _buf_size = Q*(BESSEL_APPR*2+1) + getBlockSize(); // + 1
    
    if (ring_buffer.getNumSamples() < _buf_size) {
        ring_buffer.setSize(AMBI_CHANNELS, _buf_size);
        // std::cout << "resized to: "<< _buf_size << " t: " << paramModT2ms(mod_t_param) << "ms; Q: " << Q << " samples; f: " << 1.f/paramModT2s(mod_t_param) << " Hz, Block: " << getBlockSize() << std::endl;
        
        ring_buffer.clear();
    }
}
void Ambix_wideningAudioProcessor::calcParams()
{
    checkBuffer();
    
    if (param_changed) {
        
        // set new latency
        if (single_sided)
            setLatencySamples(0);
        else
            setLatencySamples(Q*BESSEL_APPR);
        
        // parameters in rad
        
        double phi_hat = ((double)mod_depth_param)*2*M_PI;
        double rot_offset = M_PI - ((double)rot_offset_param + 0.002)*2*M_PI; // offset needed - why??
        
        // std::cout << "MATRIX:" << std::endl;
        
        for (int m=1; m <= AMBI_ORDER; m++) {
            
            // String output_cos = "cos: ";
            // String output_sin = "sin: ";
            
            for (int lambda = 0; lambda <= BESSEL_APPR; lambda++) {
                
                double bessel = jn(lambda, (double)m*phi_hat);
                
                double d_cos_coeff = cos(M_PI_2*lambda + m*rot_offset)*bessel;
                double d_sin_coeff = sin(M_PI_2*lambda + m*rot_offset)*bessel;
                
                //////
                // set zero if lower then threshold defined in .h file
                if (fabs(d_cos_coeff) < TRUNCATE) {
                    d_cos_coeff = 0.f;
                }
                if (fabs(d_cos_coeff) < TRUNCATE) {
                    d_sin_coeff = 0.f;
                }
                
                cos_coeffs[m-1][lambda] = (float)d_cos_coeff;
                sin_coeffs[m-1][lambda] = (float)d_sin_coeff;
                
                
                // output_cos << String(cos_coeffs[m-1][lambda]).substring(0, 4);
                // output_cos << " ";
                
                
                // output_sin << String(sin_coeffs[m-1][lambda]).substring(0, 4);
                // output_sin << " ";
                
            }
            // std::cout << output_cos << std::endl;
            // std::cout << output_sin << std::endl;
        }
        
        param_changed = false;
    }
    
}

void Ambix_wideningAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // backup old coefficients
    memcpy(_cos_coeffs, cos_coeffs, sizeof(float)*AMBI_ORDER*(BESSEL_APPR+1));
    memcpy(_sin_coeffs, sin_coeffs, sizeof(float)*AMBI_ORDER*(BESSEL_APPR+1));
  
  
    // calc new coefficients
    calcParams();
    
    //////////////////////
    // write to the buffer
    if (_buf_write_pos + buffer.getNumSamples() < _buf_size)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
        {
            // copy straight into buffer
            ring_buffer.copyFrom(ch, _buf_write_pos, buffer, ch, 0, buffer.getNumSamples());
        }
        // update write position
        _buf_write_pos += buffer.getNumSamples();
        
    } else { // if buffer reaches end
        
        int samples_to_write1 = _buf_size - _buf_write_pos;
        int samples_to_write2 = buffer.getNumSamples() - samples_to_write1;
        
        // std::cout << "spl_write1: " << samples_to_write1 << " spl_write2: " << samples_to_write2 << std::endl;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
        {
            
            // copy until end
            ring_buffer.copyFrom(ch, _buf_write_pos, buffer, ch, 0, samples_to_write1);
            
            
            // start copy to front
            ring_buffer.copyFrom(ch, 0, buffer, ch, samples_to_write1, samples_to_write2);
        }
        // update write position
        _buf_write_pos = samples_to_write2;
    }
    
    
    // String debug = String::empty;
    
    /////////////////////////
    // compute read positions (tap delay times)
    // Q is computed in calcParams() (SampleRate dependent)
    
    for (int i=0; i < BESSEL_APPR*2+1; i++) {
        _buf_read_pos[i] = _buf_write_pos - i*Q - buffer.getNumSamples();
        if (_buf_read_pos[i] < 0)
            _buf_read_pos[i] = _buf_size + _buf_read_pos[i]; // -1 ?
        
        // debug << _buf_read_pos[i] << " ";
    }
    
    // std::cout << "read pos: " << debug << std::endl;
    
    /////////////////////////
    // do the rotation
    buffer.clear();
    
    int fir_length = 2*BESSEL_APPR+1;
    
    if (single_sided)
        fir_length = BESSEL_APPR+1;
    
    for (int acn_out = 0; acn_out < getNumOutputChannels(); acn_out++) // iterate over output channels
    {
        int l_out = 0;
        int m_out = 0;
        
        ACNtoLM(acn_out, l_out, m_out);
        
        for (int acn_in = 0; acn_in < getNumInputChannels(); acn_in++) // iterate over input channels
        {
            int l_in=0; // degree 0, 1, 2, 3, 4, ...
            int m_in=0; // order ..., -2, -1, 0 , 1, 2, ...
            
            ACNtoLM(acn_in, l_in, m_in);
            
            if (abs(m_out) == abs (m_in) && l_in == l_out) { // if degree and order match do something
                
                int pos_index = 0; // index of _buf_read_pos
                
                ///////////////
                // pass through terms (z symmetric, m=0)
                if (m_out == 0 && m_in == 0) {
                    
                    if (!single_sided)
                        pos_index = BESSEL_APPR;
                    
                    // read from buffer
                    if (_buf_read_pos[pos_index] + buffer.getNumSamples() < _buf_size)
                    {
                        buffer.copyFrom(acn_out, 0, ring_buffer, acn_out, _buf_read_pos[pos_index], buffer.getNumSamples());
                    } else {
                        
                        int samples_to_read1 = _buf_size - _buf_read_pos[pos_index];
                        int samples_to_read2 = buffer.getNumSamples() - samples_to_read1;
                        
                        // copy until end
                        buffer.copyFrom(acn_out, 0, ring_buffer, acn_out, _buf_read_pos[pos_index], samples_to_read1);
                        
                        // start copy from front
                        buffer.copyFrom(acn_out, samples_to_read1, ring_buffer, acn_out, 0, samples_to_read2);
                    }
                    
                }
                ///////////////
                // cosine terms
                else  if (m_in < 0 && m_out < 0) // cosine
                {
                    
                    for (int i=0; i < fir_length; i++) { // ITERATE BESSEL APPR
                        
                        int j = abs(i-BESSEL_APPR);
                        if (single_sided)
                            j=i;
                        
                        float _coeff = _cos_coeffs[l_in-1][j];
                        float coeff = cos_coeffs[l_in-1][j];
                        
                        if (coeff != 0.f) { // skip zero coefficients
                            
                            // read from buffer
                            if (_buf_read_pos[i] + buffer.getNumSamples() < _buf_size)
                            {
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), buffer.getNumSamples(), _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], buffer.getNumSamples(), coeff);

                            } else {
                                
                                int samples_to_read1 = _buf_size - _buf_read_pos[i];
                                int samples_to_read2 = buffer.getNumSamples() - samples_to_read1;
                                
                                // copy until end
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), samples_to_read1, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], samples_to_read1, coeff);

                                
                                // start copy from front
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, samples_to_read1, ring_buffer.getReadPointer(acn_in, 0), samples_to_read2, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, samples_to_read1, ring_buffer, acn_in, 0, samples_to_read2, coeff);

                            }
                        }
                        
                    } // end iterate BESSEL_APPR
                }
                
                ///////////////
                // -sine terms
                else  if (m_in < 0 && m_out > 0) // -sine
                {
                    for (int i=0; i < fir_length; i++) { // ITERATE BESSEL APPR
                        
                        int j = abs(i-BESSEL_APPR);
                        if (single_sided)
                            j=i;
                        
                        float _coeff = -_sin_coeffs[l_in-1][j];
                        float coeff = -sin_coeffs[l_in-1][j];
                        
                        if (coeff != 0.f) { // skip zero coefficients
                            
                            // read from buffer
                            if (_buf_read_pos[i] + buffer.getNumSamples() < _buf_size)
                            {
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), buffer.getNumSamples(), _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], buffer.getNumSamples(), coeff);
                                
                            } else {
                                
                                int samples_to_read1 = _buf_size - _buf_read_pos[i];
                                int samples_to_read2 = buffer.getNumSamples() - samples_to_read1;
                                
                                // copy until end
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), samples_to_read1, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], samples_to_read1, coeff);
                                
                                
                                // start copy from front
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, samples_to_read1, ring_buffer.getReadPointer(acn_in, 0), samples_to_read2, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, samples_to_read1, ring_buffer, acn_in, 0, samples_to_read2, coeff);
                                
                            }
                        }
                        
                    } // end iterate BESSEL_APPR
                }
                
                ///////////////
                // cosine terms
                else  if (m_in > 0 && m_out > 0) // cosine
                {
                    for (int i=0; i < fir_length; i++) { // ITERATE BESSEL APPR
                        
                        int j = abs(i-BESSEL_APPR);
                        if (single_sided)
                            j=i;
                        
                        float _coeff = _cos_coeffs[l_in-1][j];
                        float coeff = cos_coeffs[l_in-1][j];
                        
                        if (coeff != 0.f) { // skip zero coefficients
                            
                            // read from buffer
                            if (_buf_read_pos[i] + buffer.getNumSamples() < _buf_size)
                            {
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), buffer.getNumSamples(), _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], buffer.getNumSamples(), coeff);
                                
                            } else {
                                
                                int samples_to_read1 = _buf_size - _buf_read_pos[i];
                                int samples_to_read2 = buffer.getNumSamples() - samples_to_read1;
                                
                                // copy until end
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), samples_to_read1, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], samples_to_read1, coeff);
                                
                                
                                // start copy from front
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, samples_to_read1, ring_buffer.getReadPointer(acn_in, 0), samples_to_read2, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, samples_to_read1, ring_buffer, acn_in, 0, samples_to_read2, coeff);
                                
                            }
                        }
                        
                    } // end iterate BESSEL_APPR
                }
                
                ///////////////
                // sine terms
                else  if (m_in > 0 && m_out < 0) // sine
                {
                    for (int i=0; i < fir_length; i++) { // ITERATE BESSEL APPR
                        
                        int j = abs(i-BESSEL_APPR);
                        if (single_sided)
                            j=i;
                        
                        float _coeff = _sin_coeffs[l_in-1][j];
                        float coeff = sin_coeffs[l_in-1][j];
                        
                        if (coeff != 0.f) { // skip zero coefficients
                            
                            // read from buffer
                            if (_buf_read_pos[i] + buffer.getNumSamples() < _buf_size)
                            {
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), buffer.getNumSamples(), _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], buffer.getNumSamples(), coeff);
                                
                            } else {
                                
                                int samples_to_read1 = _buf_size - _buf_read_pos[i];
                                int samples_to_read2 = buffer.getNumSamples() - samples_to_read1;
                                
                                // copy until end
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, 0, ring_buffer.getReadPointer(acn_in, _buf_read_pos[i]), samples_to_read1, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, 0, ring_buffer, acn_in, _buf_read_pos[i], samples_to_read1, coeff);
                                
                                
                                // start copy from front
                                if (_coeff != coeff) // interpolate?
                                    buffer.addFromWithRamp(acn_out, samples_to_read1, ring_buffer.getReadPointer(acn_in, 0), samples_to_read2, _coeff, coeff);
                                else
                                    buffer.addFrom(acn_out, samples_to_read1, ring_buffer, acn_in, 0, samples_to_read2, coeff);
                                
                            }
                        }
                        
                    } // end iterate BESSEL_APPR
                }
                
            }
            
        }
        
    }
    
}

//==============================================================================
bool Ambix_wideningAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_wideningAudioProcessor::createEditor()
{
    return nullptr;
    //return new Ambix_wideningAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_wideningAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Ambix_wideningAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_wideningAudioProcessor();
}
