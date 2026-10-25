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
                 [--params]           report every parameter as the edit
                                      controller describes it
                 [--set ID=VALUE]     send a normalised value through
                                      process() first, as host automation
                                      does (repeatable)
                 [--state-in FILE]    restore component state from FILE first
                 [--state-out FILE]   write the component state to FILE last
                 [--step SPEC]        scripted step on one instance: SPEC is
                                      "ID=V,ID=V" (applied in order, one
                                      block), optionally "wait=MS" and
                                      "block=N"; after each step report
                                      latency, output RMS and correlation,
                                      every value and text (repeatable)
                 [--offline]          process the steps in offline mode, so
                                      plug-ins that skip late work in real
                                      time are deterministic
                 [--late-state]       with --step: restore --state-in after
                                      activating rather than before
                 [--sweep V1,V2,...]  per automatable parameter and value, a
                                      fresh instance with only that parameter
                                      set: output RMS, changed parameters,
                                      displayed text and a state hash

 The parameter options check what a saved session depends on (VST3 IDs,
 flags, defaults, display strings, state), which JUCE hosting JUCE would hide.

 A mask is a VST3 SpeakerArrangement: bit per speaker. `(1 << n) - 1` is what
 a host that asks by channel count sends, and is also how kAmbi5th/6th/7th
 OrderACN are defined - which is why orders 5-7 behaved differently from
 orders 1-4 before this was understood.

 ==============================================================================
 */

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/common/memorystream.h"

#include <cmath>
#include <cstdio>
#include <initializer_list>
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

#if defined (_WIN32)
 #include <io.h>
#else
 #include <unistd.h>
#endif

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace
{

using GetFactoryFn = IPluginFactory* (*) ();

// The original stdout; plugin output goes to stderr so it can't corrupt the
// JSON report.
FILE* report = stdout;

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

    fprintf (report, "%s\n    {\"stage\": \"%s\", \"dir\": \"%s\", \"index\": %d, \"channels\": %d, "
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

//==============================================================================
/*  Minimal IParameterChanges: one point per parameter at offset 0. Lives on
    the stack, so reference counting is a formality. */
struct ValueQueue final : public IParamValueQueue
{
    ParamID id = 0;
    ParamValue value = 0;

    ParamID PLUGIN_API getParameterId() override                 { return id; }
    int32   PLUGIN_API getPointCount() override                  { return 1; }
    tresult PLUGIN_API getPoint (int32 index, int32& offset, ParamValue& v) override
    {
        if (index != 0) return kResultFalse;
        offset = 0;
        v = value;
        return kResultOk;
    }
    tresult PLUGIN_API addPoint (int32, ParamValue, int32&) override { return kResultFalse; }

    tresult PLUGIN_API queryInterface (const TUID, void** obj) override { *obj = nullptr; return kNoInterface; }
    uint32  PLUGIN_API addRef() override  { return 1; }
    uint32  PLUGIN_API release() override { return 1; }
};

struct ParamChanges final : public IParameterChanges
{
    std::vector<ValueQueue> queues;

    int32 PLUGIN_API getParameterCount() override { return (int32) queues.size(); }
    IParamValueQueue* PLUGIN_API getParameterData (int32 index) override
    {
        return index >= 0 && index < (int32) queues.size() ? &queues[(size_t) index] : nullptr;
    }
    IParamValueQueue* PLUGIN_API addParameterData (const ParamID&, int32&) override { return nullptr; }

    tresult PLUGIN_API queryInterface (const TUID, void** obj) override { *obj = nullptr; return kNoInterface; }
    uint32  PLUGIN_API addRef() override  { return 1; }
    uint32  PLUGIN_API release() override { return 1; }
};

/*  Process `blocks` blocks of silence, handing `changes` to the first one. */
bool processWithChanges (IComponent& component, IAudioProcessor& proc,
                         IParameterChanges* changes, int blocks)
{
    constexpr int blockSize = 64;

    BusInfo in {}, out {};
    component.getBusInfo (kAudio, kInput,  0, in);
    component.getBusInfo (kAudio, kOutput, 0, out);

    const int numIn = in.channelCount, numOut = out.channelCount;

    component.activateBus (kAudio, kInput,  0, true);
    component.activateBus (kAudio, kOutput, 0, true);

    ProcessSetup setup { kRealtime, kSample32, blockSize, 48000.0 };

    if (proc.setupProcessing (setup) != kResultOk)
        return false;

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

    bool ok = true;

    for (int b = 0; b < blocks && ok; ++b)
    {
        data.inputParameterChanges = b == 0 ? changes : nullptr;
        ok = proc.process (data) == kResultOk;
    }

    proc.setProcessing (false);
    component.setActive (false);
    return ok;
}

/*  String128 -> JSON string body; non-ASCII is replaced, not encoded. */
std::string jsonString (const TChar* s)
{
    std::string out;

    for (; s != nullptr && *s != 0; ++s)
    {
        const auto c = (unsigned) *s;

        if (c == '"' || c == '\\')  { out += '\\'; out += (char) c; }
        else if (c < 0x20 || c > 0x7e) out += '?';
        else                         out += (char) c;
    }

    return out;
}

bool readFile (const std::string& path, std::vector<char>& bytes)
{
    auto* f = fopen (path.c_str(), "rb");
    if (f == nullptr) return false;

    char buf[4096];
    size_t n;
    while ((n = fread (buf, 1, sizeof (buf), f)) > 0)
        bytes.insert (bytes.end(), buf, buf + n);

    fclose (f);
    return true;
}

bool writeFile (const std::string& path, const void* data, size_t size)
{
    auto* f = fopen (path.c_str(), "wb");
    if (f == nullptr) return false;

    const bool ok = fwrite (data, 1, size, f) == size;
    fclose (f);
    return ok;
}

/*  Each parameter as the controller reports it, plus its text for a few
    probe values (both ends, the middle and two off-axis points). */
void printParameters (IEditController& controller)
{
    static const ParamValue probes[] = { 0.0, 0.25, 0.5, 0.55, 0.75, 1.0 };

    fprintf (report, ",\n  \"parameters\": [");

    for (int32 i = 0; i < controller.getParameterCount(); ++i)
    {
        ParameterInfo info {};
        controller.getParameterInfo (i, info);

        fprintf (report, "%s\n    {\"index\": %d, \"id\": %u, \"title\": \"%s\", \"short_title\": \"%s\", "
                "\"units\": \"%s\", \"step_count\": %d, \"default\": %.9g, \"unit_id\": %d, "
                "\"can_automate\": %s, \"read_only\": %s, \"is_bypass\": %s, \"is_list\": %s, "
                "\"value\": %.9g, \"text\": {",
                i == 0 ? "" : ",", (int) i, (unsigned) info.id,
                jsonString (info.title).c_str(), jsonString (info.shortTitle).c_str(),
                jsonString (info.units).c_str(), (int) info.stepCount,
                info.defaultNormalizedValue, (int) info.unitId,
                (info.flags & ParameterInfo::kCanAutomate) ? "true" : "false",
                (info.flags & ParameterInfo::kIsReadOnly)  ? "true" : "false",
                (info.flags & ParameterInfo::kIsBypass)    ? "true" : "false",
                (info.flags & ParameterInfo::kIsList)      ? "true" : "false",
                controller.getParamNormalized (info.id));

        for (size_t p = 0; p < sizeof (probes) / sizeof (probes[0]); ++p)
        {
            String128 text {};
            controller.getParamStringByValue (info.id, probes[p], text);
            fprintf (report, "%s\"%g\": \"%s\"", p == 0 ? "" : ", ", probes[p], jsonString (text).c_str());
        }

        fprintf (report, "}}");
    }

    fprintf (report, "\n  ]");
}

//==============================================================================
/*  The per-parameter sweep. Every run gets a fresh instance so no state
    (smoothing, accumulators, cached matrices) leaks between runs. */
struct Instance
{
    IComponent*       component    = nullptr;
    IAudioProcessor*  proc         = nullptr;
    IEditController*  controller   = nullptr;
    IConnectionPoint* componentCP  = nullptr;
    IConnectionPoint* controllerCP = nullptr;

    bool create (IPluginFactory& factory, const TUID cid)
    {
        if (factory.createInstance (cid, IComponent::iid, (void**) &component) != kResultOk
            || component == nullptr || component->initialize (nullptr) != kResultOk)
            return false;

        component->queryInterface (IAudioProcessor::iid, (void**) &proc);

        TUID controllerCID {};

        if (proc == nullptr
            || component->getControllerClassId (controllerCID) != kResultOk
            || factory.createInstance (controllerCID, IEditController::iid, (void**) &controller) != kResultOk
            || controller == nullptr || controller->initialize (nullptr) != kResultOk)
            return false;

        component ->queryInterface (IConnectionPoint::iid, (void**) &componentCP);
        controller->queryInterface (IConnectionPoint::iid, (void**) &controllerCP);

        if (componentCP != nullptr && controllerCP != nullptr)
        {
            componentCP ->connect (controllerCP);
            controllerCP->connect (componentCP);
        }

        syncController();
        return true;
    }

    void syncController()
    {
        MemoryStream stream;

        if (component->getState (&stream) == kResultOk)
        {
            stream.seek (0, IBStream::kIBSeekSet, nullptr);
            controller->setComponentState (&stream);
        }
    }

    ~Instance()
    {
        if (componentCP != nullptr && controllerCP != nullptr)
        {
            componentCP ->disconnect (controllerCP);
            controllerCP->disconnect (componentCP);
        }

        for (FUnknown* p : std::initializer_list<FUnknown*> { componentCP, controllerCP, proc })
            if (p != nullptr) p->release();

        if (controller != nullptr) { controller->terminate(); controller->release(); }
        if (component  != nullptr) { component ->terminate(); component ->release(); }
    }
};

struct SweepResult
{
    bool ok = false;
    std::vector<double> rms;            // per output channel, settled part only
    std::vector<double> proj;           // per output channel: correlation with a
                                        // fixed +-1 input mix, to catch sign flips
    std::vector<ParamValue> values;     // every parameter, index order
    std::string text;                   // the swept parameter, as displayed
    uint64 stateHash = 0;               // FNV-1a over the saved state
};

/*  Fresh instance; `changes` (may be null) arrive with the first block, then
    deterministic noise until any smoothing has settled. */
SweepResult sweepRun (IPluginFactory& factory, const TUID cid,
                      IParameterChanges* changes, ParamID textId)
{
    constexpr int blockSize = 256, blocks = 16, measuredBlocks = 8;
    SweepResult r;
    Instance inst;

    if (! inst.create (factory, cid))
        return r;

    auto& component = *inst.component;
    auto& proc      = *inst.proc;

    BusInfo in {}, out {};
    component.getBusInfo (kAudio, kInput,  0, in);
    component.getBusInfo (kAudio, kOutput, 0, out);

    const int numIn = in.channelCount, numOut = out.channelCount;

    component.activateBus (kAudio, kInput,  0, true);
    component.activateBus (kAudio, kOutput, 0, true);

    ProcessSetup setup { kRealtime, kSample32, blockSize, 48000.0 };

    if (proc.setupProcessing (setup) != kResultOk)
        return r;

    component.setActive (true);
    proc.setProcessing (true);

    std::vector<std::vector<float>> inStore  ((size_t) numIn,  std::vector<float> ((size_t) blockSize));
    std::vector<std::vector<float>> outStore ((size_t) numOut, std::vector<float> ((size_t) blockSize));
    std::vector<float*> inPtr ((size_t) numIn), outPtr ((size_t) numOut);
    std::vector<uint32> seeds ((size_t) numIn);
    std::vector<double> sumSq ((size_t) numOut, 0.0), sumProj ((size_t) numOut, 0.0);
    std::vector<float> mix ((size_t) blockSize);

    for (int c = 0; c < numIn;  ++c) { inPtr [(size_t) c] = inStore [(size_t) c].data(); seeds[(size_t) c] = 7919u * (uint32) c + 1u; }
    for (int c = 0; c < numOut; ++c)   outPtr[(size_t) c] = outStore[(size_t) c].data();

    AudioBusBuffers inBus {}, outBus {};
    inBus .numChannels = numIn;  inBus .channelBuffers32 = inPtr .data();
    outBus.numChannels = numOut; outBus.channelBuffers32 = outPtr.data();

    ProcessData data {};
    data.processMode        = kRealtime;
    data.symbolicSampleSize = kSample32;
    data.numSamples         = blockSize;
    data.numInputs  = 1; data.inputs  = &inBus;
    data.numOutputs = 1; data.outputs = &outBus;

    bool ok = true;

    for (int b = 0; b < blocks && ok; ++b)
    {
        for (int c = 0; c < numIn; ++c)
            for (int s = 0; s < blockSize; ++s)
            {
                auto& seed = seeds[(size_t) c];
                seed = seed * 1664525u + 1013904223u;
                inStore[(size_t) c][(size_t) s] = (float) (seed >> 8) / (float) (1u << 24) - 0.5f;
            }

        data.inputParameterChanges = b == 0 ? changes : nullptr;
        ok = proc.process (data) == kResultOk;

        if (b >= blocks - measuredBlocks)
        {
            std::fill (mix.begin(), mix.end(), 0.0f);

            for (int c = 0; c < numIn; ++c)
            {
                const float w = (((uint32) c * 2654435761u) >> 7) & 1u ? 1.0f : -1.0f;
                for (int s = 0; s < blockSize; ++s)
                    mix[(size_t) s] += w * inStore[(size_t) c][(size_t) s];
            }

            for (int c = 0; c < numOut; ++c)
                for (int s = 0; s < blockSize; ++s)
                {
                    const double y = outStore[(size_t) c][(size_t) s];
                    sumSq  [(size_t) c] += y * y;
                    sumProj[(size_t) c] += y * mix[(size_t) s];
                }
        }
    }

    proc.setProcessing (false);
    component.setActive (false);

    if (! ok)
        return r;

    for (auto s : sumSq)
        r.rms.push_back (std::sqrt (s / (blockSize * measuredBlocks)));

    for (auto s : sumProj)
        r.proj.push_back (s / (blockSize * measuredBlocks));

    inst.syncController();

    auto& controller = *inst.controller;

    for (int32 i = 0; i < controller.getParameterCount(); ++i)
    {
        ParameterInfo info {};
        controller.getParameterInfo (i, info);
        r.values.push_back (controller.getParamNormalized (info.id));
    }

    String128 text {};
    controller.getParamStringByValue (textId, controller.getParamNormalized (textId), text);
    r.text = jsonString (text);

    MemoryStream stream;

    if (component.getState (&stream) == kResultOk)
    {
        r.stateHash = 14695981039346656037ull;
        auto* bytes = (const unsigned char*) stream.getData();

        for (TSize i = 0; i < stream.getSize(); ++i)
            r.stateHash = (r.stateHash ^ bytes[i]) * 1099511628211ull;
    }

    r.ok = true;
    return r;
}

void printDoubles (const std::vector<double>& v)
{
    fprintf (report, "[");
    for (size_t i = 0; i < v.size(); ++i)
        fprintf (report, "%s%.7g", i == 0 ? "" : ", ", v[i]);
    fprintf (report, "]");
}

/*  One run per automatable, non-bypass parameter and value, compared with
    an untouched run so side effects and coupling show up. */
void printSweep (IPluginFactory& factory, const TUID cid, const std::vector<ParamValue>& probes)
{
    auto baseline = sweepRun (factory, cid, nullptr, 0);

    if (! baseline.ok)
    {
        fprintf (report, ",\n  \"sweep\": null");
        return;
    }

    std::vector<ParameterInfo> infos;
    {
        Instance inst;
        if (inst.create (factory, cid))
            for (int32 i = 0; i < inst.controller->getParameterCount(); ++i)
            {
                ParameterInfo info {};
                inst.controller->getParameterInfo (i, info);
                infos.push_back (info);
            }
    }

    fprintf (report, ",\n  \"sweep\": {\n    \"baseline_rms\": ");
    printDoubles (baseline.rms);
    fprintf (report, ",\n    \"baseline_proj\": ");
    printDoubles (baseline.proj);
    fprintf (report, ",\n    \"baseline_state\": \"%016llx\",\n    \"runs\": [", (unsigned long long) baseline.stateHash);

    bool first = true;

    for (const auto& info : infos)
    {
        if ((info.flags & ParameterInfo::kCanAutomate) == 0 || (info.flags & ParameterInfo::kIsBypass) != 0)
            continue;

        for (auto probe : probes)
        {
            ParamChanges changes;
            ValueQueue q;
            q.id = info.id;
            q.value = probe;
            changes.queues.push_back (q);

            auto r = sweepRun (factory, cid, &changes, info.id);

            fprintf (report, "%s\n      {\"id\": %u, \"set\": %g, \"ok\": %s",
                     first ? "" : ",", (unsigned) info.id, probe, r.ok ? "true" : "false");
            first = false;

            if (r.ok)
            {
                fprintf (report, ", \"text\": \"%s\", \"state\": \"%016llx\", \"rms\": ",
                         r.text.c_str(), (unsigned long long) r.stateHash);
                printDoubles (r.rms);
                fprintf (report, ", \"proj\": ");
                printDoubles (r.proj);

                fprintf (report, ", \"changed\": {");
                bool firstChanged = true;

                for (size_t i = 0; i < r.values.size() && i < baseline.values.size() && i < infos.size(); ++i)
                    if (std::abs (r.values[i] - baseline.values[i]) > 1e-7)
                    {
                        fprintf (report, "%s\"%u\": %.9g", firstChanged ? "" : ", ",
                                 (unsigned) infos[i].id, r.values[i]);
                        firstChanged = false;
                    }

                fprintf (report, "}");
            }

            fprintf (report, "}");
        }
    }

    fprintf (report, "\n    ]\n  }");
}

//==============================================================================
/*  Scripted sequences on one instance, for behaviour that depends on order
    or history (same-block conflicts, presets then tweaks, background loads). */
struct Step
{
    std::vector<ValueQueue> changes;    // in queue order, all in the step's first block
    int waitMs = 0;                     // sleep before the step (background loading)
    int blockSize = 0;                  // re-activate with this block size first
};

/*  "48=0.25,57=1" -> two changes in that order; "wait=2000" -> a pause;
    "block=512" -> re-activate with that block size; may be combined; ""
    just processes. */
Step parseStep (const std::string& text)
{
    Step step;

    for (size_t pos = 0; pos < text.size();)
    {
        auto comma = text.find (',', pos);
        if (comma == std::string::npos) comma = text.size();

        const auto item = text.substr (pos, comma - pos);
        const auto eq = item.find ('=');

        if (eq != std::string::npos)
        {
            const auto key = item.substr (0, eq);
            const auto value = item.substr (eq + 1);

            if (key == "wait")
                step.waitMs = atoi (value.c_str());
            else if (key == "block")
                step.blockSize = atoi (value.c_str());
            else
            {
                ValueQueue q;
                q.id    = (ParamID) strtoul (key.c_str(), nullptr, 0);
                q.value = strtod (value.c_str(), nullptr);
                step.changes.push_back (q);
            }
        }

        pos = comma + 1;
    }

    return step;
}

void sleepMs (int ms)
{
   #if defined (_WIN32)
    Sleep ((DWORD) ms);
   #else
    usleep ((useconds_t) ms * 1000);
   #endif
}

void printSteps (IPluginFactory& factory, const TUID cid, const std::vector<Step>& steps,
                 bool negotiate, SpeakerArrangement inArr, SpeakerArrangement outArr,
                 const std::vector<char>& stateBytes, bool offline, bool lateState)
{
    // Offline, a plug-in may wait for its worker threads (binaural does), so
    // results don't depend on machine load.
    const auto mode = offline ? kOffline : kRealtime;

    constexpr int maxBlockSize = 4096, blocksPerStep = 8, measuredBlocks = 4;
    int blockSize = 256;

    Instance inst;

    if (! inst.create (factory, cid))
    {
        fprintf (report, ",\n  \"steps\": null");
        return;
    }

    auto& component  = *inst.component;
    auto& proc       = *inst.proc;
    auto& controller = *inst.controller;

    if (negotiate)
        proc.setBusArrangements (&inArr, 1, &outArr, 1);

    const auto restoreState = [&]
    {
        MemoryStream stream (const_cast<char*> (stateBytes.data()), (TSize) stateBytes.size());
        component.setState (&stream);
        inst.syncController();
    };

    if (! stateBytes.empty() && ! lateState)
        restoreState();

    BusInfo in {}, out {};
    component.getBusInfo (kAudio, kInput,  0, in);
    component.getBusInfo (kAudio, kOutput, 0, out);

    const int numIn = in.channelCount, numOut = out.channelCount;

    component.activateBus (kAudio, kInput,  0, true);
    component.activateBus (kAudio, kOutput, 0, true);

    ProcessSetup setup { mode, kSample32, blockSize, 48000.0 };

    if (proc.setupProcessing (setup) != kResultOk)
    {
        fprintf (report, ",\n  \"steps\": null");
        return;
    }

    component.setActive (true);
    proc.setProcessing (true);

    // A plug-in that reloads on state restore and on block-size changes
    // (binaural) loads only once if the state comes after activation.
    if (! stateBytes.empty() && lateState)
        restoreState();

    std::vector<std::vector<float>> inStore  ((size_t) numIn,  std::vector<float> ((size_t) maxBlockSize));
    std::vector<std::vector<float>> outStore ((size_t) numOut, std::vector<float> ((size_t) maxBlockSize));
    std::vector<float*> inPtr ((size_t) numIn), outPtr ((size_t) numOut);
    std::vector<uint32> seeds ((size_t) numIn);
    std::vector<float> mix ((size_t) maxBlockSize);

    for (int c = 0; c < numIn;  ++c) { inPtr [(size_t) c] = inStore [(size_t) c].data(); seeds[(size_t) c] = 7919u * (uint32) c + 1u; }
    for (int c = 0; c < numOut; ++c)   outPtr[(size_t) c] = outStore[(size_t) c].data();

    AudioBusBuffers inBus {}, outBus {};
    inBus .numChannels = numIn;  inBus .channelBuffers32 = inPtr .data();
    outBus.numChannels = numOut; outBus.channelBuffers32 = outPtr.data();

    ProcessData data {};
    data.processMode        = mode;
    data.symbolicSampleSize = kSample32;
    data.numSamples         = blockSize;
    data.numInputs  = 1; data.inputs  = &inBus;
    data.numOutputs = 1; data.outputs = &outBus;

    fprintf (report, ",\n  \"channels\": [%d, %d],\n  \"steps\": [", numIn, numOut);

    for (size_t si = 0; si < steps.size(); ++si)
    {
        const auto& step = steps[si];

        if (step.waitMs > 0)
            sleepMs (step.waitMs);

        if (step.blockSize > 0 && step.blockSize <= maxBlockSize)
        {
            proc.setProcessing (false);
            component.setActive (false);
            blockSize = setup.maxSamplesPerBlock = data.numSamples = step.blockSize;
            proc.setupProcessing (setup);
            component.setActive (true);
            proc.setProcessing (true);
        }

        ParamChanges changes;
        changes.queues = step.changes;

        std::vector<double> sumSq ((size_t) numOut, 0.0), sumProj ((size_t) numOut, 0.0);
        bool ok = true;

        for (int b = 0; b < blocksPerStep && ok; ++b)
        {
            for (int c = 0; c < numIn; ++c)
                for (int s = 0; s < blockSize; ++s)
                {
                    auto& seed = seeds[(size_t) c];
                    seed = seed * 1664525u + 1013904223u;
                    inStore[(size_t) c][(size_t) s] = (float) (seed >> 8) / (float) (1u << 24) - 0.5f;
                }

            data.inputParameterChanges = (b == 0 && ! changes.queues.empty()) ? &changes : nullptr;
            ok = proc.process (data) == kResultOk;

            if (b >= blocksPerStep - measuredBlocks)
            {
                std::fill (mix.begin(), mix.end(), 0.0f);

                for (int c = 0; c < numIn; ++c)
                {
                    const float w = (((uint32) c * 2654435761u) >> 7) & 1u ? 1.0f : -1.0f;
                    for (int s = 0; s < blockSize; ++s)
                        mix[(size_t) s] += w * inStore[(size_t) c][(size_t) s];
                }

                for (int c = 0; c < numOut; ++c)
                    for (int s = 0; s < blockSize; ++s)
                    {
                        const double y = outStore[(size_t) c][(size_t) s];
                        sumSq  [(size_t) c] += y * y;
                        sumProj[(size_t) c] += y * mix[(size_t) s];
                    }
            }
        }

        std::vector<double> rms, proj;
        for (auto s : sumSq)   rms .push_back (std::sqrt (s / (blockSize * measuredBlocks)));
        for (auto s : sumProj) proj.push_back (s / (blockSize * measuredBlocks));

        inst.syncController();

        fprintf (report, "%s\n    {\"ok\": %s, \"latency\": %d, \"rms\": ",
                 si == 0 ? "" : ",", ok ? "true" : "false", (int) proc.getLatencySamples());
        printDoubles (rms);
        fprintf (report, ", \"proj\": ");
        printDoubles (proj);
        fprintf (report, ", \"values\": {");

        for (int32 i = 0; i < controller.getParameterCount(); ++i)
        {
            ParameterInfo info {};
            controller.getParameterInfo (i, info);

            const auto v = controller.getParamNormalized (info.id);
            String128 text {};
            controller.getParamStringByValue (info.id, v, text);

            fprintf (report, "%s\"%u\": [%.9g, \"%s\"]", i == 0 ? "" : ", ",
                     (unsigned) info.id, v, jsonString (text).c_str());
        }

        fprintf (report, "}}");
    }

    fprintf (report, "\n  ]");

    proc.setProcessing (false);
    component.setActive (false);
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

   #if defined (_WIN32)
    const int reportFd = _dup (_fileno (stdout));
    _dup2 (_fileno (stderr), _fileno (stdout));
    report = _fdopen (reportFd, "w");
   #else
    const int reportFd = dup (STDOUT_FILENO);
    dup2 (STDERR_FILENO, STDOUT_FILENO);
    report = fdopen (reportFd, "w");
   #endif

    if (report == nullptr)
        report = stderr;

    setvbuf (report, nullptr, _IONBF, 0);

    std::string pluginPath;
    SpeakerArrangement inArr = 0, outArr = 0;
    bool negotiate = false, checkOrder = false, reportParams = false;
    int blocks = 8;
    std::string stateIn, stateOut;
    ParamChanges changes;
    std::vector<ParamValue> sweepProbes;
    std::vector<Step> steps;
    bool offline = false, lateState = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];

        if      (a == "--plugin"  && i + 1 < argc) pluginPath = argv[++i];
        else if (a == "--in-arr"  && i + 1 < argc) { inArr  = strtoull (argv[++i], nullptr, 0); negotiate = true; }
        else if (a == "--out-arr" && i + 1 < argc) { outArr = strtoull (argv[++i], nullptr, 0); negotiate = true; }
        else if (a == "--blocks"  && i + 1 < argc) blocks = atoi (argv[++i]);
        else if (a == "--check-order")             checkOrder = true;
        else if (a == "--params")                  reportParams = true;
        else if (a == "--state-in"  && i + 1 < argc) stateIn  = argv[++i];
        else if (a == "--state-out" && i + 1 < argc) stateOut = argv[++i];
        else if (a == "--step" && i + 1 < argc)    steps.push_back (parseStep (argv[++i]));
        else if (a == "--offline")                 offline = true;
        else if (a == "--late-state")              lateState = true;
        else if (a == "--sweep" && i + 1 < argc)
        {
            const std::string list = argv[++i];
            for (size_t pos = 0; pos < list.size();)
            {
                const auto comma = list.find (',', pos);
                sweepProbes.push_back (strtod (list.substr (pos, comma - pos).c_str(), nullptr));
                pos = comma == std::string::npos ? list.size() : comma + 1;
            }
        }
        else if (a == "--set" && i + 1 < argc)
        {
            const std::string kv = argv[++i];
            const auto eq = kv.find ('=');

            if (eq == std::string::npos)
            {
                fprintf (stderr, "ambix_bustest: --set expects ID=VALUE, got %s\n", kv.c_str());
                return 1;
            }

            ValueQueue q;
            q.id    = (ParamID) strtoul (kv.substr (0, eq).c_str(), nullptr, 0);
            q.value = strtod (kv.substr (eq + 1).c_str(), nullptr);
            changes.queues.push_back (q);
        }
    }

    if (pluginPath.empty())
    {
        fprintf (stderr,
                 "Usage: ambix_bustest --plugin <path.vst3>\n"
                 "                     [--in-arr <mask>] [--out-arr <mask>]\n"
                 "                     [--check-order] [--blocks N]\n"
                 "                     [--params] [--set ID=VALUE]...\n"
                 "                     [--state-in FILE] [--state-out FILE]\n"
                 "                     [--sweep V1,V2,...] [--step SPEC]...\n");
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

    fprintf (report, "{\n  \"plugin\": \"%s\",\n  \"buses\": [", classInfo.name);

    bool firstEntry = true;
    printBuses (*component, *proc, "default", firstEntry);

    bool negotiated = false;

    if (negotiate)
    {
        negotiated = proc->setBusArrangements (&inArr, 1, &outArr, 1) == kResultOk;
        printBuses (*component, *proc, "negotiated", firstEntry);
    }

    fprintf (report, "\n  ],\n  \"negotiated\": %s", negotiate ? (negotiated ? "true" : "false") : "null");

    if (checkOrder)
    {
        BusInfo in {}, out {};
        component->getBusInfo (kAudio, kInput,  0, in);
        component->getBusInfo (kAudio, kOutput, 0, out);

        fprintf (report, ",\n  \"reordered_channels\": %d",
                countReorderedChannels (*component, *proc, in.channelCount, out.channelCount, blocks));
    }

    const bool wantController = reportParams || ! stateIn.empty() || ! stateOut.empty()
                                || ! changes.queues.empty();

    IEditController* controller = nullptr;
    IConnectionPoint* componentCP = nullptr;
    IConnectionPoint* controllerCP = nullptr;

    if (wantController)
    {
        // JUCE's wrapper finds its processor through this connection.
        TUID controllerCID {};

        if (component->getControllerClassId (controllerCID) != kResultOk
            || module.factory->createInstance (controllerCID, IEditController::iid, (void**) &controller) != kResultOk
            || controller == nullptr || controller->initialize (nullptr) != kResultOk)
        {
            fprintf (stderr, "ambix_bustest: %s: no edit controller\n", pluginPath.c_str());
            return 2;
        }

        component ->queryInterface (IConnectionPoint::iid, (void**) &componentCP);
        controller->queryInterface (IConnectionPoint::iid, (void**) &controllerCP);

        if (componentCP != nullptr && controllerCP != nullptr)
        {
            componentCP ->connect (controllerCP);
            controllerCP->connect (componentCP);
        }

        // Push component state to the controller, as a host does; without a
        // message loop this is also how values changed by process() get there.
        const auto syncController = [&]
        {
            MemoryStream stream;

            if (component->getState (&stream) == kResultOk)
            {
                stream.seek (0, IBStream::kIBSeekSet, nullptr);
                controller->setComponentState (&stream);
            }
        };

        if (stateIn.empty())
            syncController();

        if (! stateIn.empty())
        {
            std::vector<char> bytes;

            if (! readFile (stateIn, bytes))
            {
                fprintf (stderr, "ambix_bustest: cannot read %s\n", stateIn.c_str());
                return 2;
            }

            // as a host does: component first, then the same blob to the controller
            MemoryStream forComponent (bytes.data(), (TSize) bytes.size());
            MemoryStream forController (bytes.data(), (TSize) bytes.size());

            fprintf (report, ",\n  \"state_restored\": %s",
                    component->setState (&forComponent) == kResultOk ? "true" : "false");
            controller->setComponentState (&forController);
        }

        if (! changes.queues.empty())
        {
            fprintf (report, ",\n  \"automation_processed\": %s",
                    processWithChanges (*component, *proc, &changes, blocks) ? "true" : "false");
            syncController();
        }

        if (reportParams)
            printParameters (*controller);

        if (! stateOut.empty())
        {
            MemoryStream stream;
            const bool ok = component->getState (&stream) == kResultOk
                            && writeFile (stateOut, stream.getData(), (size_t) stream.getSize());
            fprintf (report, ",\n  \"state_saved\": %s", ok ? "true" : "false");
        }
    }

    if (! sweepProbes.empty())
        printSweep (*module.factory, classInfo.cid, sweepProbes);

    if (! steps.empty())
    {
        std::vector<char> stateBytes;
        if (! stateIn.empty() && ! readFile (stateIn, stateBytes))
        {
            fprintf (stderr, "ambix_bustest: cannot read %s\n", stateIn.c_str());
            return 2;
        }
        printSteps (*module.factory, classInfo.cid, steps, negotiate, inArr, outArr, stateBytes, offline, lateState);
    }

    fprintf (report, "\n}\n");
    fflush (report);

    if (componentCP != nullptr && controllerCP != nullptr)
    {
        componentCP ->disconnect (controllerCP);
        controllerCP->disconnect (componentCP);
    }

    if (controller != nullptr)
        controller->terminate();

    component->terminate();
    return 0;
}
