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
 back to the host, which is its own bug (see discreteBusDefault below);
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
    reorder (ACN order is the same on both sides), but discrete does not
    survive the trip back: VST3 has no arrangement for it, so the plugin can
    no longer answer `IAudioProcessor::getBusArrangement()` for that bus.
    That matters because the host's proposal is only ACN-labelled by luck —
    the VST3 constants kAmbi5th/6th/7thOrderACN happen to be the plain
    36/49/64-bit masks, so a host that asks by channel count lands on
    ambisonic for orders 5-7 and on a speaker layout for orders 1-4. Without
    this, every ambix plugin came out of a 1st-to-4th-order negotiation
    unable to describe its own buses.

    Buses that are not ambisonic to begin with — the encoder's sources, the
    decoder's loudspeakers, vmic's virtual mics — have no such identity to
    fall back on and stay discrete. That no longer costs them the ability to
    describe themselves: JUCE_patches/juce_VST3Common.h.patch gives
    discreteChannels(N) an arrangement of its own, the plain N-bit mask, for
    any N up to 64. */
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

/** Default layout for a bus of N independent, non-ambisonic channels (the
    encoder's sources, the decoder's loudspeakers).

    VST3 has no speaker arrangement for `discreteChannels(N)` with N > 1: JUCE
    maps only the first discrete channel to a speaker bit, so
    `getVst3SpeakerArrangement()` comes back empty and the wrapper answers
    `IAudioProcessor::getBusArrangement()` with kResultFalse — the plugin
    cannot tell the host what is on that bus. A host that reads a bus before
    negotiating can take that failure as "not negotiable" and leave the plugin
    on its default layout - which in the universal build is 1st order, so the
    encoder looked like it could not work out the order (reported from Max 9,
    mcs.vst~, where every square-bus ambix plugin negotiated fine).

    So under VST3 start at mono and let the host negotiate the width upward —
    the same move AMBI_CH_SET makes for the ambisonic buses. Every other
    wrapper negotiates nothing and takes the default as the channel count, so
    there the full width stays. */
inline juce::AudioChannelSet discreteBusDefault (int numChannels)
{
    return juce::PluginHostType::getPluginLoadedAs() == juce::AudioProcessor::wrapperType_VST3
             ? juce::AudioChannelSet::mono()
             : juce::AudioChannelSet::discreteChannels (numChannels);
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
