/*
  ambix_testhost — offline CLI test harness for ambix VST3 plugins.

  Loads a VST3 plugin at a caller-specified discrete channel count (so
  asymmetric main buses like encoder mono→ambi, decoder 4→64, or
  binaural 4→2 are addressable), sets parameters from a JSON file,
  processes a WAV input, and writes the result. Designed to bypass
  pedalboard's symmetric-bus requirement.

  Usage:
    ambix_testhost --plugin path/to/plugin.vst3
                   --input  path/to/in.wav
                   --output path/to/out.wav
                   [--params path/to/params.json]
                   [--channels N]               // shorthand for in==out
                   [--in-channels N]            // explicit input bus width
                   [--out-channels M]           // explicit output bus width
                   [--blocksize 512]
                   [--samplerate 48000]
                   [--no-output]                // skip writing output WAV

  params.json:
    {
      "Azimuth": 0.5,
      "Elevation": 0.5
    }
  Keys are exact parameter names as returned by AudioProcessorParameter::getName().
  Values are normalised [0, 1] floats.

  Exit codes:
    0 — success
    1 — argument / file error
    2 — plugin load failure
*/

#include <JuceHeader.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void die(const String& msg, int code = 1)
{
    std::cerr << "ambix_testhost error: " << msg << "\n";
    std::exit(code);
}

static String readFile(const File& f)
{
    if (! f.existsAsFile())
        die("File not found: " + f.getFullPathName());
    return f.loadFileAsString();
}

// ---------------------------------------------------------------------------
// Application
// ---------------------------------------------------------------------------

class TestHostApp final : public JUCEApplicationBase
{
public:
    const String getApplicationName()    override { return "ambix_testhost"; }
    const String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed()    override { return true; }

    void initialise(const String& /*commandLine*/) override
    {
        const StringArray args = getCommandLineParameterArray();

        // ---- Parse arguments -------------------------------------------
        String pluginPath, paramsPath, inputPath, outputPath;
        int    numChannels    = -1;   // shorthand: applies to both if in/out not set
        int    numInChannels  = -1;
        int    numOutChannels = -1;
        int    blockSize      = 512;
        double sampleRate     = 48000.0;
        bool   noOutput       = false;

        for (int i = 0; i < args.size(); ++i)
        {
            const auto& a = args[i];
            if      (a == "--plugin"       && i + 1 < args.size()) pluginPath     = args[++i];
            else if (a == "--params"       && i + 1 < args.size()) paramsPath     = args[++i];
            else if (a == "--input"        && i + 1 < args.size()) inputPath      = args[++i];
            else if (a == "--output"       && i + 1 < args.size()) outputPath     = args[++i];
            else if (a == "--channels"     && i + 1 < args.size()) numChannels    = args[++i].getIntValue();
            else if (a == "--in-channels"  && i + 1 < args.size()) numInChannels  = args[++i].getIntValue();
            else if (a == "--out-channels" && i + 1 < args.size()) numOutChannels = args[++i].getIntValue();
            else if (a == "--blocksize"    && i + 1 < args.size()) blockSize      = args[++i].getIntValue();
            else if (a == "--samplerate"   && i + 1 < args.size()) sampleRate     = args[++i].getDoubleValue();
            else if (a == "--no-output")                            noOutput       = true;
        }

        if (numInChannels  < 0) numInChannels  = (numChannels > 0 ? numChannels : 2);
        if (numOutChannels < 0) numOutChannels = (numChannels > 0 ? numChannels : 2);

        if (pluginPath.isEmpty() || inputPath.isEmpty() || (outputPath.isEmpty() && ! noOutput))
        {
            std::cerr <<
                "Usage: ambix_testhost --plugin <path.vst3>\n"
                "                      --input  <in.wav>\n"
                "                      --output <out.wav>\n"
                "                      [--params <params.json>]\n"
                "                      [--channels N]               (shorthand for in==out)\n"
                "                      [--in-channels  N]\n"
                "                      [--out-channels M]\n"
                "                      [--blocksize 512]\n"
                "                      [--samplerate 48000]\n"
                "                      [--no-output]\n";
            std::exit(1);
        }

        // ---- Load plugin ------------------------------------------------
        AudioPluginFormatManager formatManager;
#if JUCE_PLUGINHOST_VST3
        formatManager.addFormat(new VST3PluginFormat());
#endif
#if JUCE_PLUGINHOST_AU
        formatManager.addFormat(new AudioUnitPluginFormat());
#endif

        OwnedArray<PluginDescription> found;
        KnownPluginList kpl;

        for (auto* fmt : formatManager.getFormats())
        {
            kpl.scanAndAddFile(pluginPath, false, found, *fmt);
            if (! found.isEmpty())
                break;
        }

        if (found.isEmpty())
            die("Could not scan plugin: " + pluginPath, 2);

        String loadErr;
        auto plugin = formatManager.createPluginInstance(
            *found[0], sampleRate, blockSize, loadErr);

        if (plugin == nullptr)
            die("Could not load plugin: " + loadErr, 2);

        // ---- Set bus layout (potentially asymmetric) -------------------
        //
        // JUCE's VST3 wrapper maps `AudioChannelSet::discreteChannels(N)` to a
        // generic Steinberg speaker arrangement that many ambisonic plugins
        // don't accept. For widths that match an ambisonic order
        // (1, 4, 9, 16, 25, 36, 49, 64 → orders 0..7) we try the dedicated
        // `AudioChannelSet::ambisonic(order)` first, then fall back to
        // `discreteChannels` for non-ambisonic widths (e.g. binaural's 2-out).
        auto chooseChannelSet = [](int numCh) -> AudioChannelSet {
            for (int order = 0; order <= 7; ++order)
            {
                const int n = (order + 1) * (order + 1);
                if (n == numCh)
                    return AudioChannelSet::ambisonic(order);
            }
            return AudioChannelSet::discreteChannels(numCh);
        };

        {
            auto tryLayout = [&](const AudioChannelSet& inSet,
                                 const AudioChannelSet& outSet) -> bool
            {
                AudioProcessor::BusesLayout l;
                l.inputBuses .add(inSet);
                l.outputBuses.add(outSet);
                if (plugin->checkBusesLayoutSupported(l))
                {
                    plugin->setBusesLayout(l);
                    return true;
                }
                return false;
            };

            const auto inAmbi  = chooseChannelSet(numInChannels);
            const auto outAmbi = chooseChannelSet(numOutChannels);
            const auto inDisc  = AudioChannelSet::discreteChannels(numInChannels);
            const auto outDisc = AudioChannelSet::discreteChannels(numOutChannels);

            const bool ok =
                   tryLayout(inAmbi, outAmbi)
                || tryLayout(inAmbi, outDisc)
                || tryLayout(inDisc, outAmbi)
                || tryLayout(inDisc, outDisc);

            if (! ok)
            {
                std::cerr << "ambix_testhost: " << numInChannels << " in / "
                          << numOutChannels
                          << " out not supported (tried ambisonic + discrete) — "
                             "falling back to plugin defaults\n";
                numInChannels  = plugin->getTotalNumInputChannels();
                numOutChannels = plugin->getTotalNumOutputChannels();
            }
        }

        plugin->prepareToPlay(sampleRate, blockSize);

        // ---- Apply parameters from JSON --------------------------------
        if (paramsPath.isNotEmpty())
        {
            auto json   = readFile(File(paramsPath));
            auto parsed = JSON::parse(json);

            if (auto* obj = parsed.getDynamicObject())
            {
                const auto& params = plugin->getParameters();
                for (auto* p : params)
                {
                    const String pname = p->getName(512);
                    if (obj->hasProperty(pname))
                    {
                        const float val = static_cast<float>(
                            static_cast<double>(obj->getProperty(pname)));
                        p->setValue(val);
                    }
                }
            }
        }

        // ---- Read input WAV --------------------------------------------
        AudioFormatManager fmtMgr;
        fmtMgr.registerBasicFormats();

        std::unique_ptr<AudioFormatReader> reader(
            fmtMgr.createReaderFor(File(inputPath)));
        if (reader == nullptr)
            die("Cannot open input WAV: " + inputPath);

        const int64 totalSamples = reader->lengthInSamples;
        const int   numScratchCh = jmax(numInChannels, numOutChannels);

        // Read all input samples into a buffer that has room for the wider
        // of in/out — JUCE's processBlock writes outputs into the same
        // AudioBuffer, so it must be sized to max(numIn, numOut).
        AudioBuffer<float> inputBuf(numScratchCh, (int)totalSamples);
        inputBuf.clear();

        {
            const int readerCh = jmin((int)reader->numChannels, numInChannels);
            AudioBuffer<float> tmp((int)reader->numChannels, (int)totalSamples);
            reader->read(&tmp, 0, (int)totalSamples, 0, true, true);
            for (int ch = 0; ch < readerCh; ++ch)
                inputBuf.copyFrom(ch, 0, tmp, ch, 0, (int)totalSamples);
        }
        reader.reset();

        // ---- Process in blocks -----------------------------------------
        plugin->prepareToPlay(sampleRate, blockSize);

        AudioBuffer<float> outputBuf(numOutChannels, (int)totalSamples);
        outputBuf.clear();

        MidiBuffer midi;
        int pos = 0;

        while (pos < (int)totalSamples)
        {
            const int thisBlock = jmin(blockSize, (int)totalSamples - pos);
            AudioBuffer<float> block(numScratchCh, thisBlock);
            block.clear();

            for (int ch = 0; ch < numInChannels; ++ch)
                block.copyFrom(ch, 0, inputBuf, ch, pos, thisBlock);

            midi.clear();
            plugin->processBlock(block, midi);

            for (int ch = 0; ch < numOutChannels; ++ch)
                outputBuf.copyFrom(ch, pos, block, ch, 0, thisBlock);

            pos += thisBlock;
        }

        plugin->releaseResources();

        // ---- Write output WAV -----------------------------------------
        if (noOutput)
        {
            std::cout << "ambix_testhost: processed " << totalSamples << " samples ("
                      << numInChannels << " in / " << numOutChannels
                      << " out), output discarded (--no-output)\n";
        }
        else
        {
            File outFile(outputPath);
            outFile.deleteFile();

            WavAudioFormat wavFmt;
            std::unique_ptr<AudioFormatWriter> writer(
                wavFmt.createWriterFor(
                    outFile.createOutputStream().release(),
                    sampleRate,
                    AudioChannelSet::discreteChannels(numOutChannels),
                    32,
                    {},
                    0));

            if (writer == nullptr)
                die("Cannot create output WAV: " + outputPath);

            writer->writeFromAudioSampleBuffer(outputBuf, 0, (int)totalSamples);
            writer->flush();
            writer.reset();

            std::cout << "ambix_testhost: wrote " << totalSamples << " samples ("
                      << numOutChannels << " ch) to " << outputPath << "\n";
        }

        quit();
    }

    void shutdown()    override {}
    void anotherInstanceStarted(const String&) override {}
    void systemRequestedQuit()  override { quit(); }
    void suspended()   override {}
    void resumed()     override {}
    void unhandledException(const std::exception* e,
                            const String& file, int line) override
    {
        std::cerr << "ambix_testhost unhandled exception at "
                  << file << ":" << line;
        if (e) std::cerr << ": " << e->what();
        std::cerr << "\n";
        std::exit(1);
    }
};

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

START_JUCE_APPLICATION(TestHostApp)
