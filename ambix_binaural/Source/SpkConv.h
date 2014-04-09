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

#ifndef __ambix_binaural__SpkConv__
#define __ambix_binaural__SpkConv__

#include <iostream>

#include "JuceHeader.h"

#if BINAURAL_DECODER // exclude file if not Binaural

#include <soxr.h> // resampling

#ifdef USE_ZITA_CONVOLVER
    // you may use zita convolver for osx and linux
    #include "zita-convolver.h"
#else
    //#define AUDIOFFT_FFTW3 // use fftw3 - possible license problem!

#ifdef __APPLE__
    #define AUDIOFFT_APPLE_ACCELERATE
#endif
    #include "FFTConvolver/Convolver.h"

#include "MyConvolver/MyConvolver.h"
#endif

class SpkConv
{
public:
    SpkConv ();
    ~SpkConv();
    
    bool loadIr(const File& audioFile, double sampleRate, int BufferLength, float gain=1.f, float delay=0.f, int swapChannels=0);
    void process(AudioSampleBuffer& InputBuffer, AudioSampleBuffer& OutputBuffer, int InCh=0);
    
    void unloadIr();
    
    double irLength(); // return IR length in seconds
    
private:
    bool ir_loaded;
    
    double sample_rate;
    
#ifdef USE_ZITA_CONVOLVER
	Convproc conv; /* zita-convolver engine class instances */
#else
	AudioSampleBuffer out_buffer;
	Convolver conv_win_l;
	Convolver conv_win_r;
    
    //MyConvolver my_convolver_l;
    //MyConvolver my_convolver_r;
#endif

    unsigned int bufconv_pos;
    unsigned int ir_length;
    unsigned int block_length;

};

#endif // exclude file if not Binaural


#endif /* defined(__ambix_binaural__SpkConv__) */
