#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Compatibility shim for setParameterNotifyingHost which was removed in newer JUCE.
// Replicates the old behavior: sets the parameter value and notifies the host.
//
// In JUCE 8, getParameters() returns an empty array for legacy processors that
// override getNumParameters()/setParameter() instead of using addParameter().
// Fall back to calling setParameter() directly in that case.
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
inline void setParameterNotifyingHost (juce::AudioProcessor* processor, int index, float newValue)
{
    auto& params = processor->getParameters();
    if (index < params.size())
        params[index]->setValueNotifyingHost (newValue);
    else
        processor->setParameter (index, newValue);
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
