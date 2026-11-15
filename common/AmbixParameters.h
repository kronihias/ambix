#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <cstdint>
#include <functional>

// Parameter objects replacing JUCE's legacy setParameter() API without
// changing what a host sees: the ID is the index as a string (what JUCE
// hashed for legacy parameters, so saved automation keeps its target), values
// stay plain 0..1 floats, and each set reaches the plug-in synchronously on
// the calling thread. Pinned by tests/test_parameters.py.

namespace ambix
{

// Must match the hashing of JUCE's VST3 wrapper.
#ifdef JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS
 constexpr bool kStudioOneCompatibleIds = JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS;
#else
 constexpr bool kStudioOneCompatibleIds = true;
#endif

struct ParameterOwner
{
    virtual ~ParameterOwner() = default;

    // Called on whatever thread set the value, often the audio thread:
    // must be real-time safe (no locks, allocation or message posting).
    virtual void parameterValueSet (int index, float newValue) = 0;

    // Bumped after every set; editors poll it so the audio thread never
    // has to post a change message.
    std::atomic<uint32_t> parameterGeneration { 0 };
};

class Parameter : public juce::AudioParameterFloat
{
public:
    using TextFunction = std::function<juce::String (float value, int maximumStringLength)>;

    // Quantises a value before it is stored (e.g. a 0 / 0.5 / 1 switch), so
    // getValue(), text and side effects see the snapped value while the host
    // still sees a continuous parameter without a step count.
    using SnapFunction = std::function<float (float)>;

    // parameterId: leave empty for the index as a string (see idFor()).
    Parameter (ParameterOwner& ownerToUse, int parameterIndex, const juce::String& name,
               float initialValue, TextFunction valueToText, SnapFunction snap = {},
               const juce::String& parameterId = {})
        : AudioParameterFloat (juce::ParameterID { parameterId.isNotEmpty() ? parameterId : juce::String (parameterIndex), 1 }, name,
                               makeRange (std::move (snap)), initialValue,
                               juce::AudioParameterFloatAttributes()
                                   .withStringFromValueFunction (std::move (valueToText))),
          owner (ownerToUse), index (parameterIndex)
    {
    }

    // State restore: runs the side effects even for an unchanged value and
    // doesn't notify the host (operator= skips equal values;
    // setValueNotifyingHost() notifies).
    void restore (float newValue)
    {
        static_cast<juce::AudioProcessorParameter&> (*this).setValue (newValue);
    }

    int getIndex() const noexcept { return index; }

    // The index as a string, except on VST3 where its hash would equal the
    // bypass ID (numParameters, see juce_vst3_legacy_bypass_id.patch): the
    // single digits hash to 48..57, so with 48..57 parameters one would
    // collide. That one gets a derived ID; the bypass keeps its own.
    static juce::String idFor (const juce::AudioProcessor& processor, int parameterIndex, int numParameters)
    {
        const juce::String plain (parameterIndex);

        if (processor.wrapperType == juce::AudioProcessor::wrapperType_VST3
            && juce::VST3ClientExtensions::convertJuceParameterId (plain, kStudioOneCompatibleIds)
                   == (uint32_t) numParameters)
        {
            const auto derived = plain + "_vst3";
            jassert (juce::VST3ClientExtensions::convertJuceParameterId (derived, kStudioOneCompatibleIds)
                     >= (uint32_t) (numParameters + 10)); // above every single-digit hash and the bypass
            return derived;
        }

        return plain;
    }

    static Parameter* at (juce::AudioProcessor& processor, int parameterIndex)
    {
        auto& params = processor.getParameters();
        return juce::isPositiveAndBelow (parameterIndex, params.size())
                 ? dynamic_cast<Parameter*> (params[parameterIndex]) : nullptr;
    }

private:
    static juce::NormalisableRange<float> makeRange (SnapFunction snap)
    {
        if (! snap)
            return { 0.0f, 1.0f };

        return { 0.0f, 1.0f,
                 [] (float, float, float v) { return v; },
                 [] (float, float, float v) { return v; },
                 [snap] (float, float, float v) { return snap (v); } };
    }

    void valueChanged (float newValue) override
    {
        owner.parameterValueSet (index, newValue);
        owner.parameterGeneration.fetch_add (1, std::memory_order_release);
    }

    ParameterOwner& owner;
    const int index;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Parameter)
};

// For editors, message thread only.
class ParameterChangeWatcher
{
public:
    explicit ParameterChangeWatcher (ParameterOwner& o) : owner (o) {}

    bool changed() noexcept
    {
        const auto now = owner.parameterGeneration.load (std::memory_order_acquire);
        const bool result = now != seen;
        seen = now;
        return result;
    }

private:
    ParameterOwner& owner;
    uint32_t seen = 0xffffffffu; // report a change the first time, to prime the editor
};

} // namespace ambix
