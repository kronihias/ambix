#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <cstdint>
#include <functional>

// Parameter objects for plug-ins that used JUCE's legacy setParameter() API.
//
// The point is to change nothing a host can see while moving off that API:
//  - the parameter ID is the index as a string, which is what JUCE's wrappers
//    hashed for a legacy parameter, so VST3/AU automation keeps its target,
//  - every parameter stays a plain 0..1 float, so step counts and the
//    normalised mapping of switch-like parameters are untouched,
//  - every value set on a parameter reaches the plug-in synchronously, on the
//    thread that set it, in the same place the setParameter() override used
//    to run. Side effects therefore happen in the same order as before, for
//    host automation (audio thread), state restore, editors and OSC alike.
// What does change: the reported default is the value a fresh instance
// starts at (legacy parameters reported 0), and hosts can ask for the text of
// any value, not just the current one.
//
// tests/test_parameters.py and tests/test_parameter_sweep.py hold a plug-in
// to this.

namespace ambix
{

struct ParameterOwner
{
    virtual ~ParameterOwner() = default;

    // Called for every value set on one of the owner's parameters, from
    // whatever thread set it - often the audio thread, so it has to be
    // real-time safe: no locks, no allocation, no message posting.
    virtual void parameterValueSet (int index, float newValue) = 0;

    // Bumped after every set. An editor compares it from its own timer
    // instead of listening to a ChangeBroadcaster that the audio thread
    // would have to post to.
    std::atomic<uint32_t> parameterGeneration { 0 };
};

class Parameter : public juce::AudioParameterFloat
{
public:
    using TextFunction = std::function<juce::String (float value, int maximumStringLength)>;

    // For a parameter whose legacy setParameter() stored a quantised value
    // (a switch kept as 0 / 0.5 / 1, say): maps any 0..1 value to the one it
    // stores. Applied before the value is stored, so getValue(), the text and
    // the side effects all see the snapped value - as getParameter() did -
    // while the host still sees a continuous parameter (no step count).
    // It may capture the owner, e.g. to keep the current value for inputs
    // the legacy code ignored.
    using SnapFunction = std::function<float (float)>;

    Parameter (ParameterOwner& ownerToUse, int parameterIndex, const juce::String& name,
               float initialValue, TextFunction valueToText, SnapFunction snap = {})
        : AudioParameterFloat (juce::ParameterID { juce::String (parameterIndex), 1 }, name,
                               makeRange (std::move (snap)), initialValue,
                               juce::AudioParameterFloatAttributes()
                                   .withStringFromValueFunction (std::move (valueToText))),
          owner (ownerToUse), index (parameterIndex)
    {
    }

    // State restore. Always runs the owner's side effects, even for the value
    // the parameter already holds, and doesn't notify the host - exactly what
    // calling the legacy setParameter() from setStateInformation() did.
    // (operator= skips equal values; setValueNotifyingHost() notifies.)
    void restore (float newValue)
    {
        static_cast<juce::AudioProcessorParameter&> (*this).setValue (newValue);
    }

    int getIndex() const noexcept { return index; }

    // The parameter at `parameterIndex` of `processor`, or nullptr if that
    // isn't an ambix::Parameter.
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

// For editors: true once per change of the owner's parameters since the last
// call. Message thread only.
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
