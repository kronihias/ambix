#pragma once

#include <JuceHeader.h>

// Groups let the user organise sources. Each source can be in at most one
// group (groupId 0 = none). Groups can be named, deleted, and toggled
// visible/hidden; hiding a group hides all its sources in both views.
class GroupRegistry : public juce::ChangeBroadcaster
{
public:
    struct Group
    {
        int id { 0 };
        juce::String name;
        bool visible { true };
    };

    GroupRegistry() = default;

    // Returns the id of the newly-created group.
    int  add (const juce::String& name);
    void remove (int id);
    void rename (int id, const juce::String& newName);
    void setVisible (int id, bool visible);

    // A source with groupId 0 (or an unknown group id) is always treated as
    // visible.
    bool isVisible (int groupId) const;

    std::vector<Group>    snapshot() const;
    std::optional<Group>  find (int id) const;

private:
    mutable juce::CriticalSection lock;
    std::map<int, Group> groups;
    int nextId { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupRegistry)
};
