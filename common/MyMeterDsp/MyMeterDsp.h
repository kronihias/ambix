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

#ifndef __mcfx_meter__MyMeterDsp__
#define __mcfx_meter__MyMeterDsp__

#include <cmath>

class MyMeterDsp{
    
public:
    MyMeterDsp();
    ~MyMeterDsp();
    
    // hold peak in s, fall in dB/s
    void setParams(float _hold, float _fall);
    
    void setAudioParams(int _sampleRate, int _blocksize);
    
    void calc(float* buffer, int NumSamples);
    
    void reset();
    
    float getPeak(); // holds maximum until hold time and falls with fall rate
    float getPeakHold(); // holds maximum until reset
    float getRMS();
    
private:
    void calcFall();
    
    // actual values
    float rms;
    float peak;
    
    float peak_hold;
    
    // old values
    float _rms;
    float _peak;
    
    float _pow_sum;
    
    int hold_smpl_count;
    
    // params
    float hold;
    float fall;
    
    int sampleRate;
    int blockSize;
    
    int holdSamples;
    float fallPeak;
    float fallRMS;
};

#endif /* defined(__mcfx_meter__MyMeterDsp__) */
