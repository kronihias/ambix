/*
 ==============================================================================

 Shared channel-layout helper for the ambix plug-in suite.

 In the UNIVERSAL_AMBISONIC build, plugins accept any layout the host proposes
 (isBusesLayoutSupported returns true). When the host proposes a *named*
 multichannel VST3 arrangement — e.g. k714 (7.1.4 = 12 channels) or k222
 (22.2 = 24 channels) — JUCE's VST3 wrapper applies a channel reorder table
 to match the named layout's speaker positions. That reorder is invisible
 to the plugin and results in unexpected channel swaps (e.g. track channel
 20 <-> 24 on a 24-ch track).

 Ambisonic channel counts (1, 4, 9, 16, 25, 36, 49, 64) are ACN-ordered in
 both JUCE and VST3, so the reorder is a no-op at those counts. But because
 the universal build also accepts non-ambisonic counts (a user might load
 an ambix plugin on a generic multichannel track), we protect against the
 bug by rewriting any named layout with >= 3 channels to discreteChannels(N)
 in applyBusLayouts. Discrete layouts have no VST3 named arrangement match,
 so the reorder table falls back to identity bit-position order.

 Usage:
   - Include this header in PluginProcessor.h
   - Inside the AudioProcessor subclass (public), expand:
         #ifdef UNIVERSAL_AMBISONIC
         AMBIX_APPLY_BUS_LAYOUTS_OVERRIDE
         #endif

 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

namespace ambix
{

/** Rewrite any named multichannel layout to discreteChannels(N) so JUCE's
    VST3 wrapper skips its channel-reorder table.

    Named layouts with < 3 channels (mono, stereo) are left alone — their
    VST3/JUCE orders are identical and keeping the name helps hosts pick
    layouts by semantics. Ambisonic channel sets are also left alone — they
    use ACN order in both JUCE and VST3, and keeping the ambisonic identity
    is occasionally useful for hosts that route by semantics. */
inline juce::AudioProcessor::BusesLayout
toDiscreteLayout (const juce::AudioProcessor::BusesLayout& in,
                  int minChannelsToRewrite = 3)
{
    auto rewrite = [minChannelsToRewrite] (juce::AudioChannelSet& cs)
    {
        const int n = cs.size();
        // Skip if already discrete, or if ambisonic (ACN order matches VST3 → no reorder)
        if (n >= minChannelsToRewrite
            && ! cs.isDiscreteLayout()
            && cs.getAmbisonicOrder() < 0)
        {
            cs = juce::AudioChannelSet::discreteChannels (n);
        }
    };

    auto out = in;
    for (auto& cs : out.inputBuses)  rewrite (cs);
    for (auto& cs : out.outputBuses) rewrite (cs);
    return out;
}

} // namespace ambix

/** applyBusLayouts override that rewrites any named multichannel layout
    (22.2, 7.1.4, etc.) to discreteChannels(N) before the base class stores
    it. Prevents JUCE's VST3 wrapper from reordering channels per the named
    layout's speaker positions.

    Expand this inside each ambix plugin's AudioProcessor subclass (public),
    guarded by #ifdef UNIVERSAL_AMBISONIC. */
#define AMBIX_APPLY_BUS_LAYOUTS_OVERRIDE                                        \
    bool applyBusLayouts (const BusesLayout& layouts) override                  \
    {                                                                           \
        return juce::AudioProcessor::applyBusLayouts (ambix::toDiscreteLayout (layouts)); \
    }
