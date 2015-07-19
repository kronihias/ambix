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

#include "AmbiSpeaker.h"


AmbiSpeaker::AmbiSpeaker(double SampleRate, int BufSize)
{
    
    if (SampleRate <= 0.f)
        SampleRate = 44100.;
    
    if (BufSize <= 0)
        BufSize = 256;
    
    // init meter
    _my_meter_dsp.setAudioParams((int)SampleRate, (int)BufSize);
    
    // meter peak hold in [s] and peak fall in [dB/s]
    _my_meter_dsp.setParams(0.5f, 15.0f);
    
}

AmbiSpeaker::~AmbiSpeaker()
{
    
}


void AmbiSpeaker::process(AudioSampleBuffer& InputBuffer, AudioSampleBuffer& OutputBuffer, int out_ch)
{
    int NumSamples = InputBuffer.getNumSamples();
    int NumChannels = InputBuffer.getNumChannels();

    /*
    if (NumSamples > OutputBuffer.getNumSamples())
    {
        OutputBuffer.setSize(1, NumSamples); //resize output buffer if necessary
    }
    */
    
    // add channels according decoder row
    for (int i=0; i < std::min(DecoderRow.size(), NumChannels) ; i++) {
        
        if (DecoderRow[i] != 0.f) {
            
            // add to output buffer with gainfactor from decoder row
            OutputBuffer.addFrom(out_ch, 0, InputBuffer, i, 0, NumSamples, DecoderRow[i]);
            
        }
        
    }
    
    // compute and store peak value
    
    _my_meter_dsp.calc((float*)OutputBuffer.getReadPointer(out_ch), NumSamples);
    
}


void AmbiSpeaker::setDecoderRow(Array<float>& Row)
{
    if (Row.size() > 0) {
        
        DecoderRow = Row; // copy the array
        
        /*
        std::cout << "decoder row: ";
        
        for (int i=0; i< DecoderRow.size(); i++) {
            
            std::cout << DecoderRow[i] << " ";
            
        }
        std::cout << std::endl;
         */
    }
    
    
}

float AmbiSpeaker::getPeak()
{
    return _my_meter_dsp.getPeak();
}

float AmbiSpeaker::getRMS()
{
    return _my_meter_dsp.getRMS();
}