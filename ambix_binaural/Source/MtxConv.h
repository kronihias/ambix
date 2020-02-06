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
    #define VIMAGE_H // avoid namespace clashes
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
// and the frequency domain output for each partition
class OutNode
{
public:
    
    OutNode(int out, int partitionsize, int numpartitions) // resize buffer
    {
        numpartitions_ = numpartitions;

        out_ = out;
        outbuf_.setSize(1, partitionsize);
        outbuf_.clear();
        
        // allocate complex output for each partition stage
#if SPLIT_COMPLEX
        c_re_ = new float* [numpartitions_];
        c_im_ = new float* [numpartitions_];
#else
        c_c_ = new fftwf_complex* [numpartitions_];
#endif

        for (int i=0; i<numpartitions_; i++)
        {
            // allocate memory in constructor
#if SPLIT_COMPLEX
            // vdsp framework needs N split complex samples
            c_re_[i] = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
            c_im_[i] = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
            
            
            FloatVectorOperations::clear(c_re_[i], partitionsize+1);
            FloatVectorOperations::clear(c_im_[i], partitionsize+1);
        
#else
            // fftw needs N+1 complex samples
            c_c_[i] = reinterpret_cast<fftwf_complex*>( aligned_malloc( (partitionsize+1)*sizeof(fftwf_complex), 16 ) );
            
            FloatVectorOperations::clear((float*)c_c_[i], 2*(partitionsize+1));
#endif  
        }
        
    };
    
    ~OutNode()
    {
        filternodes_.clear();
        
        // free memory in destructor
        for (int i=0; i<numpartitions_; i++)
        {
#if SPLIT_COMPLEX
            if (c_re_[i])
                aligned_free(c_re_[i]);
            if (c_im_[i])
                aligned_free(c_im_[i]);
#else
            if (c_c_[i])
                aligned_free(c_c_[i]);
#endif
        }

#if SPLIT_COMPLEX
        delete[] c_re_;
        delete[] c_im_;
#else
        delete[] c_c_;
#endif
    };
    
private:
    
    friend class MtxConvSlave;
    
    int                 out_; // output channel assignement
    
    Array<FilterNode*>  filternodes_; // a list of all assigned filternodes
    
    AudioSampleBuffer   outbuf_; // output samples, 1 channels (no concur. access problem)
    
    int                 numpartitions_;
    
        // complex fft data
#if SPLIT_COMPLEX
    float               **c_re_; // N/2+1
    float               **c_im_; // N/2+1
#else
    fftwf_complex       **c_c_; // N+1 -> interleaved complex data
#endif
};




///////////////////////////
// this is the class which does the actual processing
class MtxConvSlave : public Thread
{
public:
    MtxConvSlave ();
    
    ~MtxConvSlave ();
    
private:
    friend class MtxConvMaster;
    
    
    // threadfunct
    void run ();
    
    // this is called by the callback or thread to perform convolution of one input signal partition
    void Process (int filt_part_idx);

    // transform the new input data
    void TransformInput(bool skip);

    // transform the accumulated output
    void TransformOutput(bool skip);

    // Write the time data to the master output buffer
    void WriteToOutbuf(int numsamples, bool skip);

    // this is called to get the time domain output signal
    // returns true if all partitions finished in time, false if some have been skipped
    // if forcesync = true this will wait until all partitions are finished
    bool ReadOutput(int numsamples, bool forcesync);
    
    // start/stop thread
    void StartProc();
    void StopProc();
    
    bool Configure ( int partitionsize,
                     int numpartitions,
                     int offset,
                     int priority,
                     AudioSampleBuffer *inbuf,
                     AudioSampleBuffer *outbuf );
    
    void SetBufsize ( int inbufsize, int outbufsize, int blocksize );
    
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
    
	// write to debug file
	void WriteLog(String &text);
	
	
    AudioSampleBuffer   *inbuf_;            // Shared Input Buffer
    AudioSampleBuffer   *outbuf_;           // Shared Output Buffer
	
	int					inbufsize_;			// size of time domain input buffer (2*maxpart_)
	int                 outbufsize_;        // size of time domain output buffer (2*maxsize_)

    int                 inoffset_;          // current input ring buffer offset
    int                 outoffset_;         // current output ring buffer offset (of shared output buf)
    
    int                 numnewinsamples_;   // number of new samples in the input buffer since last process
    int                 outnodeoffset_;     // offset in the outnode buffer

    int                 part_idx_;          // partition index for Frequency Domain Delay Line

    Atomic<int>         finished_part_;     // counter how many partitions are done
    Atomic<int>         skip_cycles_;       // counter of how many cycles should be skipped on next processing cycle...

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
    
	ScopedPointer<FileOutputStream>	debug_out_; // Debug output Text File
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
                     int minpart,
                     int maxpart,
                     bool safemode=false); // will add a delay of minpart_ samples to make sure there is never a buffer underrun for hosts that dare to send partial blocks(Adobe and Steinberg)
    
    // Add an Impulse Response with dedicated Input/Output assignement
    bool AddFilter ( int in,
                     int out,
                     const AudioSampleBuffer& data );
    
    // Start the Processing
    void StartProc();
    
    // Stop the Processing
    void StopProc();
    
    // Do the processing
    // forcesync = true will make sure that all partitions are going to be finished - use this for offline rendering!
    void processBlock(AudioSampleBuffer& inbuf, juce::AudioSampleBuffer &outbuf, int numsamples, bool forcesync=true);
    
    
    // Set all Input/Output Buffers to Zero
    void Reset ();
    
    
    // Remove all Filter and Set Buffers to zero
    void Cleanup ();
    
    // print debug info
    void DebugInfo();
    
	// write to debug file
	void WriteLog(String &text);
	
    // get the number of skipped cycles due to unfinished partitions
    int getSkipCount()
    {
        return skip_count_;
    }

    int getMaxSize()
    {
        return maxsize_;
    }

private:

    AudioSampleBuffer   inbuf_;             // Holds the Time Domain Input Samples
    AudioSampleBuffer   outbuf_;            // Hold the Time Domain Output Samples

    int                 inbufsize_;         // size of time domain input buffer (2*maxpart_)
    int                 outbufsize_;        // size of time domain output buffer (2*maxsize_)

    int                 inoffset_;          // current ring buffer write offset
    int                 outoffset_;         // current ring buffer read offset

    int                 blocksize_;         // Blocksize of host process (how many samples are processed in each callback)

    int                 minpart_;           // Size of first partition
    int                 maxpart_;           // Maximum partition size

    int                 numins_;            // Number of Input Channels
    int                 numouts_;           // Number of Output Channels

    int                 numpartitions_;     // Number of Partition Levels (different blocklengths)

    int                 skip_count_;        // The number of skipped partitions

    int                 maxsize_;           // maximum filter length

    bool                isprocessing_;      // true = Processing is active

    bool                configuration_;     // isconfigured

    CriticalSection     lock_;              // lock critical section

    OwnedArray<MtxConvSlave>    partitions_;// these are my partitions with different size

	ScopedPointer<FileOutputStream>	debug_out_; // Debug output Text File
};


#endif /* defined(__mcfx_convolver__MtxConv__) */
