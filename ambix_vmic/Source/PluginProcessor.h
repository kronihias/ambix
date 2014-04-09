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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include "SphFilter/SphFilter.h"

#include <Eigen/Eigen>


typedef Eigen::Array<float, NUM_FILTERS_VMIC, 1> ArrayParam;
typedef Eigen::Array<double, NUM_FILTERS_VMIC, 2> SphCoordParam;
typedef Eigen::Array<double, NUM_FILTERS_VMIC, 3> CarthCoordParam;

typedef Eigen::Matrix<double, NUM_FILTERS_VMIC, AMBI_CHANNELS> FilterMatrix;

#define LOGTEN 2.302585092994

inline float rmstodb(float rms)
{
    return 20.f/(float)LOGTEN * logf(rms);
}

inline float dbtorms(float db)
{
    return expf(((float)LOGTEN * 0.05f) * db);
}

// scale parameter for host between 0.0 and 1.0 to -inf dB ... +20dB
inline float ParamToRMS(float param)
{
    float rms = 0.f;
    
    if (param < 0.f )
    {
        rms = 0.f;
    }
    else if ((param >= 0.f) && (param <= 0.5f))
    {
        rms = (param * 2) * (param * 2); // normalize and square
    }
    else if ((param > 0.5f) && (param < 1.f))
    {
        rms = ((param - 0.5f) / 0.5f) * ((param - 0.5f) / 0.5f) * 9.f + 1.f;
    }
    else if (param >= 1.f)
    {
        rms = 10.f; // +20 dB
    }
    
    
    return rms;
}

// scale parameter for host between 0.0 and 1.0
inline float ParamToDB(float param)
{
    return rmstodb(ParamToRMS(param));
}

//==============================================================================
/**
*/
class Ambix_vmicAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    Ambix_vmicAudioProcessor();
    ~Ambix_vmicAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    
    void calcParams();
    
    
    ArrayParam shape;
    ArrayParam width;
    ArrayParam height;
    ArrayParam gain;
    ArrayParam window;
    ArrayParam transition;
    
    SphCoordParam center_sph;
    
    
    bool _initialized;
    
    bool _param_changed; // param changed need recalculation
    
    AudioSampleBuffer output_buffer;
    
    SphericalHarmonic sph_h;
    
    SphFilter sph_filter;
    
    // Eigen stuff
    
    Eigen::MatrixXd Carth_coord; // carthesian coordinates
    Eigen::MatrixXd Sph_coord; // spherical coordinates
    
    
    Eigen::MatrixXd Sh_matrix;
    
    FilterMatrix Sh_matrix_mod; // each row is one virtual mic
    
    FilterMatrix Sh_transf; // Transformation Matrix
    
    FilterMatrix _Sh_transf; // Old Transformation Matrix
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_vmicAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
