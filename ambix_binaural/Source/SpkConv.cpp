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

#include "SpkConv.h"

#if BINAURAL_DECODER // exclude file if not Binaural

#ifdef USE_ZITA_CONVOLVER

#if ZITA_CONVOLVER_MAJOR_VERSION != 3
#error "This version of IR requires zita-convolver 3.x.x"
#endif

#define CONVPROC_SCHEDULER_PRIORITY 0
#define CONVPROC_SCHEDULER_CLASS SCHED_FIFO
#define THREAD_SYNC_MODE true

#endif

SpkConv::SpkConv() : ir_loaded(false),

#ifndef USE_ZITA_CONVOLVER
                    out_buffer(2,512),
#endif
                    bufconv_pos(0),
                    ir_length(0),
                    block_length(512)

{
    
}

SpkConv::~SpkConv()
{
#ifdef USE_ZITA_CONVOLVER
    if (ir_loaded)
    {
        conv.stop_process();
    }
    conv.cleanup();
#endif
}

void SpkConv::unloadIr()
{
#ifdef USE_ZITA_CONVOLVER
    conv.stop_process();
    conv.cleanup();
    
#else
    conv_win_l.reset();
    conv_win_r.reset();
#endif
}

bool SpkConv::loadIr(const File& audioFile, double sampleRate, int BufferLength, float gain, float delay, int swapChannels)
{
    if (!audioFile.existsAsFile())
    {
        std::cout << "ERROR: file does not exist!!" << std::endl;
        return false;
    }
    
    block_length = BufferLength;
    
    AudioFormatManager formatManager;
    
    // this can read .wav and .aiff
    formatManager.registerBasicFormats();
    
    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    
    if (!reader) {
        std::cout << "ERROR: could not read impulse response file!" << std::endl;
    }
    
	//AudioFormatReader* reader = wavFormat.createMemoryMappedReader(audioFile);
    
    ir_length = (int)reader->lengthInSamples;
    
    if (ir_length <= 0) {
        std::cout << "wav file has zero samples" << std::endl;
        return false;
    }
    
    if (reader->numChannels != 2) {
        std::cout << "wav file has incorrect channel count: " << reader->numChannels << std::endl;
        return false;
    }
    
    
    AudioSampleBuffer HrirBuffer(2, ir_length); // create buffer
    
    
    reader->read(&HrirBuffer, 0, ir_length, 0, true, true);
    
    // std::cout << "wav file samplerate: " << reader->sampleRate << " depth: " << reader->bitsPerSample << " numSamples: " << ir_length << " max value: " << HrirBuffer.getMagnitude(0, ir_length) << std::endl;
    
    
    // resample buffer if necessary using soxr
    
    if (sampleRate != reader->sampleRate) {
        // do resampling
        
        double irate = reader->sampleRate;
        double orate = sampleRate;
        
        int newsize = (int) (orate / irate * ir_length + 0.5);
        
        // std::cout << "Changing samplerate from " << reader->sampleRate  << " to " << sampleRate << " new length: " << newsize << std::endl;
        
        // allocate resample buffer
        AudioSampleBuffer ResampledBuffer(2, newsize);
        ResampledBuffer.clear();
        
        // do the resampling
        soxr_quality_spec_t const q_spec = soxr_quality_spec(SOXR_HQ, 0);
        size_t odone;
        soxr_error_t error;
        
        error = soxr_oneshot(irate, orate, 1, /* Rates and # of chans. */
                                          HrirBuffer.getSampleData(0), ir_length, NULL,         /* Input. */
                                          ResampledBuffer.getSampleData(0), newsize, &odone,    /* Output. */
                                          NULL, // soxr_io_spec_t
                                          &q_spec, // soxr_quality_spec_t
                                          NULL); // soxr_runtime_spec_t
        
        error = soxr_oneshot(irate, orate, 1, /* Rates and # of chans. */
                             HrirBuffer.getSampleData(1), ir_length, NULL,         /* Input. */
                             ResampledBuffer.getSampleData(1), newsize, &odone,    /* Output. */
                             NULL, // soxr_io_spec_t
                             &q_spec, // soxr_quality_spec_t
                             NULL); // soxr_runtime_spec_t
        
        
        // new length and reset buffer
        ir_length = ResampledBuffer.getNumSamples();
        HrirBuffer = ResampledBuffer;
        // std::cout << "Resampled! New HRIR size: " << ir_length << std::endl;
        
    }
    
    // compute delay in samples
    int delay_samples = (int) (sampleRate * (delay/1000.f));
    
    // scale ir with gain
    HrirBuffer.applyGain(gain);
    
	// index for left/right
	int ch_l = 0;
    int ch_r = 1;
    // swap left and right channel if required
    if (swapChannels)
    {
        ch_l = 1;
        ch_r = 0;
    }

#ifdef USE_ZITA_CONVOLVER
	conv.set_density(0.5f); // should be the fraction of input / output

    int bufsize = block_length; // was uint32_t
    if (bufsize < Convproc::MINPART)
    {
        bufsize = Convproc::MINPART;
    }
    
    int ret = conv.configure( 1, // n_inputs
                              2, // n_outputs
                              ir_length+delay_samples,
                              block_length,
                              bufsize,
                              Convproc::MAXPART);
    if (ret != 0) {
		fprintf(stderr, "IR: can't initialise zita-convolver engine, Convproc::configure returned %d\n", ret);
        // return false;
    }
    

    // send hrir to zita-convolver
    conv.impdata_create(0, 0, 1, HrirBuffer.getSampleData(ch_l),
                         delay_samples, ir_length+delay_samples);
    
    conv.impdata_create(0, 1, 1, HrirBuffer.getSampleData(ch_r),
                         delay_samples, ir_length+delay_samples);
    
    
    conv.start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);
    
#else
	// copy buffer with delay -> ugly but zita convolver offers more easy solution
	
	AudioSampleBuffer HrirBufferDelay(2, ir_length+delay_samples); // create buffer
	HrirBufferDelay.clear();
	HrirBufferDelay.copyFrom(0,delay_samples,HrirBuffer,0,0,ir_length);
	HrirBufferDelay.copyFrom(1,delay_samples,HrirBuffer,1,0,ir_length);

	int headBlockSize = block_length;
	int tailBlockSize = std::max(4096, nextPowerOfTwo(ir_length+delay_samples));
	conv_win_l.init(headBlockSize, tailBlockSize, HrirBufferDelay.getSampleData(ch_l), ir_length+delay_samples);
	conv_win_r.init(headBlockSize, tailBlockSize, HrirBufferDelay.getSampleData(ch_r), ir_length+delay_samples);
    
#endif
    // delete the audio file reader
    delete reader;
    
    ir_loaded = true;
    return true;
}

double SpkConv::irLength()
{
    if (ir_loaded) {
        double length = (double)ir_length / sample_rate;
        
        return length;
    } else {
        return 0.f;
    }
        
}

void SpkConv::process(AudioSampleBuffer& InputBuffer, AudioSampleBuffer& OutputBuffer, int InCh)
{
    if (ir_loaded) {
        
        //conv.print();
        
        int NumSamples = InputBuffer.getNumSamples();
        
#ifdef USE_ZITA_CONVOLVER

        // convolver samples
        float* indata = conv.inpdata(0);
        float* outdata_l = conv.outdata(0);
        float* outdata_r = conv.outdata(1);
        
        
        // processor samples
        float* in_buffer = InputBuffer.getSampleData(InCh);
        
        unsigned int bcp = bufconv_pos;
        
        unsigned int bcp_out = bufconv_pos;
        
        for (int i=0; i < NumSamples; i++) {
            indata[bcp] = (*in_buffer++);
            
            
            if (++bcp == block_length) {
                bcp = 0;
                conv.process(THREAD_SYNC_MODE);
            }
        }
        
        bufconv_pos = bcp;
        
        // copy buffer to output
        OutputBuffer.addFrom(0, 0, outdata_l+bcp_out, NumSamples);
        OutputBuffer.addFrom(1, 0, outdata_r+bcp_out, NumSamples);

#else
		out_buffer.setSize(2,NumSamples,false,false,true);
		out_buffer.clear();

		conv_win_l.process(InputBuffer.getSampleData(InCh), out_buffer.getSampleData(0), NumSamples);
		conv_win_r.process(InputBuffer.getSampleData(InCh), out_buffer.getSampleData(1), NumSamples);

		
        // copy buffer to output
        OutputBuffer.addFrom(0, 0, out_buffer.getSampleData(0), NumSamples);
        OutputBuffer.addFrom(1, 0, out_buffer.getSampleData(1), NumSamples);
#endif
        
    }
    
}

#endif // exclude file if not Binaural
