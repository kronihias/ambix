#pragma once

#include <JuceHeader.h>

#include "../model/GroupRegistry.h"
#include "../model/SourceRegistry.h"
#include "../net/NetworkAdvertiser.h"

class SourceListPanel : public juce::Component,
                        public juce::ChangeListener,
                        public juce::TableListBoxModel,
                        private juce::Timer
{
public:
    SourceListPanel (SourceRegistry& registry, GroupRegistry& groupRegistry,
                     NetworkAdvertiser& advertiser);
    ~SourceListPanel() override;

    void resized() override;

    // TableListBoxModel
    int  getNumRows() override;
    void paintRowBackground (juce::Graphics&, int rowNumber,
                             int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics&, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell (int rowNumber, int columnId,
                                              bool isRowSelected,
                                              juce::Component* existingComp) override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void commitReplyPort (const EncoderSource::Key& key, int port);
    void commitIcon      (const EncoderSource::Key& key, IconCatalogue::Icon icon);
    void commitGroup     (const EncoderSource::Key& key, int groupId);
    void removeSource    (const EncoderSource::Key& key);

    // Pops up a menu anchored at `target` with actions to create / reuse a
    // group matching `project` for the clicked source (or all sources sharing
    // the same advertised project). See ProjectCell::mouseDown.
    void showProjectGroupMenu (const EncoderSource::Key& clickedKey,
                               const juce::String& project,
                               juce::Component* target);

    const GroupRegistry& getGroupRegistry() const { return groupRegistry; }

private:
    SourceRegistry& registry;
    GroupRegistry&  groupRegistry;
    NetworkAdvertiser& advertiser;
    juce::TableListBox table;
    std::vector<EncoderSource>                       sourceCache;
    std::vector<GroupRegistry::Group>                groupCache;
    std::vector<NetworkAdvertiser::DiscoveredEncoder> encoderCache;
    // Set by changeListenerCallback when the panel isn't currently visible —
    // the 20 Hz timer picks it up and refreshes caches lazily. Avoids doing
    // snapshot work on every OSC tick while we're on a different tab.
    bool cachesDirty { false };

    enum ColumnId
    {
        ColId = 1,
        ColName,
        ColAzEl,
        ColProject,
        ColIcon,
        ColGroup,
        ColLevel,
        ColSender,
        ColReply,
        ColRemove
    };

    void timerCallback() override;

    void drawLevelMeter (juce::Graphics& g,
                         const EncoderSource& source,
                         juce::Rectangle<int> area) const;

    // Best-effort project lookup: matches an Ambix source back to the encoder
    // that's broadcasting it by (senderIp, encoderId). Returns empty for
    // MultiEncoder sources (no NSD advertisement) and for Ambix sources whose
    // encoder either never advertised `proj=` or hasn't been heard from yet.
    juce::String lookupProjectFor (const EncoderSource& s) const;

    class ReplyPortCell;
    class IconCell;
    class GroupCell;
    class ProjectCell;
    class RemoveCell;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceListPanel)
};
