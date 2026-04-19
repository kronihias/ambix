#include "GroupPanel.h"

class GroupPanel::NameCell : public juce::Component
{
public:
    NameCell (GroupPanel& owner) : panel (owner)
    {
        editor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
        editor.setColour (juce::TextEditor::outlineColourId,    juce::Colour (0x00000000));
        editor.setColour (juce::TextEditor::focusedOutlineColourId,
                          juce::Colours::white.withAlpha (0.4f));
        editor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
        editor.onReturnKey = [this]() { commit(); editor.giveAwayKeyboardFocus(); };
        editor.onFocusLost = [this]() { commit(); };
        addAndMakeVisible (editor);
    }
    void resized() override { editor.setBounds (getLocalBounds()); }

    void setGroup (int groupId, const juce::String& currentName)
    {
        id = groupId;
        if (! editor.hasKeyboardFocus (true))
            editor.setText (currentName, juce::dontSendNotification);
    }
private:
    void commit() { panel.groupRegistry.rename (id, editor.getText()); }
    GroupPanel& panel;
    juce::TextEditor editor;
    int id { 0 };
};

class GroupPanel::VisibleCell : public juce::Component
{
public:
    VisibleCell (GroupPanel& owner) : panel (owner)
    {
        toggle.onClick = [this]() { panel.groupRegistry.setVisible (id, toggle.getToggleState()); };
        addAndMakeVisible (toggle);
    }
    void resized() override { toggle.setBounds (getLocalBounds().withSizeKeepingCentre (24, 24)); }
    void setGroup (int groupId, bool visible)
    {
        id = groupId;
        toggle.setToggleState (visible, juce::dontSendNotification);
    }
private:
    GroupPanel& panel;
    juce::ToggleButton toggle;
    int id { 0 };
};

class GroupPanel::DeleteCell : public juce::Component
{
public:
    DeleteCell (GroupPanel& owner) : panel (owner)
    {
        btn.onClick = [this]() { panel.groupRegistry.remove (id); };
        addAndMakeVisible (btn);
    }
    void resized() override { btn.setBounds (getLocalBounds().reduced (4, 2)); }
    void setGroup (int groupId) { id = groupId; }
private:
    GroupPanel& panel;
    juce::TextButton btn { "Delete" };
    int id { 0 };
};

GroupPanel::GroupPanel (GroupRegistry& gr)
    : groupRegistry (gr)
{
    auto& header = table.getHeader();
    header.addColumn ("Visible", ColVisible, 70, 50, 100);
    header.addColumn ("Name",    ColName,   240, 80, 600);
    header.addColumn ("",        ColDelete,  80, 60, 120);

    table.setModel (this);
    table.setRowHeight (28);
    table.getHeader().setStretchToFitActive (true);
    addAndMakeVisible (table);

    addButton.onClick = [this]()
    {
        groupRegistry.add ({}); // auto-named "Group N"
    };
    addAndMakeVisible (addButton);

    groupRegistry.addChangeListener (this);
    cache = groupRegistry.snapshot();
}

GroupPanel::~GroupPanel()
{
    groupRegistry.removeChangeListener (this);
}

void GroupPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1B1B1B));
}

void GroupPanel::resized()
{
    auto area = getLocalBounds();
    auto top = area.removeFromTop (40).reduced (8, 6);
    addButton.setBounds (top.removeFromLeft (120));
    table.setBounds (area.reduced (4));
}

int GroupPanel::getNumRows() { return static_cast<int> (cache.size()); }

void GroupPanel::paintRowBackground (juce::Graphics& g, int row, int, int, bool selected)
{
    const auto base = (row % 2 == 0) ? juce::Colour (0xFF262626) : juce::Colour (0xFF202020);
    g.fillAll (selected ? base.brighter (0.15f) : base);
}

void GroupPanel::paintCell (juce::Graphics& g, int row, int col, int w, int h, bool)
{
    // Name / Visible / Delete all use components; nothing to paint here.
    juce::ignoreUnused (g, row, col, w, h);
}

juce::Component* GroupPanel::refreshComponentForCell (int row, int col, bool,
                                                     juce::Component* existing)
{
    if (row < 0 || row >= static_cast<int> (cache.size()))
    {
        delete existing;
        return nullptr;
    }
    const auto& group = cache[static_cast<size_t> (row)];

    switch (col)
    {
        case ColVisible:
        {
            auto* cell = dynamic_cast<VisibleCell*> (existing);
            if (cell == nullptr) { delete existing; cell = new VisibleCell (*this); }
            cell->setGroup (group.id, group.visible);
            return cell;
        }
        case ColName:
        {
            auto* cell = dynamic_cast<NameCell*> (existing);
            if (cell == nullptr) { delete existing; cell = new NameCell (*this); }
            cell->setGroup (group.id, group.name);
            return cell;
        }
        case ColDelete:
        {
            auto* cell = dynamic_cast<DeleteCell*> (existing);
            if (cell == nullptr) { delete existing; cell = new DeleteCell (*this); }
            cell->setGroup (group.id);
            return cell;
        }
        default: break;
    }
    delete existing;
    return nullptr;
}

void GroupPanel::changeListenerCallback (juce::ChangeBroadcaster*)
{
    cache = groupRegistry.snapshot();
    table.updateContent();
    table.repaint();
}
