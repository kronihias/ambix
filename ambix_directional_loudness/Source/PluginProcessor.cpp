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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HammerAitovSample.h"

#include "t_design.h"


//==============================================================================
Ambix_directional_loudnessAudioProcessor::Ambix_directional_loudnessAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET(AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
    filter_sel_id_1(0),
    filter_sel_id_2(0),
    _initialized(false),
    _param_changed(false),
    output_buffer(AMBI_CHANNELS,256)
{
    // init parameters
    shape.setConstant(0.f); // circular

    width.setConstant(0.1f); // +-18°

    height.setConstant(0.1f); // +-18°

    gain.setConstant(0.5f); // 0 dB

    window.setConstant(false); // off

    transition.setConstant(0.f); // not used yet

    center_sph.setConstant(0.5f); // 0 deg

    // set transformation matrix to identity matrix
    Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
}

Ambix_directional_loudnessAudioProcessor::~Ambix_directional_loudnessAudioProcessor()
{

}

//==============================================================================
const String Ambix_directional_loudnessAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_directional_loudnessAudioProcessor::getNumParameters()
{
    return PARAMS_PER_FILTER*NUM_FILTERS;
}

float Ambix_directional_loudnessAudioProcessor::getParameter (int index)
{
    int filter_id = (int)floor(index/PARAMS_PER_FILTER);

    if (filter_id >= NUM_FILTERS) // safety..
        return 0.f;

    switch (index%PARAMS_PER_FILTER) {

        case 0:
            return (float)center_sph(filter_id, 0);
            break;

        case 1:
            return (float)center_sph(filter_id, 1);
            break;

        case 2:
            return shape(filter_id);
            break;

        case 3:
            return width(filter_id);

        case 4:
            return height(filter_id);


        case 5:
            return gain(filter_id);


        case 6:
            return window(filter_id) ? 1.f : 0.f;


        case 7:
            return transition(filter_id);


        default:
            return 0.0f;
    }
}

/*
 Parameters:
 ////////////
 Azimuth
 Elevation
 Shape
 Width
 Height
 Gain
 Window
*/
void Ambix_directional_loudnessAudioProcessor::setParameter (int index, float newValue)
{

    int filter_id = (int)floor(index/PARAMS_PER_FILTER);

    if (filter_id < NUM_FILTERS) // safety..
    {
        _param_changed = true;

        switch (index%PARAMS_PER_FILTER) {

            case 0:
                center_sph(filter_id, 0) = newValue;
                break;

            case 1:
                center_sph(filter_id, 1) = newValue;
                break;

            case 2:
                shape(filter_id) = (newValue <= 0.5f ? 0 : 1.f);
                break;

            case 3:
                width(filter_id) = newValue;
                break;

            case 4:
                height(filter_id) = newValue;
                break;

            case 5:
                gain(filter_id) = newValue;
                break;

            case 6:
                window(filter_id) = newValue > 0.5f ? true : false;
                break;

            case 7:
                transition(filter_id) = newValue;
                break;

            default:
                _param_changed = false;
                break;
        }

    }

    sendChangeMessage();
}

const String Ambix_directional_loudnessAudioProcessor::getParameterName (int index)
{
    String text;

    int filter_id = (int)floor(index/PARAMS_PER_FILTER);

    if (filter_id >= NUM_FILTERS) // safety..
        return String();

    switch (index%PARAMS_PER_FILTER) {

        case 0:
            text = "center azimuth";
            break;

        case 1:
            text = "center elevation";
            break;

        case 2:
            text = "shape";
            break;

        case 3:
            text = "width";
            break;

        case 4:
            text = "height";
            break;

        case 5:
            text = "gain";
            break;

        case 6:
            text = "solo";
            break;

        case 7:
            text = "transition";
            break;

        default:
            return "";
    }

    text << " ";
    text << filter_id+1;
    return text;
}

const String Ambix_directional_loudnessAudioProcessor::getParameterText (int index)
{
    String text;

    int filter_id = (int)floor(index/PARAMS_PER_FILTER);

    if (filter_id >= NUM_FILTERS) // safety..
        return String();

    switch (index%PARAMS_PER_FILTER) {

        case 0:
            text = String((center_sph(filter_id, 0) - 0.5)*360).substring(0, 5);
            text << " deg";
            break;

        case 1:
            text = String((center_sph(filter_id, 1) - 0.5)*360).substring(0, 5);
            text << " deg";
            break;

        case 2:
            if (shape(filter_id) <= 0.5)
                text = "circular";
            else
                text = "rectangular";
            break;

        case 3:
            text = String(width(filter_id)*180).substring(0, 5);
            text << " deg";
            break;

        case 4:
            text = String(height(filter_id)*180).substring(0, 5);
            text << " deg";
            break;

        case 5:
            text = String(ParamToDB(gain(filter_id))).substring(0, 5);
            text << " dB";
            break;

        case 6:
            if (window(filter_id))
                text = "on";
            else
                text = "off";
            break;

        case 7:
            text = String(transition(filter_id)).substring(0, 5);
            text << "";
            break;

        default:
            return "";
    }

    return text;
}

const String Ambix_directional_loudnessAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_directional_loudnessAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_directional_loudnessAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_directional_loudnessAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_directional_loudnessAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_directional_loudnessAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_directional_loudnessAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_directional_loudnessAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_directional_loudnessAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_directional_loudnessAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_directional_loudnessAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_directional_loudnessAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_directional_loudnessAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_directional_loudnessAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    output_buffer.setSize (getTotalNumOutputChannels(), samplesPerBlock, false, false, false);
    sample_rate_ = sampleRate > 0.0 ? sampleRate : 48000.0;

    // Visualizer-only filter scratch buffers + per-channel state (4 floats
    // per channel per filter chain = z1/z2 of two cascaded biquads).
    filt_pre_buf_ .setSize (AMBI_CHANNELS, samplesPerBlock, false, false, true);
    filt_post_buf_.setSize (AMBI_CHANNELS, samplesPerBlock, false, false, true);
    state_hpf_.assign ((size_t) AMBI_CHANNELS * 4, 0.f);
    state_lpf_.assign ((size_t) AMBI_CHANNELS * 4, 0.f);
    hpf_fc_cached_ = lpf_fc_cached_ = -1.f;  // force recompute on first block

    _initialized = false;
    calcParams();
}

// ----- Visualizer-only filter design (RBJ cookbook + Butterworth Q split) ---

void Ambix_directional_loudnessAudioProcessor::designVizHighPass (VizFilter4& f, float sr, float fc)
{
    constexpr float kQ1 = 1.30656296f;  // pole pair at ±π/8
    constexpr float kQ2 = 0.54119610f;  // pole pair at ±3π/8
    auto designSec = [] (VizBiquad& b, float sampleRate, float freq, float Q)
    {
        const float w0 = 2.f * juce::MathConstants<float>::pi * freq / sampleRate;
        const float c  = std::cos (w0);
        const float s  = std::sin (w0);
        const float alpha = s / (2.f * Q);
        const float a0 = 1.f + alpha;
        b.b0 =  ((1.f + c) * 0.5f) / a0;
        b.b1 = -(1.f + c)         / a0;
        b.b2 =  ((1.f + c) * 0.5f) / a0;
        b.a1 = -2.f * c           / a0;
        b.a2 = (1.f - alpha)      / a0;
    };
    designSec (f.s1, sr, fc, kQ1);
    designSec (f.s2, sr, fc, kQ2);
}

void Ambix_directional_loudnessAudioProcessor::designVizLowPass (VizFilter4& f, float sr, float fc)
{
    constexpr float kQ1 = 1.30656296f;
    constexpr float kQ2 = 0.54119610f;
    auto designSec = [] (VizBiquad& b, float sampleRate, float freq, float Q)
    {
        const float w0 = 2.f * juce::MathConstants<float>::pi * freq / sampleRate;
        const float c  = std::cos (w0);
        const float s  = std::sin (w0);
        const float alpha = s / (2.f * Q);
        const float a0 = 1.f + alpha;
        b.b0 = ((1.f - c) * 0.5f) / a0;
        b.b1 =  (1.f - c)         / a0;
        b.b2 = ((1.f - c) * 0.5f) / a0;
        b.a1 = -2.f * c           / a0;
        b.a2 = (1.f - alpha)      / a0;
    };
    designSec (f.s1, sr, fc, kQ1);
    designSec (f.s2, sr, fc, kQ2);
}

void Ambix_directional_loudnessAudioProcessor::applyVizFilters (const juce::AudioBuffer<float>& src,
                                                                juce::AudioBuffer<float>& dst,
                                                                int numCh, int numSamples,
                                                                bool hpfOn, bool lpfOn)
{
    const auto& hpf = hpf_design_;
    const auto& lpf = lpf_design_;

    for (int c = 0; c < numCh; ++c)
    {
        const float* in  = src.getReadPointer  (c);
        float*       out = dst.getWritePointer (c);
        const size_t ofs = (size_t) c * 4;

        if (hpfOn && lpfOn)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float x = in[i];
                x = hpf.s1.process (x, state_hpf_[ofs+0], state_hpf_[ofs+1]);
                x = hpf.s2.process (x, state_hpf_[ofs+2], state_hpf_[ofs+3]);
                x = lpf.s1.process (x, state_lpf_[ofs+0], state_lpf_[ofs+1]);
                x = lpf.s2.process (x, state_lpf_[ofs+2], state_lpf_[ofs+3]);
                out[i] = x;
            }
        }
        else if (hpfOn)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float x = in[i];
                x = hpf.s1.process (x, state_hpf_[ofs+0], state_hpf_[ofs+1]);
                x = hpf.s2.process (x, state_hpf_[ofs+2], state_hpf_[ofs+3]);
                out[i] = x;
            }
        }
        else  // lpfOn
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float x = in[i];
                x = lpf.s1.process (x, state_lpf_[ofs+0], state_lpf_[ofs+1]);
                x = lpf.s2.process (x, state_lpf_[ofs+2], state_lpf_[ofs+3]);
                out[i] = x;
            }
        }
    }
}

void Ambix_directional_loudnessAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_directional_loudnessAudioProcessor::calcParams()
{
    if (!_initialized)
    {

        sph_h.Init(AMBI_ORDER);

        const String t_design_txt (t_design::des_3_240_21_txt);

        // std::cout << t_design_txt << std::endl;

        String::CharPointerType lineChar = t_design_txt.getCharPointer();

        int n = 0; // how many characters been read
        int numsamples = 0;
        int i = 0;

        int curr_n = 0;
        int max_n = lineChar.length();

        while (curr_n < max_n) { // check how many coordinates we have
            double value;
            sscanf(lineChar, "%lf\n%n", &value, &n);
            lineChar += n;
            curr_n += n;
            numsamples++;
        } // end parse numbers

        numsamples = numsamples/3; // xyz

        Carth_coord.resize(numsamples,3); // positions in cartesian coordinates

        curr_n = 0;
        lineChar = t_design_txt.getCharPointer();

        // parse line for numbers again and copy to carth coordinate matrix
        while (i < numsamples) {

            double x,y,z;

            sscanf(lineChar, "%lf%lf%lf%n", &x, &y, &z, &n);

            Carth_coord(i,0) = x;
            Carth_coord(i,1) = y;
            Carth_coord(i,2) = z;

            lineChar += n;

            curr_n += n;
            i++;

        } // end parse numbers

        // std::cout << "Coordinate size: " << Carth_coord.rows() << " x " << Carth_coord.cols() << std::endl;
        // std::cout << Carth_coord << std::endl;

        Sph_coord.resize(numsamples,2); // positions in spherical coordinates


        Sh_matrix.setZero(numsamples,AMBI_CHANNELS);

        for (int i=0; i < numsamples; i++)
        {
            Eigen::VectorXd Ymn(AMBI_CHANNELS); // Ymn result

            Sph_coord(i,0) = atan2(Carth_coord(i,1),Carth_coord(i,0)); // azimuth
            Sph_coord(i,1) = atan2(Carth_coord(i,2),sqrt(Carth_coord(i,0)*Carth_coord(i,0) + Carth_coord(i,1)*Carth_coord(i,1))); // elevation

            sph_h.Calc(Sph_coord(i,0),Sph_coord(i,1)); // phi theta
            sph_h.Get(Ymn);

            // std::cout << "Size: " << Ymn.size() << ": " << Ymn << std::endl;

            //Sh_matrix.row(i) = Ymn;
            Sh_matrix.block(i,0,1,AMBI_CHANNELS) = Ymn.transpose();
            // std::cout << "Size: " << Sh_matrix.block(i,0,1,in_ambi_channels).size() << ": " << Sh_matrix.block(i,0,1,in_ambi_channels) << std::endl;
        }

        // Sh_matrix_inv.setZero();

        //Sh_matrix_inv = (Sh_matrix.transpose()*Sh_matrix).inverse()*Sh_matrix.transpose(); // not working for dynamic input order
        // if input order is different a better solving has to be used for the inverse:
        Sh_matrix_inv = (Sh_matrix.transpose()*Sh_matrix).colPivHouseholderQr().inverse()*Sh_matrix.transpose();

        // std::cout << "Size: " << Sh_matrix_inv.rows() << " x " << Sh_matrix_inv.cols() << std::endl;
        // std::cout << Sh_matrix_inv << std::endl;

        // ----- Visualization-grid decode (independent from the audio path) -----
        // Use the IEM 426-point Hammer-Aitoff-uniform grid so the heatmap
        // matches EnergyVisualizer's sampling. Build Sh_matrix_viz (426 × C)
        // by evaluating the SH basis at each sample direction's (az, el).
        const int vN = HammerAitovSample::kNumPoints;
        Sh_matrix_viz_.setZero (vN, AMBI_CHANNELS);
        grid_dirs_cart_.resize ((size_t) vN);
        for (int i = 0; i < vN; ++i)
        {
            const double cx = (double) HammerAitovSample::kX[i];
            const double cy = (double) HammerAitovSample::kY[i];
            const double cz = (double) HammerAitovSample::kZ[i];
            grid_dirs_cart_[(size_t) i] = Eigen::Vector3d (cx, cy, cz);

            const double az = std::atan2 (cy, cx);
            const double el = std::atan2 (cz, std::sqrt (cx * cx + cy * cy));

            Eigen::VectorXd Ymn (AMBI_CHANNELS);
            sph_h.Calc (az, el);
            sph_h.Get (Ymn);
            Sh_matrix_viz_.row (i) = Ymn.transpose();
        }

        // Energy-tap buffers (pre/post-`Sh_transf` decode → smoothed grid energy).
        {
            const juce::SpinLock::ScopedLockType l (grid_energy_lock_);
            grid_energy_smoothed_     .assign ((size_t) vN, 0.f);
            grid_energy_post_smoothed_.assign ((size_t) vN, 0.f);
        }
        grid_energy_block_       .setZero (vN);
        grid_energy_post_block_  .setZero (vN);
        grid_decode_scratch_     .setZero (vN);
        input_scratch_           .setZero (AMBI_CHANNELS);

        // ----- Equirect (regular az/el) grid for the rectangular display.
        // Uniform sampling in (az, el) → straight rectangular coverage with
        // no pole-fan / antimeridian artifacts when rendered as a regular
        // mesh. Triangulation is built editor-side from (Naz, Nel).
        const int Naz = eq_grid_naz_;
        const int Nel = eq_grid_nel_;
        const int eqN = Naz * Nel;
        Sh_matrix_eq_.setZero (eqN, AMBI_CHANNELS);
        grid_dirs_eq_.resize ((size_t) eqN);
        for (int j = 0; j < Nel; ++j)
        {
            const double el_deg = -90.0 + (180.0 * j) / (Nel - 1);
            const double el     = el_deg * M_PI / 180.0;
            for (int i = 0; i < Naz; ++i)
            {
                const double az_deg = -180.0 + (360.0 * i) / Naz;
                const double az     = az_deg * M_PI / 180.0;
                const double ce = std::cos (el);
                const int    k  = j * Naz + i;
                grid_dirs_eq_[(size_t) k] = Eigen::Vector3d (std::cos (az) * ce,
                                                              std::sin (az) * ce,
                                                              std::sin (el));
                Eigen::VectorXd Ymn (AMBI_CHANNELS);
                sph_h.Calc (az, el);
                sph_h.Get (Ymn);
                Sh_matrix_eq_.row (k) = Ymn.transpose();
            }
        }
        {
            const juce::SpinLock::ScopedLockType l (grid_energy_lock_);
            grid_energy_eq_smoothed_     .assign ((size_t) eqN, 0.f);
            grid_energy_eq_post_smoothed_.assign ((size_t) eqN, 0.f);
        }
        grid_energy_eq_block_       .setZero (eqN);
        grid_energy_eq_post_block_  .setZero (eqN);
        grid_decode_eq_scratch_     .setZero (eqN);

        _initialized = true;

    }

    if (_param_changed)
    {
        // convert parameters to values for the filter
        // ArrayIntParam _shape = shape;

        ArrayParam _width = width * (float)M_PI; // 0...pi
        ArrayParam _height = height * (float)M_PI;

        ArrayParam _gain;

        for (int i=0; i < gain.rows();i++)
        {
            _gain(i) = ParamToRMS(gain(i));
        }

        SphCoordParam _center_sph = (center_sph - 0.5f)*2.f*(float)M_PI;

        // std::cout << _center_sph << std::endl;

        CarthCoordParam _center_carth;

        // convert center spherical coordinates to carthesian
        for (int i=0; i < _center_sph.rows(); i++)
        {
            _center_carth(i,0) = cos(_center_sph(i,0))*cos(_center_sph(i,1)); // x
            _center_carth(i,1) = sin(_center_sph(i,0))*cos(_center_sph(i,1)); // y
            _center_carth(i,2) = sin(_center_sph(i,1)); // z
        }

        // scale the SH_matrix and save as Sh_matrix_mod
        Sh_matrix_mod = Sh_matrix;

        bool one_filter_is_solo = false;
        for (int k=0; k < NUM_FILTERS; k++)
        {
            one_filter_is_solo = one_filter_is_solo || window(k);
        }

        // iterate over all sample points
        for (int i=0; i < Sh_matrix_mod.rows(); i++)
        {
            double multipl = 1.f;

            if (one_filter_is_solo)
                multipl = 0.f;

            // iterate over all filters
            for (int k=0; k < NUM_FILTERS; k++)
            {
                Eigen::Vector2d Sph_coord_vec = Sph_coord.row(i);
                Eigen::Vector2d _center_sph_vec = _center_sph.row(k);

                if (one_filter_is_solo)
                {
                    double newmult = (double)sph_filter.GetWeight(&Sph_coord_vec, Carth_coord.row(i), &_center_sph_vec, _center_carth.row(k), (int)shape(k), _width(k), _height(k), _gain(k), window(k), transition(k));

                    if (window(k) && newmult > -1.f)
                    {
                        if (multipl == 0.f)
                            multipl = newmult;
                        else
                            multipl *= newmult;
                    }
                } else {
                    multipl *= (double)sph_filter.GetWeight(&Sph_coord_vec, Carth_coord.row(i), &_center_sph_vec, _center_carth.row(k), (int)shape(k), _width(k), _height(k), _gain(k), window(k), transition(k));
                }
            }

            Sh_matrix_mod.row(i) *= multipl;

        }

        // calculate new transformation matrix
        Sh_transf = Sh_matrix_inv * Sh_matrix_mod;

        // threshold coefficients
        for (int i = 0; i < Sh_transf.size(); i++)
        {
            if (abs(Sh_transf(i)) < 0.00001f)
                Sh_transf(i) = 0.f;
        }

        _param_changed = false;
    }


}

bool Ambix_directional_loudnessAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
#endif
}

void Ambix_directional_loudnessAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    _initialized = false;
    sendChangeMessage();
#endif
}

void Ambix_directional_loudnessAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    _Sh_transf = Sh_transf; // buffer old values

    calcParams(); // calc new transformation matrix

    int NumSamples = buffer.getNumSamples();

    // ---------- input-energy tap (pre-Sh_transf) ----------
    // Decode the input through Sh_matrix at a strided set of samples to get
    // virtual-LS amplitudes at each t-design grid point, accumulate squared
    // energy, smooth into grid_energy_smoothed_. Cheap (240 × C × ~16 muladds
    // per block) and bounded — visualization-only, no audio impact.
    if (_initialized && grid_energy_block_.size() > 0 && Sh_matrix_viz_.rows() > 0)
    {
        const int numCh = std::min ({ (int) AMBI_CHANNELS,
                                      getTotalNumInputChannels(),
                                      (int) Sh_matrix_viz_.cols() });
        const int numGrid = (int) Sh_matrix_viz_.rows();
        const int targetTaps = 16;
        const int stride = std::max (1, NumSamples / targetTaps);

        // Optional HP / LP for the visualizer signal only. Coefficients are
        // shared across all channels (same fc), state is per channel.
        const bool hpfOn = view_vis_hpf_on.load (std::memory_order_relaxed);
        const bool lpfOn = view_vis_lpf_on.load (std::memory_order_relaxed);
        const float hpFc = juce::jlimit (10.f, (float) (sample_rate_ * 0.45),
                                          view_vis_hpf_fc.load (std::memory_order_relaxed));
        const float lpFc = juce::jlimit (10.f, (float) (sample_rate_ * 0.45),
                                          view_vis_lpf_fc.load (std::memory_order_relaxed));
        // Recompute coefficients on fc change. State is left as-is — at small
        // fc deltas the existing state is close enough to be valid, and any
        // residual transient is well below the energy smoothing time
        // constant so it's invisible in the heatmap.
        if (hpfOn && hpFc != hpf_fc_cached_)
        {
            designVizHighPass (hpf_design_, (float) sample_rate_, hpFc);
            hpf_fc_cached_ = hpFc;
        }
        if (lpfOn && lpFc != lpf_fc_cached_)
        {
            designVizLowPass  (lpf_design_, (float) sample_rate_, lpFc);
            lpf_fc_cached_ = lpFc;
        }

        const juce::AudioBuffer<float>* preSrc = &buffer;
        if (hpfOn || lpfOn)
        {
            if (filt_pre_buf_.getNumSamples() < NumSamples
                || filt_pre_buf_.getNumChannels() < numCh)
                filt_pre_buf_.setSize (juce::jmax (numCh, (int) AMBI_CHANNELS),
                                       NumSamples, false, false, true);
            applyVizFilters (buffer, filt_pre_buf_, numCh, NumSamples, hpfOn, lpfOn);
            preSrc = &filt_pre_buf_;
        }

        grid_energy_block_   .setZero();
        grid_energy_eq_block_.setZero();
        const int numGridEq = (int) Sh_matrix_eq_.rows();
        int taps = 0;
        for (int s = 0; s < NumSamples; s += stride)
        {
            for (int c = 0; c < numCh; ++c)
                input_scratch_(c) = (double) preSrc->getSample (c, s);
            // zero unused channels (in case channel count shrank dynamically)
            for (int c = numCh; c < input_scratch_.size(); ++c)
                input_scratch_(c) = 0.0;

            grid_decode_scratch_   .noalias() = Sh_matrix_viz_ * input_scratch_;
            grid_energy_block_    += grid_decode_scratch_.cwiseAbs2();

            grid_decode_eq_scratch_.noalias() = Sh_matrix_eq_  * input_scratch_;
            grid_energy_eq_block_ += grid_decode_eq_scratch_.cwiseAbs2();
            ++taps;
        }
        if (taps > 0)
        {
            const double inv = 1.0 / (double) taps;
            // User-controlled smoothing time constant (ms → seconds).
            const double blockSeconds = (double) NumSamples / sample_rate_;
            const double tauSeconds   = (double) std::max (1.0f,
                                            view_smoothing_ms.load (std::memory_order_relaxed)) * 0.001;
            const double alpha = std::exp (-blockSeconds / tauSeconds);
            const double oneMinusAlpha = 1.0 - alpha;

            const juce::SpinLock::ScopedLockType l (grid_energy_lock_);
            if ((int) grid_energy_smoothed_.size() != numGrid)
                grid_energy_smoothed_.assign ((size_t) numGrid, 0.f);
            if ((int) grid_energy_eq_smoothed_.size() != numGridEq)
                grid_energy_eq_smoothed_.assign ((size_t) numGridEq, 0.f);
            for (int g = 0; g < numGrid; ++g)
            {
                const double mean = grid_energy_block_(g) * inv;
                grid_energy_smoothed_[(size_t) g] = (float) (alpha * grid_energy_smoothed_[(size_t) g] + oneMinusAlpha * mean);
            }
            for (int g = 0; g < numGridEq; ++g)
            {
                const double mean = grid_energy_eq_block_(g) * inv;
                grid_energy_eq_smoothed_[(size_t) g] = (float) (alpha * grid_energy_eq_smoothed_[(size_t) g] + oneMinusAlpha * mean);
            }
        }
    }
    // ---------- end energy tap ----------

    output_buffer.clear();


    for (int out = 0; out < std::min(AMBI_CHANNELS,getTotalNumOutputChannels()); out++)
    {
        for (int in = 0; in < std::min(AMBI_CHANNELS,getTotalNumInputChannels()); in++)
        {
            if (_Sh_transf(in, out) != 0.f || Sh_transf(in, out) != 0.f)
            {
                if (_Sh_transf(in, out) == Sh_transf(in, out))
                {
                    output_buffer.addFrom(out, 0, buffer, in, 0, NumSamples, (float)Sh_transf(in, out));
                } else {
                    output_buffer.addFromWithRamp(out, 0, buffer.getReadPointer(in), NumSamples, (float)_Sh_transf(in, out), (float)Sh_transf(in, out));
                }

            }
        }
    }


    for (int ch = 0; ch < std::min(AMBI_CHANNELS, getTotalNumOutputChannels()); ++ch)
        buffer.copyFrom(ch, 0, output_buffer, ch, 0, NumSamples);

    // ---------- post-mod energy tap (output_buffer = post-Sh_transf signal) ----------
    if (_initialized && grid_energy_post_block_.size() > 0 && Sh_matrix_viz_.rows() > 0)
    {
        const int numCh = std::min ({ (int) AMBI_CHANNELS,
                                      getTotalNumOutputChannels(),
                                      (int) Sh_matrix_viz_.cols() });
        const int numGrid   = (int) Sh_matrix_viz_.rows();
        const int numGridEq = (int) Sh_matrix_eq_.rows();
        const int targetTaps = 16;
        const int stride = std::max (1, NumSamples / targetTaps);

        const bool hpfOn = view_vis_hpf_on.load (std::memory_order_relaxed);
        const bool lpfOn = view_vis_lpf_on.load (std::memory_order_relaxed);
        // Coefficients were already updated in the pre-mod tap above, so we
        // reuse them here. Apply with separate (per-signal) state.
        const juce::AudioBuffer<float>* postSrc = &output_buffer;
        if (hpfOn || lpfOn)
        {
            if (filt_post_buf_.getNumSamples() < NumSamples
                || filt_post_buf_.getNumChannels() < numCh)
                filt_post_buf_.setSize (juce::jmax (numCh, (int) AMBI_CHANNELS),
                                        NumSamples, false, false, true);
            applyVizFilters (output_buffer, filt_post_buf_, numCh, NumSamples, hpfOn, lpfOn);
            postSrc = &filt_post_buf_;
        }

        grid_energy_post_block_   .setZero();
        grid_energy_eq_post_block_.setZero();
        int taps = 0;
        for (int s = 0; s < NumSamples; s += stride)
        {
            for (int c = 0; c < numCh; ++c)
                input_scratch_(c) = (double) postSrc->getSample (c, s);
            for (int c = numCh; c < input_scratch_.size(); ++c)
                input_scratch_(c) = 0.0;

            grid_decode_scratch_      .noalias() = Sh_matrix_viz_ * input_scratch_;
            grid_energy_post_block_   += grid_decode_scratch_.cwiseAbs2();

            grid_decode_eq_scratch_   .noalias() = Sh_matrix_eq_  * input_scratch_;
            grid_energy_eq_post_block_+= grid_decode_eq_scratch_.cwiseAbs2();
            ++taps;
        }
        if (taps > 0)
        {
            const double inv = 1.0 / (double) taps;
            const double blockSeconds = (double) NumSamples / sample_rate_;
            const double tauSeconds   = (double) std::max (1.0f,
                                            view_smoothing_ms.load (std::memory_order_relaxed)) * 0.001;
            const double alpha = std::exp (-blockSeconds / tauSeconds);
            const double oneMinusAlpha = 1.0 - alpha;

            const juce::SpinLock::ScopedLockType l (grid_energy_lock_);
            if ((int) grid_energy_post_smoothed_.size() != numGrid)
                grid_energy_post_smoothed_.assign ((size_t) numGrid, 0.f);
            if ((int) grid_energy_eq_post_smoothed_.size() != numGridEq)
                grid_energy_eq_post_smoothed_.assign ((size_t) numGridEq, 0.f);
            for (int g = 0; g < numGrid; ++g)
            {
                const double mean = grid_energy_post_block_(g) * inv;
                grid_energy_post_smoothed_[(size_t) g] = (float) (alpha * grid_energy_post_smoothed_[(size_t) g] + oneMinusAlpha * mean);
            }
            for (int g = 0; g < numGridEq; ++g)
            {
                const double mean = grid_energy_eq_post_block_(g) * inv;
                grid_energy_eq_post_smoothed_[(size_t) g] = (float) (alpha * grid_energy_eq_post_smoothed_[(size_t) g] + oneMinusAlpha * mean);
            }
        }
    }
}

int Ambix_directional_loudnessAudioProcessor::getGridEnergySnapshot (std::vector<float>& out, bool postMod) const
{
    const juce::SpinLock::ScopedLockType l (grid_energy_lock_);
    out = postMod ? grid_energy_post_smoothed_ : grid_energy_smoothed_;
    return (int) out.size();
}

int Ambix_directional_loudnessAudioProcessor::getGridEnergySnapshotEq (std::vector<float>& out, bool postMod) const
{
    const juce::SpinLock::ScopedLockType l (grid_energy_lock_);
    out = postMod ? grid_energy_eq_post_smoothed_ : grid_energy_eq_smoothed_;
    return (int) out.size();
}

//==============================================================================
bool Ambix_directional_loudnessAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_directional_loudnessAudioProcessor::createEditor()
{
    return new Ambix_directional_loudnessAudioProcessorEditor (this);
    // return nullptr;
}

//==============================================================================
void Ambix_directional_loudnessAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // Create an outer XML element..

    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    for (int i=0; i < getNumParameters(); i++)
    {
        xml.setAttribute (String(i), getParameter(i));
    }
    xml.setAttribute("filter_sel_id_1", filter_sel_id_1);
    xml.setAttribute("filter_sel_id_2", filter_sel_id_2);

    xml.setAttribute("view_proj_ha",   view_proj_ha);
    xml.setAttribute("view_energy_on", view_energy_on);
    xml.setAttribute("view_autonorm",  view_autonorm);
    xml.setAttribute("view_pre_mod",   view_pre_mod);
    xml.setAttribute("view_range_db",  (double) view_range_db);
    xml.setAttribute("view_peak_db",   (double) view_peak_db);
    xml.setAttribute("view_smoothing_ms", (double) view_smoothing_ms.load());
    xml.setAttribute("view_vis_hpf_on", view_vis_hpf_on.load());
    xml.setAttribute("view_vis_lpf_on", view_vis_lpf_on.load());
    xml.setAttribute("view_vis_hpf_fc", (double) view_vis_hpf_fc.load());
    xml.setAttribute("view_vis_lpf_fc", (double) view_vis_lpf_fc.load());
    xml.setAttribute("view_colormap",  view_colormap);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_directional_loudnessAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            for (int i=0; i < getNumParameters(); i++) {
                setParameter(i, xmlState->getDoubleAttribute(String(i)));
            }
            filter_sel_id_1 = xmlState->getIntAttribute("filter_sel_id_1", 0);
            filter_sel_id_2 = xmlState->getIntAttribute("filter_sel_id_2", 0);

            view_proj_ha    = xmlState->getBoolAttribute  ("view_proj_ha",   view_proj_ha);
            view_energy_on  = xmlState->getBoolAttribute  ("view_energy_on", view_energy_on);
            view_autonorm   = xmlState->getBoolAttribute  ("view_autonorm",  view_autonorm);
            view_pre_mod    = xmlState->getBoolAttribute  ("view_pre_mod",   view_pre_mod);
            view_range_db   = (float) xmlState->getDoubleAttribute ("view_range_db", view_range_db);
            view_peak_db    = (float) xmlState->getDoubleAttribute ("view_peak_db",  view_peak_db);
            view_smoothing_ms.store ((float) xmlState->getDoubleAttribute ("view_smoothing_ms", (double) view_smoothing_ms.load()));
            view_vis_hpf_on.store (xmlState->getBoolAttribute ("view_vis_hpf_on", view_vis_hpf_on.load()));
            view_vis_lpf_on.store (xmlState->getBoolAttribute ("view_vis_lpf_on", view_vis_lpf_on.load()));
            view_vis_hpf_fc.store ((float) xmlState->getDoubleAttribute ("view_vis_hpf_fc", (double) view_vis_hpf_fc.load()));
            view_vis_lpf_fc.store ((float) xmlState->getDoubleAttribute ("view_vis_lpf_fc", (double) view_vis_lpf_fc.load()));
            view_colormap   = xmlState->getStringAttribute("view_colormap",  view_colormap);

            sendChangeMessage(); // editor refresh
        }

    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_directional_loudnessAudioProcessor();
}
