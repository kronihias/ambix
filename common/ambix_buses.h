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
 bug in applyBusLayouts by rewriting any named layout with >= 3 channels
 to one JUCE's reorder table leaves alone: an ambisonic bus at an ambisonic
 width becomes ambisonic(order), anything else becomes discreteChannels(N).
 Neither is a named VST3 arrangement to reorder against, so the table falls
 back to identity bit-position order. See toReorderSafeLayout() below.

 A discrete layout used to cost the plugin the ability to report that bus
 back to the host at all — VST3 had no arrangement for it, so a host that
 read a bus before negotiating (Max/MSP among them) took the failure as
 "not negotiable" and left the plugin on its default layout.
 JUCE_patches/juce_VST3Common.h.patch fixes that at the source by giving
 discreteChannels(N) an arrangement for any N up to 64.

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

#include <cmath>

namespace ambix
{

/** Rewrite any named multichannel layout so JUCE's VST3 wrapper skips its
    channel-reorder table.

    Named layouts with < 3 channels (mono, stereo) are left alone — their
    VST3/JUCE orders are identical and keeping the name helps hosts pick
    layouts by semantics. Ambisonic channel sets are also left alone — they
    use ACN order in both JUCE and VST3, and keeping the ambisonic identity
    is occasionally useful for hosts that route by semantics.

    An ambisonic bus is rewritten to `ambisonic(order)` rather than
    `discreteChannels(N)` whenever N is an ambisonic width. Both dodge the
    reorder, and both can be reported back to the host — but only the
    ambisonic one reports the truth. The host's proposal is ACN-labelled by
    luck rather than intent: the VST3 constants kAmbi5th/6th/7thOrderACN
    happen to be the plain 36/49/64-bit masks, so a host that asks by channel
    count lands on ambisonic for orders 5-7 and on a speaker layout for
    orders 1-4. Keeping the ambisonic identity means a 16-channel bus reads
    as 3rd order rather than sixteen anonymous speakers, whichever way the
    host happened to ask.

    Buses that are not ambisonic to begin with — the encoder's sources, the
    decoder's loudspeakers, vmic's virtual mics — have no such identity to
    fall back on and stay discrete. That costs them nothing:
    JUCE_patches/juce_VST3Common.h.patch gives discreteChannels(N) an
    arrangement of its own, the plain N-bit mask, for any N up to 64. */
inline juce::AudioProcessor::BusesLayout
toReorderSafeLayout (const juce::AudioProcessor& processor,
                  const juce::AudioProcessor::BusesLayout& in,
                  int minChannelsToRewrite = 3)
{
    auto rewrite = [&processor, minChannelsToRewrite] (juce::AudioChannelSet& cs,
                                                       bool isInput, int busIndex)
    {
        const int n = cs.size();
        // Skip if already discrete, or if ambisonic (ACN order matches VST3 → no reorder)
        if (n < minChannelsToRewrite
            || cs.isDiscreteLayout()
            || cs.getAmbisonicOrder() >= 0)
            return;

        if (const auto* bus = processor.getBus (isInput, busIndex))
        {
            if (bus->getDefaultLayout().getAmbisonicOrder() >= 0)
            {
                const int order = (int) std::sqrt ((double) n) - 1;

                if (order >= 0 && order <= 7 && (order + 1) * (order + 1) == n)
                {
                    cs = juce::AudioChannelSet::ambisonic (order);
                    return;
                }
            }
        }

        cs = juce::AudioChannelSet::discreteChannels (n);
    };

    auto out = in;
    for (int i = 0; i < out.inputBuses.size();  ++i) rewrite (out.inputBuses.getReference  (i), true,  i);
    for (int i = 0; i < out.outputBuses.size(); ++i) rewrite (out.outputBuses.getReference (i), false, i);
    return out;
}

} // namespace ambix

/** applyBusLayouts override that rewrites any named multichannel layout
    (22.2, 7.1.4, etc.) to a reorder-safe one before the base class stores
    it. Prevents JUCE's VST3 wrapper from reordering channels per the named
    layout's speaker positions.

    Expand this inside each ambix plugin's AudioProcessor subclass (public),
    guarded by #ifdef UNIVERSAL_AMBISONIC. */
#define AMBIX_APPLY_BUS_LAYOUTS_OVERRIDE                                        \
    bool applyBusLayouts (const BusesLayout& layouts) override                  \
    {                                                                           \
        return juce::AudioProcessor::applyBusLayouts (ambix::toReorderSafeLayout (*this, layouts)); \
    }
