#include "GroupVisibilityPanel.h"

namespace
{
    constexpr int kRowHeight   = 24;
    constexpr int kTitleHeight = 20;
    constexpr int kPadding     = 6;
}

GroupVisibilityPanel::GroupVisibilityPanel (GroupRegistry& g)
    : groups (g)
{
    setMouseClickGrabsKeyboardFocus (false);
    groups.addChangeListener (this);
    rebuildRows();
}

GroupVisibilityPanel::~GroupVisibilityPanel()
{
    groups.removeChangeListener (this);
}

int GroupVisibilityPanel::getPreferredHeight() const noexcept
{
    if (rows.empty())
        return 0;
    return kPadding * 2 + kTitleHeight + static_cast<int> (rows.size()) * kRowHeight;
}

void GroupVisibilityPanel::paint (juce::Graphics& g)
{
    if (rows.empty())
        return;

    auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.72f));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.2f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("Groups",
                bounds.removeFromTop (kTitleHeight + kPadding).reduced (kPadding, 2.0f),
                juce::Justification::centredLeft, false);
}

void GroupVisibilityPanel::resized()
{
    if (rows.empty())
        return;
    auto area = getLocalBounds().reduced (2).withTrimmedTop (kTitleHeight + kPadding);
    area.removeFromLeft (kPadding);
    area.removeFromRight (kPadding);
    for (auto& r : rows)
    {
        r.toggle->setBounds (area.removeFromTop (kRowHeight));
    }
}

void GroupVisibilityPanel::rebuildRows()
{
    rows.clear();
    const auto list = groups.snapshot();
    for (const auto& g : list)
    {
        Row r;
        r.groupId = g.id;
        r.toggle = std::make_unique<juce::ToggleButton> (
            g.name.isEmpty() ? juce::String ("Group ") + juce::String (g.id) : g.name);
        r.toggle->setToggleState (g.visible, juce::dontSendNotification);
        const int id = g.id;
        auto* gr = &groups;
        auto* t = r.toggle.get();
        r.toggle->onClick = [t, gr, id]()
        {
            gr->setVisible (id, t->getToggleState());
        };
        r.toggle->setColour (juce::ToggleButton::textColourId,
                             juce::Colours::white.withAlpha (0.9f));
        addAndMakeVisible (*r.toggle);
        rows.push_back (std::move (r));
    }
    setVisible (! rows.empty());
    resized();
}

void GroupVisibilityPanel::changeListenerCallback (juce::ChangeBroadcaster*)
{
    rebuildRows();

    // Parent layout may need to re-read our preferred size.
    if (auto* p = getParentComponent())
        p->resized();
}
