#pragma once

#include <cmath>

#ifndef AMBI_ORDER
#define AMBI_ORDER 5
#endif

#ifdef UNIVERSAL_AMBISONIC
  #ifndef MAX_AMBI_ORDER
    #define MAX_AMBI_ORDER 7
  #endif
  #define AMBI_CHANNELS ((MAX_AMBI_ORDER + 1) * (MAX_AMBI_ORDER + 1))
  #define INIT_AMBI_CHANNELS AMBI_CHANNELS
#else
  #ifndef MAX_AMBI_ORDER
    #define MAX_AMBI_ORDER AMBI_ORDER
  #endif
  #define AMBI_CHANNELS ((AMBI_ORDER + 1) * (AMBI_ORDER + 1))
  #define INIT_AMBI_CHANNELS AMBI_CHANNELS
#endif

inline bool isValidAmbisonicChannelCount (int n)
{
    if (n <= 0 || n > AMBI_CHANNELS) return false;
    int root = (int) std::sqrt ((double) n);
    return root * root == n;
}

inline int ambiOrderFromChannels (int n)
{
    return (int) std::sqrt ((double) n) - 1;
}

inline int ambiChannelsFromOrder (int order)
{
    return (order + 1) * (order + 1);
}

// Default bus channel set.
// Universal builds: VST3 defaults to ambisonic(1) so the host can negotiate
// upward; other wrappers default to ambisonic(7).
// VST2 builds use discrete channels (no speaker arrangement mapping needed).
#ifdef UNIVERSAL_AMBISONIC
  #define AMBI_CH_SET_VST3   juce::AudioChannelSet::ambisonic (1)
  #define AMBI_CH_SET_OTHER  juce::AudioChannelSet::ambisonic (ambiOrderFromChannels (AMBI_CHANNELS))
  #define AMBI_CH_SET(n) ((juce::PluginHostType::getPluginLoadedAs() \
                           == juce::AudioProcessor::wrapperType_VST3) \
                           ? AMBI_CH_SET_VST3 : AMBI_CH_SET_OTHER)
#else
  #define AMBI_CH_SET(n) juce::AudioChannelSet::discreteChannels (n)
#endif
