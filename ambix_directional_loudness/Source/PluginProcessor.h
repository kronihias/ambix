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
#include "ambix_buses.h"
#include "SphericalHarmonic/SphericalHarmonic.h"
#include "SphFilter/SphFilter.h"

#include <Eigen/Eigen>

#include "AmbisonicConfig.h"

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

   #ifdef UNIVERSAL_AMBISONIC
    AMBIX_APPLY_BUS_LAYOUTS_OVERRIDE
   #endif
    void numChannelsChanged() override;

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

    // View-state (persisted alongside parameters in the XML state but NOT
    // exposed as host-automated parameters). Mutated by the editor; read
    // back on plugin reload.
    bool   view_proj_ha    = true;        // false = equirect, true = HA
    bool   view_energy_on  = true;
    bool   view_autonorm   = true;
    bool   view_pre_mod    = false;       // true = pre-Sh_transf, false = post
    float  view_range_db   = 25.f;        // dB span of the colormap (always user-controlled)
    float  view_peak_db    = -25.f;       // top of colormap when not auto
    juce::String view_colormap = "jet";

    // Smoothing time constant for the energy heatmap (ms). Read on the audio
    // thread via std::memory_order_relaxed; the value is small and atomic.
    std::atomic<float> view_smoothing_ms { 150.f };

    // Optional 4th-order Butterworth HP/LP filter applied only to the
    // visualizer signal (pre and post mod taps), so the energy heatmap can be
    // band-limited without affecting the audio output.
    std::atomic<bool>  view_vis_hpf_on { false };
    std::atomic<bool>  view_vis_lpf_on { false };
    std::atomic<float> view_vis_hpf_fc { 500.f };
    std::atomic<float> view_vis_lpf_fc { 5000.f };

    // ------------------------------------------------------------------------
    // Energy visualization API (used by the editor on its GUI timer thread).
    // ------------------------------------------------------------------------

    // Cartesian unit vectors of the HA visualization grid (IEM 426 points).
    const std::vector<Eigen::Vector3d>& getGridDirectionsCart() const { return grid_dirs_cart_; }

    // Cartesian unit vectors of the equirect grid (regular Naz × Nel az/el).
    const std::vector<Eigen::Vector3d>& getGridDirectionsEq() const { return grid_dirs_eq_; }
    int  getEqGridNaz() const { return eq_grid_naz_; }
    int  getEqGridNel() const { return eq_grid_nel_; }

    // Copy snapshots of the smoothed per-grid-point linear RMS² energy.
    // Returns the number of grid points; uses a brief spin-lock against
    // the audio thread. `postMod=true` returns the energy of the modified
    // (post-Sh_transf) signal instead of the raw input.
    int getGridEnergySnapshot   (std::vector<float>& out, bool postMod = false) const;
    int getGridEnergySnapshotEq (std::vector<float>& out, bool postMod = false) const;

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

    // --- Input-energy tap (pre-`Sh_transf`) for the heatmap visualization ---

    // Visualization decode matrix: rows = HA-uniform sample points (426),
    // cols = AMBI_CHANNELS. Built once in calcParams(). Independent from the
    // audio-path t-design so the heatmap can match IEM's EnergyVisualizer
    // exactly without disturbing the directional-loudness DSP.
    Eigen::MatrixXd Sh_matrix_viz_;

    // Smoothed RMS² per viz grid point (size = HammerAitovSample::kNumPoints).
    // _post_ variants hold the energy of the modified (post-Sh_transf) signal.
    std::vector<float> grid_energy_smoothed_;
    std::vector<float> grid_energy_post_smoothed_;
    mutable juce::SpinLock grid_energy_lock_;

    // Pre-allocated scratch — sized in calcParams() once the grid and channel
    // count are known. Must not allocate inside processBlock.
    Eigen::VectorXd grid_energy_block_;       // size = viz grid count (pre)
    Eigen::VectorXd grid_energy_post_block_;  // size = viz grid count (post)
    Eigen::VectorXd grid_decode_scratch_;     // size = viz grid count
    Eigen::VectorXd input_scratch_;           // size = AMBI_CHANNELS

    // Cartesian unit vectors of the HA visualization grid (IEM 426 points).
    std::vector<Eigen::Vector3d> grid_dirs_cart_;

    // ----- Equirect (regular az/el) grid: independent decode + energy buffer.
    Eigen::MatrixXd Sh_matrix_eq_;
    std::vector<Eigen::Vector3d> grid_dirs_eq_;
    std::vector<float>           grid_energy_eq_smoothed_;
    std::vector<float>           grid_energy_eq_post_smoothed_;
    Eigen::VectorXd              grid_energy_eq_block_;
    Eigen::VectorXd              grid_energy_eq_post_block_;
    Eigen::VectorXd              grid_decode_eq_scratch_;
    int eq_grid_naz_ = 24;   // 15° azimuth step
    int eq_grid_nel_ = 19;   // 10° elevation step (incl. both poles)

    double sample_rate_ = 48000.0;

    // ----- 4th-order Butterworth filtering for the visualizer-only path -----
    // RBJ-cookbook biquad in Direct Form II Transposed. 4th-order = 2 sections
    // cascaded with the two Butterworth Q values for pole pairs at ±π/8 / ±3π/8.
    struct VizBiquad
    {
        float b0 = 1.f, b1 = 0.f, b2 = 0.f, a1 = 0.f, a2 = 0.f;
        inline float process (float x, float& z1, float& z2) const noexcept
        {
            const float y = b0 * x + z1;
            z1 = b1 * x + z2 - a1 * y;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };
    struct VizFilter4 { VizBiquad s1, s2; };

    static void designVizHighPass (VizFilter4& f, float sr, float fc);
    static void designVizLowPass  (VizFilter4& f, float sr, float fc);

    VizFilter4 hpf_design_;
    VizFilter4 lpf_design_;
    float      hpf_fc_cached_ = -1.f;
    float      lpf_fc_cached_ = -1.f;

    // Per-channel filter state — 4 floats per channel (s1.z1, s1.z2, s2.z1,
    // s2.z2). Pre and post taps share the state: this is for visualization
    // only, not audio playback, so the brief state-mismatch transient when
    // alternating between pre and post signals is invisible after the energy
    // smoothing and not worth a 2× state cost.
    std::vector<float> state_hpf_, state_lpf_;

    // Scratch buffers for the filtered visualizer signal (sized in
    // prepareToPlay; only populated when at least one filter is active).
    juce::AudioBuffer<float> filt_pre_buf_;
    juce::AudioBuffer<float> filt_post_buf_;

    void applyVizFilters (const juce::AudioBuffer<float>& src,
                          juce::AudioBuffer<float>& dst,
                          int numCh, int numSamples,
                          bool hpfOn, bool lpfOn);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_directional_loudnessAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
