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

enum coeff_scale_enum
{
    sn3d_scale,
    n3d_scale,
    fuma_scale
};
enum coeff_seq_enum
{
    acn_seq,
    fuma_seq,
    sid_seq
};
//==============================================================================
Ambix_binauralAudioProcessor::Ambix_binauralAudioProcessor() :
                                _AmbiChannels(0),
                                configLoaded(false),
                                box_preset_str("no preset loaded"),
                                _load_ir(true),
                                SampleRate(44100),
                                isProcessing(false),
                                _gain(0.5f),
                                Thread("ambix_binaural")

{
    presetDir = presetDir.getSpecialLocation(File::userApplicationDataDirectory).getChildFile("ambix/binaural_presets");
    std::cout << "Recursive search dir:" << presetDir.getFullPathName() << std::endl;
    
	String debug;
        debug << "Recursive search dir: " << presetDir.getFullPathName() << "\n\n";
        
        DebugPrint(debug);

    SearchPresets(presetDir);
    
    
    // this is for the open dialog of the gui
    lastDir = lastDir.getSpecialLocation(File::userHomeDirectory);
    
    BufferSize = getBlockSize();
    ConvBufferSize = getBlockSize();
    SampleRate = getSampleRate();
    
#if BINAURAL_DECODER
    #if WITH_ZITA_CONVOLVER
        _ConvBufferPos = 0;
    #endif
    num_conv = 0;
    safemode_ = false;
#endif
    
}

Ambix_binauralAudioProcessor::~Ambix_binauralAudioProcessor()
{
#if BINAURAL_DECODER
    #if WITH_ZITA_CONVOLVER
        zita_conv.stop_process();
        zita_conv.cleanup();
    #else
        mtxconv_.StopProc();
        mtxconv_.Cleanup();
    #endif
#endif
}

void Ambix_binauralAudioProcessor::SearchPresets(File SearchFolder)
{
    _presetFiles.clear();
    
    SearchFolder.findChildFiles(_presetFiles, File::findFiles, true, "*.config");
    _presetFiles.sort();	
    std::cout << "Found preset files: " << _presetFiles.size() << std::endl;
    
}

void Ambix_binauralAudioProcessor::LoadPreset(unsigned int preset)
{
    if (preset < (unsigned int)_presetFiles.size())
    {
        // ScheduleConfiguration(_presetFiles.getUnchecked(preset));
        LoadConfigurationAsync(_presetFiles.getUnchecked(preset));
    }
}

void Ambix_binauralAudioProcessor::LoadPresetByName(String presetName)
{
    Array <File> files;
    presetDir.findChildFiles(files, File::findFiles, true, presetName);
    
    if (files.size())
    {
        LoadConfigurationAsync(files.getUnchecked(0)); // Load first result
        box_preset_str = files.getUnchecked(0).getFileNameWithoutExtension();
    }
    else
    { // preset not found -> post!
        String debug_msg;
        debug_msg << "ERROR loading preset: " << presetName << ", Preset not found in search folder!\n\n";
        DebugPrint(debug_msg);
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
        
        int NumSamples = buffer.getNumSamples();
        
#if BINAURAL_DECODER
        
        // it does not make sense to use the binaural decoder with
        // less than 2 channels (prevent crash here)
        if (buffer.getNumChannels() >= 2)
        {
            #if WITH_ZITA_CONVOLVER
            
                //std::cout << "new samples: " << NumSamples << " Samples, in level: " << ambi_spk_buffer_.getMagnitude(0, NumSamples) << std::endl;
                
                for (int i=0; i < NumSpeakers ; i++)
                {
                    float* indata = zita_conv.inpdata(i)+_ConvBufferPos;
                    memcpy(indata, ambi_spk_buffer_.getReadPointer(i), NumSamples*sizeof(float));
                }
                
                _ConvBufferPos += NumSamples;
                
                if (_ConvBufferPos >= ConvBufferSize) {
                    // std::cout << "processing " << _ConvBufferPos << " Samples" << std::endl;
                    
                    int ret = zita_conv.process(THREAD_SYNC_MODE);
                    //std::cout << "convolver ret: " << ret << std::endl;
                    _ConvBufferPos = 0;
                }
                
                for (int i=0; i < 2 ; i++)
                {
                    float* outdata = zita_conv.outdata(i)+_ConvBufferPos;
                    memcpy(buffer.getWritePointer(i), outdata, NumSamples*sizeof(float));
                }
                //std::cout << "new samples: " << NumSamples << " Samples, out level: " << buffer.getMagnitude(0, NumSamples) << std::endl;
            #else
                mtxconv_.processBlock(ambi_spk_buffer_, buffer, NumSamples, true);
            #endif
        }
#else
        
        // decoder mode without convolution
        // copy back the ambisonics speaker signals
        for (int i=0; i < jmin(NumSpeakers, getTotalNumOutputChannels()); i++) {
            
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
    coeff_scale_enum coeff_scale = coeff_scale_enum::sn3d_scale;
    coeff_seq_enum coeff_seq = coeff_seq_enum::acn_seq;
    bool invert_condon_shortley = false;
    bool flip_param = false; // mirror left right
    bool flop_param = false; // mirror front back
    bool flap_param = false; // mirror top bottom
    
    // iterate over all lines
    for (int currentLine = 0; currentLine < myLines.size(); currentLine++)
    {
        
        if (threadShouldExit())
            return;

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
                    
                    if (line.startsWithIgnoreCase("/debug_msg"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        String debug;
                        debug << "\n" << line << "\n";
                        
                        DebugPrint(debug);
                        
                    } // end /debug_msg
                    
                    // /dec_mat_gain setting
                    else if (line.startsWithIgnoreCase("/dec_mat_gain"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        dec_mat_gain = line.getFloatValue();

                        if (!dec_mat_gain)
                            dec_mat_gain = 1.f;
                        
                        String debug;
						debug << "applying gain factor to decoder matrix: " << rmstodb(dec_mat_gain) << "dB (" << dec_mat_gain << ")";
                        
                        DebugPrint(debug << "\n");
                        
                    } // end /dec_mat_gain
                    
                    // /global_hrtf_gain setting
                    else if (line.startsWithIgnoreCase("/global_hrtf_gain"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        global_hrtf_gain = line.getFloatValue();
                        
                        if (!global_hrtf_gain)
                            global_hrtf_gain = 1.f;
                        
                        String debug;
						debug << "applying global gain factor to hrtfs: " << rmstodb(global_hrtf_gain) << "dB (" << global_hrtf_gain << ")";
                        
                        DebugPrint(debug << "\n");
                        
                    } // end /global_hrtf_gain
                    
                    else if (line.startsWithIgnoreCase("/coeff_scale"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        if (line.compareIgnoreCase("sn3d") && line.compareIgnoreCase("n3d") && line.compareIgnoreCase("fuma")) {
                            String debug;
							debug << "/coeff_scale unknown, falling back to default scaling: sn3d";
                            DebugPrint(debug << "\n");
                            
                        } else {
                            if (line.startsWithIgnoreCase("n3d"))
                                coeff_scale = coeff_scale_enum::n3d_scale;

                            if (line.startsWithIgnoreCase("fuma"))
                                coeff_scale = coeff_scale_enum::fuma_scale;


                            String debug;
                            debug << "coefficient scaling: " << line;
                            if (!line.startsWithIgnoreCase("sn3d"))
                                debug << " (getting rescaled to fit sn3d)";
                            
                            DebugPrint(debug << "\n");
                        }
                        
                    } // end /coeff_scale
                    
                    else if (line.startsWithIgnoreCase("/coeff_seq"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        if (line.compareIgnoreCase("acn") && line.compareIgnoreCase("fuma") && line.compareIgnoreCase("sid")) {
                            String debug;
                            debug << "/coeff_seq unknown, falling back to default sequence: " << coeff_seq;
                            DebugPrint(debug << "\n");
                        }
                        else {
                            if (line.startsWithIgnoreCase("fuma"))
                                coeff_seq = coeff_seq_enum::fuma_seq;

                            if (line.startsWithIgnoreCase("sid"))
                                coeff_seq = coeff_seq_enum::sid_seq;

                            String debug;
                            debug << "coefficient sequence: " << coeff_seq;
                            if (!line.startsWithIgnoreCase("acn"))
                                debug << " (getting adjusted to equal acn)";

                            DebugPrint(debug << "\n");
                        }
                        
                    } // end /coeff_seq
                    
                    else if (line.startsWithIgnoreCase("/invert_condon_shortley"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        if (line.getIntValue() == 1)
                            invert_condon_shortley = true;

                        if (invert_condon_shortley)
                            DebugPrint("inverting condon shortley phase!\n");
                        
                    } // end /invert_condon_shortley
                    
                    else if (line.startsWithIgnoreCase("/flip"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        if (line.getIntValue() == 1)
                            flip_param = true;

                        if (flip_param)
                            DebugPrint("flip - mirror left right!\n");

                    } // end /flip
                    
                    else if (line.startsWithIgnoreCase("/flop"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        if (line.getIntValue() == 1)
                            flop_param = true;
                        
                        if (flop_param)
                            DebugPrint("flop - mirror front back!\n");
                        
                    } // end /flop
                    
                    else if (line.startsWithIgnoreCase("/flap"))
                    {
                        line = line.fromFirstOccurrenceOf(" ", false, true);

                        if (line.getIntValue() == 1)
                            flap_param = true;

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
                    if (threadShouldExit())
                        return;

                    line = myLines[currentLine].trim();
                    
                    if (line.length() > 0) {
                        // break from getting matrix if # in line
                        if (line.contains("#"))
                            break;
                        
                        float gain = 1.0f;
                        float delay = 0.f;
                        bool swapChannels = false;
                        
                        // parse filename (.wav or .aiff)
                        String filename("");
                        
                        if (line.containsIgnoreCase(".wav"))
                        {
                            filename = line.upToFirstOccurrenceOf(".wav", true, true);
                            line = line.fromFirstOccurrenceOf(".wav", false, true).trim();
                        } else if (line.containsIgnoreCase(".aiff"))
                        {
                            filename = line.upToFirstOccurrenceOf(".aiff", true, true);
                            line = line.fromFirstOccurrenceOf(".aiff", false, true).trim();
                        }
                        else
                            break; // no valid filename

                        // parse gain, delay, swapChannels
                        if (line.length() > 0)
                        {
                            gain = line.upToFirstOccurrenceOf(" ", false, true).getFloatValue();
                            line = line.fromFirstOccurrenceOf(" ", false, true).trim();
                        }
                        if (line.length() > 0)
                        {
                            delay = line.upToFirstOccurrenceOf(" ", false, true).getFloatValue();
                            line = line.fromFirstOccurrenceOf(" ", false, true).trim();
                        }
                        if (line.length() > 0)
                        {
                            swapChannels = line.upToFirstOccurrenceOf(" ", false, true).getIntValue() > 0 ? true : false;
                        }

                        
                        File IrFilename = configFile.getParentDirectory().getChildFile(filename);
                        
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
                            
                            // compute delay in samples
                            int delay_samples = (int) (src_samplerate * (delay/1000.f));
                            
                            // add IR to my convolution data - gain is already done while reading file
                            conv_data.addIR(num_conv, 0+swapChannels, 0, delay_samples, 0, &TempAudioBuffer, 0, src_samplerate);
                            conv_data.addIR(num_conv, (1+swapChannels)%2, 0, delay_samples, 0, &TempAudioBuffer, 1, src_samplerate);
                            // addIR(int in_ch, int out_ch, int offset, int delay, int length, AudioSampleBuffer* buffer, double src_samplerate);
                            
                            String debug;
                            debug << "add conv # " << num_conv+1 << " " << filename << " gain: " << gain << " delay: " << delay << " swap: " << int(swapChannels);
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
                    
                    bool readAllCoeffs = false;
                    
                    for (int j=0; j < AMBI_CHANNELS; j++) { // parse line for numbers
                        
                        double value;
                        
                        value = CharacterFunctions::readDoubleValue(lineChar);

                        DecoderRow.add((float)value);

                        if (lineChar.isEmpty())
                            break;
                                                
                    } // end parse numbers
                    
                    if (lineChar.isEmpty())
                        readAllCoeffs = true;

                    // got my decoder row
                    
                    ///////////////////
                    // now alter sequence if necessary (resort decoder row)
                    
                    if (coeff_seq == coeff_seq_enum::fuma_seq) {
                        
                        Array<float> SortedDecoderRow; // temporary array
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            SortedDecoderRow.add(DecoderRow.getUnchecked(fuma_ch_map[i]));
                            
                        }
                        
                        DecoderRow = SortedDecoderRow;
                        // std::cout << "reordered decoder row from " << coeff_seq << " to acn!" << std::endl;
                    } // end fuma sequence
                    
                    else if (coeff_seq == coeff_seq_enum::sid_seq) {
                        
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
                    if (coeff_scale == coeff_scale_enum::fuma_scale) {
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * conv_norm_sn3d_fuma[i]);
                            
                        }
                        // std::cout << "scaled decoder row from " << coeff_scale << " to sn3d!" << std::endl;
                    } // end fuma to sn3d scale
                    else if (coeff_scale == coeff_scale_enum::n3d_scale) {
                        
                        for (int i=0; i < DecoderRow.size(); i++) {
                            
                            DecoderRow.set(i, DecoderRow.getUnchecked(i) * conv_norm_sn3d_n3d[i]);
                            
                        }
                        // std::cout << "scaled decoder row from " << coeff_scale << " to sn3d!" << std::endl;
                    } // end n3d to sn3d scale
                    
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
                    if (!readAllCoeffs)
                    {
                        debug << "\n WARNING: probably you are using a preset inteded to be used at a higher order!";
                    }
                    DebugPrint(debug << "\n");
                    
                } // end if line not zero
            } // end iterate over decoder matrix lines
            
        } // end decoder matrix
        
        
    } // end iterate over configuration file lines
    
#if BINAURAL_DECODER

    // try autodetecting host and deciding whether we need safemode (to avoid having to add another user parameter - let's see how this works for testers) 
    PluginHostType me;
    safemode_ = me.isAdobeAudition() || me.isPremiere() || me.isSteinberg(); // probably an incomplete list

    #if WITH_ZITA_CONVOLVER
    
        int err=0;
    
        unsigned int options = 0;
        
        options |= Convproc::OPT_FFTW_MEASURE;
        options |= Convproc::OPT_VECTOR_MODE;
        
        zita_conv.set_options (options);
        const float density = 0.5f;
                
        err = zita_conv.configure(conv_data.getNumInputChannels(), conv_data.getNumOutputChannels(), (unsigned int)conv_data.getMaxLength(), ConvBufferSize,Convproc::MINPART, Convproc::MAXPART, density);
        
        for (int i=0; i < conv_data.getNumIRs(); i++)
        {
            err = zita_conv.impdata_create(conv_data.getInCh(i), conv_data.getOutCh(i), 1, (float *)conv_data.getIR(i)->getReadPointer(0), 0, (unsigned int)conv_data.getLength(i));
        }
        
        zita_conv.print();
        zita_conv.start_process(CONVPROC_SCHEDULER_PRIORITY, CONVPROC_SCHEDULER_CLASS);
        
    #else
        mtxconv_.Configure(conv_data.getNumInputChannels(), conv_data.getNumOutputChannels(), BufferSize, conv_data.getMaxLength(), ConvBufferSize, 8192, safemode_);

        // std::cout << "configure: numins: " << conv_data.getNumInputChannels()
        for (int i=0; i < conv_data.getNumIRs(); i++)
        {
            if (threadShouldExit())
                return;

            mtxconv_.AddFilter(conv_data.getInCh(i), conv_data.getOutCh(i), *conv_data.getIR(i));
        }

        mtxconv_.StartProc();
    #endif

#endif
    
    ambi_spk_buffer_.setSize(_AmbiSpeakers.size(), BufferSize);
    
    configLoaded = true;

    if (safemode_)
        setLatencySamples(ConvBufferSize);
    else
        setLatencySamples(ConvBufferSize - BufferSize);


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

        #if WITH_ZITA_CONVOLVER
            zita_conv.stop_process();
            zita_conv.cleanup();
            _ConvBufferPos = 0;
        #else
            mtxconv_.StopProc();
            mtxconv_.Cleanup();
        #endif
    }
    
    //std::cout << "Unloaded Convolution..." << std::endl;
#endif
    
    _AmbiSpeakers.clear();

}

void Ambix_binauralAudioProcessor::run()
{
    LoadConfiguration(_desConfigFile);
}

void Ambix_binauralAudioProcessor::LoadConfigurationAsync(File configFile)
{
    DebugPrint("Loading preset...\n\n");
    _desConfigFile = configFile;
    startThread(6); // medium priority
}

void Ambix_binauralAudioProcessor::ReloadConfiguration()
{
    if (configLoaded)
        LoadConfigurationAsync(_configFile);
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
