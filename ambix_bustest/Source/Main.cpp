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
                 [--set ID=VALUE]     before reporting, send a normalised
                                      value through process() the way a host
                                      plays automation (repeatable)
                 [--state-in FILE]    restore component state from FILE first
                 [--state-out FILE]   write the component state to FILE last

 The parameter options exist because moving a plugin from the legacy
 setParameter() API to parameter objects changes what JUCE's wrapper exports -
 VST3 parameter IDs, flags, defaults, display strings, the state blob - and
 every one of those is something a saved session depends on. JUCE hosting
 JUCE would paper over a mismatch in several of them.

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

// Where the JSON report goes: the process's original stdout. Everything a
// plugin prints (config loading, discovery) is sent to stderr instead, so it
// can't land in the middle of the report.
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
/*  Just enough IParameterChanges for one point per parameter at offset 0 -
    what a host sends when automation is a step rather than a ramp. Lives on
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

/*  Process `blocks` blocks of silence, handing `changes` to the first one.
    Returns false if the plugin refuses to set up or process. */
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

/*  String128 -> JSON string body. Parameter names here are ASCII; anything
    else is replaced rather than encoded, which is enough to notice a change. */
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

/*  Each parameter as the controller reports it, plus how it would display a
    few normalised values. Those values cover both ends, the middle (the
    rotators' 0°) and two points off any axis. */
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
                 "                     [--state-in FILE] [--state-out FILE]\n");
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
        // Wire controller and component together the way a host does; JUCE's
        // wrapper finds its processor through this connection.
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

        // A host shows the controller the component's state after creating
        // it, and again whenever it wants the display to catch up. Without
        // a message loop, that is also how values changed by process()
        // reach the controller here.
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

            // A host restores the component, then shows the same blob to the
            // controller so its parameter display catches up.
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
