/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ==============================================================================
 */

#ifndef __JUCER_HEADER_AMBIXENCODERAUDIOPROCESSOREDITOR_PLUGINEDITOR_EF118A61__
#define __JUCER_HEADER_AMBIXENCODERAUDIOPROCESSOREDITOR_PLUGINEDITOR_EF118A61__

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "SphereOpenGL.h"
#include "HammerAitoffView.h"
#include "Settings.h"

class Ambix_encoderAudioProcessorEditor;

// One row in the per-source table. Layout is a single horizontal strip:
//   [#]  [Az]  [El]  [Size]  [S][M]
// Az/El/Size are number boxes (Slider::LinearBar with transparent bar);
// S (solo) and M (mute) are toggle buttons that work in both linked and
// unlinked modes (they're orthogonal to position). Az/El/Size are
// greyed out in linked mode but mute/solo stay editable.
class SourceTableRow : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::Button::Listener
{
public:
    SourceTableRow (Ambix_encoderAudioProcessor& p, int sourceIndex);
    void resized() override;
    void paint   (juce::Graphics& g) override;
    void sliderValueChanged (juce::Slider* s) override;
    void buttonClicked      (juce::Button* b) override;

    void refreshFromProcessor();
    void setEditable (bool canEdit);

private:
    Ambix_encoderAudioProcessor& processor;
    int idx;
    juce::Slider       num_az, num_el, num_size;
    juce::TextButton   btn_solo, btn_mute;
    juce::Label        lbl;
    bool editable { true };
};

// Column header for the source table. Painted statically.
class SourceTableHeader : public juce::Component
{
public:
    void paint (juce::Graphics& g) override;
};


//==============================================================================
class Ambix_encoderAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           public juce::Slider::Listener,
                                           public juce::ChangeListener,
                                           public juce::Button::Listener,
                                           public juce::Timer,
                                           public juce::TextEditor::Listener,
                                           public juce::ComboBox::Listener
{
public:
    Ambix_encoderAudioProcessorEditor (Ambix_encoderAudioProcessor* ownerFilter);
    ~Ambix_encoderAudioProcessorEditor();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;
    void comboBoxChanged (juce::ComboBox* box) override;

    void textEditorFocusLost (juce::TextEditor& ed) override;
    void textEditorReturnKeyPressed (juce::TextEditor& ed) override;

    void changeListenerCallback (juce::ChangeBroadcaster *source) override;

    void modifierKeysChanged (const juce::ModifierKeys &modifiers ) override;

    void timerCallback() override;

    void updateID();

    // Binary resources:
    static const char* settings_png;
    static const int settings_pngSize;
    static const char* settings_white_png;
    static const int settings_white_pngSize;

private:
    void rebuildTable();
    void updateActivePanner();
    void layoutTableRows (int viewportWidth);

    juce::LookAndFeel_V3 globalLaF;

    juce::Slider sld_el;
    juce::Slider sld_az;
    juce::Slider sld_size;
    juce::Slider sld_width;
    juce::Slider sld_speed;
    juce::Slider sld_el_move;
    juce::Slider sld_az_move;
    juce::TextEditor txt_az_move;
    juce::TextEditor txt_el_move;
    juce::Label lbl_id;
    juce::ImageButton btn_settings;
    juce::TextEditor txt_id;

    // New controls
    juce::TextButton btn_view_sphere    { "Sphere" };
    juce::TextButton btn_view_hammer    { "Hammer-Aitoff" };
    juce::TextButton btn_linked_toggle  { "Linked" };
    juce::ComboBox   cmb_active_sources;
    juce::Label      lbl_sources        { {}, "Sources:" };

    SphereOpenGL      sphere_opengl;
    HammerAitoffView  hammer_view;

    // Source table — header + scrollable list of rows.
    SourceTableHeader     table_header;
    juce::Viewport        table_viewport;
    juce::Component       table_holder;
    juce::OwnedArray<SourceTableRow> table_rows;

    juce::ResizableCornerComponent resizer { this, &resizeLimits };
    juce::ComponentBoundsConstrainer resizeLimits;

    bool changed_;
    bool _hammerView = false;

    juce::CriticalSection lock_;

    juce::Component::SafePointer<juce::DialogWindow> _settingsDialogWindow;

    Ambix_encoderAudioProcessor* getProcessor() const
    {
        return static_cast<Ambix_encoderAudioProcessor*> (getAudioProcessor());
    }

    juce::TooltipWindow tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_encoderAudioProcessorEditor)
};

#endif
