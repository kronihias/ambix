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

    // Column-width ratios for the source table (sum = 1.0 nominal).
    // The two right-most columns are fixed-pixel-width S/M buttons sitting
    // outside this ratio — handled separately in the layout.
    constexpr float kColW_Num  = 0.12f;
    constexpr float kColW_Az   = 0.30f;
    constexpr float kColW_El   = 0.28f;
    constexpr float kColW_Size = 0.26f;
    constexpr int   kColW_SoloMute = 28; // px each; total 56 px fixed
    constexpr int   kRowHeight    = 22;
    constexpr int   kHeaderHeight = 18;
}

#include "Graphics.h"

//==============================================================================
SourceTableRow::SourceTableRow (Ambix_encoderAudioProcessor& p, int sourceIndex)
    : processor (p), idx (sourceIndex)
{
    addAndMakeVisible (lbl);
    lbl.setText (juce::String (idx + 1), juce::dontSendNotification);
    lbl.setColour (juce::Label::textColourId, juce::Colours::white);
    lbl.setFont (juce::Font (juce::FontOptions { 12.f, juce::Font::bold }));
    lbl.setJustificationType (juce::Justification::centred);

    // Number-box style: Slider::LinearBar draws the value directly on the
    // bar with no separate text box. With the bar/track colours set to
    // transparent only the text shows, giving a "number-only" widget that
    // still supports drag-to-nudge and double-click-to-type.
    auto setup = [this] (juce::Slider& s, double mn, double mx, double step,
                          const juce::String& suffix,
                          const juce::String& tip)
    {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::LinearBar);
        s.setColour (juce::Slider::trackColourId,             juce::Colour (0x00000000));
        s.setColour (juce::Slider::backgroundColourId,        juce::Colour (0x00000000));
        s.setColour (juce::Slider::textBoxTextColourId,       juce::Colours::white);
        s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00000000));
        s.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colour (0x00000000));
        s.setRange (mn, mx, step);
        s.setTextValueSuffix (suffix);
        s.setTooltip (tip);
        s.addListener (this);
    };

    setup (num_az,   -180., 180., 1.,   juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")), "azimuth (deg)");
    setup (num_el,    -90.,  90., 1.,   juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")), "elevation (deg)");
    setup (num_size,    0.,   1., 0.01, juce::String(),                                     "size (0 = sharp, 1 = wide)");

    // S / M toggles. Both work in linked AND unlinked mode — they don't
    // depend on per-source positions.
    auto setupBtn = [this] (juce::TextButton& b, const juce::String& text,
                            const juce::String& tip, juce::Colour onColour)
    {
        addAndMakeVisible (b);
        b.setButtonText (text);
        b.setClickingTogglesState (true);
        b.setTooltip (tip);
        b.setColour (juce::TextButton::buttonOnColourId,  onColour);
        b.setColour (juce::TextButton::buttonColourId,    juce::Colour (0x44000000));
        b.setColour (juce::TextButton::textColourOnId,    juce::Colours::black);
        b.setColour (juce::TextButton::textColourOffId,   juce::Colour (0xff999999));
        b.addListener (this);
    };
    setupBtn (btn_solo, "S", "Solo this source (mutes all unsoloed sources)", juce::Colour (0xffe5c34a));
    setupBtn (btn_mute, "M", "Mute this source",                              juce::Colour (0xffd06464));

    refreshFromProcessor();
}

void SourceTableRow::refreshFromProcessor()
{
    // Mirror the *effective* per-source position so the table also reflects
    // the auto-spread positions in linked mode. getSourceDisplayPos reads
    // from AmbiEnc[i] which is recomputed every block by applyParamsToEncoders.
    const auto pos = processor.getSourceDisplayPos (idx);
    num_az  .setValue (pos.azDeg, juce::dontSendNotification);
    num_el  .setValue (pos.elDeg, juce::dontSendNotification);
    num_size.setValue (pos.size,  juce::dontSendNotification);

    using Proc = Ambix_encoderAudioProcessor;
    const bool soloed = processor.getParameter (Proc::sourceParamIndex (idx, Proc::SrcSolo)) >= 0.5f;
    const bool muted  = processor.getParameter (Proc::sourceParamIndex (idx, Proc::SrcMute)) >= 0.5f;
    if (btn_solo.getToggleState() != soloed) btn_solo.setToggleState (soloed, juce::dontSendNotification);
    if (btn_mute.getToggleState() != muted)  btn_mute.setToggleState (muted,  juce::dontSendNotification);
}

void SourceTableRow::setEditable (bool canEdit)
{
    if (editable == canEdit) return;
    editable = canEdit;
    const float alpha = canEdit ? 1.f : 0.45f;
    num_az  .setEnabled (canEdit); num_az  .setAlpha (alpha);
    num_el  .setEnabled (canEdit); num_el  .setAlpha (alpha);
    num_size.setEnabled (canEdit); num_size.setAlpha (alpha);
    // Solo/mute stay editable in linked mode — they're orthogonal to the
    // position auto-spread, and you'd want to mute/solo individual sources
    // even when the constellation moves as one.
}

void SourceTableRow::resized()
{
    auto r = getLocalBounds();
    // S/M buttons: square, centred in their column so they line up with the
    // centred header letters. withSizeKeepingCentre keeps them on the
    // column's horizontal centreline regardless of cell width.
    const int btnSize = juce::jmin (kColW_SoloMute - 4, kRowHeight - 4);
    btn_mute.setBounds (r.removeFromRight (kColW_SoloMute)
                          .withSizeKeepingCentre (btnSize, btnSize));
    btn_solo.setBounds (r.removeFromRight (kColW_SoloMute)
                          .withSizeKeepingCentre (btnSize, btnSize));

    const int post = r.getWidth();
    constexpr float baseSum = kColW_Num + kColW_Az + kColW_El + kColW_Size;
    const int wNum  = (int) (post * kColW_Num  / baseSum);
    const int wAz   = (int) (post * kColW_Az   / baseSum);
    const int wEl   = (int) (post * kColW_El   / baseSum);

    lbl     .setBounds (r.removeFromLeft (wNum));
    num_az  .setBounds (r.removeFromLeft (wAz) .reduced (2, 2));
    num_el  .setBounds (r.removeFromLeft (wEl) .reduced (2, 2));
    num_size.setBounds (r                      .reduced (2, 2));
}

void SourceTableRow::paint (juce::Graphics& g)
{
    // alternating row background for readability
    g.setColour ((idx % 2 == 0) ? juce::Colour (0x222b1d69) : juce::Colour (0x402b1d69));
    g.fillRect (getLocalBounds());
}

void SourceTableRow::sliderValueChanged (juce::Slider* s)
{
    if (! editable) return; // table is read-only in linked mode

    if (s == &num_az)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcAz),
            (float)((s->getValue() + 180.) / 360.));
    else if (s == &num_el)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcEl),
            (float)((s->getValue() + 180.) / 360.));
    else if (s == &num_size)
        setParameterNotifyingHost (&processor,
            Ambix_encoderAudioProcessor::sourceParamIndex (idx, Ambix_encoderAudioProcessor::SrcSize),
            (float) s->getValue());
}

void SourceTableRow::buttonClicked (juce::Button* b)
{
    using Proc = Ambix_encoderAudioProcessor;
    if (b == &btn_solo)
        setParameterNotifyingHost (&processor,
            Proc::sourceParamIndex (idx, Proc::SrcSolo),
            btn_solo.getToggleState() ? 1.f : 0.f);
    else if (b == &btn_mute)
        setParameterNotifyingHost (&processor,
            Proc::sourceParamIndex (idx, Proc::SrcMute),
            btn_mute.getToggleState() ? 1.f : 0.f);
}

//==============================================================================
void SourceTableHeader::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff2b1d69));
    g.fillRect (getLocalBounds());

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 11.f, juce::Font::bold }));

    auto r = getLocalBounds();
    // Mirror SourceTableRow::resized — fixed-pixel S/M columns on the
    // right, ratios applied to the remaining width.
    const auto mCol = r.removeFromRight (kColW_SoloMute);
    const auto sCol = r.removeFromRight (kColW_SoloMute);

    const int post = r.getWidth();
    constexpr float baseSum = kColW_Num + kColW_Az + kColW_El + kColW_Size;
    const int wNum  = (int) (post * kColW_Num / baseSum);
    const int wAz   = (int) (post * kColW_Az  / baseSum);
    const int wEl   = (int) (post * kColW_El  / baseSum);

    g.drawText ("#",         r.removeFromLeft (wNum), juce::Justification::centred);
    g.drawText ("Azimuth",   r.removeFromLeft (wAz),  juce::Justification::centred);
    g.drawText ("Elevation", r.removeFromLeft (wEl),  juce::Justification::centred);
    g.drawText ("Size",      r,                       juce::Justification::centred);
    g.drawText ("S",         sCol,                    juce::Justification::centred);
    g.drawText ("M",         mCol,                    juce::Justification::centred);
}

void SourceTableHeader::mouseDown (const juce::MouseEvent& e)
{
    if (! processor) return;

    // Recompute column rects the same way paint() does so the hit-test
    // stays in sync with the visible layout.
    auto r = getLocalBounds();
    const auto mCol = r.removeFromRight (kColW_SoloMute);
    const auto sCol = r.removeFromRight (kColW_SoloMute);

    const bool clickedSolo = sCol.contains (e.getPosition());
    const bool clickedMute = mCol.contains (e.getPosition());
    if (! clickedSolo && ! clickedMute) return;

    using Proc = Ambix_encoderAudioProcessor;
    const auto sub = clickedSolo ? Proc::SrcSolo : Proc::SrcMute;
    // Clear the param for every source slot (not just active ones — if
    // the user later raises the active count, sources shouldn't come
    // back in a stale soloed/muted state).
    for (int i = 0; i < Proc::kMaxSources; ++i)
        setParameterNotifyingHost (processor, Proc::sourceParamIndex (i, sub), 0.f);
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
    lbl_id.setFont (juce::Font (juce::FontOptions { 13.f, juce::Font::plain }));
    lbl_id.setJustificationType (juce::Justification::centredRight);
    lbl_id.setEditable (false, false, false);
    lbl_id.setColour (juce::Label::textColourId, juce::Colour (0xff888888));

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

    // Popout: opens / closes a detached panner window the user can move to
    // a different screen or take fullscreen via the OS title-bar buttons.
    addAndMakeVisible (btn_popout);
    btn_popout.setTooltip ("Open a detached panner window (drag to a second screen, fullscreen via title bar)");
    btn_popout.addListener (this);

    // Presets button — opens a menu with load/save-from-file, save-as-named,
    // and the on-disk preset list. Format is JSON, SPARTA AmbiENC / IEM
    // MultiEncoder compatible. Same toolbar style as mcfx_mimoeq.
    addAndMakeVisible (btn_presets);
    btn_presets.setTooltip ("Save / load named source-layout presets (stored under the user's app-data folder), "
                            "or import / export a JSON file anywhere on disk. "
                            "JSON format is compatible with SPARTA AmbiENC and IEM MultiEncoder.");
    btn_presets.addListener (this);

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

    // Source table
    addAndMakeVisible (table_header);
    table_header.setProcessor (ownerFilter);
    // Click-to-reset on S/M is signalled by the hand cursor — full Component
    // doesn't ship SettableTooltipClient so we lean on the cursor + comment
    // in the header for discoverability.
    table_header.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible (table_viewport);
    table_viewport.setViewedComponent (&table_holder, false);
    table_viewport.setScrollBarsShown (true, false);

    // Resize: the encoder editor scales the panner + table proportionally.
    // Minimum size keeps all controls legible; maximum is generous so a user
    // who wants a giant panner gets one. Size + active-view are persisted
    // on the processor so reopening the editor (or recalling a session)
    // restores whatever the user left behind.
    resizeLimits.setSizeLimits (520, 420, 1400, 1200);
    addAndMakeVisible (resizer);
    setResizable (true, true);
    _hammerView = ownerFilter->editor_hammer_view;
    btn_view_sphere.setToggleState (! _hammerView, juce::dontSendNotification);
    btn_view_hammer.setToggleState (  _hammerView, juce::dontSendNotification);
    setSize (juce::jlimit (520, 1400, ownerFilter->editor_width),
             juce::jlimit (420, 1200, ownerFilter->editor_height));

    ownerFilter->addChangeListener (this);
    ownerFilter->sendChangeMessage();

    juce::String str_id;
    str_id << ownerFilter->m_id;
    txt_id.setText (str_id, juce::dontSendNotification);

    rebuildTable();
    updateActivePanner();
    timerCallback();
    startTimer (45);

    // Re-open the popout if it was open at save time. Use the persisted
    // popout view + size so the user finds the layout they left.
    if (ownerFilter->popout_open)
    {
        popout = std::make_unique<PopoutPanner> (
            *ownerFilter,
            ownerFilter->popout_hammer_view,
            ownerFilter->popout_width,
            ownerFilter->popout_height,
            [this]()
            {
                getProcessor()->popout_open = false;
                popout.reset();
            });
    }
}

Ambix_encoderAudioProcessorEditor::~Ambix_encoderAudioProcessorEditor()
{
    if (auto* p = getProcessor())
        p->removeChangeListener (this);
    setLookAndFeel (nullptr);
}

void Ambix_encoderAudioProcessorEditor::rebuildTable()
{
    table_rows.clear();
    Ambix_encoderAudioProcessor* p = getProcessor();
    const int active = p ? p->getActiveSources() : 1;
    const bool linked = p ? p->isLinked() : true;

    for (int i = 0; i < active; ++i)
    {
        auto* row = new SourceTableRow (*p, i);
        row->setEditable (! linked);
        table_rows.add (row);
        table_holder.addAndMakeVisible (row);
    }
    layoutTableRows (table_viewport.getWidth());
}

void Ambix_encoderAudioProcessorEditor::layoutTableRows (int viewportWidth)
{
    const int width = juce::jmax (50, viewportWidth - 12); // leave room for scrollbar
    const int active = (int) table_rows.size();
    table_holder.setSize (width, juce::jmax (1, active * kRowHeight));

    int y = 0;
    for (auto* row : table_rows)
    {
        row->setBounds (0, y, width, kRowHeight);
        y += kRowHeight;
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

    // Title strip — centered along the top, sized to fit between left controls
    // (settings + view toggle) and right controls (linked + ID).
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 17.2f, juce::Font::bold }));
    {
        int order = ambiOrderFromChannels (getProcessor()->getTotalNumOutputChannels());
        juce::String title = "AMBIX-ENCODER";
        if (order > 0) title << " O" << order;
        g.drawText (title, 200, 4, getWidth() - 400, 22,
                    juce::Justification::centred, true);
    }

    // Movement controls backdrop (bottom strip)
    const int bottomY = getHeight() - 90;
    g.setColour (juce::Colour (0xff2b1d69));
    g.fillRoundedRectangle (24.f, (float) bottomY, (float) getWidth() - 48.f, 80.f, 4.f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions { 10.f, juce::Font::plain }));
    const bool linked = getProcessor()->isLinked();
    if (linked) {
        g.drawText ("size",       34, bottomY + 64, 36, 14, juce::Justification::centred, true);
        g.drawText ("width",      80, bottomY + 64, 40, 14, juce::Justification::centred, true);
    }
    g.drawText ("azimuth move",   135, bottomY + 4,  100, 14, juce::Justification::centredLeft, true);
    g.drawText ("elevation move", 135, bottomY + 38, 100, 14, juce::Justification::centredLeft, true);
    g.drawText ("max speed",      280, bottomY + 64, 130, 14, juce::Justification::centred, true);

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
    const int W = getWidth();
    const int H = getHeight();

    // Persist the current size on the processor so a close/reopen reads it
    // back. View-toggle re-derives width from height + new view's aspect
    // (see buttonClicked for btn_view_sphere / btn_view_hammer).
    if (auto* proc = getProcessor())
    {
        proc->editor_width  = W;
        proc->editor_height = H;
    }

    // --- Header strip ---------------------------------------------------------
    btn_settings.setBounds (4, 4, 24, 24);
    btn_view_sphere.setBounds (32, 6, 60, 22);
    btn_view_hammer.setBounds (92, 6, 90, 22);
    btn_popout     .setBounds (188, 6, 26, 22);

    // ID at the far right
    lbl_id.setBounds (W - 76, 6, 24, 22);
    txt_id.setBounds (W - 50, 7, 42, 20);

    // Linked + Sources below the title strip, left of ID
    btn_linked_toggle.setBounds (W - 220, 32, 80, 22);
    lbl_sources.setBounds       (W - 138, 32, 56, 22);
    cmb_active_sources.setBounds (W -  80, 32, 60, 22);

    // --- Bottom controls (fixed height) --------------------------------------
    const int bottomY = H - 90;

    // Presets sits in the header, to the left of the ID label. Matches the
    // mcfx_mimoeq toolbar placement so users moving between plugins see the
    // same affordance in the same spot.
    btn_presets.setBounds (W - 178, 6, 96, 22);
    // Global size + width only apply in linked mode (where they drive the
    // auto-spread for all sources). Hide them in unlinked mode — each
    // source has its own size column in the table, and width is meaningless
    // when sources are independently positioned.
    const bool linkedMode = getProcessor()->isLinked();
    sld_size .setVisible (linkedMode);
    sld_width.setVisible (linkedMode);
    sld_size.setBounds  (34, bottomY + 30, 30, 30);
    sld_width.setBounds (80, bottomY + 30, 30, 30);

    txt_az_move.setBounds (135, bottomY + 18, 100, 18);
    sld_az_move.setBounds (238, bottomY + 14, 26, 26);

    txt_el_move.setBounds (135, bottomY + 52, 100, 18);
    sld_el_move.setBounds (238, bottomY + 48, 26, 26);

    sld_speed.setBounds  (280, bottomY + 30, 130, 30);

    // --- Main work area: panner | table -------------------------------------
    const int contentTop    = 60;
    const int contentBottom = bottomY - 8;
    const int contentHeight = contentBottom - contentTop;

    // Reserve right column for the table — 240 px wide on small windows, up
    // to ~38% of width on larger ones.
    const int tableW = juce::jlimit (220, 360, (int) (W * 0.40f));
    const int panelW = W - tableW - 8;            // gap between panner+table

    // In unlinked mode the global az/el sliders are inert (each source has
    // its own position via the table), so hide them and expand the panner
    // into the freed space.
    const bool linked = getProcessor()->isLinked();
    sld_az.setVisible (linked);
    sld_el.setVisible (linked);

    const int sliderW    = linked ? 36 : 0;
    const int sliderGap  = linked ?  6 : 0;
    const int sliderPad  = linked ?  8 : 0;
    const int azSliderH  = linked ? 32 : 0;  // horizontal slider + padding

    const int pannerLeft = 12;
    const int availW = panelW - sliderW - sliderGap - sliderPad;
    const int availH = contentHeight - azSliderH;

    // Sphere is square; Hammer-Aitoff is 2:1 (wider than tall). Pick the
    // largest rectangle of the appropriate aspect ratio that fits in the
    // available area so neither view leaves a chunk of empty space.
    int pannerW, pannerH;
    if (_hammerView)
    {
        pannerW = juce::jmin (availW, 2 * availH);
        pannerH = pannerW / 2;
    }
    else
    {
        pannerW = pannerH = juce::jmin (availW, availH);
    }

    // Pin the panner area to the top of the content row — when the editor
    // is much taller than the natural panner size, any extra space falls
    // below the panner (between it and the bottom-strip controls) rather
    // than pushing the panner toward the centre. Matches the visual weight
    // distribution most users seem to expect (controls + visual at the top,
    // dead space below if any).
    const int pannerY = contentTop;

    sphere_opengl.setBounds (pannerLeft, pannerY, pannerW, pannerH);
    hammer_view  .setBounds (pannerLeft, pannerY, pannerW, pannerH);

    if (linked)
    {
        sld_el.setBounds (pannerLeft + pannerW + sliderGap, pannerY, sliderW, pannerH);
        sld_az.setBounds (pannerLeft, pannerY + pannerH + 4,
                          pannerW + sliderW + sliderGap, 28);
    }

    // Table on the right. The rows live inside the viewport with a 12-px
    // inset on the right to leave room for the scrollbar (see
    // layoutTableRows). The header must match that inset, otherwise its
    // rightmost columns sit 12 px further right than the row buttons and
    // the S/M letters no longer align over the toggles.
    constexpr int kScrollBarReserve = 12;
    const int tableX = W - tableW - 8;
    table_header  .setBounds (tableX, contentTop,
                              tableW - kScrollBarReserve, kHeaderHeight);
    table_viewport.setBounds (tableX, contentTop + kHeaderHeight,
                              tableW, contentHeight - kHeaderHeight);
    layoutTableRows (table_viewport.getWidth());

    // Resizer in the bottom-right corner
    resizer.setBounds (W - 16, H - 16, 16, 16);
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
        rebuildTable();
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

            // Linked button mirror. If the mode changed via automation /
            // OSC (not the button itself), trigger a relayout too so the
            // global az/el sliders appear / disappear.
            const bool linked = ourProcessor->isLinked();
            const bool linkedWasMirrored = (btn_linked_toggle.getToggleState() == linked);
            if (! linkedWasMirrored)
                btn_linked_toggle.setToggleState (linked, juce::dontSendNotification);
            btn_linked_toggle.setButtonText (linked ? "Linked" : "Unlinked");
            if (! linkedWasMirrored)
                resized();

            // Active sources combo + table rebuild on change
            const int active = ourProcessor->getActiveSources();
            if (cmb_active_sources.getSelectedId() != active
                || (int) table_rows.size() != active)
            {
                cmb_active_sources.setSelectedId (active, juce::dontSendNotification);
                rebuildTable();
            }

            // Mode (editable vs read-only) for every row
            for (auto* row : table_rows)
                row->setEditable (! linked);
        }

        // Table values always tick (linked mode pushes computed positions).
        for (auto* row : table_rows)
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
    else if (buttonThatWasClicked == &btn_presets)
    {
        showPresetsMenu();
    }
    else if (buttonThatWasClicked == &btn_popout)
    {
        // Toggle: if a popout is open, close it; otherwise create one
        // using the persisted size/view (falling back to the main editor's
        // current view on first open). The popout's OS close button calls
        // our lambda to drop the unique_ptr and clear popout_open, so
        // destruction always happens on the message thread.
        if (popout != nullptr)
        {
            ourProcessor->popout_open = false;
            popout.reset();
        }
        else
        {
            // First-ever open inherits the main view; subsequent reopens
            // use the popout's own remembered view.
            const bool useHammer = ourProcessor->popout_open ? ourProcessor->popout_hammer_view
                                                              : _hammerView;
            ourProcessor->popout_hammer_view = useHammer;
            ourProcessor->popout_open = true;
            popout = std::make_unique<PopoutPanner> (
                *ourProcessor,
                useHammer,
                ourProcessor->popout_width,
                ourProcessor->popout_height,
                [this]()
                {
                    getProcessor()->popout_open = false;
                    popout.reset();
                });
        }
    }
    else if (buttonThatWasClicked == &btn_view_sphere || buttonThatWasClicked == &btn_view_hammer)
    {
        // Keep the editor's height, recompute width to match the new view's
        // natural aspect (sphere ≈ height+148 because the panner is square;
        // H-A ≈ 2·height-10 because the panner is 2:1). The constants come
        // from the resized() layout: 158 px of header + bottom strip + table
        // column ~256 px + ~50 px of margins.
        const bool newHammer = (buttonThatWasClicked == &btn_view_hammer);
        _hammerView = newHammer;
        ourProcessor->editor_hammer_view = newHammer;
        updateActivePanner();

        const int currentH = getHeight();
        const int targetW  = newHammer ? juce::jlimit (520, 1400, 2 * currentH - 10)
                                       : juce::jlimit (520, 1400,     currentH + 148);
        setSize (targetW, currentH);
    }
    else if (buttonThatWasClicked == &btn_linked_toggle)
    {
        const bool nowLinked = btn_linked_toggle.getToggleState();
        setParameterNotifyingHost (ourProcessor,
                                   Ambix_encoderAudioProcessor::LinkedParam,
                                   nowLinked ? 1.f : 0.f);
        btn_linked_toggle.setButtonText (nowLinked ? "Linked" : "Unlinked");
        for (auto* row : table_rows)
            row->setEditable (! nowLinked);
        resized();   // global az/el sliders appear / disappear with the mode
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

//==============================================================================
// Presets menu
//
// One toolbar button replaces the old Import/Export pair. Named presets live
// as JSON files under userApplicationDataDirectory/ambix_encoder/presets and
// are listed inline in the menu so a single click loads one. Save/load to
// arbitrary paths is still available via the "Load from file..." /
// "Save to file..." entries. All prompts are async — no blocking modals.

namespace
{
    // Start the file pickers in the user's preset folder when it exists, so
    // the chooser drops them right where named presets live; otherwise fall
    // back to the processor's lastConfigDir (if any), then the home folder.
    juce::File pickStartDir (const PresetManager& pm, const juce::File& lastConfigDir)
    {
        auto dir = pm.getPresetDir();
        if (dir.isDirectory()) return dir;
        if (lastConfigDir.isDirectory()) return lastConfigDir;
        return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
    }
}

void Ambix_encoderAudioProcessorEditor::savePresetFile (const juce::File& file)
{
    auto r = getProcessor()->saveConfigurationToFile (file);
    if (r.failed())
        juce::AlertWindow::showAsync (juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::WarningIcon)
            .withTitle ("Save preset failed")
            .withMessage (r.getErrorMessage())
            .withButton ("OK"), nullptr);
}

void Ambix_encoderAudioProcessorEditor::loadPresetFile (const juce::File& file)
{
    auto r = getProcessor()->loadConfigurationFromFile (file);
    if (r.failed())
        juce::AlertWindow::showAsync (juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::WarningIcon)
            .withTitle ("Load preset failed")
            .withMessage (r.getErrorMessage())
            .withButton ("OK"), nullptr);
}

void Ambix_encoderAudioProcessorEditor::showPresetsMenu()
{
    juce::PopupMenu m;

    m.addItem ("Load from file...", [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> (
                          "Load source layout (JSON, SPARTA/IEM compatible)",
                          pickStartDir (presets_, getProcessor()->lastConfigDir),
                          "*.json");
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                const auto f = fc.getResult();
                if (! f.existsAsFile()) return;
                loadPresetFile (f);
            });
    });

    m.addItem ("Save to file...", [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> (
                          "Save source layout (JSON, SPARTA/IEM compatible)",
                          pickStartDir (presets_, getProcessor()->lastConfigDir)
                              .getChildFile ("ambix_encoder_layout.json"),
                          "*.json");
        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode
                                  | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting,
            [this] (const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.getFullPathName().isEmpty()) return;
                if (! f.hasFileExtension ("json"))
                    f = f.withFileExtension ("json");
                savePresetFile (f);
            });
    });

    m.addSeparator();

    m.addItem ("Save as named preset...", [this] { promptSaveAsNamedPreset(); });

    m.addSeparator();

    const auto entries = presets_.listPresets();
    if (entries.empty())
    {
        m.addItem ("(no presets yet)", false, false, [] {});
    }
    else
    {
        for (const auto& e : entries)
        {
            auto file = e.file;
            m.addItem (e.name, [this, file] { loadPresetFile (file); });
        }
    }

    m.addSeparator();

    juce::PopupMenu renameMenu, deleteMenu;
    for (const auto& e : entries)
    {
        auto file = e.file;
        renameMenu.addItem (e.name, [this, file] { promptRenamePreset (file); });
        deleteMenu.addItem (e.name, [this, file] { confirmDeletePreset (file); });
    }
    m.addSubMenu ("Rename preset", renameMenu, ! entries.empty());
    m.addSubMenu ("Delete preset", deleteMenu, ! entries.empty());

    m.addSeparator();

    m.addItem ("Reveal preset folder", [this]
    {
        presets_.ensurePresetDirExists();
        presets_.getPresetDir().revealToUser();
    });

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&btn_presets));
}

void Ambix_encoderAudioProcessorEditor::promptSaveAsNamedPreset()
{
    alertWindow_ = std::make_unique<juce::AlertWindow> (
                       "Save preset",
                       "Enter a name for this preset:",
                       juce::MessageBoxIconType::QuestionIcon);

    alertWindow_->addTextEditor ("name", "", {}, false);
    alertWindow_->addButton ("Save",   1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow_->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    alertWindow_->enterModalState (true,
        juce::ModalCallbackFunction::create ([this] (int result)
        {
            if (result == 0 || alertWindow_ == nullptr) { alertWindow_.reset(); return; }

            const auto raw   = alertWindow_->getTextEditorContents ("name");
            const auto clean = PresetManager::sanitise (raw);
            alertWindow_.reset();

            if (clean.isEmpty())
            {
                juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                    .withTitle ("Save preset")
                    .withMessage ("Preset name is empty.")
                    .withButton ("OK"), nullptr);
                return;
            }

            if (! presets_.ensurePresetDirExists())
            {
                juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                    .withTitle ("Save preset")
                    .withMessage ("Could not create the preset folder.")
                    .withButton ("OK"), nullptr);
                return;
            }

            const auto file = presets_.fileForName (clean);

            if (file.existsAsFile())
            {
                alertWindow_ = std::make_unique<juce::AlertWindow> (
                                   "Overwrite preset?",
                                   "A preset named \"" + clean + "\" already exists.\n"
                                   "Overwrite it?",
                                   juce::MessageBoxIconType::WarningIcon);
                alertWindow_->addButton ("Overwrite", 1, juce::KeyPress (juce::KeyPress::returnKey));
                alertWindow_->addButton ("Cancel",    0, juce::KeyPress (juce::KeyPress::escapeKey));
                alertWindow_->enterModalState (true,
                    juce::ModalCallbackFunction::create ([this, file] (int r)
                    {
                        alertWindow_.reset();
                        if (r != 1) return;
                        savePresetFile (file);
                    }), false);
                return;
            }

            savePresetFile (file);
        }), false);
}

void Ambix_encoderAudioProcessorEditor::promptRenamePreset (const juce::File& file)
{
    const auto oldName = file.getFileNameWithoutExtension();

    alertWindow_ = std::make_unique<juce::AlertWindow> (
                       "Rename preset",
                       "Enter a new name for \"" + oldName + "\":",
                       juce::MessageBoxIconType::QuestionIcon);

    alertWindow_->addTextEditor ("name", oldName, {}, false);
    alertWindow_->addButton ("Rename", 1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow_->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    alertWindow_->enterModalState (true,
        juce::ModalCallbackFunction::create ([this, file, oldName] (int result)
        {
            if (result == 0 || alertWindow_ == nullptr) { alertWindow_.reset(); return; }

            const auto raw   = alertWindow_->getTextEditorContents ("name");
            const auto clean = PresetManager::sanitise (raw);
            alertWindow_.reset();

            if (clean.isEmpty() || clean == oldName) return;

            const auto target = presets_.fileForName (clean);
            if (target.existsAsFile())
            {
                juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                    .withTitle ("Rename preset")
                    .withMessage ("A preset named \"" + clean + "\" already exists.")
                    .withButton ("OK"), nullptr);
                return;
            }

            if (! file.moveFileTo (target))
                juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                    .withTitle ("Rename preset")
                    .withMessage ("Rename failed.")
                    .withButton ("OK"), nullptr);
        }), false);
}

void Ambix_encoderAudioProcessorEditor::confirmDeletePreset (const juce::File& file)
{
    alertWindow_ = std::make_unique<juce::AlertWindow> (
                       "Delete preset?",
                       "Delete preset \"" + file.getFileNameWithoutExtension() + "\"?\n"
                       "This cannot be undone.",
                       juce::MessageBoxIconType::WarningIcon);
    alertWindow_->addButton ("Delete", 1, juce::KeyPress (juce::KeyPress::returnKey));
    alertWindow_->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    alertWindow_->enterModalState (true,
        juce::ModalCallbackFunction::create ([this, file] (int result)
        {
            alertWindow_.reset();
            if (result != 1) return;

            if (! file.deleteFile())
                juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                    .withTitle ("Delete preset")
                    .withMessage ("Delete failed.")
                    .withButton ("OK"), nullptr);
        }), false);
}
