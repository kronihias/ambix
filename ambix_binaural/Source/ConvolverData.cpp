/*
 ==============================================================================
 
 This file is part of the mcfx (Multichannel Effects) plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com
 
 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)
 
 Details of these licenses can be found at: www.gnu.org/licenses
 
 mcfx is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
 ==============================================================================
 */

#include "ConvolverData.h"

ConvolverData::ConvolverData()
{
    NumInCh = 0;
    NumOutCh = 0;
    MaxLen = 0;
    SampleRate = 44100.f;
}

ConvolverData::~ConvolverData()
{
    
}

void ConvolverData::clear()
{
    NumInCh = 0;
    NumOutCh = 0;
    MaxLen = 0;
    
    IR.clear();
}

int ConvolverData::getNumInputChannels()
{
    return NumInCh + 1;
}

int ConvolverData::getNumOutputChannels()
{
    return NumOutCh + 1;
}

int ConvolverData::getNumIRs()
{
    return IR.size();
}


void ConvolverData::addIR(int in_ch, int out_ch, int offset, int delay, int length, AudioSampleBuffer* buffer, double src_samplerate)
{
    if (delay < 0)
        delay = 0;
    if (offset < 0)
        offset = 0;
    
    if ( (length == 0) || (length > buffer->getNumSamples()) )
        length = buffer->getNumSamples();
    
    
    IR.add(new IR_Data);
    IR.getLast()->InCh = in_ch;
    IR.getLast()->OutCh = out_ch;
    
    AudioSampleBuffer *IRBuf = &IR.getLast()->Data;
    
    int size = length + delay - offset;
    
    IRBuf->setSize(1, size);
    IRBuf->clear();
    IRBuf->copyFrom(0, delay, (*buffer), 0, offset, length);
    
    if (src_samplerate != SampleRate)
    {
        // do resampling
        double sr_conv_fact = SampleRate / src_samplerate;
        
        int newsize = ceil(size*sr_conv_fact);
        
        AudioSampleBuffer ResampledBuffer(1, newsize);
        ResampledBuffer.clear();
        
        // do the resampling
        soxr_quality_spec_t const q_spec = soxr_quality_spec(SOXR_HQ, 0);
        size_t odone;
        soxr_error_t error;
        
        error = soxr_oneshot(src_samplerate, SampleRate, 1, /* Rates and # of chans. */
                             IRBuf->getReadPointer(0), size, NULL,         /* Input. */
                             ResampledBuffer.getWritePointer(0), newsize, &odone,    /* Output. */
                             NULL, // soxr_io_spec_t
                             &q_spec, // soxr_quality_spec_t
                             NULL); // soxr_runtime_spec_t
        
        (*IRBuf) = ResampledBuffer;
        
    }
    
    NumInCh = (NumInCh < in_ch) ? in_ch : NumInCh;
    NumOutCh = (NumOutCh < out_ch) ? out_ch : NumOutCh;
    
    if (IRBuf->getNumSamples() > MaxLen)
        MaxLen = IRBuf->getNumSamples();
}


AudioSampleBuffer* ConvolverData::getIR(int id)
{
    if (id < IR.size()) {
        return &IR.getUnchecked(id)->Data;
    } else
        return 0;
}

int ConvolverData::getInCh(int id)
{
    if (id < IR.size()) {
        return IR.getUnchecked(id)->InCh;
    } else
        return 0;
}

int ConvolverData::getOutCh(int id)
{
    if (id < IR.size()) {
        return IR.getUnchecked(id)->OutCh;
    } else
        return 0;
}

int ConvolverData::getLength(int id)
{
    if (id < IR.size()) {
        return IR.getUnchecked(id)->Data.getNumSamples();
    } else
        return 0;
}

int ConvolverData::getMaxLength()
{
    return MaxLen;
}

void ConvolverData::setSampleRate(double samplerate)
{
    if (samplerate > 0.f)
        SampleRate = samplerate;
}
