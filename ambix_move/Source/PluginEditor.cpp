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

#include "PluginProcessor.h"
#include "../../common/JuceCompat.h"
#include "PluginEditor.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

using P = Ambix_moveAudioProcessor;

namespace
{
    // Slightly longer than ±180 so the slider can overshoot and wrap.
    constexpr double ROT_SLIDER_MAX = 180 + 12;

    // Width in pixels of the text box on the left of a linear slider.
    // Must be wide enough to show values like "-180.0 deg" / "-5.000 m".
    static constexpr int kSliderTextBoxW = 68;
    static constexpr int kSliderTextBoxH = 18;

    Slider& configurePosSlider (Slider& s, Slider::Listener* l, const juce::String& tip)
    {
        s.setRange (-5.0, 5.0, 0.001);
        s.setSliderStyle (Slider::LinearHorizontal);
        s.setTextBoxStyle (Slider::TextBoxLeft, false, kSliderTextBoxW, kSliderTextBoxH);
        s.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
        s.setDoubleClickReturnValue (true, 0.0);
        s.setTextValueSuffix (" m");
        s.setTooltip (tip);
        s.addListener (l);
        return s;
    }

    Slider& configureRotSlider (Slider& s, Slider::Listener* l, const juce::String& tip)
    {
        s.setRange (-ROT_SLIDER_MAX, ROT_SLIDER_MAX, 0.1);
        s.setSliderStyle (Slider::LinearHorizontal);
        s.setTextBoxStyle (Slider::TextBoxLeft, false, kSliderTextBoxW, kSliderTextBoxH);
        s.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
        s.setDoubleClickReturnValue (true, 0.0);
        s.setTextValueSuffix (" deg");
        s.setTooltip (tip);
        s.addListener (l);
        return s;
    }

    Label& configureLabel (Label& lbl, const juce::String& text)
    {
        lbl.setText (text, dontSendNotification);
        lbl.setFont (Font (FontOptions {14.0f, Font::plain}));
        lbl.setJustificationType (Justification::centredRight);
        lbl.setColour (Label::textColourId, Colours::white);
        return lbl;
    }

    Label& configureTitleLabel (Label& lbl, const juce::String& text)
    {
        lbl.setText (text, dontSendNotification);
        lbl.setFont (Font (FontOptions {13.0f, Font::plain}));
        lbl.setJustificationType (Justification::centredRight);
        lbl.setColour (Label::textColourId, Colours::white);
        return lbl;
    }

    void configureQuatEditor (TextEditor& e, TextEditor::Listener* l, const juce::String& tip)
    {
        e.setTooltip (tip);
        e.setMultiLine (false);
        e.setReturnKeyStartsNewLine (false);
        e.setReadOnly (false);
        e.setScrollbarsShown (false);
        e.setCaretVisible (true);
        e.setPopupMenuEnabled (true);
        e.setJustification (Justification::centred);

        // Match the slider text-box colour scheme: dark background, white
        // text, subtle outline, white caret.  LookAndFeel_V3 defaults paint
        // TextEditors with a light background / dark text which clashes
        // with the rest of the plugin.
        e.setColour (TextEditor::backgroundColourId,      Colour (0xff1a1a1a));
        e.setColour (TextEditor::textColourId,            Colours::white);
        e.setColour (TextEditor::highlightColourId,       Colour (0x802b1d69));
        e.setColour (TextEditor::highlightedTextColourId, Colours::white);
        e.setColour (TextEditor::outlineColourId,         Colour (0xff3a3a3a));
        e.setColour (TextEditor::focusedOutlineColourId,  Colour (0xff5a5a8a));
        e.setColour (CaretComponent::caretColourId,       Colours::white);

        e.setText ("0.0000");
        e.setInputRestrictions (0, "0123456789.-+eE");
        e.addListener (l);
    }

    // Wraps rotation slider value back into -180..+180 after overshoot.
    double sliderWrap (Slider& sld)
    {
        double v  = sld.getValue();
        double v0 = v;
        if (sld.isMouseButtonDown())
        {
            v = juce::jlimit (-180., 180., v);
        }
        else
        {
            while (v < -180.) v += 360.;
            while (v >  180.) v -= 360.;
        }
        if (v0 != v)
            sld.setValue (v);
        return v;
    }
}

//==============================================================================
Ambix_moveAudioProcessorEditor::Ambix_moveAudioProcessorEditor (Ambix_moveAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      _changed (true)
{
    setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears (700);

    addAndMakeVisible (view);

    addAndMakeVisible (configurePosSlider (sld_x, this, "listener X position (front/back), metres"));
    addAndMakeVisible (configurePosSlider (sld_y, this, "listener Y position (left/right), metres"));
    addAndMakeVisible (configurePosSlider (sld_z, this, "listener Z position (up/down), metres"));

    addAndMakeVisible (sld_radius);
    sld_radius.setRange (1.0, 5.0, 0.01);
    sld_radius.setSliderStyle (Slider::LinearHorizontal);
    sld_radius.setTextBoxStyle (Slider::TextBoxLeft, false, kSliderTextBoxW, kSliderTextBoxH);
    sld_radius.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_radius.setDoubleClickReturnValue (true, 1.0);
    sld_radius.setTextValueSuffix (" m");
    sld_radius.setTooltip ("reference radius - radius of the virtual sampling sphere");
    sld_radius.addListener (this);

    addAndMakeVisible (configureLabel (lbl_x,      "X"));
    addAndMakeVisible (configureLabel (lbl_y,      "Y"));
    addAndMakeVisible (configureLabel (lbl_z,      "Z"));
    addAndMakeVisible (configureLabel (lbl_radius, "Radius"));

    addAndMakeVisible (configureTitleLabel (lbl_trans_title, "Translation"));
    lbl_trans_title.setFont (Font (FontOptions { 15.0f, Font::bold }));
    lbl_trans_title.setJustificationType (Justification::centredLeft);

    addAndMakeVisible (tgl_transl_enabled);
    tgl_transl_enabled.setButtonText ("enabled");
    tgl_transl_enabled.setTooltip ("enable / bypass listener translation");
    tgl_transl_enabled.setToggleState (true, dontSendNotification);
    tgl_transl_enabled.addListener (this);
    tgl_transl_enabled.setColour (ToggleButton::textColourId, Colours::white);

    // --- Rotation section -------------------------------------------------
    addAndMakeVisible (configureRotSlider (sld_yaw,   this, "yaw (left-right), degrees"));
    addAndMakeVisible (configureRotSlider (sld_pitch, this, "pitch (up-down), degrees"));
    addAndMakeVisible (configureRotSlider (sld_roll,  this, "roll (tilt), degrees"));

    addAndMakeVisible (configureLabel (lbl_yaw,   "Yaw"));
    addAndMakeVisible (configureLabel (lbl_pitch, "Pitch"));
    addAndMakeVisible (configureLabel (lbl_roll,  "Roll"));

    addAndMakeVisible (configureTitleLabel (lbl_rot_title,   "Rotation"));
    lbl_rot_title.setFont (Font (FontOptions { 15.0f, Font::bold }));
    lbl_rot_title.setJustificationType (Justification::centredLeft);

    addAndMakeVisible (tgl_rot_enabled);
    tgl_rot_enabled.setButtonText ("enabled");
    tgl_rot_enabled.setTooltip ("enable / bypass listener rotation");
    tgl_rot_enabled.setToggleState (true, dontSendNotification);
    tgl_rot_enabled.addListener (this);
    tgl_rot_enabled.setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (configureTitleLabel (lbl_euler_title, "Euler"));
    lbl_euler_title.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (configureTitleLabel (lbl_quat_title,  "Quaternion"));
    lbl_quat_title.setJustificationType (Justification::centredLeft);

    addAndMakeVisible (tgl_ypr);
    tgl_ypr.setButtonText ("ypr");
    tgl_ypr.setTooltip ("yaw -> pitch -> roll rotation order");
    tgl_ypr.setRadioGroupId (1);
    tgl_ypr.addListener (this);
    tgl_ypr.setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (tgl_rpy);
    tgl_rpy.setButtonText ("rpy");
    tgl_rpy.setTooltip ("roll -> pitch -> yaw rotation order");
    tgl_rpy.setRadioGroupId (1);
    tgl_rpy.addListener (this);
    tgl_rpy.setColour (ToggleButton::textColourId, Colours::white);

    configureQuatEditor (txt_q0, this, "quaternion w (q0)");
    configureQuatEditor (txt_q1, this, "quaternion x (q1)");
    configureQuatEditor (txt_q2, this, "quaternion y (q2)");
    configureQuatEditor (txt_q3, this, "quaternion z (q3)");
    addAndMakeVisible (txt_q0);
    addAndMakeVisible (txt_q1);
    addAndMakeVisible (txt_q2);
    addAndMakeVisible (txt_q3);

    addAndMakeVisible (configureLabel (lbl_q0, "W"));
    addAndMakeVisible (configureLabel (lbl_q1, "X"));
    addAndMakeVisible (configureLabel (lbl_q2, "Y"));
    addAndMakeVisible (configureLabel (lbl_q3, "Z"));

    addAndMakeVisible (tgl_qinvert);
    tgl_qinvert.setButtonText ("invert");
    tgl_qinvert.setTooltip ("internally invert the quaternion -> inverse rotation");
    tgl_qinvert.addListener (this);
    tgl_qinvert.setColour (ToggleButton::textColourId, Colours::white);

#ifdef WITH_OSC
    addAndMakeVisible (configureTitleLabel (lbl_osc_title, "OSC"));
    lbl_osc_title.setFont (Font (FontOptions { 15.0f, Font::bold }));
    lbl_osc_title.setJustificationType (Justification::centredLeft);

    addAndMakeVisible (tgl_osc_enable);
    tgl_osc_enable.setButtonText ("enable");
    tgl_osc_enable.setTooltip ("enable / disable the OSC receiver");
    tgl_osc_enable.addListener (this);
    tgl_osc_enable.setColour (ToggleButton::textColourId, Colours::white);
    tgl_osc_enable.setToggleState (getProcessor()->osc_enabled, dontSendNotification);

    addAndMakeVisible (configureLabel (lbl_osc_port, "Port"));
    lbl_osc_port.setJustificationType (Justification::centredRight);

    addAndMakeVisible (txt_osc_port);
    txt_osc_port.setMultiLine (false);
    txt_osc_port.setReturnKeyStartsNewLine (false);
    txt_osc_port.setScrollbarsShown (false);
    txt_osc_port.setCaretVisible (true);
    txt_osc_port.setPopupMenuEnabled (true);
    txt_osc_port.setJustification (Justification::centred);
    txt_osc_port.setColour (TextEditor::backgroundColourId,      Colour (0xff1a1a1a));
    txt_osc_port.setColour (TextEditor::textColourId,            Colours::white);
    txt_osc_port.setColour (TextEditor::highlightColourId,       Colour (0x802b1d69));
    txt_osc_port.setColour (TextEditor::highlightedTextColourId, Colours::white);
    txt_osc_port.setColour (TextEditor::outlineColourId,         Colour (0xff3a3a3a));
    txt_osc_port.setColour (TextEditor::focusedOutlineColourId,  Colour (0xff5a5a8a));
    txt_osc_port.setColour (CaretComponent::caretColourId,       Colours::white);
    txt_osc_port.setInputRestrictions (6, "0123456789");
    txt_osc_port.setText (getProcessor()->osc_in_port);
    txt_osc_port.setTooltip ("UDP port for incoming OSC messages (default 7130)");
    txt_osc_port.addListener (this);

    addAndMakeVisible (lbl_osc_syntax);
    lbl_osc_syntax.setText (
        "/rotation [pitch yaw roll]  deg\n"
        "/quaternion [qw qx qy qz]\n"
        "/xyz [x y z]  metres   (alias: /translation)\n"
        "/6dof [x y z qw qx qy qz]",
        dontSendNotification);
    lbl_osc_syntax.setFont (Font (FontOptions { Font::getDefaultMonospacedFontName(), 11.0f, Font::plain }));
    lbl_osc_syntax.setJustificationType (Justification::topLeft);
    lbl_osc_syntax.setColour (Label::textColourId, Colour (0xffb0b0b0));
    lbl_osc_syntax.setTooltip ("supported OSC message patterns");
#endif

    // --- In Order (IncDecButtons) ---
    addAndMakeVisible (sld_in_order);
    sld_in_order.setRange (0, AMBI_ORDER, 1);
    sld_in_order.setSliderStyle (Slider::IncDecButtons);
    sld_in_order.setTextBoxStyle (Slider::TextBoxLeft, false, 30, 18);
    sld_in_order.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_in_order.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);
    sld_in_order.addListener (this);
    sld_in_order.setTooltip ("input ambisonic order");

    addAndMakeVisible (configureLabel (lbl_in_order, "In Order"));

    // --- Out Order (IncDecButtons) ---
    addAndMakeVisible (sld_out_order);
    sld_out_order.setRange (0, AMBI_ORDER, 1);
    sld_out_order.setSliderStyle (Slider::IncDecButtons);
    sld_out_order.setTextBoxStyle (Slider::TextBoxLeft, false, 30, 18);
    sld_out_order.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_out_order.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);
    sld_out_order.addListener (this);
    sld_out_order.setTooltip ("output ambisonic order");

    addAndMakeVisible (configureLabel (lbl_out_order, "Out Order"));

    // --- Reset / Undo / Redo buttons ---
    addAndMakeVisible (btn_reset);
    btn_reset.setButtonText ("Reset XYZ");
    btn_reset.addListener (this);
    btn_reset.setTooltip ("reset listener position to origin");

    addAndMakeVisible (btn_undo);
    btn_undo.setButtonText ("Undo");
    btn_undo.addListener (this);
    btn_undo.setTooltip ("undo last parameter change");
    btn_undo.setEnabled (false);

    addAndMakeVisible (btn_redo);
    btn_redo.setButtonText ("Redo");
    btn_redo.addListener (this);
    btn_redo.setTooltip ("redo last undone change");
    btn_redo.setEnabled (false);

    // --- Drag callbacks from view -----------------------------------------
    view.onListenerXYDragged = [this] (float xMeters, float yMeters)
    {
        auto* p = getProcessor();
        setParameterNotifyingHost (p, P::XParam, P::xParamFromMeters (xMeters));
        setParameterNotifyingHost (p, P::YParam, P::xParamFromMeters (yMeters));
    };
    view.onListenerYZDragged = [this] (float yMeters, float zMeters)
    {
        auto* p = getProcessor();
        setParameterNotifyingHost (p, P::YParam, P::xParamFromMeters (yMeters));
        setParameterNotifyingHost (p, P::ZParam, P::xParamFromMeters (zMeters));
    };
    view.onRadiusWheel = [this] (float radiusDeltaM)
    {
        auto* p = getProcessor();
        const float currentM = P::radiusMetersFromParam (p->getParameter (P::RadiusParam));
        const float newM     = juce::jlimit (1.f, 5.f, currentM + radiusDeltaM);
        setParameterNotifyingHost (p, P::RadiusParam, P::radiusParamFromMeters (newM));
    };
    view.onResetListener = [this] ()
    {
        auto* p = getProcessor();
        setParameterNotifyingHost (p, P::XParam, P::xParamFromMeters (0.f));
        setParameterNotifyingHost (p, P::YParam, P::xParamFromMeters (0.f));
        setParameterNotifyingHost (p, P::ZParam, P::xParamFromMeters (0.f));
    };
    view.onGestureEnded = [this] () { saveUndoState(); };

    // --- Register for processor change notifications ---------------------
    getProcessor()->addChangeListener (this);

    // --- Window size ------------------------------------------------------
    // Wider default to make room for the rotation panel on the right, and
    // tall enough that all rotation rows (yaw/pitch/roll + Euler/quaternion
    // + invert + OSC) fit without clipping.
    constrainer.setMinimumSize (700, 600);
    constrainer.setFixedAspectRatio (880.0 / 700.0);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (880, 700);

    startTimer (40); // ~25 fps

    timerCallback();
    saveUndoState();
}

Ambix_moveAudioProcessorEditor::~Ambix_moveAudioProcessorEditor()
{
    getProcessor()->removeChangeListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void Ambix_moveAudioProcessorEditor::paint (Graphics& g)
{
    // Background gradient (matching other ambix plugins)
    g.setGradientFill (ColourGradient (Colour (0xff4e4e4e),
                                        proportionOfWidth (0.6400f),
                                        proportionOfHeight (0.6933f),
                                        Colours::black,
                                        proportionOfWidth (0.1143f),
                                        proportionOfHeight (0.0800f),
                                        true));
    g.fillAll();

    g.setColour (Colours::black);
    g.drawRect (getLocalBounds(), 1);

    // Title
    g.setColour (Colours::white);
    g.setFont (Font (FontOptions {17.2f, Font::bold}));
    {
        int order = ambiOrderFromChannels (getProcessor()->getTotalNumInputChannels());
        String title = "AMBIX-MOVE";
        if (order > 0)
            title << " O" << order;
        g.drawText (title, 0, 4, getWidth(), 30, Justification::centred, true);
    }

    // Version string (bottom-right)
    g.setFont (Font (FontOptions {10.0f, Font::plain}));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string, getWidth() - 51, getHeight() - 11, 50, 10,
                Justification::bottomRight, true);
}

void Ambix_moveAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const float scale = juce::jmin (w / 880.0f, h / 700.0f);

    const int margin     = juce::roundToInt (10 * scale);
    const int titleH     = juce::roundToInt (35 * scale);
    const int rowH       = juce::roundToInt (24 * scale);
    const int gap        = juce::roundToInt (4 * scale);
    const int sectionGap = juce::roundToInt (8 * scale);

    // Top-right buttons
    const int btnW = juce::roundToInt (62 * scale);
    const int smallBtnW = juce::roundToInt (36 * scale);
    const int btnH = juce::roundToInt (16 * scale);
    const int btnY = juce::roundToInt (8 * scale);
    btn_redo.setBounds  (w - smallBtnW - margin,                       btnY, smallBtnW, btnH);
    btn_undo.setBounds  (w - 2 * smallBtnW - gap - margin,             btnY, smallBtnW, btnH);
    btn_reset.setBounds (w - 2 * smallBtnW - btnW - 2 * gap - margin,  btnY, btnW, btnH);

    // Two columns of controls sit side-by-side.  The left column holds the
    // Translation section, the right the Rotation section — both now start
    // with a bold title row so they match visually.
    //
    //   Left column (Translation)             Right column (Rotation)
    //   Row 1 : Translation title            Row 1 : Rotation title
    //   Row 2 : X                            Row 2 : Yaw
    //   Row 3 : Y                            Row 3 : Pitch
    //   Row 4 : Z                            Row 4 : Roll
    //   Row 5 : Radius                       Row 5 : Euler  + [ypr] [rpy]
    //   Row 6 : In / Out order               Row 6 : Quaternion  W X Y Z
    //                                        Row 7 : invert quaternion
    //
    // Below both columns, spanning the full width, sits the OSC section
    // (enable toggle + port + message syntax help).
    const int rotRowCount = 7;
    const int colRowsH    = rotRowCount * rowH + (rotRowCount - 1) * gap;

    // OSC section height: 1 row for controls + 4 rows of help text.
    const int oscHelpH  = juce::roundToInt (60 * scale);
    const int oscRowsH  = rowH + gap + oscHelpH;
    const int oscTotalH = oscRowsH + sectionGap;

    const int controlsH   = colRowsH + oscTotalH;
    const int controlsTop = h - controlsH - margin;

    // Split the width 50/50 with a small inter-column margin.
    const int contentW = w - 2 * margin;
    const int colGap   = margin;
    const int halfW    = (contentW - colGap) / 2;
    const int x0       = margin;                 // left column origin
    const int xRot     = x0 + halfW + colGap;    // right column origin

    // 3D view fills the top area across the full width.
    const int vizTop = titleH;
    const int vizH   = controlsTop - vizTop - gap;
    view.setBounds (margin, vizTop, contentW, vizH);

    // ---- Translation rows (left half) -----------------------------------
    const int lblW = juce::roundToInt (halfW * 0.18f);
    const int sldW = halfW - lblW - gap;

    int y = controlsTop;

    // Row 1 : title + bypass toggle
    const int transTitleW  = juce::roundToInt (90 * scale);
    const int bypassGap    = juce::roundToInt (16 * scale);
    const int bypassToggleW = juce::roundToInt (80 * scale);
    lbl_trans_title.setBounds (x0, y, transTitleW, rowH);
    tgl_transl_enabled.setBounds (x0 + transTitleW + bypassGap, y, bypassToggleW, rowH);
    y += rowH + gap;

    auto rowSlider = [&] (Label& lbl, Slider& sld)
    {
        lbl.setBounds (x0, y, lblW, rowH);
        sld.setBounds (x0 + lblW + gap, y, sldW, rowH);
        y += rowH + gap;
    };

    rowSlider (lbl_x,      sld_x);
    rowSlider (lbl_y,      sld_y);
    rowSlider (lbl_z,      sld_z);
    rowSlider (lbl_radius, sld_radius);

    // In/Out order row (still within the left half)
    const int colHalf = halfW / 2;
    const int incDecW = juce::roundToInt (58 * scale);
    const int incDecH = juce::roundToInt (18 * scale);
    const int incDecY = y + (rowH - incDecH) / 2;
    lbl_in_order.setBounds  (x0, y, lblW, rowH);
    sld_in_order.setBounds  (x0 + lblW + gap, incDecY, incDecW, incDecH);
    lbl_out_order.setBounds (x0 + colHalf, y, lblW, rowH);
    sld_out_order.setBounds (x0 + colHalf + lblW + gap, incDecY, incDecW, incDecH);

    // ---- Rotation rows (right half) -------------------------------------
    const int rotLblW = juce::roundToInt (halfW * 0.22f);
    const int rotSldW = halfW - rotLblW - gap;
    int yR = controlsTop;

    // Row 1 : title + bypass toggle
    const int rotTitleW = juce::roundToInt (80 * scale);
    lbl_rot_title.setBounds (xRot, yR, rotTitleW, rowH);
    tgl_rot_enabled.setBounds (xRot + rotTitleW + bypassGap, yR, bypassToggleW, rowH);
    yR += rowH + gap;

    // Rows 2..4 : yaw / pitch / roll sliders
    auto rotRowSlider = [&] (Label& lbl, Slider& sld)
    {
        lbl.setBounds (xRot, yR, rotLblW, rowH);
        sld.setBounds (xRot + rotLblW + gap, yR, rotSldW, rowH);
        yR += rowH + gap;
    };

    rotRowSlider (lbl_yaw,   sld_yaw);
    rotRowSlider (lbl_pitch, sld_pitch);
    rotRowSlider (lbl_roll,  sld_roll);

    // Row 5 : Euler label + ypr/rpy toggles
    const int toggleW = juce::roundToInt (56 * scale);
    lbl_euler_title.setBounds (xRot, yR, rotLblW, rowH);
    tgl_ypr.setBounds (xRot + rotLblW + gap,               yR, toggleW, rowH);
    tgl_rpy.setBounds (xRot + rotLblW + gap + toggleW + gap, yR, toggleW, rowH);
    yR += rowH + gap;

    // Row 6 : Quaternion label + W/X/Y/Z editors
    lbl_quat_title.setBounds (xRot, yR, rotLblW, rowH);
    const int quatX = xRot + rotLblW + gap;
    const int quatW = (rotSldW - 3 * gap) / 4;
    const int quatH = juce::roundToInt (20 * scale);
    const int quatY = yR + (rowH - quatH) / 2;
    txt_q0.setBounds (quatX + 0 * (quatW + gap), quatY, quatW, quatH);
    txt_q1.setBounds (quatX + 1 * (quatW + gap), quatY, quatW, quatH);
    txt_q2.setBounds (quatX + 2 * (quatW + gap), quatY, quatW, quatH);
    txt_q3.setBounds (quatX + 3 * (quatW + gap), quatY, quatW, quatH);

    // The tiny W/X/Y/Z letter labels overlay the title column so they stay
    // next to the corresponding editor header letters.  Hide them — the
    // tooltip + editor value are enough — by collapsing their bounds.
    lbl_q0.setBounds (0, 0, 0, 0);
    lbl_q1.setBounds (0, 0, 0, 0);
    lbl_q2.setBounds (0, 0, 0, 0);
    lbl_q3.setBounds (0, 0, 0, 0);
    yR += rowH + gap;

    // Row 7 : invert quaternion
    tgl_qinvert.setBounds (xRot + rotLblW + gap, yR, rotSldW, rowH);
    yR += rowH + gap;

    // ---- OSC section (full width, bottom) -------------------------------
#ifdef WITH_OSC
    const int oscTop = controlsTop + colRowsH + sectionGap;

    // Row 1 : "OSC" title | enable toggle | Port label | port editor
    const int oscTitleW  = juce::roundToInt (60  * scale);
    const int oscEnableW = juce::roundToInt (80  * scale);
    const int oscPortLblW = juce::roundToInt (38 * scale);
    const int oscPortEdW = juce::roundToInt (70  * scale);
    const int oscEdH     = juce::roundToInt (20  * scale);
    const int oscEdY     = oscTop + (rowH - oscEdH) / 2;

    int xo = x0;
    lbl_osc_title.setBounds (xo, oscTop, oscTitleW, rowH);
    xo += oscTitleW + gap;
    tgl_osc_enable.setBounds (xo, oscTop, oscEnableW, rowH);
    xo += oscEnableW + gap;
    lbl_osc_port.setBounds (xo, oscTop, oscPortLblW, rowH);
    xo += oscPortLblW + gap;
    txt_osc_port.setBounds (xo, oscEdY, oscPortEdW, oscEdH);
    xo += oscPortEdW + gap;

    // Row 2 : multi-line syntax help, indented under the controls row
    lbl_osc_syntax.setBounds (x0, oscTop + rowH + gap,
                              contentW, oscHelpH);
#else
    (void) lbl_osc_title;
    (void) lbl_osc_port;
    (void) txt_osc_port;
    (void) tgl_osc_enable;
    (void) lbl_osc_syntax;
#endif
}

//==============================================================================
void Ambix_moveAudioProcessorEditor::sliderValueChanged (Slider* s)
{
    auto* p = getProcessor();

    if (s == &sld_x)
        setParameterNotifyingHost (p, P::XParam, P::xParamFromMeters ((float) sld_x.getValue()));
    else if (s == &sld_y)
        setParameterNotifyingHost (p, P::YParam, P::xParamFromMeters ((float) sld_y.getValue()));
    else if (s == &sld_z)
        setParameterNotifyingHost (p, P::ZParam, P::xParamFromMeters ((float) sld_z.getValue()));
    else if (s == &sld_radius)
        setParameterNotifyingHost (p, P::RadiusParam, P::radiusParamFromMeters ((float) sld_radius.getValue()));
    else if (s == &sld_yaw)
        setParameterNotifyingHost (p, P::YawParam,   P::paramFromDeg ((float) sliderWrap (sld_yaw)));
    else if (s == &sld_pitch)
        setParameterNotifyingHost (p, P::PitchParam, P::paramFromDeg ((float) sliderWrap (sld_pitch)));
    else if (s == &sld_roll)
        setParameterNotifyingHost (p, P::RollParam,  P::paramFromDeg ((float) sliderWrap (sld_roll)));
    else if (s == &sld_in_order)
        setParameterNotifyingHost (p, P::InOrderParam,
                                   (float) sld_in_order.getValue() / (float) juce::jmax (1.0, sld_in_order.getMaximum()));
    else if (s == &sld_out_order)
        setParameterNotifyingHost (p, P::OutOrderParam,
                                   (float) sld_out_order.getValue() / (float) juce::jmax (1.0, sld_out_order.getMaximum()));
}

void Ambix_moveAudioProcessorEditor::sliderDragStarted (Slider*) {}

void Ambix_moveAudioProcessorEditor::sliderDragEnded (Slider*)
{
    if (! undoInProgress)
        saveUndoState();
}

void Ambix_moveAudioProcessorEditor::buttonClicked (Button* b)
{
    auto* p = getProcessor();

    if (b == &btn_reset)
    {
        setParameterNotifyingHost (p, P::XParam, P::xParamFromMeters (0.f));
        setParameterNotifyingHost (p, P::YParam, P::xParamFromMeters (0.f));
        setParameterNotifyingHost (p, P::ZParam, P::xParamFromMeters (0.f));
        if (! undoInProgress) saveUndoState();
    }
    else if (b == &tgl_ypr)
    {
        setParameterNotifyingHost (p, P::RotOrderParam, 0.f);
    }
    else if (b == &tgl_rpy)
    {
        setParameterNotifyingHost (p, P::RotOrderParam, 1.f);
    }
    else if (b == &tgl_qinvert)
    {
        setParameterNotifyingHost (p, P::QinvertParam, (float) tgl_qinvert.getToggleState());
    }
    else if (b == &tgl_transl_enabled)
    {
        setParameterNotifyingHost (p, P::TranslEnabledParam, tgl_transl_enabled.getToggleState() ? 1.f : 0.f);
    }
    else if (b == &tgl_rot_enabled)
    {
        setParameterNotifyingHost (p, P::RotEnabledParam, tgl_rot_enabled.getToggleState() ? 1.f : 0.f);
    }
#ifdef WITH_OSC
    else if (b == &tgl_osc_enable)
    {
        p->setOscEnabled (tgl_osc_enable.getToggleState());
    }
#endif
    else if (b == &btn_undo)
    {
        if (undoIndex > 0)
        {
            undoInProgress = true;
            --undoIndex;
            auto& snap = undoHistory[(size_t) undoIndex];
            for (int i = 0; i < P::totalNumParams; ++i)
                setParameterNotifyingHost (p, i, snap.params[i]);
            undoInProgress = false;
        }
    }
    else if (b == &btn_redo)
    {
        if (undoIndex < (int) undoHistory.size() - 1)
        {
            undoInProgress = true;
            ++undoIndex;
            auto& snap = undoHistory[(size_t) undoIndex];
            for (int i = 0; i < P::totalNumParams; ++i)
                setParameterNotifyingHost (p, i, snap.params[i]);
            undoInProgress = false;
        }
    }
}

void Ambix_moveAudioProcessorEditor::textEditorReturnKeyPressed (TextEditor& ed)
{
    auto* p = getProcessor();

#ifdef WITH_OSC
    if (&ed == &txt_osc_port)
    {
        const String newPort = ed.getText();
        if (newPort != p->osc_in_port)
            p->setOscPort (newPort);
        ed.setText (p->osc_in_port, dontSendNotification);
        return;
    }
#endif

    // Quaternion editors: map text -> -1..+1 -> 0..1 parameter
    auto applyQuat = [&] (TextEditor& e, int paramIndex)
    {
        float v = e.getText().getFloatValue();
        v = juce::jlimit (-1.f, 1.f, v);
        setParameterNotifyingHost (p, paramIndex, (v + 1.f) * 0.5f);
    };

    if (&ed == &txt_q0)      applyQuat (txt_q0, P::Q0Param);
    else if (&ed == &txt_q1) applyQuat (txt_q1, P::Q1Param);
    else if (&ed == &txt_q2) applyQuat (txt_q2, P::Q2Param);
    else if (&ed == &txt_q3) applyQuat (txt_q3, P::Q3Param);
}

void Ambix_moveAudioProcessorEditor::textEditorFocusLost (TextEditor& ed)
{
    textEditorReturnKeyPressed (ed);
}

void Ambix_moveAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster*)
{
    _changed = true;
}

void Ambix_moveAudioProcessorEditor::timerCallback()
{
    auto* p = getProcessor();

    const float xN  = p->getParameter (P::XParam);
    const float yN  = p->getParameter (P::YParam);
    const float zN  = p->getParameter (P::ZParam);
    const float rN  = p->getParameter (P::RadiusParam);
    const float ioN = p->getParameter (P::InOrderParam);
    const float ooN = p->getParameter (P::OutOrderParam);

    const float xM = P::xMetersFromParam (xN);
    const float yM = P::xMetersFromParam (yN);
    const float zM = P::xMetersFromParam (zN);
    const float rM = P::radiusMetersFromParam (rN);

    // Update order slider range based on active channel count
    int maxOrder = AMBI_ORDER;
#ifdef UNIVERSAL_AMBISONIC
    {
        int activeOrder = ambiOrderFromChannels (p->getTotalNumInputChannels());
        if (activeOrder > 0)
            maxOrder = activeOrder;
    }
#endif
    sld_in_order.setRange (0, maxOrder, 1);
    sld_out_order.setRange (0, maxOrder, 1);

    sld_x.setValue      (xM, dontSendNotification);
    sld_y.setValue      (yM, dontSendNotification);
    sld_z.setValue      (zM, dontSendNotification);
    sld_radius.setValue (rM, dontSendNotification);
    sld_in_order.setValue  (round (ioN * maxOrder), dontSendNotification);
    sld_out_order.setValue (round (ooN * maxOrder), dontSendNotification);

    // Rotation
    const float yawN   = p->getParameter (P::YawParam);
    const float pitchN = p->getParameter (P::PitchParam);
    const float rollN  = p->getParameter (P::RollParam);
    const float rotOrd = p->getParameter (P::RotOrderParam);

    sld_yaw.setValue   (P::degFromParam (yawN),   dontSendNotification);
    sld_pitch.setValue (P::degFromParam (pitchN), dontSendNotification);
    sld_roll.setValue  (P::degFromParam (rollN),  dontSendNotification);

    tgl_ypr.setToggleState (rotOrd <  0.5f, dontSendNotification);
    tgl_rpy.setToggleState (rotOrd >= 0.5f, dontSendNotification);

    if (! txt_q0.hasKeyboardFocus (true))
        txt_q0.setText (String (p->getParameter (P::Q0Param) * 2. - 1., 4), dontSendNotification);
    if (! txt_q1.hasKeyboardFocus (true))
        txt_q1.setText (String (p->getParameter (P::Q1Param) * 2. - 1., 4), dontSendNotification);
    if (! txt_q2.hasKeyboardFocus (true))
        txt_q2.setText (String (p->getParameter (P::Q2Param) * 2. - 1., 4), dontSendNotification);
    if (! txt_q3.hasKeyboardFocus (true))
        txt_q3.setText (String (p->getParameter (P::Q3Param) * 2. - 1., 4), dontSendNotification);

    tgl_qinvert.setToggleState (p->getParameter (P::QinvertParam) > 0.5f, dontSendNotification);
    tgl_transl_enabled.setToggleState (p->getParameter (P::TranslEnabledParam) > 0.5f, dontSendNotification);
    tgl_rot_enabled.setToggleState    (p->getParameter (P::RotEnabledParam)    > 0.5f, dontSendNotification);

    // Visually indicate which rotation input is driving the matrix
    if (p->isQuaternionActive())
    {
        lbl_euler_title.setColour (Label::textColourId, Colours::dimgrey);
        lbl_quat_title .setColour (Label::textColourId, Colours::yellow);
    }
    else
    {
        lbl_euler_title.setColour (Label::textColourId, Colours::yellow);
        lbl_quat_title .setColour (Label::textColourId, Colours::dimgrey);
    }

#ifdef WITH_OSC
    if (! txt_osc_port.hasKeyboardFocus (true))
        txt_osc_port.setText (p->osc_in_port, dontSendNotification);
    tgl_osc_enable.setToggleState (p->osc_enabled, dontSendNotification);
    txt_osc_port.setEnabled (p->osc_enabled);
    lbl_osc_port.setColour (Label::textColourId, p->osc_enabled ? Colours::white : Colours::dimgrey);
#endif

    // Update view
    view.setListener (xM, yM, zM);
    view.setReferenceRadius (rM);

    float fwd[3] = { 1.f, 0.f, 0.f };
    p->getLookForward (fwd);
    view.setLookForward (fwd[0], fwd[1], fwd[2]);

    // Undo/redo button states
    btn_undo.setEnabled (undoIndex > 0);
    btn_redo.setEnabled (undoIndex < (int) undoHistory.size() - 1);
}

void Ambix_moveAudioProcessorEditor::saveUndoState()
{
    auto* p = getProcessor();
    ParamSnapshot snap;
    for (int i = 0; i < P::totalNumParams; ++i)
        snap.params[i] = p->getParameter (i);

    if (undoIndex >= 0 && undoIndex < (int) undoHistory.size() - 1)
        undoHistory.resize ((size_t) undoIndex + 1);

    if (! undoHistory.empty())
    {
        auto& top = undoHistory.back();
        bool same = true;
        for (int i = 0; i < P::totalNumParams; ++i)
        {
            if (std::abs (top.params[i] - snap.params[i]) > 1e-6f)
            {
                same = false;
                break;
            }
        }
        if (same) return;
    }

    undoHistory.push_back (snap);

    if (undoHistory.size() > 200)
        undoHistory.erase (undoHistory.begin());

    undoIndex = (int) undoHistory.size() - 1;
}
