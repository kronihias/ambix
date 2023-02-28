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


AmbiSpeaker::AmbiSpeaker(double SampleRate, int BufSize, float gainfactor) : newgainvalue(false)
{

    if (SampleRate <= 0.f)
        SampleRate = 44100.;

    if (BufSize <= 0)
        BufSize = 256;

    gain = jlimit(0.f, 20.f, gainfactor);

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

    if (newgainvalue)
    {
        int n = DecoderRow.size();
        for (int i = 0; i < n; i++) {
            DecoderRow.set(i, DecoderRow_orig.getUnchecked(i) * gain);
        }

        // add channels according decoder row with interpolation
        for (int i=0; i < std::min(DecoderRow.size(), NumChannels) ; i++)
        {

            if (DecoderRow[i] != 0.f) {

                OutputBuffer.addFromWithRamp(out_ch, 0, InputBuffer.getReadPointer(i), NumSamples, DecoderRow_[i], DecoderRow[i]);
            }

        }

        newgainvalue = false;

    } else {

        // add channels according decoder row without interpolation
        for (int i=0; i < std::min(DecoderRow.size(), NumChannels) ; i++)
        {

            if (DecoderRow[i] != 0.f) {

                // add to output buffer with gainfactor from decoder row
                OutputBuffer.addFrom(out_ch, 0, InputBuffer, i, 0, NumSamples, DecoderRow[i]);

            }

        }

    }




    // compute and store peak value

    _my_meter_dsp.calc((float*)OutputBuffer.getReadPointer(out_ch), NumSamples);

}


void AmbiSpeaker::setDecoderRow(Array<float>& Row)
{
    if (Row.size() > 0) {

        // copy the array
        DecoderRow_orig = Row;
        DecoderRow = Row;

        int n = DecoderRow_orig.size();
        for (int i = 0; i < n; i++) {
            DecoderRow.set(i, DecoderRow_orig.getUnchecked(i) * gain);

        }
        DecoderRow_ = DecoderRow;
    }
}

void AmbiSpeaker::setGainFactor(float gainfactor)
{
    // save the old values first
    int n = DecoderRow_.size();
    for (int i = 0; i < n; i++) {
        DecoderRow_.set(i, DecoderRow_orig.getUnchecked(i) * gain);
    }

    gain = gainfactor;
    newgainvalue = true;
}

float AmbiSpeaker::getPeak()
{
    return _my_meter_dsp.getPeak();
}

float AmbiSpeaker::getRMS()
{
    return _my_meter_dsp.getRMS();
}