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

#ifndef __mcfx_convolver__ConvolverData__
#define __mcfx_convolver__ConvolverData__

#include "JuceHeader.h"


struct IR_Data {
    int InCh;
    int OutCh;
    AudioBuffer<float> Data;
};

class ConvolverData {


public:
    ConvolverData();
    ~ConvolverData();

    void clear();

    int getNumInputChannels();
    int getNumOutputChannels();

    int getNumIRs();

    // delay and length are assumed to be in samples related to the source samplerate!
    // this method does zero padding for delay and cuts the beginning/end for offset/delay
    // length is the absolute length, not taking into account the offset
    void addIR(int in_ch, int out_ch, int offset, int delay, int length, AudioBuffer<float> &buffer, int buffer_ch, double src_samplerate);

    AudioBuffer<float>* getIR(int id); // get the audio buffer for the corresponding IR

    int getInCh(int id); // get the output channel for the corresponding IR
    int getOutCh(int id); // get the input channel for the corresponding IR
    int getLength(int id);

    int getMaxLength();
    double getMaxLengthInSeconds();

    // set the target samplerate -> ir gets resampled if added with different samplingrate
    void setSampleRate(double samplerate);
private:

    int NumInCh; // number of input channels (max(InCh))
    int NumOutCh; // number of output channels (max(OutCh))

    int MaxLen; // the longest impulse response in the set

    double SampleRate; // the sample rate of the impulse responses within this object

    OwnedArray<IR_Data> IR;

};

#endif /* defined(__mcfx_convolver__ConvolverData__) */
