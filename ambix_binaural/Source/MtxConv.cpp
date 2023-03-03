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
                                 inbufsize_(512),
                                 outbufsize_(512),
                                 inoffset_(0),
                                 outoffset_(0),
                                 blocksize_(256),
                                 numins_(0),
                                 numouts_(0),
                                 numpartitions_(0),
                                 maxsize_(0),
                                 skip_count_(0),
                                 isprocessing_(false),
                                 configuration_(false),
                                 debug_out_(nullptr)
{
#ifdef DEBUG_COUT
	File file;
	String filename("MtxConvMaster.txt");
	file = file.getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile(filename);
	debug_out_ = new FileOutputStream(file);
#endif

#if !SPLIT_COMPLEX
    fftwf_make_planner_thread_safe(); // this works since fftw-3.3.6-pl2
#endif
}

MtxConvMaster::~MtxConvMaster()
{
}

void MtxConvMaster::WriteLog(String &text)
{
	if (debug_out_->openedOk())
		debug_out_->writeText(text, false, false, nullptr);
}

void MtxConvMaster::processBlock(juce::AudioSampleBuffer &inbuf, juce::AudioSampleBuffer &outbuf, int numsamples, bool forcesync)
{

    if (configuration_)
    {

        // lock configuration
        const ScopedLock myScopedLock (lock_);

        /////////////////////////
        // write to input ring buffer
        int numchannels = jmin(numins_, inbuf.getNumChannels());

#ifdef DEBUG_COUT
        String dbg_text;
        dbg_text << "master process block: " << numsamples << " inoffset: " << inoffset_ << " outoffset: " << outoffset_ << "\n";
        WriteLog(dbg_text);
#endif

        int smplstowrite_end = numsamples; // write to the end
        int smplstowrite_start = 0; // write to the start

        if (inoffset_ + numsamples >= inbufsize_)
        {
            smplstowrite_end = inbufsize_ - inoffset_;
            smplstowrite_start = numsamples - smplstowrite_end;
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

        if (inoffset_ >= inbufsize_)
            inoffset_ -= inbufsize_;

        /////////////////////////
        // collect the output signals from each partitionsize

        bool finished = true; // if false one of the partitions did not finish

        for (int i=0; i < numpartitions_; i++) {

            finished &= partitions_.getUnchecked(i)->ReadOutput(numsamples, forcesync);
        }

        if (!finished)
            skip_count_++;

        /////////////////////////
        // write to output buffer

        outbuf.clear();

        int smplstoread = numsamples;
        numchannels = jmin(outbuf.getNumChannels(), numouts_);

        int smplstoread_end = smplstoread;
        int smplstoread_start = 0;

        if (outoffset_ + smplstoread >= outbufsize_)
        {
            smplstoread_end = outbufsize_ - outoffset_;
            smplstoread_start = smplstoread - smplstoread_end;
        }

        if (smplstoread_end > 0)
        {
            for (int chan = 0; chan < numchannels; chan++)
            {
                outbuf.copyFrom(chan, 0, outbuf_, chan, outoffset_, smplstoread_end);
            }
            outbuf_.clear(outoffset_, smplstoread_end); // clear the output buffer

            outoffset_ += smplstoread_end;
        }

        if (smplstoread_start > 0)
        {
            for (int chan = 0; chan < numchannels; chan++)
            {
                outbuf.copyFrom(chan, smplstoread_end, outbuf_, chan, 0, smplstoread_start);
            }
            outbuf_.clear(0, smplstoread_start); // clear the output buffer

            outoffset_ = smplstoread_start;
        }

        if (outoffset_ >= outbufsize_)
            outoffset_ -= outbufsize_;

    } // configured true
    else
    {
        outbuf.clear();
    }

}

bool MtxConvMaster::Configure(int numins, int numouts, int blocksize, int maxsize, int minpart, int maxpart, bool safemode)
{
    if (!numins || !numouts || !blocksize || configuration_)
        return false;

	if (minpart < blocksize)
		minpart = blocksize;

    if (maxpart < blocksize)
        maxpart = blocksize;

	minpart_ = nextPowerOfTwo(minpart);
	maxpart_ = nextPowerOfTwo(maxpart);


    numins_ = numins;
    numouts_ = numouts;


    blocksize_ = nextPowerOfTwo(blocksize);

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

    int partsize = minpart_;
    numpartitions_=0;
    int priority = 0;
    int offset = 0;


    while (maxsize > 0) {

        int numpartitions = 4;

        numpartitions_++;
        partitions_.add(new MtxConvSlave());


        if (partsize >= maxpart_)
        {
            // max partition size reached -> put rest into this partition...
            numpartitions = (int)ceilf((float)maxsize/(float)partsize);

        } else {

            numpartitions = jmin(numpartitions, (int)ceilf((float)maxsize/(float)partsize));

        }

        partitions_.getLast()->Configure(partsize, numpartitions, offset, priority, &inbuf_, &outbuf_);

        maxsize_ += numpartitions*partsize;

        offset += numpartitions*partsize;
        maxsize -= numpartitions*partsize;

        priority--;

        partsize *= 2;

    }
#endif

    // resize the in/out buffers
	inbufsize_ = 4 * maxpart_;

    outbufsize_ = jmax(2*maxsize_, blocksize_);

    inbuf_.setSize(numins_, inbufsize_);
    outbuf_.setSize(numouts_, outbufsize_);

    inbuf_.clear();
    outbuf_.clear();

    // set the outoffset which will be != 0 if minpart_ > blocksize is used
    if (safemode)
        outoffset_ = -minpart_; // safe mode, has higher latency
    else
        outoffset_ = blocksize_ - minpart_;    // minimum latency mode -> might cause problems with some hosts that send varying (incomplete) blocks (eg. adobe premiere, nuendo)

    if (outoffset_ < 0)
        outoffset_ += outbufsize_;

    // set the actual buffersize and compute correct offsets!
    for (int i=0; i < numpartitions_; i++) {
        MtxConvSlave *partition = partitions_.getUnchecked(i);
        partition->SetBufsize(inbufsize_, outbufsize_, blocksize_);
    }


    // print debug info
    DebugInfo();

    configuration_ = true;

    skip_count_ = 0;

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

    skip_count_ = 0;
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
	String dbg_text;

	dbg_text << "Blocksize: " << blocksize_ << " MinPart: " << minpart_ << " MaxPart: " << maxpart_ << " Partitions: " << numpartitions_<< " Maxsize: " << maxsize_  << " InputBufsize: " << inbufsize_ << " OutputBufsize: " << outbufsize_ << " InOffset: " << inoffset_ << " Outoffset: " << outoffset_ << "\n";
    std::cout << dbg_text << std::endl;
#ifdef DEBUG_COUT
	WriteLog(dbg_text);
#endif

    for (int i=0; i < partitions_.size(); i++) {
        partitions_.getUnchecked(i)->DebugInfo();
    }
}

//////////////////////////////////////
// SLAVE METHODS
//////////////////////////////////////
MtxConvSlave::MtxConvSlave() : Thread("mtx_convolver_slave")
{
}

MtxConvSlave::~MtxConvSlave()
{}

void MtxConvSlave::WriteLog(String &text)
{
	if (debug_out_->openedOk())
		debug_out_->writeText(text, false, false, nullptr);
}

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

    part_idx_ = 0;

#if SPLIT_COMPLEX
    fft_norm_ = 0.25f / ( 2.f*(float)partitionsize_ ); // vDSP has a different scaling factors for fft/ifft than fftw
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
    waitprocessing_.signal();

#ifdef DEBUG_COUT
	// open debug txt

	if (debug_out_ == nullptr)
	{
		File file;
		// file = file.createTempFile(".txt");
		String filename("MtxConvSlave");
		filename << priority;
		filename << ".txt";
		file = file.getSpecialLocation(File::SpecialLocationType::tempDirectory).getChildFile(filename);
		debug_out_ = new FileOutputStream(file);
	}
#endif

    finished_part_.set(numpartitions_);
    skip_cycles_.set(0);

    return true;
}

void MtxConvSlave::SetBufsize(int inbufsize, int outbufsize, int blocksize)
{
    inbufsize_ = inbufsize;
	outbufsize_ = outbufsize;

#ifdef DEBUG_COUT
    std::cout << "Slave::SETBUFSIZE: inbufsize_: " << inbufsize_ << "outbufsize_: " << outbufsize_ << "offset_: " << offset_ << std::endl;
#endif

    inoffset_ = inbufsize_ - partitionsize_ + 1; // offset due to overlap/save

	outoffset_ = offset_;

}


void MtxConvSlave::StartProc()
{
    // start a thread for each partitionsize
    startThread(Thread::Priority::highest);
}


void MtxConvSlave::StopProc()
{
    signalThreadShouldExit();

    waitnewdata_.signal();

    stopThread(50);

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

    finished_part_.set(numpartitions_);
    skip_cycles_.set(0);
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

// thread function - do the background tasks
void MtxConvSlave::run()
{
	// thread function for highest priority
	// does only process the later partitions - first partition has to be computed immediateley
	if (priority_ == 0)
	{
		while (true)
		{
			waitnewdata_.wait();
			waitnewdata_.reset();

			if ( threadShouldExit() )
				return;

			for (int i = 1; i < numpartitions_; i++)
			{
				Process(i);
			}

            waitprocessing_.signal(); // signal callback we are done in case he is waiting
        }
	}
	else // lower priority - do forward/backward transform in the thread!
	{
		while(true)
		{
			waitnewdata_.wait();
			waitnewdata_.reset();

			if ( threadShouldExit() )
				return;

            // first check wheter we have to skip a cycle
            while (skip_cycles_.get() > 0)
            {
                TransformInput(true);
                TransformOutput(true);

                WriteToOutbuf(partitionsize_, true);

                skip_cycles_.operator--();
            }

			TransformInput(false);

			Process(0);

			TransformOutput(false);
			WriteToOutbuf(partitionsize_, false);

			for (int i = 1; i < numpartitions_; i++)
			{
				Process(i);
			}

            waitprocessing_.signal(); // signal callback we are done in case he is waiting
		}

	}

}

// this should be done in the callback
void MtxConvSlave::TransformInput(bool skip)
{
	// new data is available - increment the partition pointer
	part_idx_++;
	if (part_idx_ >= numpartitions_)
		part_idx_ = 0;

#ifdef DEBUG_COUT
	if (finished_part_.get() < numpartitions_)
  {
    String txt = "Did not finish all partitions\n";
    WriteLog(txt);
  }
	else
  {
    String txt = "Finished all partitions\n";
    WriteLog(txt);
  }
#endif

    // reset the finished counter
    if (skip)
	    finished_part_.set(numpartitions_);
    else
        finished_part_.set(0);

    if (!skip)
    {
        ////////////////
        // do all the inputs
        // copy and transform new time data for each needed input channel

        int smplstoread_end = 2 * partitionsize_; // read from the end
        int smplstoread_start = 0; // read from the start

        if (inoffset_ + smplstoread_end >= inbufsize_)
        {
            smplstoread_end = inbufsize_ - inoffset_;
            smplstoread_start = 2 * partitionsize_ - smplstoread_end;
        }

        int numinch = innodes_.size();

        for (int i = 0; i < numinch; i++) {
            InNode *innode = innodes_.getUnchecked(i);

            int chan = innode->in_;
            if (smplstoread_end)
                FloatVectorOperations::copy(fft_t_, inbuf_->getReadPointer(chan, inoffset_), smplstoread_end);

            if (smplstoread_start)
                FloatVectorOperations::copy(fft_t_ + smplstoread_end, inbuf_->getReadPointer(chan, 0), smplstoread_start);


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


#else
            fftwf_execute_dft_r2c(fftwf_plan_r2c_, fft_t_, innode->a_c_[part_idx_]);

#endif

        } // iterate over number of inputs

    } // end not skip
    else
    {
        // skip - clear innode->a_c_[part_idx_]
        int numinch = innodes_.size();

        for (int i = 0; i < numinch; i++) {
            InNode *innode = innodes_.getUnchecked(i);
#if SPLIT_COMPLEX
            FloatVectorOperations::clear(innode->a_re_[part_idx_], partitionsize_ + 1);
            FloatVectorOperations::clear(innode->a_im_[part_idx_], partitionsize_ + 1);
#else
            FloatVectorOperations::clear((float*)innode->a_c_[part_idx_], 2 * (partitionsize_ + 1));
#endif
        }

    } // end skip

    inoffset_ += partitionsize_;

    if (inoffset_ >= inbufsize_)
        inoffset_ = inoffset_ - inbufsize_;
}

// this should be done in callback thread as well
void MtxConvSlave::TransformOutput(bool skip)
{
    if (!skip)
    {
        int numouts = outnodes_.size();

        for (int i = 0; i < numouts; i++)
        {
            OutNode *outnode = outnodes_.getUnchecked(i);

#if SPLIT_COMPLEX
            DSPSplitComplex     splitcomplex;
            splitcomplex.realp = outnode->c_re_[part_idx_];
            splitcomplex.imagp = outnode->c_im_[part_idx_];

            outnode->c_im_[part_idx_][0] = outnode->c_re_[part_idx_][partitionsize_]; // special vDSP packing
            vDSP_fft_zrip(vdsp_fft_setup_, &splitcomplex, 1, vdsp_log2_, FFT_INVERSE); // ifft
            vDSP_ztoc(&splitcomplex, 1, reinterpret_cast<COMPLEX*>(fft_t_), 2, partitionsize_); // reorder
#else
            fftwf_execute_dft_c2r(fftwf_plan_c2r_, outnode->c_c_[part_idx_], fft_t_);
#endif

            outnode->outbuf_.copyFrom(0, 0, fft_t_ + partitionsize_ - 1, partitionsize_);

            // clear freq accumulation buffers
#if SPLIT_COMPLEX
            FloatVectorOperations::clear(outnode->c_re_[part_idx_], partitionsize_ + 1);
            FloatVectorOperations::clear(outnode->c_im_[part_idx_], partitionsize_ + 1);
#else
            FloatVectorOperations::clear((float*)outnode->c_c_[part_idx_], 2 * (partitionsize_ + 1));
#endif
        } // end iterate over all outputs

    }// skip
    else
    {
        // only clear buffer
        int numouts = outnodes_.size();

        for (int i = 0; i < numouts; i++)
        {
            OutNode *outnode = outnodes_.getUnchecked(i);

            // clear freq accumulation buffers
#if SPLIT_COMPLEX
            FloatVectorOperations::clear(outnode->c_re_[part_idx_], partitionsize_ + 1);
            FloatVectorOperations::clear(outnode->c_im_[part_idx_], partitionsize_ + 1);
#else
            FloatVectorOperations::clear((float*)outnode->c_c_[part_idx_], 2 * (partitionsize_ + 1));
#endif
        } // end iterate over all outputs
    }

    outnodeoffset_ = 0;

}

void MtxConvSlave::WriteToOutbuf(int numsamples, bool skip)
{
	int smplstowrite_end = numsamples; // write to the end
	int smplstowrite_start = 0; // write to the start

	if (smplstowrite_end + outoffset_ >= outbufsize_)
	{
		smplstowrite_end = outbufsize_ - outoffset_;
		// smplstowrite_start = partitionsize_ - smplstowrite_end;
		smplstowrite_start = numsamples - smplstowrite_end;
	}

	// std::cout << "outoffset: " << outoffset_ << " end: " << smplstowrite_end << " start: " << smplstowrite_start << std::endl;

    if (!skip)
    {
        int numouts = outnodes_.size();

        // std::cout << "numoutnodes: " << numouts << std::endl;

        for (int i = 0; i < numouts; i++) {

            OutNode *outnode = outnodes_.getUnchecked(i);

            if (smplstowrite_end)
                outbuf_->addFrom(outnode->out_, outoffset_, outnode->outbuf_, 0, outnodeoffset_, smplstowrite_end);

            if (smplstowrite_start)
                outbuf_->addFrom(outnode->out_, 0, outnode->outbuf_, 0, outnodeoffset_ + smplstowrite_end, smplstowrite_start);

        }
    }


	if (smplstowrite_start)
		outoffset_ = smplstowrite_start;
	else
		outoffset_ += smplstowrite_end;

	if (outoffset_ >= outbufsize_)
		outoffset_ -= outbufsize_;

	outnodeoffset_ += numsamples;
}


void MtxConvSlave::Process(int filt_part_idx)
{

#ifdef DEBUG_COUT
    std::cout << "Slave Process, numnewsamples: " << numnewinsamples_ << " partsize: " << partitionsize_ << " inoffset: " << inoffset_ << std::endl;
#endif

    /////////////
    // iterate over all outputs and query filters
    // outputs -> filters -> inputs

	int out_part_idx = part_idx_ + filt_part_idx;

	if (out_part_idx >= numpartitions_)
		out_part_idx -= numpartitions_;

    int numouts = outnodes_.size();

    for (int i=0; i < numouts; i++)
    {

        OutNode *outnode = outnodes_.getUnchecked(i);

        int numfilters = outnode->filternodes_.size();

		// std::cout << "NumPartitions: " << numpartitions_ << " Partition: " << filt_part_idx << std::endl;

		for (int j=0; j < numfilters; j++)
		{

			FilterNode *filternode = outnode->filternodes_.getUnchecked(j);

			// only process if this filter has that many partitions
			if (filt_part_idx < filternode->numpartitions_)
			{

#if SPLIT_COMPLEX
                float *a_re = filternode->innode_->a_re_[part_idx_];
				float *a_im = filternode->innode_->a_im_[part_idx_];

				float *b_re = filternode->b_re_[filt_part_idx];
				float *b_im = filternode->b_im_[filt_part_idx];

				float *c_re = outnode->c_re_[out_part_idx];
				float *c_im = outnode->c_im_[out_part_idx];

    #if JUCE_USE_SSE_INTRINSICS
				// this is the SSE convolution version (Intel Mac)
				for (int k = 0; k < partitionsize_; k+=4)
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
    #elif JUCE_USE_ARM_NEON
                // this is for Apple Silicon
                // https://physicalaudio.co.uk/testing-simd-performance-on-apples-new-m1-processor/
                // https://github.com/otim/SSE-to-NEON/blob/master/sse_to_neon.hpp

				for (int k = 0; k < partitionsize_; k+=4)
				{
					const float32x4_t ra = vld1q_f32(&a_re[k]);
					const float32x4_t rb = vld1q_f32(&b_re[k]);
					const float32x4_t ia = vld1q_f32(&a_im[k]);
					const float32x4_t ib = vld1q_f32(&b_im[k]);

					// destination
					float32x4_t rc = vld1q_f32(&c_re[k]);
					float32x4_t ic = vld1q_f32(&c_im[k]);

					// real part: real = ra*rb-ia*ib
					rc = vaddq_f32(rc, vmulq_f32(ra, rb));
					rc = vsubq_f32(rc, vmulq_f32(ia, ib));
					vst1q_f32(&c_re[k], rc);

					// imag part: imag = ra*ib + ia*rb
					ic = vaddq_f32(ic, vmulq_f32(ra, ib));
					ic = vaddq_f32(ic, vmulq_f32(ia, rb));
					vst1q_f32(&c_im[k], ic);
				}
				// handle last bin separately
				c_re[partitionsize_] += a_re[partitionsize_] * b_re[partitionsize_];
				// c_im[partitionsize_] = 0; // should be zero anyway
    #else
                // fallback to not simd

                for (int k = 0; k <= partitionsize_; k++)
				{
                    c_re[k] += a_re[k] * b_re[k] - a_im[k] * b_im[k];
                    c_im[k] += a_re[k] * b_im[k] + a_im[k] * b_re[k];
                }
    #endif
#else
    #if JUCE_USE_SSE_INTRINSICS
                // Intel Win/Linux
				// sse 3 from http://yangkunlun.blogspot.de/2011/09/fast-complex-multiply-with-sse.html

				float *A = (float *) filternode->innode_->a_c_[part_idx_];
				float *B = (float *) filternode->b_c_[filt_part_idx];
				float *D = (float *) outnode->c_c_[out_part_idx];


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
				outnode->c_c_[out_part_idx][partitionsize_][0] += filternode->innode_->a_c_[part_idx_] [partitionsize_][0] * filternode->b_c_[filt_part_idx] [partitionsize_][0];
				// fft_c_ [partitionsize_][1] = 0; // should be zero anyway
    #else
            // fallback to not simd
            fftwf_complex *A = filternode->innode_->a_c_[part_idx_];
            fftwf_complex *B = filternode->b_c_[filt_part_idx];
            fftwf_complex *C = outnode->c_c_[out_part_idx];

            for (int k = 0; k <= partitionsize_; k++)
            {
                C[k][0] += A[k][0] * B[k][0] - A[k][1] * B[k][1];
                C[k][1] += A[k][0] * B[k][1] + A[k][1] * B[k][0];
            }
    #endif
#endif

			}
		}

    } // end iterate over all outputs

	// increment finished atomic int
	finished_part_.operator++();
}


// this is called from the master
bool MtxConvSlave::ReadOutput(int numsamples, bool forcesync)
{
    bool skip = false;

    numnewinsamples_ += numsamples;

    // do processing if enough samples arrived
    if (numnewinsamples_ >= partitionsize_)
    {
        if (forcesync)
            waitprocessing_.wait(1000); // maximum wait for 1 seconds if force sync... this is a long time anyway...
        //else
        //    waitprocessing_.wait(1); // should we try to wait in realtime mode as well?

        if (finished_part_.get() < numpartitions_)
        {
            // did not finish all partitions... skip this cycle...
            skip_cycles_.operator++(); // signal the next cycle to skip a cycle... (ensure threadsafety...)

            skip = true;
        }
        else
        {
            if (priority_ == 0) // highest priority has to deliver immediateley
            {

                // first check wheter we have to skip a cycle
                while (skip_cycles_.get() > 0)
                {
                    TransformInput(true);
                    TransformOutput(true);

                    WriteToOutbuf(partitionsize_, true);

                    skip_cycles_.operator--();
                }

                TransformInput(false);

                // compute first partition directly
                Process(0);

                // signal thread to do the other partitions
                waitprocessing_.reset();
                waitnewdata_.signal();

                TransformOutput(false);

                WriteToOutbuf(partitionsize_, false); // should i do something different here if skipped??!!
            }
            else // lower priority has some time for computations...
            {
                // signal thread to do the work...
                waitprocessing_.reset();
                waitnewdata_.signal();
            }
        }

        numnewinsamples_ -= partitionsize_;
    }

#ifdef DEBUG_COUT
    std::cout << "ReadOutput, outnodeoffset_: " << outnodeoffset_ << " outoffset_: " << outoffset_ << std::endl;
#endif

	// what to do with smaller buffers?

    return !skip;
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
        outnodes_.add(new OutNode(out, partitionsize_, numpartitions_));
        ret = outnodes_.size()-1;
    }

    return ret;
}

void MtxConvSlave::DebugInfo()
{
	String dbg_text;
	dbg_text << "Priority: " << priority_ << " Partitionsize: " << partitionsize_ << " Subpartitions: " << numpartitions_ << " Offset: " << offset_ << " Inoffset: " << inoffset_ << " Outoffset: " << outoffset_ << "\n";

#ifdef DEBUG_COUT
	WriteLog(dbg_text);
#endif

	std::cout << dbg_text << std::endl;
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

