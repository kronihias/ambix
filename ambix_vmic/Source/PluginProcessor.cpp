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

#include "Ressources/t_design.h"


//==============================================================================
Ambix_vmicAudioProcessor::Ambix_vmicAudioProcessor() :
    AudioProcessor (BusesProperties()
        .withInput  ("Input",  AMBI_CH_SET(AMBI_CHANNELS), true)
        // Output is NUM_FILTERS_VMIC virtual-mic signals (not ambisonic), so the
        // count is independent of the input order.
        .withOutput ("Output", juce::AudioChannelSet::discreteChannels (NUM_FILTERS_VMIC), true)
    ),
    filter_sel_id_1(0),
    filter_sel_id_2(0),
    _initialized(false),
    _param_changed(false),
    output_buffer(AMBI_CHANNELS,256)
{
    // init parameters - order, names, texts and initial values as the
    // legacy index-based API had them. Every value is stored as set (no
    // snapping); the texts format it the way getParameterText() did.
    auto degText360 = [] (float v, int) // center_sph was a double array
    {
        String text = String(((double) v - 0.5)*360).substring(0, 5);
        text << " deg";
        return text;
    };
    auto degText180 = [] (float v, int)
    {
        String text = String(v*180).substring(0, 5);
        text << " deg";
        return text;
    };
    auto shapeText = [] (float v, int) { return String (v <= 0.5 ? "circular" : "rectangular"); };
    auto gainText = [] (float v, int)
    {
        String text = String(ParamToDB(v)).substring(0, 5);
        text << " dB";
        return text;
    };
    auto windowText = [] (float v, int) { return String (v <= 0.5 ? "no" : "yes"); };

    for (int f = 0; f < NUM_FILTERS_VMIC; f++)
    {
        auto add = [this, f] (int param, const char* name, float initial, ambix::Parameter::TextFunction text)
        {
            const int index = PARAMS_PER_FILTER*f + param;
            String paramName (name);
            paramName << " " << f+1;
            // (on VST3, parameter 8 gets an ID of its own: see idFor)
            addParameter (params[index] = new ambix::Parameter (*this, index, paramName, initial, std::move (text), {},
                                                                ambix::Parameter::idFor (*this, index, PARAMS_PER_FILTER*NUM_FILTERS_VMIC)));
        };

        add (AzimuthParam,   "center azimuth",   0.5f, degText360); // 0 deg
        add (ElevationParam, "center elevation", 0.5f, degText360); // 0 deg
        add (ShapeParam,     "shape",            0.f,  shapeText);  // circular
        add (WidthParam,     "width",            0.1f, degText180); // +-18°
        add (HeightParam,    "height",           0.1f, degText180); // +-18°
        add (GainParam,      "gain",             0.5f, gainText);   // 0 dB
        add (WindowParam,    "window",           0.f,  windowText); // not used
    }

    // set transformation matrix to zero matrix
    Sh_transf = Eigen::MatrixXd::Zero(NUM_FILTERS_VMIC, AMBI_CHANNELS);
    _Sh_transf = Eigen::MatrixXd::Zero(NUM_FILTERS_VMIC, AMBI_CHANNELS);
}

Ambix_vmicAudioProcessor::~Ambix_vmicAudioProcessor()
{

}

//==============================================================================
const String Ambix_vmicAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void Ambix_vmicAudioProcessor::parameterValueSet (int index, float)
{
    // the value itself lives in the parameter object; calcParams() reads it.
    // Every set recalculates, as legacy setParameter() did - also a restore
    // of the value the parameter already had.
    int filter_id = (int)floor(index/PARAMS_PER_FILTER);

    // flag after the write so the audio thread picks up the new value
    if (filter_id < NUM_FILTERS_VMIC) // safety..
        _param_changed = true;
}

const String Ambix_vmicAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String Ambix_vmicAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool Ambix_vmicAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_vmicAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_vmicAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_vmicAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_vmicAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_vmicAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ambix_vmicAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_vmicAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_vmicAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_vmicAudioProcessor::getProgramName (int index)
{
    return String();
}

void Ambix_vmicAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Ambix_vmicAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    output_buffer.setSize (getTotalNumOutputChannels(), samplesPerBlock, false, false, false);
    _initialized = false;
    calcParams();
}

void Ambix_vmicAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_vmicAudioProcessor::calcParams()
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

            Sh_matrix.block(i,0,1,AMBI_CHANNELS) = Ymn.transpose();
        }


        _initialized = true;

    }

    if (_param_changed.exchange(false))
    {
        // one read of every parameter (atomics), in place of the arrays
        // legacy setParameter() wrote from the setting thread
        ArrayParam shape;
        ArrayParam width;
        ArrayParam height;
        ArrayParam gain;
        SphCoordParam center_sph;

        for (int k=0; k < NUM_FILTERS_VMIC; k++)
        {
            center_sph(k, 0) = getParam(k, AzimuthParam).get();
            center_sph(k, 1) = getParam(k, ElevationParam).get();
            shape(k) = getParam(k, ShapeParam).get();
            width(k) = getParam(k, WidthParam).get();
            height(k) = getParam(k, HeightParam).get();
            gain(k) = getParam(k, GainParam).get();
        }

        const float transition = 0.f; // not used yet (never a parameter)

        // convert parameters to values for the filter
        // ArrayIntParam _shape = shape;

        ArrayParam _width = width * (float)M_PI; // 0...pi
        ArrayParam _height = height * (float)M_PI;

        ArrayParam _gain;

        for (int i=0; i < gain.rows();i++)
        {
            _gain(i) = ParamToRMS(gain(i));
        }

        SphCoordParam _center_sph = (center_sph - 0.5f)*2.f*M_PI;

        // std::cout << _center_sph << std::endl;

        CarthCoordParam _center_carth;

        // convert center spherical coordinates to carthesian
        for (int i=0; i < _center_sph.rows(); i++)
        {
            _center_carth(i,0) = cos(_center_sph(i,0))*cos(_center_sph(i,1)); // x
            _center_carth(i,1) = sin(_center_sph(i,0))*cos(_center_sph(i,1)); // y
            _center_carth(i,2) = sin(_center_sph(i,1)); // z
        }

        Sh_matrix_mod.setZero();

        // iterate over all filters
        for (int k=0; k < NUM_FILTERS_VMIC; k++)
        {
            // iterate over all sample points
            for (int i=0; i < Sh_matrix.rows(); i++)
            {

                Eigen::Vector2d Sph_coord_vec = Sph_coord.row(i);
                Eigen::Vector2d _center_sph_vec = _center_sph.row(k);


                double multipl = sph_filter.GetWeight(&Sph_coord_vec, Carth_coord.row(i), &_center_sph_vec, _center_carth.row(k), (int)floor(shape(k)+0.5f), _width(k), _height(k), 1, true, transition);

                if (multipl < 0.f) // -1.f in case of outside region
                    multipl = 0.f;

                // Sh_matrix_mod row is the weighted sum over all sample points
                Sh_matrix_mod.row(k) += multipl * Sh_matrix.row(i);
            }

            // normalize and apply gain
            if (Sh_matrix_mod.row(k)(0) > 0.f)
                Sh_matrix_mod.row(k) *= 1/Sh_matrix_mod.row(k)(0)*_gain(k);
        }

        // std::cout << "Size Sh_matrix_mod : " << Sh_matrix_mod.rows() << " x " << Sh_matrix_mod.cols() << std::endl;

        // std::cout << Sh_matrix_mod << std::endl;


        // this is the new transformation matrix
        Sh_transf = Sh_matrix_mod;

        // threshold coefficients
        for (int i = 0; i < Sh_transf.size(); i++)
        {
            if (abs(Sh_transf(i)) < 0.00001f)
                Sh_transf(i) = 0.f;
        }
    }


}

bool Ambix_vmicAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#ifdef UNIVERSAL_AMBISONIC
    return true;
#else
    return ((layouts.getMainOutputChannelSet().size() == AMBI_CHANNELS) &&
            (layouts.getMainInputChannelSet().size() == AMBI_CHANNELS));
#endif
}

void Ambix_vmicAudioProcessor::numChannelsChanged()
{
#ifdef UNIVERSAL_AMBISONIC
    _initialized = false;
    sendChangeMessage();
#endif
}

void Ambix_vmicAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    _Sh_transf = Sh_transf; // buffer old values

    calcParams(); // calc new transformation matrix

    int NumSamples = buffer.getNumSamples();

    output_buffer.clear();


    for (int out = 0; out < std::min(NUM_FILTERS_VMIC,getTotalNumOutputChannels()); out++)
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

    for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        buffer.copyFrom(ch, 0, output_buffer, ch, 0, NumSamples);
}

//==============================================================================
bool Ambix_vmicAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* Ambix_vmicAudioProcessor::createEditor()
{
    return new Ambix_vmicAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_vmicAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // Create an outer XML element..

    XmlElement xml ("MYPLUGINSETTINGS");

    // add some attributes to it..
    for (int i=0; i < PARAMS_PER_FILTER*NUM_FILTERS_VMIC; i++)
    {
        xml.setAttribute (String(i), (double) params[i]->get());
    }
    xml.setAttribute("filter_sel_id_1", filter_sel_id_1);
    xml.setAttribute("filter_sel_id_2", filter_sel_id_2);

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_vmicAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // legacy setParameter(): side effects, no host notification
            for (int i=0; i < PARAMS_PER_FILTER*NUM_FILTERS_VMIC; i++) {
                params[i]->restore((float) xmlState->getDoubleAttribute(String(i)));
            }
            filter_sel_id_1 = xmlState->getIntAttribute("filter_sel_id_1", 0);
            filter_sel_id_2 = xmlState->getIntAttribute("filter_sel_id_2", 0);
        }

    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_vmicAudioProcessor();
}
