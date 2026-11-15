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
inline double round( double d )
{
    return floor( d + 0.5 );
}
#endif

//==============================================================================
Ambix_warpAudioProcessor::Ambix_warpAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET(AMBI_CHANNELS), true)
        .withOutput ("Output", AMBI_CH_SET(AMBI_CHANNELS), true)
    ),
    _phi_param(-10.f),
    _phi_curve_param(0.f),
    _theta_param(-10.f),
    _theta_curve_param(0.f),
    _preemp_param(1.f),
    _in_order(-1),
    _out_order(-1),
    _initialized(false),
    output_buffer(AMBI_CHANNELS,256)
{
    // legacy display texts, as getParameterText() built them
    auto factorText = [] (float v, int) -> String { return String(v*1.8f-0.9f).substring(0, 5); };
    auto phiCurve   = [] (float v, int) -> String { return (v <= 0.5f) ? "-90 / 90 deg" : "180 / 0 deg"; };
    auto thetaCurve = [] (float v, int) -> String { return (v <= 0.5f) ? "northpole" : "equator"; };
    auto orderText  = [this] (float v, int) -> String { return String (orderForDisplay (v)); };
    auto onOff      = [] (float v, int) -> String { return (v <= 0.5f) ? "Off" : "On"; };

    addParameter (params[PhiParam]        = new ambix::Parameter (*this, PhiParam,        "Az Warp factor", 0.5f, factorText));
    addParameter (params[PhiCurveParam]   = new ambix::Parameter (*this, PhiCurveParam,   "Az Warp curve",  0.f,  phiCurve));
    addParameter (params[ThetaParam]      = new ambix::Parameter (*this, ThetaParam,      "El Warp factor", 0.5f, factorText));
    addParameter (params[ThetaCurveParam] = new ambix::Parameter (*this, ThetaCurveParam, "El Warp curve",  0.f,  thetaCurve));
    addParameter (params[InOrderParam]    = new ambix::Parameter (*this, InOrderParam,    "Ambi In order",  1.f,  orderText));
    addParameter (params[OutOrderParam]   = new ambix::Parameter (*this, OutOrderParam,   "Ambi Out order", 1.f,  orderText));
    addParameter (params[PreEmpParam]     = new ambix::Parameter (*this, PreEmpParam,     "PreEmphasis",    1.f,  onOff));

    // sampling points (t-design), parsed once
    {
        const String t_design_txt (t_design::des_3_240_21_txt);

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
    }

    const int numsamples = (int) Carth_coord.rows();

    Sph_coord.resize(numsamples,2); // positions in spherical coordinates

    for (int i=0; i < numsamples; i++)
    {
        Sph_coord(i,0) = atan2(Carth_coord(i,1),Carth_coord(i,0)); // azimuth
        Sph_coord(i,1) = atan2(Carth_coord(i,2),sqrt(Carth_coord(i,0)*Carth_coord(i,0) + Carth_coord(i,1)*Carth_coord(i,1))); // elevation
    }

    // the undistorted SH matrix inverse for every possible input order, and
    // the output spherical harmonics for every output order
    Sh_matrix_inv_for_order.resize(AMBI_ORDER+1);

    for (int order = 0; order <= AMBI_ORDER; order++)
    {
        SphericalHarmonic sph_h_in;
        sph_h_in.Init(order);

        Eigen::MatrixXd Sh_matrix;
        Sh_matrix.setZero(numsamples,AMBI_CHANNELS);

        int in_ambi_channels = (order+1)*(order+1);
        for (int i=0; i < numsamples; i++)
        {
            sph_h_in.Calc(Sph_coord(i,0),Sph_coord(i,1)); // phi theta

            Sh_matrix.block(i,0,1,in_ambi_channels) = sph_h_in.Ymn.transpose();
        }

        // if input order is different a better solving has to be used for the inverse:
        Sh_matrix_inv_for_order[(size_t) order] = (Sh_matrix.transpose()*Sh_matrix).colPivHouseholderQr().inverse()*Sh_matrix.transpose();

        sph_h_out_for_order[(size_t) order].Init(order);
    }

    // sized once here so calcParams() does not allocate
    Sh_matrix_mod.setZero(numsamples,AMBI_CHANNELS);
    pre_emphasis.setOnes(numsamples);
    Sph_coord_mod.resize(numsamples,2);

    // set transformation matrix to identity matrix
	Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
	_Sh_transf = Eigen::MatrixXd::Identity(AMBI_CHANNELS, AMBI_CHANNELS);
}

Ambix_warpAudioProcessor::~Ambix_warpAudioProcessor()
{
}

//==============================================================================
const String Ambix_warpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void Ambix_warpAudioProcessor::parameterValueSet (int index, float newValue)
{
    switch (index)
	{

		default:
            break;
	}
}

int Ambix_warpAudioProcessor::orderForDisplay (float orderParam)
{
    int order = (int)round(orderParam*AMBI_ORDER);
#ifdef UNIVERSAL_AMBISONIC
    int activeOrder = ambiOrderFromChannels (getTotalNumInputChannels());
    if (activeOrder > 0)
        order = juce::jmin (order, activeOrder);
#endif
    return order;
}

const String Ambix_warpAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_warpAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_warpAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_warpAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_warpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_warpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_warpAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_warpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_warpAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_warpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_warpAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_warpAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_warpAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_warpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    output_buffer.setSize (getTotalNumOutputChannels(), samplesPerBlock, false, false, false);
    _initialized = false;
    calcParams();
}

void Ambix_warpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_warpAudioProcessor::calcParams()
{
    // one atomic read per parameter and block
    const float phi_param         = params[PhiParam]->get();
    const float phi_curve_param   = params[PhiCurveParam]->get();
    const float theta_param       = params[ThetaParam]->get();
    const float theta_curve_param = params[ThetaCurveParam]->get();
    const float preemp_param      = params[PreEmpParam]->get();

    // orders read afresh each block and capped to the host's channel count;
    // never written back, so a concurrent set from another thread isn't lost
    int inOrder  = (int)round(params[InOrderParam]->get()*AMBI_ORDER);
    int outOrder = (int)round(params[OutOrderParam]->get()*AMBI_ORDER);

#ifdef UNIVERSAL_AMBISONIC
    const int activeOrder = ambiOrderFromChannels (getTotalNumInputChannels());
    if (activeOrder > 0) {
        inOrder = juce::jmin (inOrder, activeOrder);
        outOrder = juce::jmin (outOrder, activeOrder);
    }
#endif

    if (!_initialized.exchange (true) || inOrder != _in_order)
    {
        // pick the inverse built for this input order (constructor)
        Sh_matrix_inv = &Sh_matrix_inv_for_order[(size_t) inOrder];
    }

    if ( (phi_param != _phi_param) ||
        (floor(phi_curve_param+0.5f) != floor(_phi_curve_param+0.5f)) ||
        (theta_param != _theta_param) ||
        (floor(theta_curve_param+0.5f) != floor(_theta_curve_param+0.5f)) ||
        (inOrder != _in_order) ||
        (outOrder != _out_order) ||
        (floor(preemp_param+0.5f) != floor(_preemp_param+0.5f))
        )
    {

        if (_out_order != outOrder)
        {
            // spherical harmonics for this order, initialised in the constructor
            sph_h_out = &sph_h_out_for_order[(size_t) outOrder];
        }

        Sh_matrix_mod.setZero();

        pre_emphasis.setOnes();

        Sph_coord_mod = Sph_coord; // same size -> no reallocation

        // warping parameters - use inverse parameters! (see paper)
        double phi_alpha = -((double)phi_param*1.8f - 0.9f);
        double theta_alpha = -((double)theta_param*1.8f - 0.9f);

        int out_ambi_channels = (outOrder+1)*(outOrder+1);

        for (int i=0; i < Sph_coord_mod.rows(); i++)
        {
            if (theta_alpha != 0.)
            {
                // warp elevation
                // as we are using elevation from -pi/2....pi/2
                double mu = sin(Sph_coord(i,1));

                if (theta_curve_param <= 0.5f)
                {
                    // warp towards a pole

                    Sph_coord_mod(i,1) = asin((mu+theta_alpha)/(1+theta_alpha*mu));

                    // pre emphasis
                    pre_emphasis(i) = (1+theta_alpha*mu)/sqrt(1-theta_alpha*theta_alpha);
                } else {
                    //warp towards equator (< 0.)or away from equator (> 0.)
                    // in the paper this parameter is called beta

                    // pre emphasis
                    pre_emphasis(i) = sqrt((1-abs(theta_alpha))*(1+abs(theta_alpha)*mu*mu))/(1-abs(theta_alpha)*mu*mu);

                    if (theta_alpha > 0.) {

                        Sph_coord_mod(i,1) = asin((theta_alpha-1+sqrt(pow(theta_alpha-1,2)+4*theta_alpha*pow(mu,2)))/(2*theta_alpha*mu));

                    } else {
                        Sph_coord_mod(i,1) = asin(((1+theta_alpha)*mu)/(1+theta_alpha*pow(mu,2)));
                        pre_emphasis(i) = 1/pre_emphasis(i); // this is ^sgn(beta)
                    }

                }
            }
            /*
             // this is not fully working yet
            if (phi_alpha != 0.)
            {
                // warp azimuth
                // as we are using azimuth from -pi....pi
                double phi_temp = Sph_coord(i,0) * 0.5f;

                double mu = sin(phi_temp);

                if (phi_curve_param <= 0.5f)
                {
                    // warp towards a pole

                    Sph_coord_mod(i,0) = asin((mu+phi_alpha)/(1+phi_alpha*mu));

                    // pre emphasis
                    pre_emphasis(i) *= (1+phi_alpha*mu)/sqrt(1-phi_alpha*phi_alpha);
                } else {
                    //warp towards equator (< 0.)or away from equator (> 0.)
                    // in the paper this parameter is called beta

                    // pre emphasis
                    double pre_emph_temp = sqrt((1-abs(phi_alpha))*(1+abs(phi_alpha)*mu*mu))/(1-abs(phi_alpha)*mu*mu);

                    if (phi_alpha > 0.) {

                        Sph_coord_mod(i,0) = asin((phi_alpha-1+sqrt(pow(phi_alpha-1,2)+4*phi_alpha*pow(mu,2)))/(2*phi_alpha*mu));
                        pre_emphasis(i) *= pre_emph_temp;

                    } else {
                        Sph_coord_mod(i,0) = asin(((1+phi_alpha)*mu)/(1+phi_alpha*pow(mu,2)));
                        pre_emphasis(i) *= 1/pre_emph_temp; // this is ^sgn(beta)
                    }

                }
                Sph_coord_mod(i,0) *= 2;
            }
            */

            if (phi_alpha != 0.f)
            {
                // warp azimuth
                // as we are using azimuth from -pi....pi
                double phi_temp = Sph_coord(i,0) * 0.5f;

                if (phi_curve_param <= 0.5f)
                {
                    // warp towards poles
                    Sph_coord_mod(i,0) = asin((sin(phi_temp)+phi_alpha)/(1.f+phi_alpha*sin(phi_temp)));
                } else {
                    //warp towards equator

                    if (phi_temp > 0)
                    {
                        Sph_coord_mod(i,0) = acos((cos(phi_temp*2)+phi_alpha)/(1+phi_alpha*cos(phi_temp*2))) / 2;
                    } else {
                        Sph_coord_mod(i,0) = -acos((cos(M_PI-(phi_temp-M_PI/2)*2)+phi_alpha)/(1+phi_alpha*cos(M_PI-(phi_temp-M_PI/2)*2))) / 2;
                    }
                }

                Sph_coord_mod(i,0) *= 2;

            }

            // rotate
            // Sph_mod(i,0) = Sph(i,0);

            sph_h_out->Calc(Sph_coord_mod(i,0),Sph_coord_mod(i,1)); // phi theta

            // Sh_matrix_mod.row(i) = Ymn;
            Sh_matrix_mod.block(i,0,1,out_ambi_channels) = sph_h_out->Ymn.transpose();
        }

        // apply preemphasis if wanted
        if (preemp_param > 0.5f)
        {
            // == pre_emphasis.asDiagonal() * Sh_matrix_mod, in place (row i scaled by pre_emphasis(i))
            Sh_matrix_mod.array().colwise() *= pre_emphasis.array();
        }

        // calculate new transformation matrix

        Sh_transf.noalias() = *Sh_matrix_inv * Sh_matrix_mod;

        // threshold coefficients
        for (int i = 0; i < Sh_transf.size(); i++)
        {
            if (abs(Sh_transf(i)) < 0.00001f)
                Sh_transf(i) = 0.f;
        }

        _phi_param = phi_param;
        _phi_curve_param = phi_curve_param;
        _theta_param = theta_param;
        _theta_curve_param = theta_curve_param;
        _in_order = inOrder;
        _out_order = outOrder;
        _preemp_param = preemp_param;
    }



}

bool Ambix_warpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
#endif
}

void Ambix_warpAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    _initialized = false;
    sendChangeMessage();
#endif
}

void Ambix_warpAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    _Sh_transf = Sh_transf; // buffer old values

    calcParams(); // calc new transformation matrix

    int NumSamples = buffer.getNumSamples();

    output_buffer.clear();


    for (int out = 0; out < std::min(AMBI_CHANNELS,getTotalNumOutputChannels()); out++)
    {
        for (int in = 0; in < std::min(AMBI_CHANNELS,getTotalNumInputChannels()); in++)
        {
            if (_Sh_transf(out, in) != 0.f || Sh_transf(out, in) != 0.f)
            {
                if (_Sh_transf(out, in) == Sh_transf(out, in))
                {
                    output_buffer.addFrom(out, 0, buffer, in, 0, NumSamples, (float)Sh_transf(out, in));
                } else {
                    output_buffer.addFromWithRamp(out, 0, buffer.getReadPointer(in), NumSamples, (float)_Sh_transf(out, in), (float)Sh_transf(out, in));
                }

            }
        }
    }


    for (int ch = 0; ch < std::min(AMBI_CHANNELS, getTotalNumOutputChannels()); ++ch)
        buffer.copyFrom(ch, 0, output_buffer, ch, 0, NumSamples);

}

//==============================================================================
bool Ambix_warpAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* Ambix_warpAudioProcessor::createEditor()
{
    return new Ambix_warpAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_warpAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // Create an outer XML element..

    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it.. (legacy layout, so older versions can read projects)
    for (int i=0; i < totalNumParams; i++)
    {
        xml.setAttribute (String(i), params[i]->get());
    }

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_warpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // restore() runs the side effects like the legacy setParameter() did
            for (int i=0; i < totalNumParams; i++) {
                params[i]->restore ((float) xmlState->getDoubleAttribute(String(i)));
            }
        }

    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_warpAudioProcessor();
}
