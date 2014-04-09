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

#ifndef __ambix_binaural__MyConvolver__
#define __ambix_binaural__MyConvolver__

#include "JuceHeader.h"

#include "include/ffts.h"

#ifdef HAVE_SSE
#include <xmmintrin.h>
#endif

class MyConvolver {
    
    
public:
    MyConvolver();
    ~MyConvolver();
    
    void init(int blockSize, const float* irBuffer, int irLength);
    
    void process(const float* inBuffer, float* outBuffer, int len);
    
private:
    
    ffts_plan_t* _ffts_fwd_plan;
    ffts_plan_t* _ffts_inv_plan;
    
    int _blockSize;
    int _segmentSize;
    
    float* _inFFTBuf;
    float* _outFFTBuf;
    
    float* _irFFTBuf;
    
    float* _outTDBuf;
    
    bool _init;
};


#endif /* defined(__ambix_binaural__MyConvolver__) */
