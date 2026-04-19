#pragma once

#include <JuceHeader.h>

#include "../model/GroupRegistry.h"

// Groups management tab: list of groups with visibility toggle, inline-rename,
// delete button, plus an "Add group" button at the top.
class GroupPanel : public juce::Component,
                   public juce::ChangeListener,
                   public juce::TableListBoxModel
{
public:
    explicit GroupPanel (GroupRegistry& groupRegistry);
    ~GroupPanel() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    // TableListBoxModel
    int  getNumRows() override;
    void paintRowBackground (juce::Graphics&, int row, int w, int h, bool selected) override;
    void paintCell (juce::Graphics&, int row, int col, int w, int h, bool selected) override;
    juce::Component* refreshComponentForCell (int row, int col, bool selected,
                                              juce::Component* existing) override;

    // ChangeListener
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

private:
    enum ColumnId { ColVisible = 1, ColName, ColDelete };

    class NameCell;
    class VisibleCell;
    class DeleteCell;

    GroupRegistry& groupRegistry;
    juce::TableListBox table;
    juce::TextButton addButton { "+ Add group" };
    std::vector<GroupRegistry::Group> cache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupPanel)
};
