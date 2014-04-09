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

#include "MyConvolver.h"


MyConvolver::MyConvolver() : _init(false)
{
    
}

MyConvolver::~MyConvolver()
{
    if (_ffts_fwd_plan)
        ffts_free(_ffts_fwd_plan);
    
    if (_ffts_inv_plan)
        ffts_free(_ffts_inv_plan);
    
}

void MyConvolver::init(int blockSize, const float* irBuffer, int irLength)
{
    
    // get next ^2
    int nextPower2 = 1;
    while (blockSize < nextPower2) {
        nextPower2 *= 2;
    }
    
    _blockSize = nextPower2;
    
    _segmentSize = 2 * nextPower2; // the segment with overlap
    
    // init fft
    _ffts_fwd_plan = ffts_init_1d_real(_segmentSize, NEGATIVE_SIGN);
    _ffts_inv_plan = ffts_init_1d_real(_segmentSize, POSITIVE_SIGN);
    
    // init aligned complex Buffers
#ifdef HAVE_SSE
    _inFFTBuf = (float*)_mm_malloc(2 * _segmentSize * sizeof(float), 32);
    _outFFTBuf = (float*)_mm_malloc(2 * _segmentSize * sizeof(float), 32);
    _irFFTBuf = (float*)_mm_malloc(2 * _segmentSize * sizeof(float), 32);
#else
    _inFFTBuf = (float*)valloc(2 * _segmentSize * sizeof(float));
    _outFFTBuf = (float*)valloc(2 * _segmentSize * sizeof(float));
    _irFFTBuf = (float*)valloc(2 * _segmentSize * sizeof(float));
#endif
    
    // set input buffer zero
    /*
    for(int i=0;i<_segmentSize;i++) {
        _inFFTBuf[2*i]   = 0.0f;
        _inFFTBuf[2*i+1] = 0.0f;
    }
     */
    memset(_inFFTBuf, 0, 2 * _segmentSize * sizeof(float));
    
    // copy ir to real part of the input buffer
    for (int i=0; i < jmin(irLength, _blockSize);i++)
    {
        _inFFTBuf[2*i] = irBuffer[i];
    }
    
    ffts_execute(_ffts_fwd_plan, _inFFTBuf, _irFFTBuf);
    
    
    _init = true;
    
}

void MyConvolver::process(const float *inBuffer, float *outBuffer, int len)
{
    if (!_init)
        return;
    
    // set input buffer zero
    memset(_inFFTBuf, 0, 2 * _segmentSize * sizeof(float));
    
    // copy ir to real part of the input buffer
    for (int i=0; i < jmin(len, _blockSize);i++)
    {
        _inFFTBuf[2*i] = inBuffer[i];
    }
    
    ffts_execute(_ffts_fwd_plan, _inFFTBuf, _outFFTBuf);
    
    // complex multiplication
    for (int i=0; i < 2*_segmentSize; i++) {
        _inFFTBuf[2*i] = _irFFTBuf[2*i]*_outFFTBuf[2*i] - _irFFTBuf[2*i+1]*_outFFTBuf[2*i+1]; // real
        _inFFTBuf[2*i+1] = (_outFFTBuf[2*i]+_outFFTBuf[2*i+1])*(_irFFTBuf[2*i]+_irFFTBuf[2*i+1]) - _inFFTBuf[2*i]; // imag
    }
    
    
    // inverse FFT
    ffts_execute(_ffts_inv_plan, _inFFTBuf, _outFFTBuf);
    
    
    // overlap and copy to output buffer
    for (int i=0; i < jmin(len, _blockSize); i++) {
        outBuffer[i] = _outFFTBuf[2*i] + _outTDBuf[i];
    }
    
    // store overlap
    for (int i=0; i < (_segmentSize-jmin(len, _blockSize));i++)
    {
        _outTDBuf[i] = _outFFTBuf[2*i+2*jmin(len, _blockSize)];
    }
    
}