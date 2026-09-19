/*
 ==============================================================================

 ambix_bustest — raw VST3 bus-layout probe.

 Talks to IComponent / IAudioProcessor directly, without JUCE's plugin host,
 because JUCE's host is exactly what hides the failure this tool looks for:
 when a plugin cannot answer getBusArrangement(), VST3PluginFormat quietly
 substitutes discreteChannels(count) and carries on, so a plugin with
 undescribable buses looks healthy from inside JUCE (pluginval included).
 Real hosts are not so forgiving — Max/MSP left ambix_encoder stuck on its
 1st-order default rather than negotiate with a bus it could not read.

 Reports, as JSON on stdout:
   - the default layout of every audio bus: channel count, and whether the
     plugin can name the arrangement,
   - the same after negotiating a requested pair of arrangements,
   - optionally, whether audio survives the negotiated layout in the order it
     went in (a reordered bus is the bug the applyBusLayouts guard exists to
     prevent, so the guard needs a test that would notice it going away).

 Usage:
   ambix_bustest --plugin <path.vst3>
                 [--in-arr  <mask>]   arrangement to request on the input bus
                 [--out-arr <mask>]   ... and the output bus; both decimal or
                                      0x-hex. Omit both to report the default
                                      layout only.
                 [--check-order]      push a ramp through and report the
                                      channel permutation, if any
                 [--blocks N]         process N blocks before reading (8)

 A mask is a VST3 SpeakerArrangement: bit per speaker. `(1 << n) - 1` is what
 a host that asks by channel count sends, and is also how kAmbi5th/6th/7th
 OrderACN are defined - which is why orders 5-7 behaved differently from
 orders 1-4 before this was understood.

 ==============================================================================
 */

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if defined (__APPLE__)
 #include <CoreFoundation/CoreFoundation.h>
#elif defined (_WIN32)
 #include <windows.h>
#else
 #include <dlfcn.h>
#endif

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace
{

using GetFactoryFn = IPluginFactory* (*) ();

//==============================================================================
/*  Loading a .vst3 bundle: same three-step dance everywhere (find the binary
    inside the bundle, load it, call the platform's module entry point), but
    each platform spells all three differently. */
struct Module
{
    IPluginFactory* factory = nullptr;
    std::string error;

    explicit Module (const std::string& bundlePath) { open (bundlePath); }

private:
   #if defined (__APPLE__)
    void open (const std::string& bundlePath)
    {
        auto path = CFStringCreateWithCString (nullptr, bundlePath.c_str(), kCFStringEncodingUTF8);
        auto url  = CFURLCreateWithFileSystemPath (nullptr, path, kCFURLPOSIXPathStyle, true);
        auto bundle = CFBundleCreate (nullptr, url);
        CFRelease (url);
        CFRelease (path);

        if (bundle == nullptr)                        { error = "cannot open bundle"; return; }
        if (! CFBundleLoadExecutable (bundle))        { error = "cannot load executable"; return; }

        using BundleEntryFn = bool (*) (CFBundleRef);
        auto entry   = (BundleEntryFn) CFBundleGetFunctionPointerForName (bundle, CFSTR ("bundleEntry"));
        auto getFact = (GetFactoryFn)  CFBundleGetFunctionPointerForName (bundle, CFSTR ("GetPluginFactory"));

        if (entry == nullptr || getFact == nullptr)   { error = "missing entry points"; return; }

        entry (bundle);
        factory = getFact();
    }
   #elif defined (_WIN32)
    void open (const std::string& bundlePath)
    {
        // A VST3 bundle on Windows may be a plain DLL or a bundle directory.
        std::string dll = bundlePath;

        const DWORD attributes = GetFileAttributesA (bundlePath.c_str());

        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            const auto slash = bundlePath.find_last_of ("/\\");
            const auto name  = slash == std::string::npos ? bundlePath : bundlePath.substr (slash + 1);
            dll = bundlePath + "\\Contents\\x86_64-win\\" + name;
        }

        auto handle = LoadLibraryA (dll.c_str());
        if (handle == nullptr)                        { error = "cannot load " + dll; return; }

        using InitDllFn = bool (*) ();
        auto entry   = (InitDllFn)    GetProcAddress (handle, "InitDll");
        auto getFact = (GetFactoryFn) GetProcAddress (handle, "GetPluginFactory");

        if (getFact == nullptr)                       { error = "missing entry points"; return; }

        if (entry != nullptr)
            entry();

        factory = getFact();
    }
   #else
    void open (const std::string& bundlePath)
    {
        std::string so = bundlePath;
        const auto slash = bundlePath.find_last_of ('/');
        const auto name  = slash == std::string::npos ? bundlePath : bundlePath.substr (slash + 1);
        const auto stem  = name.size() > 5 ? name.substr (0, name.size() - 5) : name; // drop ".vst3"

       #if defined (__x86_64__)
        so = bundlePath + "/Contents/x86_64-linux/" + stem + ".so";
       #elif defined (__aarch64__)
        so = bundlePath + "/Contents/aarch64-linux/" + stem + ".so";
       #endif

        auto handle = dlopen (so.c_str(), RTLD_LOCAL | RTLD_LAZY);
        if (handle == nullptr)                        { error = "cannot dlopen " + so; return; }

        using ModuleEntryFn = bool (*) (void*);
        auto entry   = (ModuleEntryFn) dlsym (handle, "ModuleEntry");
        auto getFact = (GetFactoryFn)  dlsym (handle, "GetPluginFactory");

        if (getFact == nullptr)                       { error = "missing entry points"; return; }

        if (entry != nullptr)
            entry (handle);

        factory = getFact();
    }
   #endif
};

int countBits (uint64 v) noexcept
{
    int n = 0;
    for (; v != 0; v >>= 1) n += (int) (v & 1);
    return n;
}

/*  One bus, as the host sees it: how many channels IComponent admits to, and
    whether IAudioProcessor can name the arrangement. The second question is
    the point of this tool - a bus that cannot be named is one a host cannot
    negotiate with. */
void printBus (IComponent& component, IAudioProcessor& proc,
               BusDirection dir, int32 index, const char* tag, bool& firstEntry)
{
    BusInfo info {};
    component.getBusInfo (kAudio, dir, index, info);

    SpeakerArrangement arr = 0;
    const bool named = proc.getBusArrangement (dir, index, arr) == kResultOk;

    printf ("%s\n    {\"stage\": \"%s\", \"dir\": \"%s\", \"index\": %d, \"channels\": %d, "
            "\"arrangement_named\": %s, \"arrangement\": \"0x%llx\", \"arrangement_channels\": %d}",
            firstEntry ? "" : ",", tag, dir == kInput ? "in" : "out", (int) index,
            (int) info.channelCount, named ? "true" : "false",
            (unsigned long long) arr, named ? countBits ((uint64) arr) : 0);

    firstEntry = false;
}

void printBuses (IComponent& component, IAudioProcessor& proc, const char* tag, bool& firstEntry)
{
    for (const auto dir : { kInput, kOutput })
        for (int32 i = 0; i < component.getBusCount (kAudio, dir); ++i)
            printBus (component, proc, dir, i, tag, firstEntry);
}

/*  Feed channel i the constant i+1 and report how many channels came back
    carrying something else. Only meaningful for a plugin that passes audio
    through unchanged at its default settings, so the caller opts in. */
int countReorderedChannels (IComponent& component, IAudioProcessor& proc,
                            int numIn, int numOut, int blocks)
{
    constexpr int blockSize = 64;

    component.activateBus (kAudio, kInput,  0, true);
    component.activateBus (kAudio, kOutput, 0, true);

    ProcessSetup setup { kRealtime, kSample32, blockSize, 48000.0 };

    if (proc.setupProcessing (setup) != kResultOk)
        return -1;

    component.setActive (true);
    proc.setProcessing (true);

    std::vector<std::vector<float>> inStore  ((size_t) numIn,  std::vector<float> ((size_t) blockSize));
    std::vector<std::vector<float>> outStore ((size_t) numOut, std::vector<float> ((size_t) blockSize));
    std::vector<float*> inPtr ((size_t) numIn), outPtr ((size_t) numOut);

    for (int c = 0; c < numIn;  ++c) inPtr [(size_t) c] = inStore [(size_t) c].data();
    for (int c = 0; c < numOut; ++c) outPtr[(size_t) c] = outStore[(size_t) c].data();

    AudioBusBuffers inBus {}, outBus {};
    inBus .numChannels = numIn;  inBus .channelBuffers32 = inPtr .data();
    outBus.numChannels = numOut; outBus.channelBuffers32 = outPtr.data();

    ProcessData data {};
    data.processMode        = kRealtime;
    data.symbolicSampleSize = kSample32;
    data.numSamples         = blockSize;
    data.numInputs  = 1; data.inputs  = &inBus;
    data.numOutputs = 1; data.outputs = &outBus;

    for (int b = 0; b < blocks; ++b)
    {
        for (int c = 0; c < numIn; ++c)
            for (int s = 0; s < blockSize; ++s)
                inStore[(size_t) c][(size_t) s] = (float) (c + 1);

        if (proc.process (data) != kResultOk)
        {
            proc.setProcessing (false);
            component.setActive (false);
            return -1;
        }
    }

    int reordered = 0;

    for (int c = 0; c < numOut; ++c)
        if (outStore[(size_t) c][(size_t) blockSize - 1] != (float) (c + 1))
            ++reordered;

    proc.setProcessing (false);
    component.setActive (false);
    return reordered;
}

} // namespace

//==============================================================================
int main (int argc, char** argv)
{
    // Plugins bring their own threads along (ambix_encoder starts a network
    // discovery hub as it constructs), and on Windows those have been enough
    // to lose whatever was still sitting in stdout's buffer when the process
    // exits — the JSON below went missing while the exit code stayed 0.
    // Unbuffered from the start, so nothing depends on a clean teardown.
    setvbuf (stdout, nullptr, _IONBF, 0);

    std::string pluginPath;
    SpeakerArrangement inArr = 0, outArr = 0;
    bool negotiate = false, checkOrder = false;
    int blocks = 8;

    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];

        if      (a == "--plugin"  && i + 1 < argc) pluginPath = argv[++i];
        else if (a == "--in-arr"  && i + 1 < argc) { inArr  = strtoull (argv[++i], nullptr, 0); negotiate = true; }
        else if (a == "--out-arr" && i + 1 < argc) { outArr = strtoull (argv[++i], nullptr, 0); negotiate = true; }
        else if (a == "--blocks"  && i + 1 < argc) blocks = atoi (argv[++i]);
        else if (a == "--check-order")             checkOrder = true;
    }

    if (pluginPath.empty())
    {
        fprintf (stderr,
                 "Usage: ambix_bustest --plugin <path.vst3>\n"
                 "                     [--in-arr <mask>] [--out-arr <mask>]\n"
                 "                     [--check-order] [--blocks N]\n");
        return 1;
    }

    Module module (pluginPath);

    if (module.factory == nullptr)
    {
        fprintf (stderr, "ambix_bustest: %s: %s\n", pluginPath.c_str(), module.error.c_str());
        return 2;
    }

    PClassInfo classInfo {};
    IComponent* component = nullptr;

    for (int32 i = 0; i < module.factory->countClasses(); ++i)
    {
        module.factory->getClassInfo (i, &classInfo);

        if (std::strcmp (classInfo.category, kVstAudioEffectClass) == 0
            && module.factory->createInstance (classInfo.cid, IComponent::iid, (void**) &component) == kResultOk)
            break;
    }

    if (component == nullptr || component->initialize (nullptr) != kResultOk)
    {
        fprintf (stderr, "ambix_bustest: %s: no audio component\n", pluginPath.c_str());
        return 2;
    }

    IAudioProcessor* proc = nullptr;
    component->queryInterface (IAudioProcessor::iid, (void**) &proc);

    if (proc == nullptr)
    {
        fprintf (stderr, "ambix_bustest: %s: no IAudioProcessor\n", pluginPath.c_str());
        return 2;
    }

    printf ("{\n  \"plugin\": \"%s\",\n  \"buses\": [", classInfo.name);

    bool firstEntry = true;
    printBuses (*component, *proc, "default", firstEntry);

    bool negotiated = false;

    if (negotiate)
    {
        negotiated = proc->setBusArrangements (&inArr, 1, &outArr, 1) == kResultOk;
        printBuses (*component, *proc, "negotiated", firstEntry);
    }

    printf ("\n  ],\n  \"negotiated\": %s", negotiate ? (negotiated ? "true" : "false") : "null");

    if (checkOrder)
    {
        BusInfo in {}, out {};
        component->getBusInfo (kAudio, kInput,  0, in);
        component->getBusInfo (kAudio, kOutput, 0, out);

        printf (",\n  \"reordered_channels\": %d",
                countReorderedChannels (*component, *proc, in.channelCount, out.channelCount, blocks));
    }

    printf ("\n}\n");
    fflush (stdout);

    component->terminate();
    return 0;
}
