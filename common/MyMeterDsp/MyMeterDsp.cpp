/*
 ==============================================================================
 
 This file is part of the mcfx (Multichannel Effects) plug-in suite.
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

#include "MyMeterDsp.h"


MyMeterDsp::MyMeterDsp() :
rms(0.f),
peak(0.f),
peak_hold(0.f),
_rms(0.f),
_peak(0.f),
_pow_sum(0.f),
hold_smpl_count(0),
hold(1.f),
fall(15.f),
holdSamples(0),
fallPeak(0.f)
{
    
}

MyMeterDsp::~MyMeterDsp()
{
}

void MyMeterDsp::calcFall()
{
    float t = (float)blockSize / (float)sampleRate;
    
    fallPeak = powf (10.0f, -0.05f * fall * t);
    fallRMS = powf (10.0f, -0.05f * 10 * t);
    
    holdSamples = (int)(sampleRate*hold);
}

void MyMeterDsp::setParams(float _hold, float _fall)
{
    hold = _hold;
    fall = _fall;
    
    calcFall();
    
}

void MyMeterDsp::setAudioParams(int _sampleRate, int _blockSize)
{
    sampleRate = _sampleRate;
    blockSize = _blockSize;
}

void MyMeterDsp::calc(float *buffer, int NumSamples)
{
    
    float pow_sum = 0.f;
    float* b_ptr = buffer;
    
    _peak = 0.f; // reset
    
    // iterate over samples, get maximum and summed square
    for (int i=0; i < NumSamples; i++) {
        
        if (fabs(*b_ptr) > _peak)
        {
            _peak = fabs(*b_ptr);
        }
            
        pow_sum += (*b_ptr)*(*b_ptr);
        b_ptr++;
        
    }
    
    // peak
    if (_peak > peak)
    {
        peak = _peak;
        hold_smpl_count = 0;
    }
    else
    {
        if (hold_smpl_count > holdSamples)
        {
            if (peak > 0.0001f) // denorm protection
                peak *= fallPeak;
        } else
            hold_smpl_count += NumSamples;
    }
    
    // peak hold
    if (peak > peak_hold)
        peak_hold = peak;
    
    // rms
    _rms = sqrtf(pow_sum / NumSamples);
    
    if (_rms > rms)
        rms = _rms;
    else
        if (rms > 0.0001f) // denorm protection
            rms *= fallRMS;
}

void MyMeterDsp::reset()
{
    peak = 0.f;
    rms = 0.f;
    peak_hold = 0.f;
}

float MyMeterDsp::getPeak()
{
    return peak;
}

float MyMeterDsp::getPeakHold()
{
    return peak_hold;
}

float MyMeterDsp::getRMS()
{
    return rms;
}