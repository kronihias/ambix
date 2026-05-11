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

#include "PluginEditor.h"
#include "../../common/JuceCompat.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

#define SLIDER_MAX (180+12)

namespace {
    double sliderWrap (juce::Slider &sld)
    {
        double v = sld.getValue();
        double v0 = v;
        if (sld.isMouseButtonDown())
            v = juce::jlimit(-180., 180., v);
        else {
            while(v < -180.) v += 360.;
            while(v >  180.) v -= 360.;
        }
        if (v0 != v)
            sld.setValue(v);
        return v;
    }
}

#include "Graphics.h"

//==============================================================================
SourceInspectorRow::SourceInspectorRow (Ambix_encoderAudioProcessor& p, int sourceIndex)
    : processor (p), idx (sourceIndex)
{
    addAndMakeVisible (lbl);
    lbl.setText ("Src " + juce::String (idx + 1),
                 juce::dontSendNotification);
    lbl.setColour (juce::Label::textColourId, juce::Colours::white);
    lbl.setFont (juce::Font (juce::FontOptions { 12.f, juce::Font::bold }));

    auto setup = [this] (juce::Slider& s, double mn, double mx, double step,
                          const juce::String& tip)
    {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 16);
        s.setRange (mn, mx, step);
        s.setColour (juce::Slider::thumbColourId, juce::Colours::grey);
        s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
        s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
        s.setTooltip (tip);
        s.addListener (this);
    };

    setup (sld_az,   -180., 180., 1.,    "azimuth (deg)");
    setup (sld_el,    -90.,  90., 1.,    "elevation (deg)");
    setup (sld_size,    0.,   1., 0.01,  "size");
    setup (sld_gain,    0.,   1., 0.01,  "gain");

    refreshFromProcessor();
}

void SourceInspectorRow::refreshFromProcessor()
{
    sld_az  .setValue ((processor.getParameter (Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcAz))   - 0.5f) * 360.f, juce::dontSendNotification);
    sld_el  .setValue ((processor.getParameter (Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcEl))   - 0.5f) * 180.f, juce::dontSendNotification);
    sld_size.setValue ( processor.getParameter (Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcSize)),  juce::dontSendNotification);
    sld_gain.setValue ( processor.getParameter (Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcGain)),  juce::dontSendNotification);
}

void SourceInspectorRow::resized()
{
    auto r = getLocalBounds();
    lbl.setBounds (r.removeFromLeft (50));
    const int sliderH = (r.getHeight() - 4) / 4;
    sld_az  .setBounds (r.removeFromTop (sliderH));
    sld_el  .setBounds (r.removeFromTop (sliderH));
    sld_size.setBounds (r.removeFromTop (sliderH));
    sld_gain.setBounds (r.removeFromTop (sliderH));
}

void SourceInspectorRow::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0x40ffffff));
    g.drawRect (getLocalBounds(), 1);
}

void SourceInspectorRow::sliderValueChanged (juce::Slider* s)
{
    if (s == &sld_az)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcAz),
            (float)((s->getValue() + 180.) / 360.));
    else if (s == &sld_el)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcEl),
            (float)((s->getValue() + 90.) / 180.));
    else if (s == &sld_size)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcSize),
            (float) s->getValue());
    else if (s == &sld_gain)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcGain),
            (float) s->getValue());
}

//==============================================================================
Ambix_encoderAudioProcessorEditor::Ambix_encoderAudioProcessorEditor (Ambix_encoderAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
    changed_(true)
{
    setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears (700);

    addAndMakeVisible (sld_el);
    sld_el.setTooltip ("elevation");
    sld_el.setRange (-SLIDER_MAX, SLIDER_MAX, 1);
    sld_el.setSliderStyle (juce::Slider::LinearVertical);
    sld_el.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 41, 20);
    sld_el.setColour (juce::Slider::thumbColourId, juce::Colours::grey);
    sld_el.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
    sld_el.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
    sld_el.addListener (this);

    addAndMakeVisible (sld_az);
    sld_az.setTooltip ("azimuth");
    sld_az.setRange (-SLIDER_MAX, SLIDER_MAX, 1);
    sld_az.setSliderStyle (juce::Slider::LinearHorizontal);
    sld_az.setTextBoxStyle (juce::Slider::TextBoxRight, false, 40, 20);
    sld_az.setColour (juce::Slider::thumbColourId, juce::Colours::grey);
    sld_az.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
    sld_az.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
    sld_az.addListener (this);

    addAndMakeVisible (sld_size);
    sld_size.setTooltip ("higher order scaling - decrease spatial sharpness");
    sld_size.setRange (0, 1, 0.01);
    sld_size.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sld_size.setTextBoxStyle (juce::Slider::NoTextBox, false, 40, 20);
    sld_size.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    sld_size.setColour (juce::Slider::trackColourId, juce::Colours::white);
    sld_size.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    sld_size.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::white);
    sld_size.addListener (this);
    sld_size.setPopupDisplayEnabled(true, true, this);

    addAndMakeVisible (sld_speed);
    sld_speed.setTooltip ("movement speed in deg/sec");
    sld_speed.setRange (0, 360, 1);
    sld_speed.setSliderStyle (juce::Slider::LinearHorizontal);
    sld_speed.setTextBoxStyle (juce::Slider::TextBoxRight, false, 40, 20);
    sld_speed.setColour (juce::Slider::thumbColourId, juce::Colour (0xffe98273));
    sld_speed.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
    sld_speed.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::white);
    sld_speed.addListener (this);

    addAndMakeVisible (sld_el_move);
    sld_el_move.setTooltip ("elevation movement speed");
    sld_el_move.setRange (0, 1, 0.01);
    sld_el_move.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sld_el_move.setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    sld_el_move.setColour (juce::Slider::trackColourId, juce::Colour (0xff2d7dff));
    sld_el_move.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    sld_el_move.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::azure);
    sld_el_move.addListener (this);

    addAndMakeVisible (sld_az_move);
    sld_az_move.setTooltip ("azimuth movement speed");
    sld_az_move.setRange (0, 1, 0.01);
    sld_az_move.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sld_az_move.setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    sld_az_move.setColour (juce::Slider::trackColourId, juce::Colour (0xff2d7dff));
    sld_az_move.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    sld_az_move.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::azure);
    sld_az_move.addListener (this);

    addAndMakeVisible (txt_az_move);
    txt_az_move.setMultiLine (false);
    txt_az_move.setReturnKeyStartsNewLine (false);
    txt_az_move.setReadOnly (true);
    txt_az_move.setScrollbarsShown (false);
    txt_az_move.setCaretVisible (false);
    txt_az_move.setPopupMenuEnabled (false);
    txt_az_move.setColour (juce::TextEditor::outlineColourId, juce::Colour (0x706884ff));
    txt_az_move.setText ("-180 deg/s");

    addAndMakeVisible (txt_el_move);
    txt_el_move.setTooltip ("elevation movement speed");
    txt_el_move.setMultiLine (false);
    txt_el_move.setReturnKeyStartsNewLine (false);
    txt_el_move.setReadOnly (true);
    txt_el_move.setScrollbarsShown (false);
    txt_el_move.setCaretVisible (false);
    txt_el_move.setPopupMenuEnabled (false);
    txt_el_move.setColour (juce::TextEditor::outlineColourId, juce::Colour (0x706884ff));
    txt_el_move.setText ("-180 deg/s");

    addAndMakeVisible (txt_id);
    txt_id.setTooltip (TRANS("ID"));
    txt_id.setMultiLine (false);
    txt_id.setReturnKeyStartsNewLine (false);
    txt_id.setReadOnly (false);
    txt_id.setScrollbarsShown (false);
    txt_id.setCaretVisible (true);
    txt_id.setPopupMenuEnabled (true);
    txt_id.setText ("11");
    txt_id.setInputRestrictions (4, "1234567890");
    txt_id.addListener (this);

    addAndMakeVisible (sphere_opengl);
    sphere_opengl.setName ("new OpenGl");
    sphere_opengl.processor = ownerFilter;

    addChildComponent (hammer_view);
    hammer_view.setProcessor (ownerFilter);

    sld_az.setDoubleClickReturnValue (true, 0);
    sld_size.setDoubleClickReturnValue (true, 0);
    sld_el.setDoubleClickReturnValue (true, 0);
    sld_speed.setDoubleClickReturnValue (true, 90);
    sld_az_move.setDoubleClickReturnValue (true, 0.5f);
    sld_el_move.setDoubleClickReturnValue (true, 0.5f);

    addAndMakeVisible (lbl_id);
    lbl_id.setText ("ID:", juce::dontSendNotification);
    lbl_id.setFont (juce::Font (juce::FontOptions { 15.00f, juce::Font::plain }));
    lbl_id.setJustificationType (juce::Justification::centredRight);
    lbl_id.setEditable (false, false, false);
    lbl_id.setColour (juce::Label::textColourId, juce::Colour (0xff888888));
    lbl_id.setColour (juce::TextEditor::textColourId, juce::Colours::black);
    lbl_id.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    addAndMakeVisible (sld_width);
    sld_width.setTooltip ("multiple input sources are equally spread along this range");
    sld_width.setRange (0, 360, 1);
    sld_width.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sld_width.setTextBoxStyle (juce::Slider::NoTextBox, false, 41, 20);
    sld_width.setColour (juce::Slider::thumbColourId, juce::Colour (0xffe982cd));
    sld_width.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::white);
    sld_width.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::white);
    sld_width.addListener (this);
    sld_width.setPopupDisplayEnabled (true, true, this);
    sld_width.setDoubleClickReturnValue (true, 45);

    addAndMakeVisible (btn_settings);
    btn_settings.setTooltip ("OSC settings");
    btn_settings.setButtonText ("settings");
    btn_settings.addListener (this);
    btn_settings.setImages (false, true, true,
                            juce::ImageCache::getFromMemory (settings_png, settings_pngSize), 1.f, juce::Colour(0),
                            juce::ImageCache::getFromMemory (settings_white_png, settings_white_pngSize), 1.f, juce::Colour(0),
                            juce::ImageCache::getFromMemory (settings_png, settings_pngSize), 1.f, juce::Colour(0));

    // View toggle (Sphere vs Hammer-Aitoff)
    addAndMakeVisible (btn_view_sphere);
    btn_view_sphere.setRadioGroupId (1001);
    btn_view_sphere.setClickingTogglesState (true);
    btn_view_sphere.setToggleState (true, juce::dontSendNotification);
    btn_view_sphere.setConnectedEdges (juce::Button::ConnectedOnRight);
    btn_view_sphere.addListener (this);

    addAndMakeVisible (btn_view_hammer);
    btn_view_hammer.setRadioGroupId (1001);
    btn_view_hammer.setClickingTogglesState (true);
    btn_view_hammer.setConnectedEdges (juce::Button::ConnectedOnLeft);
    btn_view_hammer.addListener (this);

    // Linked toggle
    addAndMakeVisible (btn_linked_toggle);
    btn_linked_toggle.setClickingTogglesState (true);
    btn_linked_toggle.setToggleState (ownerFilter->isLinked(), juce::dontSendNotification);
    btn_linked_toggle.setButtonText (ownerFilter->isLinked() ? "Linked" : "Unlinked");
    btn_linked_toggle.setTooltip ("Linked: all sources share params (auto-spread).\n"
                                  "Unlinked: each source independently positioned.");
    btn_linked_toggle.addListener (this);

    // Active sources combo
    addAndMakeVisible (lbl_sources);
    lbl_sources.setColour (juce::Label::textColourId, juce::Colours::white);
    lbl_sources.setFont (juce::Font (juce::FontOptions { 11.f, juce::Font::plain }));
    lbl_sources.setJustificationType (juce::Justification::centredRight);

    addAndMakeVisible (cmb_active_sources);
    for (int i = 1; i <= Ambix_encoderAudioProcessor::kMaxSources; ++i)
        cmb_active_sources.addItem (juce::String (i), i);
    cmb_active_sources.setSelectedId (ownerFilter->getActiveSources(), juce::dontSendNotification);
    cmb_active_sources.addListener (this);

    // Inspector
    addChildComponent (inspector_viewport);
    inspector_viewport.setViewedComponent (&inspector_holder, false);
    inspector_viewport.setScrollBarsShown (true, false);
    rebuildInspector();

    // Make the editor a bit bigger to accomodate the new controls.
    setSize (480, 460);

    ownerFilter->addChangeListener (this);
    ownerFilter->sendChangeMessage();

    juce::String str_id;
    str_id << ownerFilter->m_id;
    txt_id.setText (str_id, juce::dontSendNotification);

    updateActivePanner();
    timerCallback();
    startTimer (45);
}

Ambix_encoderAudioProcessorEditor::~Ambix_encoderAudioProcessorEditor()
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();
    ourProcessor->removeChangeListener (this);
    setLookAndFeel (nullptr);
}

void Ambix_encoderAudioProcessorEditor::rebuildInspector()
{
    inspector_rows.clear();
    Ambix_encoderAudioProcessor* p = getProcessor();
    const int active = p ? p->getActiveSources() : 1;

    constexpr int rowH = 92;
    for (int i = 0; i < active; ++i)
    {
        auto* row = new SourceInspectorRow (*p, i);
        inspector_rows.add (row);
        inspector_holder.addAndMakeVisible (row);
    }
    inspector_holder.setSize (inspector_viewport.getWidth() - 12,
                              juce::jmax (1, active * (rowH + 4)));

    auto y = 0;
    for (auto* row : inspector_rows)
    {
        row->setBounds (0, y, inspector_holder.getWidth(), rowH);
        y += rowH + 4;
    }
}

void Ambix_encoderAudioProcessorEditor::updateActivePanner()
{
    sphere_opengl.setVisible (! _hammerView);
    hammer_view  .setVisible (  _hammerView);
}

//==============================================================================
void Ambix_encoderAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4e4e4e),
                                       (float) (proportionOfWidth (0.6314f)), (float) (proportionOfHeight (0.5842f)),
                                       juce::Colours::black,
                                       (float) (proportionOfWidth (0.1143f)), (float) (proportionOfHeight (0.0800f)),
                                       true));
    g.fillRect (0, 0, getWidth(), getHeight());

    g.setColour (juce::Colours::black);
    g.drawRect (0, 0, getWidth(), getHeight(), 1);

    // Title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 17.2f, juce::Font::bold }));
    {
        int order = ambiOrderFromChannels (getProcessor()->getTotalNumOutputChannels());
        juce::String title = "AMBIX-ENCODER";
        if (order > 0) title << " O" << order;
        g.drawText (title, 35, 2, getWidth() - 70, 30, juce::Justification::centred, true);
    }

    // Group box behind movement controls (bottom area)
    g.setColour (juce::Colour (0xff2b1d69));
    g.fillRoundedRectangle (165.0f, 310.0f, 154.0f, 77.0f, 4.0000f);
    g.fillRoundedRectangle (24.0f, 310.0f, 226.0f, 43.0f, 4.0000f);

    // Labels around the panner
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 10.f, juce::Font::plain }));
    g.drawText ("elevation",   266, 29, 48, 16, juce::Justification::centred, true);
    g.drawText ("azimuth",     216, 272, 48, 16, juce::Justification::centredRight, true);
    g.drawText ("size",        31, 382, 23, 16, juce::Justification::centredRight, true);
    g.drawText ("max speed",   206, 372, 57, 16, juce::Justification::centredRight, true);
    g.drawText ("width",       61, 382, 95, 16, juce::Justification::centredRight, true);
    g.drawText ("elevation move", 219, 338, 81, 16, juce::Justification::centredRight, true);
    g.drawText ("azimuth move",   59, 338, 81, 16, juce::Justification::centredRight, true);

    // Inspector panel header
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 11.f, juce::Font::bold }));
    g.drawText (getProcessor()->isLinked() ? "Sources (linked)" : "Sources (unlinked)",
                330, 36, 140, 16, juce::Justification::centredLeft, true);

    // Version
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 10.f, juce::Font::plain }));
    juce::String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string,
                getWidth() - 51, getHeight() - 11, 50, 10,
                juce::Justification::bottomRight, true);
}

void Ambix_encoderAudioProcessorEditor::resized()
{
    sphere_opengl.setBounds (23, 32, 240, 240);
    hammer_view  .setBounds (23, 32, 240, 240);

    sld_el.setBounds (270, 38, 40, 232);
    sld_az.setBounds (27, 270, 282, 40);
    sld_size.setBounds (32, 355, 29, 29);

    sld_speed.setBounds (169, 347, 140, 40);
    sld_el_move.setBounds (275, 312, 29, 29);
    sld_az_move.setBounds (118, 312, 29, 29);
    txt_az_move.setBounds (37, 316, 78, 22);
    txt_el_move.setBounds (191, 316, 78, 22);

    lbl_id.setBounds (250, 2, 39, 24);
    txt_id.setBounds (289, 5, 33, 19);

    btn_settings.setBounds (3, 3, 26, 25);

    sld_width.setBounds (93, 355, 29, 29);

    // View toggle (top of editor, near settings)
    btn_view_sphere.setBounds (32, 8, 64, 18);
    btn_view_hammer.setBounds (96, 8, 90, 18);

    // Linked toggle + sources count (right of inspector header)
    btn_linked_toggle.setBounds (330, 6, 70, 22);
    lbl_sources       .setBounds (330, 30, 60, 16);
    cmb_active_sources.setBounds (390, 30, 50, 18);

    // Inspector viewport
    inspector_viewport.setBounds (330, 56, 145, 340);
    rebuildInspector();
}

void Ambix_encoderAudioProcessorEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();

    if (sliderThatWasMoved == &sld_el)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::ElevationParam, (sliderWrap(sld_el) + 180) / 360.f);
    else if (sliderThatWasMoved == &sld_az)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::AzimuthParam,   (sliderWrap(sld_az) + 180) / 360.f);
    else if (sliderThatWasMoved == &sld_size)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::SizeParam, (float) sld_size.getValue());
    else if (sliderThatWasMoved == &sld_width)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::WidthParam, (float) sld_width.getValue() / 360.f);
    else if (sliderThatWasMoved == &sld_az_move)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::AzimuthMvParam, (float) sld_az_move.getValue());
    else if (sliderThatWasMoved == &sld_el_move)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::ElevationMvParam, (float) sld_el_move.getValue());
    else if (sliderThatWasMoved == &sld_speed)
        setParameterNotifyingHost (ourProcessor, Ambix_encoderAudioProcessor::SpeedParam, (float) sld_speed.getValue() / 360.f);
}

void Ambix_encoderAudioProcessorEditor::comboBoxChanged (juce::ComboBox* box)
{
    if (box == &cmb_active_sources)
    {
        const int n = juce::jmax (1, cmb_active_sources.getSelectedId());
        const float norm = (float)(n - 1) / (float)(Ambix_encoderAudioProcessor::kMaxSources - 1);
        setParameterNotifyingHost (getProcessor(),
                                   Ambix_encoderAudioProcessor::NumActiveSourcesParam, norm);
        rebuildInspector();
    }
}

void Ambix_encoderAudioProcessorEditor::timerCallback()
{
    const juce::ScopedTryLock myScopedTryLock (lock_);

    if (myScopedTryLock.isLocked())
    {
        if (changed_)
        {
            changed_ = false;

            Ambix_encoderAudioProcessor* ourProcessor = getProcessor();

            sld_az.setValue ((ourProcessor->getParameter (Ambix_encoderAudioProcessor::AzimuthParam)   - 0.5f) * 360.f, juce::dontSendNotification);
            sld_el.setValue ((ourProcessor->getParameter (Ambix_encoderAudioProcessor::ElevationParam) - 0.5f) * 360.f, juce::dontSendNotification);
            sld_size.setValue (ourProcessor->getParameter (Ambix_encoderAudioProcessor::SizeParam), juce::dontSendNotification);
            sld_width.setValue (ourProcessor->getParameter (Ambix_encoderAudioProcessor::WidthParam) * 360.f, juce::dontSendNotification);
            sld_speed.setValue (ourProcessor->getParameter (Ambix_encoderAudioProcessor::SpeedParam) * 360, juce::dontSendNotification);

            float azimuth_mv_param   = ourProcessor->getParameter (Ambix_encoderAudioProcessor::AzimuthMvParam);
            float elevation_mv_param = ourProcessor->getParameter (Ambix_encoderAudioProcessor::ElevationMvParam);
            float speed_param        = ourProcessor->getParameter (Ambix_encoderAudioProcessor::SpeedParam);

            sld_az_move.setValue (azimuth_mv_param,   juce::dontSendNotification);
            sld_el_move.setValue (elevation_mv_param, juce::dontSendNotification);

            juce::String az_mv;
            if (azimuth_mv_param <= 0.48f)
                az_mv << "-" << juce::String ((int)(pow(speed_param*360.f, (0.45f - azimuth_mv_param)*2.22222f)+0.5f)).substring(0, 5) << " deg/s";
            else if (azimuth_mv_param >= 0.52f)
                az_mv << juce::String ((int)(pow(speed_param*360.f, (azimuth_mv_param - 0.55f)*2.22222f) + 0.5f)).substring(0, 5) << " deg/s";
            else
                az_mv << "0 deg/s";
            txt_az_move.setText (az_mv);

            juce::String el_mv;
            if (elevation_mv_param <= 0.48f)
                el_mv << "-" << juce::String ((int)(pow(speed_param*360.f, (0.45f - elevation_mv_param)*2.22222f) + 0.5f)).substring(0, 4) << " deg/s";
            else if (elevation_mv_param >= 0.52f)
                el_mv << juce::String ((int)(pow(speed_param*360.f, (elevation_mv_param - 0.55f)*2.22222f) + 0.5f)).substring(0, 4) << " deg/s";
            else
                el_mv << "0 deg/s";
            txt_el_move.setText (el_mv);

            // Linked button label + state mirror
            const bool linked = ourProcessor->isLinked();
            if (btn_linked_toggle.getToggleState() != linked)
                btn_linked_toggle.setToggleState (linked, juce::dontSendNotification);
            btn_linked_toggle.setButtonText (linked ? "Linked" : "Unlinked");

            // Active sources combo mirror
            const int active = ourProcessor->getActiveSources();
            if (cmb_active_sources.getSelectedId() != active)
            {
                cmb_active_sources.setSelectedId (active, juce::dontSendNotification);
                rebuildInspector();
            }
        }

        // Inspector rows always tick so meter halo / values stay live.
        for (auto* row : inspector_rows)
            row->refreshFromProcessor();
    }
}

void Ambix_encoderAudioProcessorEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    changed_ = true;
}

void Ambix_encoderAudioProcessorEditor::modifierKeysChanged (const juce::ModifierKeys &modifiers)
{
    if (modifiers.isShiftDown())
    {
        sld_el.setColour (juce::Slider::thumbColourId, juce::Colours::black);
        sld_el.setEnabled(false);
    }
    else
    {
        sld_el.setColour (juce::Slider::thumbColourId, juce::Colours::grey);
        sld_el.setEnabled(true);
    }

    if (modifiers.isCtrlDown())
    {
        sld_az.setColour (juce::Slider::thumbColourId, juce::Colours::black);
        sld_az.setEnabled(false);
    }
    else
    {
        sld_az.setColour (juce::Slider::thumbColourId, juce::Colours::grey);
        sld_az.setEnabled(true);
    }
}


void Ambix_encoderAudioProcessorEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();

    if (buttonThatWasClicked == &btn_settings)
    {
        if (! _settingsDialogWindow)
        {
            juce::DialogWindow::LaunchOptions launchOptions;
            launchOptions.dialogTitle = "Settings";
            launchOptions.content.setOwned (new Settings (*ourProcessor));
            launchOptions.componentToCentreAround = this;
            launchOptions.escapeKeyTriggersCloseButton = true;
            launchOptions.useNativeTitleBar = true;
            launchOptions.resizable = false;
            launchOptions.useBottomRightCornerResizer = false;
            _settingsDialogWindow = launchOptions.launchAsync();
        }
    }
    else if (buttonThatWasClicked == &btn_view_sphere)
    {
        _hammerView = false;
        updateActivePanner();
    }
    else if (buttonThatWasClicked == &btn_view_hammer)
    {
        _hammerView = true;
        updateActivePanner();
    }
    else if (buttonThatWasClicked == &btn_linked_toggle)
    {
        const bool nowLinked = btn_linked_toggle.getToggleState();
        setParameterNotifyingHost (ourProcessor,
                                   Ambix_encoderAudioProcessor::LinkedParam,
                                   nowLinked ? 1.f : 0.f);
        btn_linked_toggle.setButtonText (nowLinked ? "Linked" : "Unlinked");
        repaint();
    }
}

void Ambix_encoderAudioProcessorEditor::textEditorFocusLost (juce::TextEditor& ed)
{
    updateID();
}

void Ambix_encoderAudioProcessorEditor::textEditorReturnKeyPressed (juce::TextEditor& ed)
{
    updateID();
}

void Ambix_encoderAudioProcessorEditor::updateID()
{
    Ambix_encoderAudioProcessor* ourProcessor = getProcessor();
    ourProcessor->m_id = txt_id.getText().getIntValue();
}
