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

#include "JuceHeader.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include "SphFilter/SphFilter.h"

#include <Eigen/Eigen>

#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

#ifndef NUM_FILTERS
    #define NUM_FILTERS 8 // number of directional filters that can be controlled
#endif

#define PARAMS_PER_FILTER 7


typedef Eigen::Array<float, NUM_FILTERS, 1> ArrayParam;
typedef Eigen::Array<bool, NUM_FILTERS, 1> ArrayBoolParam;
typedef Eigen::Array<double, NUM_FILTERS, 2> SphCoordParam;
typedef Eigen::Array<double, NUM_FILTERS, 3> CarthCoordParam;

#define LOGTEN 2.302585092994

inline float rmstodb(float rms)
{
    return (float) (20.f/LOGTEN * logf(rms));
}

inline float dbtorms(float db)
{
    return expf((float) (LOGTEN * 0.05f) * db);
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


inline float DbToParam(float db)
{
    if (db <= -99.f) {
        return 0.f;
    }
    else if (db <= 0.f)
    {
        return sqrt(dbtorms(db))*0.5f;
    }
    else// if (db > 0.f)
    {
        // return    0.5f+sqrt(dbtorms(db)-1.f);

        return sqrt((dbtorms(db) - 1.f) / 9.f)*0.5f+0.5f;
    }
}

inline float ParamToDeg360(float param)
{
    return (param-0.5f)*360.f;
}

inline float Deg360ToParam(float deg)
{
    return (deg+180.f)*(1.f/360.f);
}

inline float ParamToDeg180(float param)
{
    return param*180.f;
}

inline float Deg180ToParam(float deg)
{
    return deg*(1.f/180.f);
}

//==============================================================================
/**
*/
class Ambix_directional_loudnessAudioProcessor  : public AudioProcessor,
                                                  public ChangeBroadcaster
{
public:
    //==============================================================================
    Ambix_directional_loudnessAudioProcessor();
    ~Ambix_directional_loudnessAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    int getNumParameters() override;

    float getParameter (int index) override;
    void setParameter (int index, float newValue) override;

    const String getParameterName (int index) override;
    const String getParameterText (int index) override;

    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum Parameters
	{
        AzimuthParam,
        ElevationParam,
        ShapeParam,
        WidthParam,
        HeightParam,
        GainParam,
        WindowParam
	};

    int filter_sel_id_1;
    int filter_sel_id_2;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

    void calcParams();


    ArrayParam shape;
    ArrayParam width;
    ArrayParam height;
    ArrayParam gain;
    ArrayBoolParam window;
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

    // Eigen::VectorXd Ymn; // Ymn result
    Eigen::MatrixXd Sh_matrix_inv; // undistorted SH Matrix
    Eigen::MatrixXd Sh_matrix_mod; // distorted SH Matrix

    Eigen::MatrixXd Sh_transf; // Transformation Matrix

    Eigen::MatrixXd _Sh_transf; // Old Transformation Matrix

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_directional_loudnessAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
