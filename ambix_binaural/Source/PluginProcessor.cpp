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
#include "SphericalHarmonic/normalization.h"
#include "SphericalHarmonic/ch_cs.h"
#include "SphericalHarmonic/ch_sequence.h"
#include "SphericalHarmonic/tools.h"

#define LOGTEN 2.302585092994

#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

//==============================================================================
Ambix_binauralAudioProcessor::Ambix_binauralAudioProcessor() :
                                _AmbiChannels(0),
                                configLoaded(false),
                                box_preset_str("no preset loaded"),
                                _load_ir(true),
                                SampleRate(44100),
                                isProcessing(false),
                                _gain(0.5f)

{
    presetDir = presetDir.getSpecialLocation(File::userApplicationDataDirectory).getChildFile("ambix/binaural_presets");
    std::cout << "Search dir:" << presetDir.getFullPathName() << std::endl;
    
	String debug;
        debug << "Search dir: " << presetDir.getFullPathName() << "\n\n";
        
        DebugPrint(debug);

    SearchPresets(presetDir);
    
    
    // this is for the open dialog of the gui
    lastDir = lastDir.getSpecialLocation(File::userHomeDirectory);
    
    BufferSize = getBlockSize();
    ConvBufferSize = getBlockSize();
    SampleRate = getSampleRate();
    
#if BINAURAL_DECODER
    num_conv = 0;
#endif
    
}

Ambix_binauralAudioProcessor::~Ambix_binauralAudioProcessor()
{
#if BINAURAL_DECODER
    mtxconv_.StopProc();
    mtxconv_.Cleanup();
#endif
}

void Ambix_binauralAudioProcessor::SearchPresets(File SearchFolder)
{
    _presetFiles.clear();
    
    SearchFolder.findChildFiles(_presetFiles, File::findFiles, true, "*.config");
    std::cout << "Found preset files: " << _presetFiles.size() << std::endl;
    
}

void Ambix_binauralAudioProcessor::LoadPreset(unsigned int preset)
{
    if (preset < (unsigned int)_presetFiles.size())
    {
        // ScheduleConfiguration(_presetFiles.getUnchecked(preset));
        LoadConfiguration(_presetFiles.getUnchecked(preset));
    }
}

void Ambix_binauralAudioProcessor::LoadPresetByName(String presetName)
{
    Array <File> files;
    presetDir.findChildFiles(files, File::findFiles, true, presetName);
    
    if (files.size())
    {
        LoadConfiguration(files.getUnchecked(0)); // Load first result
        box_preset_str = files.getUnchecked(0).getFileNameWithoutExtension();
    }
    
}

//==============================================================================
const String Ambix_binauralAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int Ambix_binauralAudioProcessor::getNumParameters()
{
    return 1;
}

float Ambix_binauralAudioProcessor::getParameter (int index)
{
    return _gain;
}

void Ambix_binauralAudioProcessor::setParameter (int index, float newValue)
{
    _gain = jlimit(0.f, 1.f, newValue);
    
    int NumSpeakers = _AmbiSpeakers.size();
    
    for (int i=0; i < NumSpeakers; i++) {
        _AmbiSpeakers.getUnchecked(i)->setGainFactor(ParamToRMS(_gain));
    }
}

const String Ambix_binauralAudioProcessor::getParameterName (int index)
{
    return String("OutGain");
}

const String Ambix_binauralAudioProcessor::getParameterText (int index)
{
    String text;
    text = String(ParamToDB(_gain)).substring(0, 5);
    text << " dB";
    return text;
}

const String Ambix_binauralAudioProcessor::getInputChannelName (int channelIndex) const
{
    String text ("ACN ");
    text << channelIndex;
    return text;
}

const String Ambix_binauralAudioProcessor::getOutputChannelName (int channelIndex) const
{
    String text;
    if (BINAURAL_DECODER)
    {
        if (channelIndex == 0)
            text << "LeftHeadphone";
        else
            text << "RightHeadphone";
    } else {
        text << "LS ";
        text << channelIndex + 1;
    }
    
    return text;
}

bool Ambix_binauralAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_binauralAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool Ambix_binauralAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_binauralAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ambix_binauralAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double Ambix_binauralAudioProcessor::getTailLengthSeconds() const
{
#if BINAURAL_DECODER
    if (configLoaded)
    {
        // return _SpkConv.getFirst()->irLength();
        // double tail_s = (double)conv_data.getMaxLength()/getSampleRate();
        return 0.f;
        
        
    } else {
        return 0.f;
    }
#else
    return 0.0;
#endif
    
}

int Ambix_binauralAudioProcessor::getNumPrograms()
{
    return 0;
}

int Ambix_binauralAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ambix_binauralAudioProcessor::setCurrentProgram (int index)
{
}

const String Ambix_binauralAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void Ambix_binauralAudioProcessor::changeProgramName (int index, const String& newName)
{
    
}

//==============================================================================
void Ambix_binauralAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    if (SampleRate != sampleRate || BufferSize != samplesPerBlock)
    {
        SampleRate = sampleRate;
        BufferSize = samplesPerBlock;
        
        ReloadConfiguration();
    }
    
    if (configLoaded)
    {
#if BINAURAL_DECODER
        mtxconv_.Reset();
#endif
        
        ambi_spk_buffer_.setSize(_AmbiSpeakers.size(), BufferSize);
    }
}

void Ambix_binauralAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void Ambix_binauralAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    isProcessing = true;
    
    if (!configLoaded)
    {
        // if nothing is loaded just clear the buffer...
        buffer.clear();
        
    } else {
        
        int NumSpeakers = _AmbiSpeakers.size();
        
        ambi_spk_buffer_.clear();
        
        // decode into ambisonics speaker signals
        for (int i=0; i < NumSpeakers; i++) {
            
            _AmbiSpeakers.getUnchecked(i)->process(buffer, ambi_spk_buffer_, i);
            
        }
        
        // clear buffer
        buffer.clear();
        
#if BINAURAL_DECODER
        
        // it does not make sense to use the binaural decoder with
        // less than 2 channels (prevent crash here)
        if (buffer.getNumChannels() >= 2)
        {
            /*
          int NumConvolutions = _SpkConv.size();
        
          // do convolution
        
          for (int i=0; i < jmin(NumSpeakers, NumConvolutions); i++) {
            
                _SpkConv.getUnchecked(i)->process(_AmbiSpeakers.getUnchecked(i)->OutputBuffer, buffer);
            
            }
             */
            mtxconv_.processBlock(ambi_spk_buffer_, buffer);
        }
#else
        int NumSamples = buffer.getNumSamples();
        
        // decoder mode without convolution
        // copy back the ambisonics speaker signals
        for (int i=0; i < jmin(NumSpeakers, getNumOutputChannels()); i++) {
            
            buffer.copyFrom(i, 0,  ambi_spk_buffer_, i, 0, NumSamples);
        }
#endif
    }
    
    isProcessing = false;
}

void Ambix_binauralAudioProcessor::LoadConfiguration(File configFile)
{
    if (!configFile.existsAsFile())
    {
        
        String debug;
        debug << "Configuration file does not exist!" << configFile.getFullPathName() << "\n\n";
        
        //std::cout << debug << std::endl;
        DebugPrint(debug);
        
        return;
    }
    
    // unload first....
    if (configLoaded) {
        
        while (isProcessing) {
            Sleep(1);
        }
        
        std::cout << "Unloading Config..." << std::endl;
        UnloadConfiguration();
        _DebugText = String(); // clear debug window
        std::cout << "Config Unloaded..." << std::endl;
    }
    
    if (ConvBufferSize < BufferSize)
        ConvBufferSize = BufferSize;
    
    ConvBufferSize = nextPowerOfTwo(ConvBufferSize);
    
#if BINAURAL_DECODER
    conv_data.setSampleRate(getSampleRate());
#endif
    
    String debug;
    debug << "\ntrying to load " << configFile.getFullPathName() << "\n\n";
    
    DebugPrint(debug);
    
    // debug print samplerate and buffer size
    debug = "Samplerate: ";
    debug << SampleRate;
    debug << " Host Buffer Size: ";
    debug << (int)BufferSize;
#if BINAURAL_DECODER
    debug << " Convolution Buffer Size: ";
    debug << (int)ConvBufferSize;
#endif
    DebugPrint(debug);
    
    activePreset = configFile.getFileName(); // store filename only, on restart search preset folder for it!
    box_preset_str = configFile.getFileNameWithoutExtension();
    
    StringArray myLines;
    
    configFile.readLines(myLines);
    
    
    // global settings
    float dec_mat_gain = 1.f; // dec matrix gain
    float global_hrtf_gain = 1.f; // optional global hrtf gain
    char coeff_scale[10];
    char coeff_seq[10];
    int invert_condon_shortley = 0;
    int flip_param = 0; // mirror left right
    int flop_param = 0; // mirror front back
    int flap_param = 0; // mirror top bottom
    
    // iterate over all lines
    for (int currentLine = 0; currentLine < myLines.size(); currentLine++)
    {
        
        // get the line and remove spaces from start and end
        String line (myLines[currentLine].trim());
        
        
        // first check for global settings
        
        if (line.contains("#GLOBAL")) {
            for (currentLine = currentLine+1; currentLine < myLines.size(); currentLine++)
            {
                
                line = myLines[currentLine].trim();
                
                if (line.length() > 0) {
                    // break from getting matrix if # in line
                    if (line.contains("#"))
                        break;
                    
                    String::CharPointerType lineChar = line.getCharPointer();
                    
                    char path[100];
                    
                    sscanf(lineChar, "%s", path);
                    
                    // /debug_msg
                    if (!strcmp(path, "/debug_msg"))
                    {
                        line = line.trimCharactersAtStart("/debug_msg").trim();
                        
                        String debug;
                        debug << "\n" << line << "\n";
                        
                        DebugPrint(debug);
                        
                    } // end /debug_msg
                    
                    // /dec_mat_gain setting
                    else if (!strcmp(path, "/dec_mat_gain"))
                    {
                        
                        sscanf(lineChar, "%s%f", path, &dec_mat_gain);
                        
                        if (!dec_mat_gain)
                            dec_mat_gain = 1.f;
                        
                        String debug;
						debug << "applying gain factor to decoder matrix: " << rmstodb(dec_mat_gain) << "dB (" << dec_mat_gain << ")";
                        
                        DebugPrint(debug << "\n");
                        
                    } // end /dec_mat_gain
                    
                    // /global_hrtf_gain setting
                    else if (!strcmp(path, "/global_hrtf_gain"))
                    {
                        
                        sscanf(lineChar, "%s%f", path, &global_hrtf_gain);
                        
                        if (!global_hrtf_gain)
                            global_hrtf_gain = 1.f;
                        
                        String debug;
						debug << "applying global gain factor to hrtfs: " << rmstodb(global_hrtf_gain) << "dB (" << global_hrtf_gain << ")";
                        
                        DebugPrint(debug << "\n");
                        
                    } // end /global_hrtf_gain
                    
                    else if (!strcmp(path, "/coeff_scale"))
                    {
                        
                        sscanf(lineChar, "%s%s", path, coeff_scale);
                        
                        if (strcmp(coeff_scale, "sn3d") && strcmp(coeff_scale, "n3d") && strcmp(coeff_scale, "fuma")) {
                            
                            sprintf(coeff_scale, "sn3d"); // fallback to default if unknown
                            
                            String debug;
							debug << "/coeff_scale unknown, falling back to default scaling: ";
							debug << coeff_scale;
                            DebugPrint(debug << "\n");
                            
                        } else {
                            String debug;
							debug << "coefficient scaling: " << coeff_scale;
                            if (!strcmp(coeff_scale, "n3d") || !strcmp(coeff_scale, "fuma"))
                                debug << " (getting rescaled to fit sn3d)";
                            
                            DebugPrint(debug << "\n");
                        }
                        
                    } // end /coeff_scale
                    
                    else if (!strcmp(path, "/coeff_seq"))
                    {
                        
                        sscanf(lineChar, "%s%s", path, coeff_seq);
                        
                        if (strcmp(coeff_seq, "acn") && strcmp(coeff_seq, "fuma") && strcmp(coeff_seq, "sid")) {
                            
                            sprintf(coeff_seq, "acn"); // fallback to default if unknown
                            
                            String debug;
							debug << "/coeff_seq unknown, falling back to default sequence: " << coeff_seq;
                            DebugPrint(debug << "\n");
                            
                        } else {
                            
                            String debug;
							debug << "coefficient sequence: " << coeff_seq;
                            if (!strcmp(coeff_seq, "sid") || !strcmp(coeff_seq, "fuma"))
                                debug << " (getting adjusted to equal acn)";
                            
                            DebugPrint(debug << "\n");
                        }
                        
                    } // end /coeff_seq
                    
                    else if (!strcmp(path, "/invert_condon_shortley"))
                    {
                        
                        sscanf(lineChar, "%s%i", path, &invert_condon_shortley);
                        
                        if (invert_condon_shortley)
                            DebugPrint("inverting condon shortley phase!\n");
                        
                    } // end /invert_condon_shortley
                    
                    else if (!strcmp(path, "/flip"))
                    {
                        
                        sscanf(lineChar, "%s%i", path, &flip_param);
                        
                        if (flip_param)
                            DebugPrint("flip - mirror left right!\n");
                        
                    } // end /flip
                    
                    else if (!strcmp(path, "/flop"))
                    {
                        
                        sscanf(lineChar, "%s%i", path, &flop_param);
                        
                        if (flop_param)
                            DebugPrint("flop - mirror front back!\n");
                        
                    } // end /flop
                    
                    else if (!strcmp(path, "/flap"))
                    {
                        
                        sscanf(lineChar, "%s%i", path, &flap_param);
                        
                        if (flap_param)
                            DebugPrint("flap - mirror top bottom!\n");
                        
                    } // end /flap
                    
                }
            } // end iterate over global lines
        } // end if global
               
        
#if BINAURAL_DECODER
        
        // then check for #HRTF files
        
        if (_load_ir) { // make it possible to load preset without (re)loading impulse responses
            
            if (line.contains("#HRTF")) {
                
                // _SpkConv.clear();
                num_conv = 0;
                
                for (currentLine = currentLine+1; currentLine < myLines.size(); currentLine++)
                {
                    
                    line = myLines[currentLine].trim();
                    
                    if (line.length() > 0) {
                        // break from getting matrix if # in line
                        if (line.contains("#"))
                            break;
                        
                        float gain = 1.0f;
                        float delay = 0.f;
                        int swapChannels = 0;
                        
                        char filename[120];
                        
                        String::CharPointerType lineChar = line.getCharPointer();
                        sscanf(lineChar, "%s%f%f%i", filename, &gain, &delay, &swapChannels);
                        
                        
                        if ( (swapChannels != 0 ) && (swapChannels != 1) )
                            swapChannels = 0;
                        
                        File IrFilename = configFile.getParentDirectory().getChildFile(String(filename));
                        
                        /*
                        _SpkConv.add(new SpkConv());
                        if (_SpkConv.getLast()->loadIr(IrFilename, SampleRate, BufferSize, gain * global_hrtf_gain, delay, swapChannels))
                        {
                            String debug;
                            debug << "add conv # " << _SpkConv.size() << " " << filename << " gain: " << gain << " delay: " << delay << " swap: " << swapChannels;
                            DebugPrint(debug << "\n");
                            
                        } else { // something went wrong with loading...
                            String debug;
                            debug << "ERROR: could not load file " << filename;
                            DebugPrint(debug << "\n");
                        }
                         */
                        
                        AudioSampleBuffer TempAudioBuffer(2,256);
                        double src_samplerate;
                        
                        if (loadIr(&TempAudioBuffer, IrFilename, src_samplerate, gain, 0, 0))
                        {
                            int numspl = TempAudioBuffer.getNumSamples();
                            
                            AudioSampleBuffer TempAudioBufferL(1, numspl);
                            AudioSampleBuffer TempAudioBufferR(1, numspl);
                            
                            TempAudioBufferL.copyFrom(0, 0, TempAudioBuffer, 0, 0, numspl);
                            TempAudioBufferR.copyFrom(0, 0, TempAudioBuffer, 1, 0, numspl);
                            
                            // compute delay in samples
                            int delay_samples = (int) (src_samplerate * (delay/1000.f));
                            
                            // add IR to my convolution data - gain is already done while reading file
                            conv_data.addIR(num_conv, 0+swapChannels, 0, delay_samples, 0, &TempAudioBufferL, src_samplerate);
                            conv_data.addIR(num_conv, (1+swapChannels)%2, 0, delay_samples, 0, &TempAudioBufferR, src_samplerate);
                            // addIR(int in_ch, int out_ch, int offset, int delay, int length, AudioSampleBuffer* buffer, double src_samplerate);
                            
                            String debug;
                            debug << "add conv # " << num_conv+1 << " " << filename << " gain: " << gain << " delay: " << delay << " swap: " << swapChannels;
                            DebugPrint(debug << "\n");
                            
                            num_conv++;
                            
                        } else {
                            String debug;
                            debug << "ERROR: could not load file " << filename;
                            DebugPrint(debug << "\n");
                        }
                        
                    }
                } // iterate over lines in #HRTF
                
            } // end #HRTF
        } // end if load ir
        
#endif
        
        if (line.contains("#DECODERMATRIX")) {
            
            // delete all ambisonic speakers
            _AmbiSpeakers.clear();
            
            _AmbiChannels = 0;
            
            for (currentLine = currentLine+1; currentLine < myLines.size(); currentLine++)
            {
                line = myLines[currentLine].trim();
                
                
                // break from getting matrix if #end line
                if (line.contains("#END"))
                    break;
                                
                // std::cout << "current line " << line << std::endl;
                
                Array<float> DecoderRow;

                // if something is left -> add a new speaker!
                if (line.length() > 0)
                {
                    
                    String::CharPointerType lineChar = line.getCharPointer();
                    
                    int n = 0; // how many characters been read
                    
                    int curr_n = 0;
                    int max_n = lineChar.length();
                    
                    for (int j=0; j < AMBI_CHANNELS; j++) { // parse line for numbers
                        
                        double value;
                        
                        sscanf(lineChar, "%lf%n", &value, &n);
                      
                        if (curr_n >= max_n)
                          break;
                      
                        DecoderRow.add((float)value);
                        
                         // std::cout << "added coefficient " << j << " with " << value << std::endl;
                        
                        lineChar += n;
                        
                        curr_n += n;
                                                
                    } // end parse numbers
                    
                    
                    // got my decoder row
                    
                    ///////////////////
                    // now alter sequence if necessary (resort decoder row)
                    
                    if (!strcmp(coeff_seq, "fuma")) {
                        
                        Array<float> SortedDecoderRow; // temporary array
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            SortedDecoderRow.add(DecoderRow.getUnchecked(fuma_ch_map[i]));
                            
                        }
                        
                        DecoderRow = SortedDecoderRow;
                        // std::cout << "reordered decoder row from " << coeff_seq << " to acn!" << std::endl;
                    } // end fuma sequence
                    
                    else if (!strcmp(coeff_seq, "sid")) {
                        
                        Array<float> SortedDecoderRow; // temporary array
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            SortedDecoderRow.add(DecoderRow.getUnchecked(sid_ch_map[i]));
                            
                        }
                        
                        DecoderRow = SortedDecoderRow;
                        // std::cout << "reordered decoder row from " << coeff_seq << " to acn!" << " size old:" << DecoderRow.size() << " size new: " << SortedDecoderRow.size() << std::endl;
                    } // end sid sequence
                    
                    
                    ///////////////////
                    // decoder matrix gain option ( /dec_mat_gain )
                    
                    if (dec_mat_gain != 1.f)
                    {
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * dec_mat_gain);
                            
                        }
                    } // end decoder matrix gain
                    
                    ///////////////////
                    // scale decoder row if necessary to achieve sn3d normalization!
                    if (!strcmp(coeff_scale, "fuma")) {
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            // DecoderRow.set(i, DecoderRow.getUnchecked(i) * conv_norm_fuma_sn3d[i]); // wrong
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * conv_norm_sn3d_fuma[i]);
                            
                        }
                        // std::cout << "scaled decoder row from " << coeff_scale << " to sn3d!" << std::endl;
                    } // end fuma to sn3d scale
                    else if (!strcmp(coeff_scale, "n3d")) {
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            // DecoderRow.set(i, DecoderRow.getUnchecked(i) * conv_norm_n3d_sn3d[i]); // wrong!
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * conv_norm_sn3d_n3d[i]);
                            
                        }
                        // std::cout << "scaled decoder row from " << coeff_scale << " to sn3d!" << std::endl;
                    } // end n3d to sn3d scale
                    
                    
#if 0
                    ///////////////////
                    // invert condon shortley phase if necessary
                    if (invert_condon_shortley != 0) {
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * acn_cs_phase[i]);
                            
                        }
                        //std::cout << "inverted cs phase... "  << invert_condon_shortley << std::endl;
                        
                    } // end invert cs
#endif
                    
                    ///////////////////
                    // condon shortley phase, flip flop or flap
                    if (invert_condon_shortley || flip_param || flop_param || flap_param) {
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            int l = 0; // manchmal auch n
                            int m = 0;
                            
                            // decode ambisonics channel numbering
                            ACNtoLM(i, l, m);
                            
                            signed int flip, flop, flap, total;
                            flip = flop = flap = total = 1;
                            
                            // taken from paper Symmetries of Spherical Harmonics by Michael Chapman (Ambi Symp 2009),
                            
                            // mirror left right
                            if ( flip_param && (m < 0) ) // m < 0 -> invert
                                flip = -1;
                            
                            // mirror front back
                            if ( flop_param && ( ((m < 0) && !(m % 2)) || ((m >= 0) && (m % 2)) ) ) // m < 0 and even || m >= 0 and odd
                                flop = -1;
                            
                            // mirror top bottom
                            if ( flap_param && ( (l + m) % 2 ) ) // l+m odd   ( (odd, even) or (even, odd) )
                                flap = -1;
                            
                            // compute total multiplicator
                            if (invert_condon_shortley)
                                total = acn_cs_phase[i] * flip * flop * flap;
                            else
                                total = flip * flop * flap;
                            
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * total);
                            
                        }
                        //std::cout << "inverted cs phase... "  << invert_condon_shortley << std::endl;
                        
                    } // end invert cs
                    
                    ///////////////////
                    // finally add speaker with decoder row
                    
                    _AmbiSpeakers.add(new AmbiSpeaker(getSampleRate(), getBlockSize(), ParamToRMS(_gain) ));
                    _AmbiSpeakers.getLast()->setDecoderRow(DecoderRow);
                    
                    _AmbiChannels = jmax(_AmbiChannels, DecoderRow.size());
                    
                    // output debug...
                    String debug;
					debug << "speaker # " << _AmbiSpeakers.size() << " with " << DecoderRow.size() << " coefficients";
                    // decoder has higher order than the plugin!! -> report warning
                    if (curr_n < max_n)
                    {
                        debug << "\n WARNING: probably you are using a preset inteded to be used at a higher order!";
                    }
                    DebugPrint(debug << "\n");
                    
                } // end if line not zero
            } // end iterate over decoder matrix lines
            
        } // end decoder matrix
        
        
    } // end iterate over configuration file lines
    
#if BINAURAL_DECODER
    
    
    setLatencySamples(ConvBufferSize-BufferSize);
    
    mtxconv_.Configure(conv_data.getNumInputChannels(), conv_data.getNumOutputChannels(), ConvBufferSize, conv_data.getMaxLength(), 8192);
    
    // std::cout << "configure: numins: " << conv_data.getNumInputChannels()
    for (int i=0; i < conv_data.getNumIRs(); i++)
    {
        
        mtxconv_.AddFilter(conv_data.getInCh(i), conv_data.getOutCh(i), *conv_data.getIR(i));
        
    }
    
    mtxconv_.StartProc();
    
#endif
    
    ambi_spk_buffer_.setSize(_AmbiSpeakers.size(), BufferSize);
    
    configLoaded = true;


    _configFile = configFile;
    
    sendChangeMessage(); // notify editor
    
}



void Ambix_binauralAudioProcessor::UnloadConfiguration()
{
    // delete configuration
    configLoaded = false;
    
#if BINAURAL_DECODER
    //std::cout << "Unloading Convolution..." << std::endl;
    
    if (_load_ir)
    {
        conv_data.clear();
        num_conv = 0;
        
        mtxconv_.StopProc();
    
        mtxconv_.Cleanup();
    }
    
    //std::cout << "Unloaded Convolution..." << std::endl;
#endif
    
    _AmbiSpeakers.clear();

}

void Ambix_binauralAudioProcessor::ReloadConfiguration()
{
    if (configLoaded)
        LoadConfiguration(_configFile);
}

#if BINAURAL_DECODER
bool Ambix_binauralAudioProcessor::loadIr(AudioSampleBuffer* IRBuffer, const File& audioFile, double &samplerate, float gain, int offset, int length)
{
    if (!audioFile.existsAsFile())
    {
        std::cout << "ERROR: file does not exist!!" << std::endl;
        return false;
    }
    
    AudioFormatManager formatManager;
    
    // this can read .wav and .aiff
    formatManager.registerBasicFormats();
    
    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    
    if (!reader) {
        std::cout << "ERROR: could not read impulse response file!" << std::endl;
    }
    
    int ir_length = (int)reader->lengthInSamples;
    
    if (ir_length <= 0) {
        std::cout << "wav file has zero samples" << std::endl;
        return false;
    }
    
    if (reader->numChannels != 2) {
        std::cout << "wav file has incorrect channel count: " << reader->numChannels << std::endl;
        return false;
    }
    
    samplerate = reader->sampleRate;
    
    IRBuffer->setSize(2, ir_length);
    
    reader->read(IRBuffer, 0, ir_length, 0, true, true);
    
    // resampling is done in conv_data
    
    // scale ir with gain
    IRBuffer->applyGain(gain);
    
    delete reader;
    
    return true;
}
#endif

void Ambix_binauralAudioProcessor::DebugPrint(String debugText)
{
    String temp;
    
    temp << debugText;
    temp << _DebugText;
    
    _DebugText = temp;
}

unsigned int Ambix_binauralAudioProcessor::getBufferSize()
{
    return BufferSize;
}

unsigned int Ambix_binauralAudioProcessor::getConvBufferSize()
{
    return ConvBufferSize;
}


void Ambix_binauralAudioProcessor::setConvBufferSize(unsigned int bufsize)
{
    if (nextPowerOfTwo(bufsize) != ConvBufferSize)
    {
        ConvBufferSize = nextPowerOfTwo(bufsize);
        ReloadConfiguration();
    }
}

//==============================================================================
bool Ambix_binauralAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ambix_binauralAudioProcessor::createEditor()
{
    return new Ambix_binauralAudioProcessorEditor (this);
}

//==============================================================================
void Ambix_binauralAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");
    
    // add some attributes to it..
    xml.setAttribute ("activePreset", activePreset);
    xml.setAttribute ("presetDir", presetDir.getFullPathName());
    xml.setAttribute("ConvBufferSize", (int)ConvBufferSize);
    xml.setAttribute("Gain", _gain);
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void Ambix_binauralAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        String newPresetDir;
        
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            activePreset  = xmlState->getStringAttribute("activePreset", "");
            
            newPresetDir = xmlState->getStringAttribute("presetDir", presetDir.getFullPathName());
            
            ConvBufferSize = xmlState->getIntAttribute("ConvBufferSize", ConvBufferSize);
            
            _gain = jlimit(0.f, 1.f, (float)xmlState->getDoubleAttribute("Gain", 0.5f));
        }
      
        File tempDir(newPresetDir);
        if (tempDir.exists()) {
            presetDir = tempDir;
            SearchPresets(presetDir);
        }
      
        if (activePreset.isNotEmpty()) {
          LoadPresetByName(activePreset);
        }
    }
    

}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ambix_binauralAudioProcessor();
}
