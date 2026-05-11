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

// Per-source row in the inspector list (unlinked mode).
class SourceInspectorRow : public juce::Component,
                           public juce::Slider::Listener
{
public:
    SourceInspectorRow (Ambix_encoderAudioProcessor& p, int sourceIndex);
    void resized() override;
    void paint   (juce::Graphics& g) override;
    void sliderValueChanged (juce::Slider* s) override;

    // Pull current param values into the slider widgets without firing
    // notification callbacks. Called from the editor's timer.
    void refreshFromProcessor();

private:
    Ambix_encoderAudioProcessor& processor;
    int idx;
    juce::Slider sld_az, sld_el, sld_size, sld_gain;
    juce::Label  lbl;
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
    void rebuildInspector();
    void updateActivePanner();

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

    // Per-source inspector — a viewport with rows, scrollable when many sources.
    juce::Viewport       inspector_viewport;
    juce::Component      inspector_holder;
    juce::OwnedArray<SourceInspectorRow> inspector_rows;

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
