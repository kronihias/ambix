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

#include "MtxConv.h"

// #define DEBUG_COUT 1

MtxConvMaster::MtxConvMaster() : inbuf_(1,256),
                                 outbuf_(1, 256),
                                 bufsize_(512),
                                 inoffset_(0),
                                 outoffset_(0),
                                 blocksize_(256),
                                 numins_(0),
                                 numouts_(0),
                                 numpartitions_(0),
                                 isprocessing_(false),
                                 configuration_(false)
{
    
}

MtxConvMaster::~MtxConvMaster()
{
}

void MtxConvMaster::processBlock(juce::AudioSampleBuffer &inbuf, juce::AudioSampleBuffer &outbuf)
{
    
    if (configuration_)
    {
        
        // lock configuration
        const ScopedLock myScopedLock (lock_);
        
        /////////////////////////
        // write to input ring buffer
        int smplstowrite = inbuf.getNumSamples();
        int numchannels = jmin(numins_, inbuf.getNumChannels());
        
#ifdef DEBUG_COUT
        std::cout << "master process block: " << smplstowrite << " inoffset: " << inoffset_ << " outoffset: " << outoffset_ << std::endl;
#endif
        
        int smplstowrite_end = smplstowrite; // write to the end
        int smplstowrite_start = 0; // write to the start
        
        if (inoffset_ + smplstowrite >= bufsize_)
        {
            smplstowrite_end = bufsize_ - inoffset_;
            smplstowrite_start = smplstowrite - smplstowrite_end;
        }
        
        if (smplstowrite_end > 0)
        {
            for (int chan=0; chan < numchannels; chan++)
            {
                inbuf_.copyFrom(chan, inoffset_, inbuf, chan, 0, smplstowrite_end);
            }
            inoffset_ += smplstowrite_end;
        }
        
        if (smplstowrite_start > 0)
        {
            for (int chan=0; chan < numchannels; chan++)
            {
                inbuf_.copyFrom(chan, 0, inbuf, chan, smplstowrite_end, smplstowrite_start);
            }
            inoffset_ = smplstowrite_start;
        }
        
        if (inoffset_ >= bufsize_)
            inoffset_ -= bufsize_;
        
        
        /////////////////////////
        // Process partitions in callback....
        
        /*
        // std::cout << "process partition" << std::endl;
        for (int i=0; i < numpartitions_; i++) {
            
            partitions_.getUnchecked(i)->Process();
            
        }
        */
        
        
        // std::cout << "processed partition! JUHU" << std::endl;
        
        
        /////////////////////////
        // collect the output signals from each partitionsize
        
        for (int i=0; i < numpartitions_; i++) {
            
            partitions_.getUnchecked(i)->ReadOutput(smplstowrite);
        }
        // std::cout << "READ OUTPUT! JUHU" << std::endl;
        
        // FINISH!!!
        // WHEN TO READ/WRITE??!
        
        /////////////////////////
        // write to output buffer
        
        outbuf.clear();
        
        int smplstoread = outbuf.getNumSamples();
        numchannels = jmin(outbuf.getNumChannels(), numouts_);
        
        int smplstoread_end = smplstoread;
        int smplstoread_start = 0;
        
        if (outoffset_ + smplstoread >= bufsize_)
        {
            smplstowrite_end = bufsize_ - outoffset_;
            smplstoread_start = smplstoread - smplstoread_end;
        }
        
        if (smplstoread_end > 0)
        {
            for (int chan=0; chan < numchannels; chan++)
            {
                outbuf.copyFrom(chan, 0, outbuf_, chan, outoffset_, smplstowrite_end);
            }
            outbuf_.clear(outoffset_, smplstowrite_end); // clear the output buffer
            
            outoffset_ += smplstowrite_end;
        }
        
        if (smplstoread_start > 0)
        {
            for (int chan=0; chan < numchannels; chan++)
            {
                outbuf.copyFrom(chan, smplstowrite_end, outbuf_, chan, outoffset_, smplstoread_start);
            }
            outbuf_.clear(outoffset_, smplstoread_start); // clear the output buffer
            
            outoffset_ = smplstoread_start;
        }
        
        if (outoffset_ >= bufsize_)
            outoffset_ -= bufsize_;
        
    } // configured true
    else
    {
        outbuf.clear();
    }
    
}

bool MtxConvMaster::Configure(int numins, int numouts, int blocksize, int maxsize, int maxpart)
{
    if (!numins || !numouts || !blocksize || configuration_)
        return false;
    
    if (maxpart < blocksize)
        maxsize = blocksize;
    
    numins_ = numins;
    numouts_ = numouts;
    
    
    blocksize_ = nextPowerOfTwo(blocksize);
    
    // std::cout << "Blocksize: " << blocksize << " new Block: " << blocksize_ << " maxsize: " << maxsize << std::endl;
    
    
    maxsize_ = 0;
    
#if 0
    // for now use uniform partitioning and no multithreading
    partitions_.add(new MtxConvSlave());
    
    int numpartitions = ceil((float)maxsize/blocksize_);
    
    partitions_.getLast()->Configure(blocksize_, numpartitions, 0, 0, &inbuf_, &outbuf_); // for now no offset and zero priority
    
    numpartitions_ = 1;
    
    maxsize_ = maxsize;
    
#else
    
    
    // gardener scheme -> n, n, 2n 2n, 4n 4n, 8n 8n, ....
    
    // try bit different...
    
    blocksize = blocksize_;
    numpartitions_=0;
    int priority = 0;
    int offset = 0;
    
    
    while (maxsize > 0) {
        
        int numpartitions = 4;
        
        numpartitions_++;
        partitions_.add(new MtxConvSlave());
        
        
        if (blocksize >= maxpart)
        {
            // max partition size reached -> put rest into this partition...
            numpartitions = (int)ceilf((float)maxsize/(float)blocksize);
            
        } else {
            
            numpartitions = jmin(numpartitions, (int)ceilf((float)maxsize/(float)blocksize));
            
        }
        
        partitions_.getLast()->Configure(blocksize, numpartitions, offset, priority, &inbuf_, &outbuf_);
        
        maxsize_ += numpartitions*blocksize;
        
        offset += numpartitions*blocksize;
        maxsize -= numpartitions*blocksize;
        
        priority--;
        
        blocksize *= 2;
        
    }
#endif
    
    // resize the in/out buffers
    bufsize_ = jmax(2*maxsize_, blocksize_);
    
    inbuf_.setSize(numins_, bufsize_);
    outbuf_.setSize(numouts_, bufsize_);
    
    inbuf_.clear();
    outbuf_.clear();
    
    
    
    
    // set the actual buffersize and compute correct offsets!
    for (int i=0; i < numpartitions_; i++) {
        MtxConvSlave *partition = partitions_.getUnchecked(i);
        partition->SetBufsize(bufsize_, blocksize_);
    }
    
    
    // print debug info
    DebugInfo();
    
    configuration_ = true;
    
    return true;
}

void MtxConvMaster::StartProc()
{
    for (int i=0; i < numpartitions_; i++) {
        MtxConvSlave *partition = partitions_.getUnchecked(i);
        partition->StartProc();
    }
}

void MtxConvMaster::StopProc()
{
    for (int i=0; i < numpartitions_; i++) {
        MtxConvSlave *partition = partitions_.getUnchecked(i);
        partition->StopProc();
    }
}

void MtxConvMaster::Cleanup()
{
    configuration_ = false;
    
    // lock configuration
    const ScopedLock myScopedLock (lock_);
    
    for (int i=0; i < partitions_.size(); i++)
    {
        partitions_.getUnchecked(i)->Cleanup();
    }
    
    partitions_.clear();
    
    inoffset_ = 0;
    outoffset_ = 0;
    numins_ = 0;
    numouts_ = 0;
    numpartitions_ = 0;
    
}

void MtxConvMaster::Reset()
{
    inbuf_.clear();
    outbuf_.clear();
    for (int i=0; i < partitions_.size(); i++)
    {
        partitions_.getUnchecked(i)->Reset();
    }
}

bool MtxConvMaster::AddFilter(int in, int out, const juce::AudioSampleBuffer &data)
{
    for (int i=0; i < partitions_.size(); i++) {
        partitions_.getUnchecked(i)->AddFilter(in, out, data);
    }
    
    return true;
}

void MtxConvMaster::DebugInfo()
{
    std::cout << "Blocksize: " << blocksize_ << " Partitions: " << numpartitions_<< " Maxsize: " << maxsize_  << " Bufsize: " << bufsize_ << std::endl;
    
    for (int i=0; i < partitions_.size(); i++) {
        partitions_.getUnchecked(i)->DebugInfo();
    }
}

//////////////////////////////////////
// SLAVE METHODS
//////////////////////////////////////
bool MtxConvSlave::Configure(int partitionsize, int numpartitions, int offset, int priority, AudioSampleBuffer *inbuf, AudioSampleBuffer *outbuf)
{
    partitionsize_ = partitionsize;
    numpartitions_ = numpartitions;
    offset_ = offset;
    priority_ = priority;
    
    inbuf_ = inbuf;
    outbuf_ = outbuf;
    
    numnewinsamples_ = 0;
    outnodeoffset_ = 0;
    
    pingpong_ = 0;
    
    part_idx_ = 0;
    
#if SPLIT_COMPLEX
    fft_norm_ = 0.5f / ( 2.f*(float)partitionsize_ ); // vDSP has a factor 2 different scaling
    
#else
    fft_norm_ = 1.f / ( 2.f*(float)partitionsize_ );
#endif
    
    // prepare FFT data and plans
    fft_t_ = reinterpret_cast<float*>( aligned_malloc( 2*partitionsize*sizeof(float), 16 ) ); // the same for all implementations
    
#if SPLIT_COMPLEX
    
    fft_re_ = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
    fft_im_ = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
    
    splitcomplex_.realp = fft_re_;
    splitcomplex_.imagp = fft_im_;
    
    vdsp_log2_ = 0;
    while ((1 << vdsp_log2_) < 2*partitionsize_)
    {
        ++vdsp_log2_; // N=2^vdsp_log2_
    }
    
    vdsp_fft_setup_ = vDSP_create_fftsetup(vdsp_log2_, FFT_RADIX2);
    
#else
    
    fft_c_ = reinterpret_cast<fftwf_complex*>( aligned_malloc( (partitionsize+1)*sizeof(fftwf_complex), 16 ) );
    
    fftwf_plan_r2c_ = fftwf_plan_dft_r2c_1d (2*partitionsize_, fft_t_, fft_c_, fftwopt);
    fftwf_plan_c2r_ = fftwf_plan_dft_c2r_1d (2*partitionsize_, fft_c_, fft_t_, fftwopt);
    
#endif
    
    waitnewdata_.reset();
    waitprocessing_.reset();
    
    return true;
}

void MtxConvSlave::SetBufsize(int bufsize, int blocksize)
{
    bufsize_ = bufsize;

#ifdef DEBUG_COUT
    std::cout << "Slave::SETBUFSIZE: " << bufsize_ << "offset_: " << offset_ << std::endl;
#endif
    
    inoffset_ = bufsize_ - partitionsize_ + 1; // offset due to overlap/save
    
    outoffset_ = offset_ - partitionsize_ + blocksize;
}


void MtxConvSlave::StartProc()
{
    int priority = 8 + priority_;
    priority = jmax(priority,0);
    
    // don't start thread for first partitionsize -> perform inside the callback!
    if (priority_ < 0)
        startThread(priority); // priority is negative... juce: 10 is highest...0
}


void MtxConvSlave::StopProc()
{
    signalThreadShouldExit();
    
    waitnewdata_.signal();
    
    stopThread(20);
    
    // waitForThreadToExit(20);
}


void MtxConvSlave::Cleanup()
{
    
    // delete all fft plans and memory...
#if SPLIT_COMPLEX
    if (vdsp_fft_setup_)
        vDSP_destroy_fftsetup(vdsp_fft_setup_);
    
    aligned_free(fft_re_);
    aligned_free(fft_im_);
    
#else
    if (fftwf_plan_r2c_)
        fftwf_destroy_plan(fftwf_plan_r2c_);
    if (fftwf_plan_c2r_)
        fftwf_destroy_plan(fftwf_plan_c2r_);
    
    aligned_free(fft_c_);
#endif
    
    aligned_free(fft_t_);
    
    innodes_.clear();
    outnodes_.clear();
    filternodes_.clear();
    
    inoffset_ = 0;
    outoffset_ = 0;
}

void MtxConvSlave::Reset()
{
    // clear all stored input data
    int numinch = innodes_.size();
    
    for (int i=0; i < numinch; i++) {
        InNode *innode = innodes_.getUnchecked(i);
        

        for (int j=0; j < numpartitions_; j++) {
#if SPLIT_COMPLEX
            FloatVectorOperations::clear(innode->a_re_[j], partitionsize_+1);
            FloatVectorOperations::clear(innode->a_im_[j], partitionsize_+1);
#else
            FloatVectorOperations::clear((float*)innode->a_c_[j], 2*(partitionsize_+1));
#endif
        }
    }
    
    
    // clear all stored output data
    int numouts = outnodes_.size();
    
    for (int i=0; i < numouts; i++) {
        OutNode *outnode = outnodes_.getUnchecked(i);
        
        outnode->outbuf_.clear();
        
    }
    
    // numnewinsamples_ = 0;
    
    // outnodeoffset_ = 0;
}

bool MtxConvSlave::AddFilter(int in, int out, const juce::AudioSampleBuffer &data)
{
    
    // this is the number of samples we use from the filter....
    int num_samples = data.getNumSamples() - offset_;
    
    if (num_samples > 0)
    {
        // check if the filter is zero -> if so: don't add it and save cpu time (for spare matrices maybe useful..)
        if (data.getRMSLevel(0, offset_, num_samples) == 0.f)
            return false;
        
        
        // check if we already have an input node
        InNode *innode = innodes_.getUnchecked( CheckInNode(in, true) );
        
        // check if we already have an output node
        OutNode *outnode = outnodes_.getUnchecked( CheckOutNode(out, true) );
        
        
        // this is <= the number of partitions
        int filter_parts = jmin(numpartitions_, (int)ceilf((float)num_samples/(float)partitionsize_));
        
        
        // add a new filter with the necessary partitions
        filternodes_.add(new FilterNode(innode, filter_parts, partitionsize_));
        
        FilterNode *filternode = filternodes_.getLast(); // the new filternode
        
        
        // add filter to the list in specific output node
        outnode->filternodes_.add(filternode);
        
        // iterate over all subpartitions
        for (int i=0; i < filter_parts; i++)
        {
            // prepare time data
            FloatVectorOperations::clear(fft_t_, 2*partitionsize_);
            
            int num_cpy_smpls = jmin(partitionsize_, num_samples-i*partitionsize_);
            
            // copy input samples and scale for normalization
            FloatVectorOperations::copyWithMultiply(fft_t_, data.getReadPointer(0,offset_+i*partitionsize_), fft_norm_, num_cpy_smpls);
            
            // perform fft of filter part
#if SPLIT_COMPLEX
            
            // use the filternodes buffers directly...
            DSPSplitComplex splitcomplex;
            splitcomplex.realp = filternode->b_re_[i];
            splitcomplex.imagp = filternode->b_im_[i];
            
            vDSP_ctoz(reinterpret_cast<const COMPLEX*>(fft_t_), 2, &splitcomplex, 1, partitionsize_);
            vDSP_fft_zrip(vdsp_fft_setup_, &splitcomplex, 1, vdsp_log2_, FFT_FORWARD);
            
            // unpack
            // fft_re_[partitionsize_] = fft_im_[0];
            // fft_im_[0] = 0.0f;
            // fft_im_[partitionsize_] = 0.0f; // not necessary-> we wont use this value anyway
            
            filternode->b_re_[i][partitionsize_] = filternode->b_im_[i][0];
            filternode->b_im_[i][0] = 0.f;
            filternode->b_im_[i][partitionsize_] = 0.f;
            
            
            
            // copy the fft filter data to the filternode
            // FloatVectorOperations::copy(filternode->b_re_[i], fft_re_, partitionsize_+1);
            // FloatVectorOperations::copy(filternode->b_im_[i], fft_im_, partitionsize_+1);
            
#else
            fftwf_execute_dft_r2c (fftwf_plan_r2c_, fft_t_, filternode->b_c_[i]);
            
            // copy the fft filter data to the filternode
            // FloatVectorOperations::copy((float*)filternode->b_c_[i], (float*)fft_c_, 2*(partitionsize_+1));
#endif
        }
        
        
        return true;
    }
    
    else // no samples left for this partition
        return false;
    
}


void MtxConvSlave::run()
{
    while (true)
    {
        waitnewdata_.wait();
        
        waitnewdata_.reset();
        
        if ( threadShouldExit() )
            return;
        
        Process();
        
        waitprocessing_.signal();
    }
}


void MtxConvSlave::Process()
{
    
#ifdef DEBUG_COUT
    std::cout << "Slave Process, numnewsamples: " << numnewinsamples_ << " partsize: " << partitionsize_ << " inoffset: " << inoffset_ << std::endl;
#endif
    
    bool pingpong = !pingpong_;
    
    ////////////////
    // do all the inputs
    // copy and transform new time data for each needed input channel
    
    int smplstoread_end = 2*partitionsize_; // write to the end
    int smplstoread_start = 0; // write to the start
    
    if (inoffset_+smplstoread_end >= bufsize_)
    {
        smplstoread_end = bufsize_ - inoffset_;
        smplstoread_start = 2*partitionsize_ - smplstoread_end;
    }
    
    int numinch = innodes_.size();
    
    for (int i=0; i < numinch; i++) {
        InNode *innode = innodes_.getUnchecked(i);
        
        int chan = innode->in_;
        if (smplstoread_end)
            FloatVectorOperations::copy(fft_t_, inbuf_->getReadPointer(chan, inoffset_), smplstoread_end);
        
        if (smplstoread_start)
            FloatVectorOperations::copy(fft_t_+smplstoread_end, inbuf_->getReadPointer(chan, 0), smplstoread_start);
        
        
        // std::cout << "RMS IN_FFT max: " << FloatVectorOperations::findMaximum(fft_t_, partitionsize_) << " PartIdx: " << part_idx_ << std::endl;
        
        // do the fft
#if SPLIT_COMPLEX
        DSPSplitComplex splitcomplex;
        splitcomplex.realp = innode->a_re_[part_idx_];
        splitcomplex.imagp = innode->a_im_[part_idx_];
        
        vDSP_ctoz(reinterpret_cast<const COMPLEX*>(fft_t_), 2, &splitcomplex, 1, partitionsize_);
        vDSP_fft_zrip(vdsp_fft_setup_, &splitcomplex, 1, vdsp_log2_, FFT_FORWARD);
        
        // unpack
        innode->a_re_[part_idx_][partitionsize_] = innode->a_im_[part_idx_][0];
        innode->a_im_[part_idx_][0] = 0.0f;
        // fft_im_[partitionsize_] = 0.0f; // not necessary-> we wont use this value anyway
        
        // copy the fft filter data to the input node
        // FloatVectorOperations::copy(innode->a_re_[i], fft_re_, partitionsize_+1);
        // FloatVectorOperations::copy(innode->a_im_[i], fft_im_, partitionsize_+1);
        
#else
        fftwf_execute_dft_r2c (fftwf_plan_r2c_, fft_t_, innode->a_c_[part_idx_]);
        
        // copy the fft filter data to the input node
        // FloatVectorOperations::copy((float*)innode->a_c_[part_idx_], (float*)fft_c_, 2*(partitionsize_+1));
#endif
        
    }
    
    inoffset_ += partitionsize_;
    
    if (inoffset_ >= bufsize_)
        inoffset_ = inoffset_ - bufsize_;
    
    
    /////////////
    // iterate over all outputs and query filters
    // outputs -> filters -> inputs
    
    int numouts = outnodes_.size();
    
    for (int i=0; i < numouts; i++)
    {
        
        // clear freq accumulation buffers
#if SPLIT_COMPLEX
        FloatVectorOperations::clear(fft_re_, partitionsize_+1);
        FloatVectorOperations::clear(fft_im_, partitionsize_+1);
#else
        FloatVectorOperations::clear((float*)fft_c_, 2*(partitionsize_+1));
#endif
        
        OutNode *outnode = outnodes_.getUnchecked(i);
        
        int numfilters = outnode->filternodes_.size();
        
        int part_idx = part_idx_;
        
        // iterate over all sub-partitions
        for (int p = 0; p < numpartitions_; p++)
        {
            
            // std::cout << "NumPartitions: " << numpartitions_ << " Partition: " << part_idx << std::endl;
            
            for (int j=0; j < numfilters; j++)
            {
                
                FilterNode *filternode = outnode->filternodes_.getUnchecked(j);
                
                // only process if this filter has that many partitions
                if (p < filternode->numpartitions_)
                {
                    
#if 1 // deactivate for now..
#if SPLIT_COMPLEX
                    float *a_re = filternode->innode_->a_re_[part_idx];
                    float *a_im = filternode->innode_->a_im_[part_idx];
                    
                    float *b_re = filternode->b_re_[p];
                    float *b_im = filternode->b_im_[p];
                    
                    float *c_re = fft_re_;
                    float *c_im = fft_im_;
                    
                    // this is the SSE convolution version
                    for (int k=0; k<partitionsize_; k+=4)
                    {
                        const __m128 ra = _mm_load_ps(&a_re[k]);
                        const __m128 rb = _mm_load_ps(&b_re[k]);
                        const __m128 ia = _mm_load_ps(&a_im[k]);
                        const __m128 ib = _mm_load_ps(&b_im[k]);
                        
                        // destination
                        __m128 rc = _mm_load_ps(&c_re[k]);
                        __m128 ic = _mm_load_ps(&c_im[k]);
                        
                        // real part: real = ra*rb-ia*ib
                        rc = _mm_add_ps(rc, _mm_mul_ps(ra, rb));
                        rc = _mm_sub_ps(rc, _mm_mul_ps(ia, ib));
                        _mm_store_ps(&c_re[k], rc);
                        
                        // imag part: imag = ra*ib + ia*rb
                        ic = _mm_add_ps(ic, _mm_mul_ps(ra, ib));
                        ic = _mm_add_ps(ic, _mm_mul_ps(ia, rb));
                        _mm_store_ps(&c_im[k], ic);
                    }
                    // handle last bin separately
                    c_re[partitionsize_] += a_re[partitionsize_] * b_re[partitionsize_];
                    // c_im[partitionsize_] = 0; // should be zero anyway
#else
                    // sse 3 from http://yangkunlun.blogspot.de/2011/09/fast-complex-multiply-with-sse.html
                    
                    float *A = (float *) filternode->innode_->a_c_[part_idx];
                    float *B = (float *) filternode->b_c_[p];
                    float *D = (float *) fft_c_;
                    
                    
                    for (int i=0; i < partitionsize_; i+=2)
                    {
                        // complex multiplication
                        __m128 aa, bb, dc, x0, x1, out;
                        __m128 ab = _mm_load_ps(A);
                        __m128 cd = _mm_load_ps(B);
                        aa = _mm_moveldup_ps(ab); // duplicate A to R1 R1 R2 R2
                        bb = _mm_movehdup_ps(ab); // duplicate A to I1 I1 I2 I2
                        
                        // the upper part can be done during initialization -> but double the need of space!
                        
                        x0 = _mm_mul_ps(aa, cd);    //ac ad
                        dc = _mm_shuffle_ps(cd, cd, _MM_SHUFFLE(2,3,0,1));
                        x1 = _mm_mul_ps(bb, dc);    //bd bc
                        
                        
                        // adding result to output
                        out = _mm_load_ps(D);
                        out = _mm_add_ps(out, _mm_addsub_ps(x0, x1));
                        _mm_store_ps(D, out);
                        
                        A += 4;
                        B += 4;
                        D += 4;
                    }
                    // treat last bin separately
                    fft_c_ [partitionsize_][0] += filternode->innode_->a_c_[part_idx] [partitionsize_][0] * filternode->b_c_[p] [partitionsize_][0];
                    // fft_c_ [partitionsize_][1] = 0; // should be zero anyway
#endif
#endif
                    
#if 0
                    // try to bypass the convolution
                    if (p == 0) {
#if SPLIT_COMPLEX
                        FloatVectorOperations::copyWithMultiply(fft_re_, filternode->innode_->a_re_[part_idx], fft_norm_, partitionsize_+1);
                        FloatVectorOperations::copyWithMultiply(fft_im_, filternode->innode_->a_im_[part_idx], fft_norm_, partitionsize_+1);
                        
                        //FloatVectorOperations::copy(fft_re_, filternode->b_re_[part_idx], partitionsize_+1);
                        //FloatVectorOperations::copy(fft_im_, filternode->b_im_[part_idx], partitionsize_+1);
#else
                        FloatVectorOperations::copyWithMultiply((float*)fft_c_, (float*)filternode->innode_->a_c_[part_idx], fft_norm_, 2*(partitionsize_+1));
#endif
                        
                    }
                    
#endif
                }
            }
            
            
            part_idx--;
            if (part_idx < 0)
                part_idx = numpartitions_-1;
            
        } // iterate over all sub-partitions
        // all filters are computed now do the ifft for this output channel
        
#if SPLIT_COMPLEX
        fft_im_[0] = fft_re_[partitionsize_]; // special vDSP packing
        vDSP_fft_zrip(vdsp_fft_setup_, &splitcomplex_, 1, vdsp_log2_, FFT_INVERSE); // ifft
        vDSP_ztoc(&splitcomplex_, 1, reinterpret_cast<COMPLEX*>(fft_t_), 2, partitionsize_); // reorder
#else
        fftwf_execute_dft_c2r (fftwf_plan_c2r_, fft_c_, fft_t_);
#endif
        
        outnode->outbuf_.copyFrom(pingpong, 0, fft_t_+partitionsize_-1, partitionsize_);
        
    } // end iterate over all outputs
    
    
    // FloatVectorOperations::fill(fft_t_, 0.1f, partitionsize_);
    ////////////
    // FDL
    part_idx_++;
    if (part_idx_ >= numpartitions_)
        part_idx_ = 0;
    
    pingpong_ = pingpong;
    
    outnodeoffset_ = 0;
}


void MtxConvSlave::ReadOutput(int numsamples)
{
    
    numnewinsamples_+=numsamples;
    
    // do processing if enough samples arrived
    if (numnewinsamples_ >= partitionsize_)
    {
        if (priority_ < 0) {
            
            waitprocessing_.reset();
            waitnewdata_.signal();
            waitprocessing_.wait();
            
        } else {
            
            // perform first partition in callback
            Process();
        }
        
        numnewinsamples_ -= partitionsize_;
    }
    
#ifdef DEBUG_COUT
    std::cout << "ReadOutput, outnodeoffset_: " << outnodeoffset_ << " outoffset_: " << outoffset_ << std::endl;
#endif
    
    
    //int smplstowrite_end = partitionsize_; // write to the end
    int smplstowrite_end = numsamples; // write to the end
    int smplstowrite_start = 0; // write to the start
    
    if (smplstowrite_end + outoffset_ >= bufsize_)
    {
        smplstowrite_end = bufsize_ - outoffset_;
        // smplstowrite_start = partitionsize_ - smplstowrite_end;
        smplstowrite_start = numsamples - smplstowrite_end;
    }
    
    // std::cout << "outoffset: " << outoffset_ << " end: " << smplstowrite_end << " start: " << smplstowrite_start << " pingpong: " << (int)pingpong_ << std::endl;
    
    
    int numouts = outnodes_.size();
    
    // std::cout << "numoutnodes: " << numouts << std::endl;
    
    for (int i=0; i < numouts; i++) {
        
        OutNode *outnode = outnodes_.getUnchecked(i);
        
        if (smplstowrite_end)
            outbuf_->addFrom(outnode->out_, outoffset_, outnode->outbuf_, (int)pingpong_, outnodeoffset_, smplstowrite_end);
        
        if (smplstowrite_start)
            outbuf_->addFrom(outnode->out_, 0, outnode->outbuf_, (int)pingpong_, outnodeoffset_+smplstowrite_end, smplstowrite_start);
        
        
    }
    
    // std::cout << "outbuf ch1 rms: " << outbuf_->getRMSLevel(0, 0, bufsize_) << "outbuf ch2 rms: " << outbuf_->getRMSLevel(1, 0, bufsize_) << std::endl;
    
    
    if (smplstowrite_start)
        outoffset_ = smplstowrite_start;
    else
        outoffset_ += smplstowrite_end;
    
    if (outoffset_ >= bufsize_)
        outoffset_ -= bufsize_;
    
    outnodeoffset_ += numsamples;
    
    
    // reset is done in the processing methode
    // if (outnodeoffset_ >= partitionsize_)
    //    outnodeoffset_ = 0;
    
}


int MtxConvSlave::CheckInNode(int in, bool create)
{
    int ret=-1;
    
    for (int i=0; i<innodes_.size(); i++)
    {
        if (innodes_.getUnchecked(i)->in_ == in)
            ret = i;
    }
    
    // create a node if it does not exist
    if (create && (ret == -1)) {
        innodes_.add(new InNode(in, numpartitions_, partitionsize_));
        ret = innodes_.size()-1;
    }
    
    return ret;
    
}

int MtxConvSlave::CheckOutNode(int out, bool create)
{
    int ret=-1;
    
    for (int i=0; i<outnodes_.size(); i++)
    {
        if (outnodes_.getUnchecked(i)->out_ == out)
            ret = i;
    }
    
    // create a node if it does not exist
    if (create && (ret == -1))
    {
        outnodes_.add(new OutNode(out, partitionsize_));
        ret = outnodes_.size()-1;
    }
    
    return ret;
}

void MtxConvSlave::DebugInfo()
{
    std::cout << "Partitionsize: " << partitionsize_ << " Subpartitions: " << numpartitions_ << " Offset: " << offset_ << " Inoffset: " << inoffset_ << " Priority: " << priority_ << std::endl;
}

/////////////////////////////////
// Node Constructors/Destructors

// FilterNode

FilterNode::FilterNode(InNode *innode, int numpartitions, int partitionsize)
{
    innode_ = innode;
    
    numpartitions_ = numpartitions;
    
#if SPLIT_COMPLEX
    b_re_ = new float* [numpartitions_];
    b_im_ = new float* [numpartitions_];
#else
    b_c_ = new fftwf_complex* [numpartitions_];
#endif
    
    for (int i=0; i<numpartitions_; i++)
    {
        // allocate memory in constructor
#if SPLIT_COMPLEX
        // vdsp framework needs N split complex samples
        b_re_[i] = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
        b_im_[i] = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
        
        
        FloatVectorOperations::clear(b_re_[i], partitionsize+1);
        FloatVectorOperations::clear(b_im_[i], partitionsize+1);
        
#else
        // fftw needs N+1 complex samples
        b_c_[i] = reinterpret_cast<fftwf_complex*>( aligned_malloc( (partitionsize+1)*sizeof(fftwf_complex), 16 ) );
        
        FloatVectorOperations::clear((float*)b_c_[i], 2*(partitionsize+1));
#endif
    }
    
}


FilterNode::~FilterNode()
{
    // free memory in destructor
    for (int i=0; i<numpartitions_; i++)
    {
#if SPLIT_COMPLEX
        if (b_re_[i])
            aligned_free(b_re_[i]);
        if (b_im_[i])
            aligned_free(b_im_[i]);
#else
        if (b_c_[i])
            aligned_free(b_c_[i]);
#endif
    }

#if SPLIT_COMPLEX
    delete[] b_re_;
    delete[] b_im_;
#else
    delete[] b_c_;
#endif
    
}

/////////////////////////////////
// InNode

InNode::InNode(int in, int numpartitions, int partitionsize)
{
    
    in_ = in;
    numpartitions_ = numpartitions;
    
#if SPLIT_COMPLEX
    a_re_ = new float* [numpartitions_];
    a_im_ = new float* [numpartitions_];
#else
    a_c_ = new fftwf_complex* [numpartitions_];
#endif
    
    for (int i=0; i<numpartitions_; i++)
    {
        // allocate memory in constructor
#if SPLIT_COMPLEX
        // vdsp framework needs N split complex samples
        a_re_[i] = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
        a_im_[i] = reinterpret_cast<float*>( aligned_malloc( (partitionsize+1)*sizeof(float), 16 ) );
        
        FloatVectorOperations::clear(a_re_[i], partitionsize+1);
        FloatVectorOperations::clear(a_im_[i], partitionsize+1);
#else
        // fftw needs N+1 complex samples
        a_c_[i] = reinterpret_cast<fftwf_complex*>( aligned_malloc( (partitionsize+1)*sizeof(fftwf_complex), 16 ) );
        
        FloatVectorOperations::clear((float*)a_c_[i], 2*(partitionsize+1));
        // memset(a_c_[i], 0, (partitionsize+1)*sizeof(fftwf_complex));
#endif
    }
    
}

InNode::~InNode()
{
    // free memory in destructor
    for (int i=0; i<numpartitions_; i++)
    {
#if SPLIT_COMPLEX
        if (a_re_[i])
            aligned_free(a_re_[i]);
        if (a_im_[i])
            aligned_free(a_im_[i]);
#else
        if (a_c_[i])
            aligned_free(a_c_[i]);
#endif
    }
    
#if SPLIT_COMPLEX
    delete[] a_re_;
    delete[] a_im_;
#else
    delete[] a_c_;
#endif
}

