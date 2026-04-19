#include "GroupRegistry.h"

int GroupRegistry::add (const juce::String& name)
{
    int id;
    {
        const juce::ScopedLock sl (lock);
        id = nextId++;
        groups[id] = { id, name.isEmpty() ? juce::String ("Group ") + juce::String (id) : name, true };
    }
    sendChangeMessage();
    return id;
}

void GroupRegistry::remove (int id)
{
    {
        const juce::ScopedLock sl (lock);
        groups.erase (id);
    }
    sendChangeMessage();
}

void GroupRegistry::rename (int id, const juce::String& newName)
{
    bool changed = false;
    {
        const juce::ScopedLock sl (lock);
        auto it = groups.find (id);
        if (it != groups.end() && it->second.name != newName)
        {
            it->second.name = newName;
            changed = true;
        }
    }
    if (changed)
        sendChangeMessage();
}

void GroupRegistry::setVisible (int id, bool v)
{
    bool changed = false;
    {
        const juce::ScopedLock sl (lock);
        auto it = groups.find (id);
        if (it != groups.end() && it->second.visible != v)
        {
            it->second.visible = v;
            changed = true;
        }
    }
    if (changed)
        sendChangeMessage();
}

bool GroupRegistry::isVisible (int groupId) const
{
    if (groupId <= 0)
        return true;
    const juce::ScopedLock sl (lock);
    auto it = groups.find (groupId);
    return it == groups.end() || it->second.visible;
}

std::vector<GroupRegistry::Group> GroupRegistry::snapshot() const
{
    const juce::ScopedLock sl (lock);
    std::vector<Group> out;
    out.reserve (groups.size());
    for (const auto& kv : groups)
        out.push_back (kv.second);
    return out;
}

std::optional<GroupRegistry::Group> GroupRegistry::find (int id) const
{
    const juce::ScopedLock sl (lock);
    auto it = groups.find (id);
    if (it == groups.end()) return std::nullopt;
    return it->second;
}
