/*
 ==============================================================================
 ReaperVST3Integration.h — pull the currently loaded REAPER project file name
 through the VST3 host-application bridge so we can include it in our NSD
 announcement payload. No-op in every other host.

 Uses JUCE's VST3ClientExtensions hook: the wrapper calls setIHostApplication
 shortly after the AudioProcessor is constructed; if the host is REAPER 5.02+,
 its IHostApplication also exposes IReaperHostApplication, which lets us look
 up REAPER API functions by name — here we grab `EnumProjects`.

 Part of the ambix Ambisonic plug-in suite (GPLv2+).
 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshadow-field-in-constructor",
                                     "-Wnon-virtual-dtor",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wunused-parameter")

#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace ambix_reaper
{
    using namespace Steinberg;
    using uint32 = Steinberg::uint32;

    // ---- Minimal REAPER IReaperHostApplication declaration --------------
    // Mirrors JUCE/examples/Plugins/extern/reaper_vst3_interfaces.h — we
    // vendor it here (header-only) so the encoder doesn't need to pick up
    // the whole JUCE demo include tree. Only the bits we actually call are
    // declared; everything else is pass-through.
    class IReaperHostApplication : public FUnknown
    {
    public:
        virtual void* PLUGIN_API getReaperApi (CStringA funcname) = 0;
        virtual void* PLUGIN_API getReaperParent (uint32 w) = 0;
        virtual void* PLUGIN_API reaperExtended (uint32 call, void* p1, void* p2, void* p3) = 0;

        static const FUID iid;
    };

    DECLARE_CLASS_IID (IReaperHostApplication,
                       0x79655E36, 0x77EE4267, 0xA573FEF7, 0x4912C27C)

    // Must be emitted exactly once per binary; PluginProcessor.cpp owns the
    // DEF_CLASS_IID sibling via the provided macro.
}

//==============================================================================
/** VST3ClientExtensions subclass that, when loaded in REAPER, lets us ask
    REAPER which project *this* plugin instance belongs to — not just the
    active tab — and returns a thread-safe `getProjectName()` suitable for the
    NSD announcement payload.

    How the per-instance lookup works:
      - REAPER's IReaperHostApplication exposes `getReaperParent(3)`, which
        returns the `ReaProject*` that owns the AudioProcessor calling it.
        Reference: reaper-sdk/sdk/reaper_vst3_interfaces.h:10
            // get parent track(=1), take(=2), project(=3), fxdsp(=4), ...
      - We then call `GetProjectName(ReaProject*, buf, sz)` (a stateless C
        function from REAPER's extension API) to turn that handle into a
        displayable name.

    Two plugin instances in two different project tabs return two different
    `ReaProject*` values, so each correctly advertises its own project name. */
class ReaperVST3Integration final : public juce::VST3ClientExtensions
{
public:
    ReaperVST3Integration() = default;

    ~ReaperVST3Integration() override
    {
        // Release the host reference we addref'd via queryInterface.
        if (auto* h = reaperHost.exchange (nullptr, std::memory_order_acq_rel))
            h->release();
    }

    void setIHostApplication (Steinberg::FUnknown* ptr) override
    {
        if (ptr == nullptr)
            return;

        void* obj = nullptr;
        if (ptr->queryInterface (ambix_reaper::IReaperHostApplication::iid, &obj) != Steinberg::kResultOk
            || obj == nullptr)
            return;

        auto* host = static_cast<ambix_reaper::IReaperHostApplication*> (obj);

        // Hold the host pointer for the lifetime of this integration — we
        // need it later to call getReaperParent(3). queryInterface addref'd
        // for us; dtor releases.
        if (auto* old = reaperHost.exchange (host, std::memory_order_acq_rel))
            old->release();

        // Cache the stateless C APIs we'll use. Both may be null on ancient
        // REAPER builds; we handle that by falling back to EnumProjects(-1).
        if (auto* fn = host->getReaperApi ("GetProjectName"))
        {
            using GetProjectNameFn = void (*)(void*, char*, int);
            getProjectNameFn.store (reinterpret_cast<GetProjectNameFn> (fn),
                                    std::memory_order_release);
        }

        // Fallback: EnumProjects(-1, buf, sz) fills buf with the *active*
        // project's full path. Only used if getReaperParent(3) fails.
        if (auto* fn = host->getReaperApi ("EnumProjects"))
        {
            using EnumProjectsFn = void* (*)(int, char*, int);
            enumProjectsFn.store (reinterpret_cast<EnumProjectsFn> (fn),
                                  std::memory_order_release);
        }
    }

    /** Returns the base filename (no extension, no path) of the project this
        plugin instance lives in, or an empty string if we're not in REAPER or
        the project is unsaved. */
    juce::String getProjectName() const
    {
        auto* host = reaperHost.load (std::memory_order_acquire);

        // Preferred path: ask REAPER for OUR owning project, then stringify.
        if (host != nullptr)
        {
            if (auto* proj = host->getReaperParent (3))
            {
                if (auto* getName = getProjectNameFn.load (std::memory_order_acquire))
                {
                    char buf[2048] = {};
                    getName (proj, buf, (int) sizeof (buf) - 1);
                    buf[sizeof (buf) - 1] = '\0';

                    if (buf[0] == '\0')
                        return {}; // unsaved project → no stable name

                    // GetProjectName typically returns the basename already,
                    // but defend against variants by letting juce::File strip
                    // any directory / extension that happens to sneak in.
                    return juce::File (juce::String (juce::CharPointer_UTF8 (buf)))
                             .getFileNameWithoutExtension();
                }
            }
        }

        // Fallback for very old REAPER builds without getReaperParent or
        // GetProjectName: report the active project so at least one instance
        // gets the right name.
        if (auto* enumP = enumProjectsFn.load (std::memory_order_acquire))
        {
            char buf[2048] = {};
            enumP (-1, buf, (int) sizeof (buf) - 1);
            buf[sizeof (buf) - 1] = '\0';

            if (buf[0] == '\0')
                return {};

            return juce::File (juce::String (juce::CharPointer_UTF8 (buf)))
                     .getFileNameWithoutExtension();
        }

        return {};
    }

private:
    using EnumProjectsFn   = void* (*)(int, char*, int);
    using GetProjectNameFn = void  (*)(void*, char*, int);

    std::atomic<ambix_reaper::IReaperHostApplication*> reaperHost { nullptr };
    std::atomic<EnumProjectsFn>   enumProjectsFn   { nullptr };
    std::atomic<GetProjectNameFn> getProjectNameFn { nullptr };
};
