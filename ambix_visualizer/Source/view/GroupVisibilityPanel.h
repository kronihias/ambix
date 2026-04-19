#pragma once

#include <JuceHeader.h>

#include "../model/GroupRegistry.h"

// Small floating panel shown in the top-right of the 2D and 3D views. Lists
// every group with a checkbox that toggles its visibility live. When no
// groups exist the panel hides itself entirely.
class GroupVisibilityPanel : public juce::Component,
                             public juce::ChangeListener
{
public:
    explicit GroupVisibilityPanel (GroupRegistry& groups);
    ~GroupVisibilityPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    // Caller should resize to this height so every group fits.
    int getPreferredHeight() const noexcept;
    int getPreferredWidth()  const noexcept { return 170; }

private:
    void rebuildRows();

    GroupRegistry& groups;
    struct Row
    {
        int groupId;
        std::unique_ptr<juce::ToggleButton> toggle;
    };
    std::vector<Row> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupVisibilityPanel)
};
