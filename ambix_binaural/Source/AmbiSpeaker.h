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

#ifndef __ambix_binaural__AmbiSpeaker__
#define __ambix_binaural__AmbiSpeaker__

#include "JuceHeader.h"

#include "MyMeterDsp/MyMeterDsp.h"

class AmbiSpeaker
{
public:
    AmbiSpeaker (double SampleRate, int BufSize, float gainfactor);
    ~AmbiSpeaker();
    
    void process(AudioSampleBuffer& InputBuffer, AudioSampleBuffer& OutputBuffer, int out_ch);
    void setDecoderRow(Array<float>& Row);
    
    void setGainFactor(float gainfactor);
    
    float getPeak();
    float getRMS();
    
    // AudioSampleBuffer OutputBuffer; // holds the speaker output
    
private:
    Array<float> DecoderRow_orig; // row of the decoder matrix belonging to this speaker (without gainfactor)
    Array<float> DecoderRow; // includes gain factor
    Array<float> DecoderRow_; // used for gain interpolation
    
    float gain;
    bool newgainvalue; // this flag is true if new gain value arrived -> do interpolation
    
    MyMeterDsp _my_meter_dsp;

};

#endif /* defined(__ambix_binaural__AmbiSpeaker__) */
