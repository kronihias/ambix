/*
 ==============================================================================
 
 Copyright (c) 2013/2015 - Matthias Kronlachner
 www.matthiaskronlachner.com
 
 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)
 
 Details of these licenses can be found at: www.gnu.org/licenses
 
 mcfx is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
 ==============================================================================
 */

#ifndef __mcfx_convolver__MtxConv__
#define __mcfx_convolver__MtxConv__

#include "JuceHeader.h"

#if JUCE_MAC
    #include <Accelerate/Accelerate.h>
    #include <vector>

    #define SPLIT_COMPLEX 1
#else
    #include <fftw3.h>
    const int fftwopt = FFTW_MEASURE; // FFTW_ESTIMATE || FFTW_MEASURE

    #define SPLIT_COMPLEX 0
#endif


// #if JUCE_USE_SSE_INTRINSICS
#if JUCE_WINDOWS
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif
// #endif


// Aligned Memory Allocation and Deallocation

inline void* aligned_malloc(size_t size, size_t align) {
    void *result;
#ifdef _MSC_VER
    result = _aligned_malloc(size, align);
#else
    if(posix_memalign(&result, align, size)) result = 0;
#endif
    return result;
}

inline void aligned_free(void *ptr) {
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    free(ptr);
#endif
    
}


////////////////////////////
// class which holds frequency domain data of input signal
// Frequency Domain Delay Line (FDL)
class InNode
{
public:
    
    InNode(int in, int numpartitions, int partitionsize); // allocates memory
    
    ~InNode(); // deallocates memory
    
private:
    
    friend class MtxConvSlave;
    
    int                 in_; // input channel assignment
    int                 numpartitions_; // number of partitions (length of FDL)
    
    // complex fft data
#if SPLIT_COMPLEX
    float               **a_re_; // N/2+1
    float               **a_im_; // N/2+1
#else
    fftwf_complex       **a_c_; // N+1 -> interleaved complex data
#endif
    
};

////////////////////////////
// class which holds frequency domain data of filter (x numpartitions_)
// Frequency Domain Delay Line (FDL)
class FilterNode
{
public:
    
    FilterNode(InNode *innode, int numpartitions, int partitionsize); // allocates memory
    
    ~FilterNode(); // deallocates memory
    
private:
    
    friend class MtxConvSlave;
    
    InNode              *innode_; // assigned input node
    
    int numpartitions_; // number of partitions (length of FDL)
    
    // complex fft data
#if SPLIT_COMPLEX
    float               **b_re_; // N/2+1
    float               **b_im_; // N/2+1
#else
    fftwf_complex       **b_c_; // N+1 -> interleaved complex data
#endif
    
};

////////////////////////////
// class which holds time domain output data
class OutNode
{
public:
    
    OutNode(int out, int partitionsize) // resize buffer
    {
        out_ = out;
        outbuf_.setSize(2, partitionsize);
        outbuf_.clear();
    };
    
    ~OutNode()
    {
        filternodes_.clear();
    };
    
private:
    
    friend class MtxConvSlave;
    
    int                 out_; // output channel assignement
    
    Array<FilterNode*>  filternodes_; // a list of all assigned filternodes
    
    AudioSampleBuffer   outbuf_; // output samples, 2 channels for access
};




///////////////////////////
// this is the class which does the actual processing
class MtxConvSlave : public Thread
{
public:
    MtxConvSlave () : Thread("mtx_convolver_slave")
    {
        
    };
    
    ~MtxConvSlave () {};
    
private:
    friend class MtxConvMaster;
    
    
    // threadfunct
    void run ();
    
    // this is called by the thread to perform convolution
    void Process ();
    
    // this is called to get the time domain output signal
    void ReadOutput(int numsamples);
    
    // start/stop thread
    void StartProc();
    void StopProc();
    
    bool Configure ( int partitionsize,
                     int numpartitions,
                     int offset,
                     int priority,
                     AudioSampleBuffer *inbuf,
                     AudioSampleBuffer *outbuf );
    
    void SetBufsize ( int bufsize, int blocksize );
    
    bool AddFilter ( int in,
                     int out,
                     const AudioSampleBuffer& data);
    
    int CheckInNode (int in, bool create); // return InNode ID if found, otherwise returns id of newly created or -1 if not available
    int CheckOutNode (int out, bool create); // return OutNode ID if found, otherwise returns id of newly created or -1 if not available
    
    // Set all Input/Output Buffers to Zero
    void Reset ();
    
    // Destroy all allocated Memory....
    void Cleanup ();
    
    // print debug info
    void DebugInfo();
    
    AudioSampleBuffer   *inbuf_;            // Shared Input Buffer
    AudioSampleBuffer   *outbuf_;           // Shared Output Buffer
    int                 bufsize_;           // size of the input/output buffer
    int                 inoffset_;          // current input ring buffer offset
    int                 outoffset_;         // current output ring buffer offset (of shared output buf)
    
    int                 numnewinsamples_;   // number of new samples in the input buffer since last process
    int                 outnodeoffset_;     // offset in the outnode buffer
    
    bool                pingpong_;         // ping pong for output buffer to allow readout without threading problems
    
    int                 part_idx_;          // partition index for Frequency Domain Delay Line
    
    int                 numpartitions_;     // number of partitions within this level (-> with same size)
    int                 partitionsize_;     // size of the partition (fft will be 2x this size!)
    int                 offset_;            // offset in the impulse response for the first partition of this size
    int                 priority_;          // thread priority... short partitions have to deliver first!
    
    WaitableEvent       waitnewdata_;       // this will signal the threads that new work is here!
    WaitableEvent       waitprocessing_;    // this will signal work is done.
    
    float               *fft_t_;             // time data for fft/ifft -> 2*N
    float               fft_norm_;           // normalization for fft
    
#if SPLIT_COMPLEX
    FFTSetup            vdsp_fft_setup_;     // Apple vDSP FFT plan
    DSPSplitComplex     splitcomplex_;
    
    float               *fft_re_;            // N+1
    float               *fft_im_;            // N+1
    int                 vdsp_log2_;      // vDSP needs the exponent of two
#else
    fftwf_plan          fftwf_plan_r2c_;     // FFTWF forward plan
    fftwf_plan          fftwf_plan_c2r_;     // FFTWF inverse plan
    float               *fftwf_t_data_;      // FFTWF buffer for time domain signal (2*N)
    fftwf_complex       *fft_c_;             // FFTWF buffer for complex signal (N+1)
#endif
    
    OwnedArray<InNode> innodes_;            // holds input nodes
    OwnedArray<FilterNode> filternodes_;    // holds filter nodes
    OwnedArray<OutNode> outnodes_;          // holds output nodes
    
};


///////////////////////////
// this is the class which manages the processing

class MtxConvMaster
{
    
    
public:
    
    friend class MtxConvSlave;
    
    
    MtxConvMaster();
    ~MtxConvMaster();
    
    
    // Configure the Convolution Machine with fixed In/Out Configuration,
    // Blocksize and Max Impulse Response length
    bool Configure ( int numins,
                     int numouts,
                     int blocksize,
                     int maxsize,
                     int maxpart);
    
    // Add an Impulse Response with dedicated Input/Output assignement
    bool AddFilter ( int in,
                     int out,
                     const AudioSampleBuffer& data );
    
    // Start the Processing
    void StartProc();
    
    // Stop the Processing
    void StopProc();
    
    // Do the processing
    void processBlock(AudioSampleBuffer& inbuf, juce::AudioSampleBuffer &outbuf);
    
    
    // Set all Input/Output Buffers to Zero
    void Reset ();
    
    
    // Remove all Filter and Set Buffers to zero
    void Cleanup ();
    
    // print debug info
    void DebugInfo();
    
private:
    
    AudioSampleBuffer   inbuf_;             // Holds the Time Domain Input Samples
    AudioSampleBuffer   outbuf_;            // Hold the Time Domain Output Samples
    
    int                 bufsize_;           // size of input/output buffer (2*maxsize)
    
    int                 inoffset_;          // current ring buffer write offset
    int                 outoffset_;         // current ring buffer read offset
    
    int                 blocksize_;         // Blocksize of first segment
    
    int                 numins_;            // Number of Input Channels
    int                 numouts_;           // Number of Output Channels
    
    int                 numpartitions_;     // Number of Partition Levels (different blocklengths)
    
    int                 maxsize_;           // maximum filter length
    
    bool                isprocessing_;      // true = Processing is active
    
    bool                configuration_;     // isconfigured
    
    CriticalSection     lock_;              // lock critical section
    
    OwnedArray<MtxConvSlave>    partitions_;// these are my partitions with different size
    
};


#endif /* defined(__mcfx_convolver__MtxConv__) */
