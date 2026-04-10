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
#include "t_design.h"

#ifdef _WIN32
inline double round (double d)
{
    return floor (d + 0.5);
}
#endif

//==============================================================================
Ambix_moveAudioProcessor::Ambix_moveAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET (AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET (AMBI_CHANNELS), true)
    ),
    x_param (0.5f),       _x_param (-10.f),
    y_param (0.5f),       _y_param (-10.f),
    z_param (0.5f),       _z_param (-10.f),
    radius_param (0.f),   _radius_param (-10.f),
    in_order_param (1.f),
    out_order_param (1.f),
    in_order (AMBI_ORDER),
    _in_order (-1),
    out_order (AMBI_ORDER),
    _out_order (-1),
    _initialized (false),
    output_buffer (AMBI_CHANNELS, 256)
{
    // start as identity (pass-through) until calcParams() runs
    Sh_transf  = Eigen::MatrixXd::Identity (AMBI_CHANNELS, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Identity (AMBI_CHANNELS, AMBI_CHANNELS);
}

Ambix_moveAudioProcessor::~Ambix_moveAudioProcessor()
{
}

//==============================================================================
const String Ambix_moveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_moveAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float Ambix_moveAudioProcessor::getParameter (int index)
{
    switch (index)
    {
        case XParam:        return x_param;
        case YParam:        return y_param;
        case ZParam:        return z_param;
        case RadiusParam:   return radius_param;
        case InOrderParam:  return in_order_param;
        case OutOrderParam: return out_order_param;
        default:            return 0.0f;
    }
}

void Ambix_moveAudioProcessor::setParameter (int index, float newValue)
{
    switch (index)
    {
        case XParam:        x_param = newValue;      break;
        case YParam:        y_param = newValue;      break;
        case ZParam:        z_param = newValue;      break;
        case RadiusParam:   radius_param = newValue; break;

        case InOrderParam:
            in_order_param = newValue;
            in_order = (int) round (in_order_param * AMBI_ORDER);
            _initialized = false; // reinit SH basis at new order
            break;

        case OutOrderParam:
            out_order_param = newValue;
            out_order = (int) round (out_order_param * AMBI_ORDER);
            break;

        default:
            break;
    }
    sendChangeMessage();
}

const String Ambix_moveAudioProcessor::getParameterName (int index)
{
    switch (index)
    {
        case XParam:        return "X";
        case YParam:        return "Y";
        case ZParam:        return "Z";
        case RadiusParam:   return "Reference radius";
        case InOrderParam:  return "Ambi In order";
        case OutOrderParam: return "Ambi Out order";
        default:            break;
    }
    return String();
}

const String Ambix_moveAudioProcessor::getParameterText (int index)
{
    String text;
    switch (index)
    {
        case XParam:        text = String (xMetersFromParam (x_param), 3) + " m"; break;
        case YParam:        text = String (xMetersFromParam (y_param), 3) + " m"; break;
        case ZParam:        text = String (xMetersFromParam (z_param), 3) + " m"; break;
        case RadiusParam:   text = String (radiusMetersFromParam (radius_param), 2) + " m"; break;
        case InOrderParam:  text = String (in_order); break;
        case OutOrderParam: text = String (out_order); break;
        default: break;
    }
    return text;
}

const String Ambix_moveAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_moveAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_moveAudioProcessor::isInputChannelStereoPair (int) const  { return true; }
bool Ambix_moveAudioProcessor::isOutputChannelStereoPair (int) const { return true; }

bool Ambix_moveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_moveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_moveAudioProcessor::silenceInProducesSilenceOut() const { return false; }
double Ambix_moveAudioProcessor::getTailLengthSeconds() const     { return 0.0; }

int Ambix_moveAudioProcessor::getNumPrograms()                    { return 0; }
int Ambix_moveAudioProcessor::getCurrentProgram()                 { return 0; }
void Ambix_moveAudioProcessor::setCurrentProgram (int)            {}
const String Ambix_moveAudioProcessor::getProgramName (int)       { return String(); }
void Ambix_moveAudioProcessor::changeProgramName (int, const String&) {}

//==============================================================================
void Ambix_moveAudioProcessor::prepareToPlay (double, int)
{
}

void Ambix_moveAudioProcessor::releaseResources()
{
}

void Ambix_moveAudioProcessor::calcParams()
{
#ifdef UNIVERSAL_AMBISONIC
    // Cap order to active channel count from host
    int activeOrder = ambiOrderFromChannels (getTotalNumInputChannels());
    if (activeOrder > 0)
    {
        in_order  = juce::jmin (in_order,  activeOrder);
        out_order = juce::jmin (out_order, activeOrder);
    }
#endif

    if (! _initialized)
    {
        sph_h_in.Init (in_order);

        // Parse the t-design sampling points (240 points on the unit sphere)
        const String t_design_txt (t_design::des_3_240_21_txt);

        String::CharPointerType lineChar = t_design_txt.getCharPointer();

        int n = 0;
        int numsamples = 0;
        int curr_n = 0;
        int max_n  = lineChar.length();

        while (curr_n < max_n)
        {
            double value;
            sscanf (lineChar, "%lf\n%n", &value, &n);
            lineChar += n;
            curr_n += n;
            numsamples++;
        }

        numsamples /= 3; // x,y,z triples

        Carth_coord.resize (numsamples, 3);

        lineChar = t_design_txt.getCharPointer();

        for (int i = 0; i < numsamples; ++i)
        {
            double x, y, z;
            sscanf (lineChar, "%lf%lf%lf%n", &x, &y, &z, &n);
            Carth_coord (i, 0) = x;
            Carth_coord (i, 1) = y;
            Carth_coord (i, 2) = z;
            lineChar += n;
        }

        // Build the undistorted SH basis at the sampling points and compute its
        // pseudo-inverse — this stays fixed regardless of listener position.
        Eigen::MatrixXd Sh_matrix;
        Sh_matrix.setZero (numsamples, AMBI_CHANNELS);

        const int in_ambi_channels = (in_order + 1) * (in_order + 1);
        for (int i = 0; i < numsamples; ++i)
        {
            const double az = atan2 (Carth_coord (i, 1), Carth_coord (i, 0));
            const double el = atan2 (Carth_coord (i, 2),
                                     sqrt (Carth_coord (i, 0) * Carth_coord (i, 0)
                                         + Carth_coord (i, 1) * Carth_coord (i, 1)));

            Eigen::VectorXd Ymn (in_ambi_channels);
            sph_h_in.Calc (az, el);
            sph_h_in.Get  (Ymn);

            Sh_matrix.block (i, 0, 1, in_ambi_channels) = Ymn.transpose();
        }

        Sh_matrix_inv = (Sh_matrix.transpose() * Sh_matrix)
                            .colPivHouseholderQr().inverse() * Sh_matrix.transpose();

        _initialized = true;
    }

    // Recompute the transformation matrix only if any relevant parameter changed
    if ((x_param      != _x_param)      ||
        (y_param      != _y_param)      ||
        (z_param      != _z_param)      ||
        (radius_param != _radius_param) ||
        (in_order     != _in_order)     ||
        (out_order    != _out_order))
    {
        if (_out_order != out_order)
            sph_h_out.Init (out_order);

        const int numsamples       = (int) Carth_coord.rows();
        const int out_ambi_channels = (out_order + 1) * (out_order + 1);

        const double R  = radiusMetersFromParam (radius_param);
        const double lx = xMetersFromParam (x_param);
        // Y is negated so that moving the listener "right" (as per the GUI
        // and DAW convention) actually translates toward ambix -Y. The SH
        // library used by the suite has Y_1,-1 = -sin(az)*cos(el) (i.e. the
        // standard ambix sign is flipped), which makes a mathematically
        // correct left/right translation read inverted in downstream tools.
        // Flipping the parameter here keeps the rest of the chain consistent
        // without touching the GUI labels or the shared SH code.
        const double ly = -xMetersFromParam (y_param);
        const double lz = xMetersFromParam (z_param);

        Eigen::MatrixXd Sh_matrix_mod;
        Sh_matrix_mod.setZero (numsamples, AMBI_CHANNELS);

        for (int i = 0; i < numsamples; ++i)
        {
            // Sampling point on the reference sphere (world frame).
            // Coordinate convention: x = front, y = left, z = up.
            const double sx = R * Carth_coord (i, 0);
            const double sy = R * Carth_coord (i, 1);
            const double sz = R * Carth_coord (i, 2);

            // Vector from translated listener to sample
            const double dx = sx - lx;
            const double dy = sy - ly;
            const double dz = sz - lz;
            const double newDist = std::sqrt (dx * dx + dy * dy + dz * dz);

            // Avoid div-by-zero singularity if listener exactly hits a sample
            const double safeDist = (newDist < 1.0e-6) ? 1.0e-6 : newDist;

            // New arrival direction in ambix convention
            //   azimuth = atan2(y, x)  -> CCW from front
            //   elevation = asin(z / r)
            const double newAz = std::atan2 (dy, dx);
            const double newEl = std::asin  (dz / safeDist);

            // 1/r gain compensation. Unity when listener is at the origin
            // (newDist == R). No delay compensation; c = speedOfSound is left
            // documented for future use.
            const double gain = R / safeDist;

            Eigen::VectorXd Ymn (out_ambi_channels);
            sph_h_out.Calc (newAz, newEl);
            sph_h_out.Get  (Ymn);

            Sh_matrix_mod.block (i, 0, 1, out_ambi_channels) = gain * Ymn.transpose();
        }

        // Final transformation matrix.
        //
        // Sh_matrix_inv is (in_ch x N) and Sh_matrix_mod is (N x out_ch), so
        // the product is (in_ch x out_ch) — i.e. element [i, j] is the
        // contribution of input channel i to output channel j. processBlock
        // however indexes the matrix as Sh_transf(out, in), so we transpose
        // here to get an (out_ch x in_ch) matrix that lines up with that
        // indexing convention. At zero translation Sh_transf is the identity
        // (symmetric), which is why the missing transpose only manifests as a
        // bug once the listener is actually moved.
        Sh_transf = (Sh_matrix_inv * Sh_matrix_mod).transpose();

        // threshold tiny values to zero (lets the inner loop in processBlock
        // skip them entirely)
        for (int i = 0; i < Sh_transf.size(); ++i)
        {
            if (std::abs (Sh_transf (i)) < 0.00001)
                Sh_transf (i) = 0.0;
        }

        _x_param      = x_param;
        _y_param      = y_param;
        _z_param      = z_param;
        _radius_param = radius_param;
        _in_order     = in_order;
        _out_order    = out_order;
    }
}

bool Ambix_moveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size()  == AMBI_CHANNELS));
#endif
}

void Ambix_moveAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    _initialized = false;
    sendChangeMessage();
#endif
}

void Ambix_moveAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer&)
{
    _Sh_transf = Sh_transf; // buffer old values for ramping

    calcParams();

    const int NumSamples = buffer.getNumSamples();

    output_buffer.setSize (buffer.getNumChannels(), NumSamples);
    output_buffer.clear();

    for (int out = 0; out < std::min (AMBI_CHANNELS, getTotalNumOutputChannels()); ++out)
    {
        for (int in = 0; in < std::min (AMBI_CHANNELS, getTotalNumInputChannels()); ++in)
        {
            if (_Sh_transf (out, in) != 0.0 || Sh_transf (out, in) != 0.0)
            {
                if (_Sh_transf (out, in) == Sh_transf (out, in))
                {
                    output_buffer.addFrom (out, 0, buffer, in, 0, NumSamples,
                                           (float) Sh_transf (out, in));
                }
                else
                {
                    output_buffer.addFromWithRamp (out, 0, buffer.getReadPointer (in), NumSamples,
                                                   (float) _Sh_transf (out, in),
                                                   (float) Sh_transf  (out, in));
                }
            }
        }
    }

    buffer = output_buffer;
}

//==============================================================================
bool Ambix_moveAudioProcessor::hasEditor() const { return true; }

AudioProcessorEditor* Ambix_moveAudioProcessor::createEditor()
{
    return new Ambix_moveAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_moveAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml ("MYPLUGINSETTINGS");

    for (int i = 0; i < getNumParameters(); ++i)
        xml.setAttribute (String (i), getParameter (i));

    copyXmlToBinary (xml, destData);
}

void Ambix_moveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName ("MYPLUGINSETTINGS"))
    {
        for (int i = 0; i < getNumParameters(); ++i)
            setParameter (i, xmlState->getDoubleAttribute (String (i)));
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_moveAudioProcessor();
}
