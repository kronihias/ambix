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
    Slider& configurePosSlider (Slider& s, Slider::Listener* l, const juce::String& tip)
    {
        s.setRange (-5.0, 5.0, 0.001);
        s.setSliderStyle (Slider::LinearHorizontal);
        s.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
        s.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
        s.setDoubleClickReturnValue (true, 0.0);
        s.setTextValueSuffix (" m");
        s.setTooltip (tip);
        s.addListener (l);
        return s;
    }

    Label& configureLabel (Label& lbl, const juce::String& text)
    {
        lbl.setText (text, dontSendNotification);
        lbl.setFont (Font (FontOptions {15.0f, Font::plain}));
        lbl.setJustificationType (Justification::centredRight);
        lbl.setColour (Label::textColourId, Colours::white);
        return lbl;
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
    sld_radius.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
    sld_radius.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_radius.setDoubleClickReturnValue (true, 1.0);
    sld_radius.setTextValueSuffix (" m");
    sld_radius.setTooltip ("reference radius - radius of the virtual sampling sphere");
    sld_radius.addListener (this);

    addAndMakeVisible (configureLabel (lbl_x,      "X"));
    addAndMakeVisible (configureLabel (lbl_y,      "Y"));
    addAndMakeVisible (configureLabel (lbl_z,      "Z"));
    addAndMakeVisible (configureLabel (lbl_radius, "Radius"));

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
    constrainer.setMinimumSize (440, 460);
    constrainer.setFixedAspectRatio (580.0 / 580.0);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (580, 580);

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
    const float scale = w / 580.0f;

    const int margin     = juce::roundToInt (10 * scale);
    const int titleH     = juce::roundToInt (35 * scale);
    const int rowH       = juce::roundToInt (28 * scale);
    const int gap        = juce::roundToInt (5 * scale);
    const int sectionGap = juce::roundToInt (10 * scale);

    // Top-right buttons
    const int btnW = juce::roundToInt (62 * scale);
    const int smallBtnW = juce::roundToInt (36 * scale);
    const int btnH = juce::roundToInt (16 * scale);
    const int btnY = juce::roundToInt (8 * scale);
    btn_redo.setBounds  (w - smallBtnW - margin,                       btnY, smallBtnW, btnH);
    btn_undo.setBounds  (w - 2 * smallBtnW - gap - margin,             btnY, smallBtnW, btnH);
    btn_reset.setBounds (w - 2 * smallBtnW - btnW - 2 * gap - margin,  btnY, btnW, btnH);

    // Bottom controls: 4 position rows + 1 order row
    const int controlsH = 5 * rowH + 4 * gap + sectionGap;
    const int controlsTop = h - controlsH - margin;

    // 3D view fills the remaining space
    const int vizTop = titleH;
    const int vizH = controlsTop - vizTop - gap;
    view.setBounds (margin, vizTop, w - 2 * margin, vizH);

    const int contentW = w - 2 * margin;
    const int lblW = juce::roundToInt (contentW * 0.14f);
    const int sldW = contentW - lblW - gap;
    const int x0 = margin;

    int y = controlsTop;
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

    // Final row: In Order + Out Order
    y += sectionGap - gap;
    const int colW = contentW / 2;
    const int incDecW = juce::roundToInt (58 * scale);
    const int incDecH = juce::roundToInt (18 * scale);
    const int incDecY = y + (rowH - incDecH) / 2;
    lbl_in_order.setBounds  (x0, y, lblW, rowH);
    sld_in_order.setBounds  (x0 + lblW + gap, incDecY, incDecW, incDecH);

    lbl_out_order.setBounds (x0 + colW, y, lblW, rowH);
    sld_out_order.setBounds (x0 + colW + lblW + gap, incDecY, incDecW, incDecH);
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

    // Update view
    view.setListener (xM, yM, zM);
    view.setReferenceRadius (rM);

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
