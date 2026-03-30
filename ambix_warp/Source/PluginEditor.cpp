/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ambix is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 ==============================================================================
 */

#include "PluginProcessor.h"
#include "../../common/JuceCompat.h"
#include "PluginEditor.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

//==============================================================================
Ambix_warpAudioProcessorEditor::Ambix_warpAudioProcessorEditor (Ambix_warpAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      _changed (true)
{
    setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears (700);

    // --- Visualization ---
    addAndMakeVisible (grid);
    addAndMakeVisible (visualizer);

    // --- Az Warp slider ---
    addAndMakeVisible (sld_phi);
    sld_phi.setRange (-0.9, 0.9, 0.01);
    sld_phi.setSliderStyle (Slider::LinearHorizontal);
    sld_phi.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
    sld_phi.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_phi.addListener (this);
    sld_phi.setDoubleClickReturnValue (true, 0.0);
    sld_phi.setTooltip ("azimuth warp factor");

    addAndMakeVisible (lbl_phi);
    lbl_phi.setText ("Az Warp", dontSendNotification);
    lbl_phi.setFont (Font (15.0f, Font::plain));
    lbl_phi.setJustificationType (Justification::centredRight);
    lbl_phi.setColour (Label::textColourId, Colours::white);

    // --- Az Curve slider (two-state: 0=toward poles, 1=toward equator) ---
    addAndMakeVisible (sld_phi_curve);
    sld_phi_curve.setRange (0.0, 1.0, 1.0);
    sld_phi_curve.setSliderStyle (Slider::LinearHorizontal);
    sld_phi_curve.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    sld_phi_curve.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_phi_curve.addListener (this);
    sld_phi_curve.setTooltip ("azimuth warp curve type");

    addAndMakeVisible (lbl_phi_curve);
    lbl_phi_curve.setText ("-90 / 90 deg", dontSendNotification);
    lbl_phi_curve.setFont (Font (12.0f, Font::plain));
    lbl_phi_curve.setJustificationType (Justification::centredRight);
    lbl_phi_curve.setColour (Label::textColourId, Colours::white);

    // --- El Warp slider ---
    addAndMakeVisible (sld_theta);
    sld_theta.setRange (-0.9, 0.9, 0.01);
    sld_theta.setSliderStyle (Slider::LinearHorizontal);
    sld_theta.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
    sld_theta.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_theta.addListener (this);
    sld_theta.setDoubleClickReturnValue (true, 0.0);
    sld_theta.setTooltip ("elevation warp factor");

    addAndMakeVisible (lbl_theta);
    lbl_theta.setText ("El Warp", dontSendNotification);
    lbl_theta.setFont (Font (15.0f, Font::plain));
    lbl_theta.setJustificationType (Justification::centredRight);
    lbl_theta.setColour (Label::textColourId, Colours::white);

    // --- El Curve slider (two-state: 0=toward pole, 1=toward equator) ---
    addAndMakeVisible (sld_theta_curve);
    sld_theta_curve.setRange (0.0, 1.0, 1.0);
    sld_theta_curve.setSliderStyle (Slider::LinearHorizontal);
    sld_theta_curve.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    sld_theta_curve.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_theta_curve.addListener (this);
    sld_theta_curve.setTooltip ("elevation warp curve type");

    addAndMakeVisible (lbl_theta_curve);
    lbl_theta_curve.setText ("northpole", dontSendNotification);
    lbl_theta_curve.setFont (Font (12.0f, Font::plain));
    lbl_theta_curve.setJustificationType (Justification::centredRight);
    lbl_theta_curve.setColour (Label::textColourId, Colours::white);

    // --- In Order (IncDecButtons) ---
    addAndMakeVisible (sld_in_order);
    sld_in_order.setRange (0, AMBI_ORDER, 1);
    sld_in_order.setSliderStyle (Slider::IncDecButtons);
    sld_in_order.setTextBoxStyle (Slider::TextBoxLeft, false, 30, 18);
    sld_in_order.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_in_order.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);
    sld_in_order.addListener (this);
    sld_in_order.setTooltip ("input ambisonic order");

    addAndMakeVisible (lbl_in_order);
    lbl_in_order.setText ("In Order", dontSendNotification);
    lbl_in_order.setFont (Font (15.0f, Font::plain));
    lbl_in_order.setJustificationType (Justification::centredRight);
    lbl_in_order.setColour (Label::textColourId, Colours::white);

    // --- Out Order (IncDecButtons) ---
    addAndMakeVisible (sld_out_order);
    sld_out_order.setRange (0, AMBI_ORDER, 1);
    sld_out_order.setSliderStyle (Slider::IncDecButtons);
    sld_out_order.setTextBoxStyle (Slider::TextBoxLeft, false, 30, 18);
    sld_out_order.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_out_order.setIncDecButtonsMode (Slider::incDecButtonsDraggable_Vertical);
    sld_out_order.addListener (this);
    sld_out_order.setTooltip ("output ambisonic order");

    addAndMakeVisible (lbl_out_order);
    lbl_out_order.setText ("Out Order", dontSendNotification);
    lbl_out_order.setFont (Font (15.0f, Font::plain));
    lbl_out_order.setJustificationType (Justification::centredRight);
    lbl_out_order.setColour (Label::textColourId, Colours::white);

    // --- Pre-Emphasis (toggle button) ---
    addAndMakeVisible (btn_preemp);
    btn_preemp.setButtonText ("Pre-Emp");
    btn_preemp.setColour (ToggleButton::textColourId, Colours::white);
    btn_preemp.addListener (this);
    btn_preemp.setTooltip ("pre-emphasis compensation");

    // --- Register for processor change notifications ---
    getProcessor()->addChangeListener (this);

    // --- Window size ---
    constrainer.setMinimumSize (400, 340);
    constrainer.setFixedAspectRatio (600.0 / 470.0);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (600, 470);

    // --- Start timer for UI refresh ---
    startTimer (40); // ~25 fps

    // --- Load initial state ---
    timerCallback();
}

Ambix_warpAudioProcessorEditor::~Ambix_warpAudioProcessorEditor()
{
    getProcessor()->removeChangeListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void Ambix_warpAudioProcessorEditor::paint (Graphics& g)
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

    // Border
    g.setColour (Colours::black);
    g.drawRect (getLocalBounds(), 1);

    // Title
    g.setColour (Colours::white);
    g.setFont (Font (17.2f, Font::bold));
    g.drawText ("AMBIX-WARP", 0, 4, getWidth(), 30, Justification::centred, true);

    // Version string (bottom-right)
    g.setFont (Font (10.0f, Font::plain));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string, getWidth() - 51, getHeight() - 11, 50, 10, Justification::bottomRight, true);
}

void Ambix_warpAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const float scale = w / 600.0f;

    // Margins and spacing scaled proportionally
    const int margin = juce::roundToInt (10 * scale);
    const int titleH = juce::roundToInt (35 * scale);
    const int rowH = juce::roundToInt (28 * scale);
    const int gap = juce::roundToInt (5 * scale);
    const int sectionGap = juce::roundToInt (10 * scale);

    // Controls area: 3 rows at bottom (2 warp rows + 1 order row + gaps)
    const int controlsH = 3 * rowH + 2 * gap + sectionGap;
    const int controlsTop = h - controlsH - margin;

    // Visualization fills from title to controls
    const int vizTop = titleH;
    const int vizH = controlsTop - vizTop - gap;
    grid.setBounds (margin, vizTop, w - 2 * margin, vizH);
    visualizer.setBounds (margin, vizTop, w - 2 * margin, vizH);

    // Control area layout
    const int contentW = w - 2 * margin;
    const int lblW = juce::roundToInt (contentW * 0.12f);
    const int sldW = juce::roundToInt (contentW * 0.38f);
    const int curveLblW = juce::roundToInt (contentW * 0.16f);
    const int curveSliderW = contentW - lblW - sldW - curveLblW - 3 * gap;
    const int x0 = margin;

    // Row 1: Az Warp
    int y = controlsTop;
    lbl_phi.setBounds (x0, y, lblW, rowH);
    sld_phi.setBounds (x0 + lblW + gap, y, sldW, rowH);
    lbl_phi_curve.setBounds (x0 + lblW + sldW + 2 * gap, y, curveLblW, rowH);
    sld_phi_curve.setBounds (x0 + lblW + sldW + curveLblW + 3 * gap, y, curveSliderW, rowH);

    // Row 2: El Warp
    y += rowH + gap;
    lbl_theta.setBounds (x0, y, lblW, rowH);
    sld_theta.setBounds (x0 + lblW + gap, y, sldW, rowH);
    lbl_theta_curve.setBounds (x0 + lblW + sldW + 2 * gap, y, curveLblW, rowH);
    sld_theta_curve.setBounds (x0 + lblW + sldW + curveLblW + 3 * gap, y, curveSliderW, rowH);

    // Row 3: In Order, Out Order, Pre-Emp
    y += rowH + sectionGap;
    const int col3W = contentW / 3;
    const int incDecW = juce::roundToInt (70 * scale);

    lbl_in_order.setBounds (x0, y, lblW, rowH);
    sld_in_order.setBounds (x0 + lblW + gap, y + 2, incDecW, rowH - 4);

    lbl_out_order.setBounds (x0 + col3W, y, lblW, rowH);
    sld_out_order.setBounds (x0 + col3W + lblW + gap, y + 2, incDecW, rowH - 4);

    btn_preemp.setBounds (x0 + col3W * 2 + sectionGap, y, col3W - sectionGap - gap, rowH);
}

//==============================================================================
void Ambix_warpAudioProcessorEditor::sliderValueChanged (Slider* s)
{
    auto* p = getProcessor();

    if (s == &sld_phi)
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::PhiParam,
                                   (float) (sld_phi.getValue() + 0.9) / 1.8f);
    else if (s == &sld_theta)
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::ThetaParam,
                                   (float) (sld_theta.getValue() + 0.9) / 1.8f);
    else if (s == &sld_phi_curve)
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::PhiCurveParam,
                                   (float) sld_phi_curve.getValue());
    else if (s == &sld_theta_curve)
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::ThetaCurveParam,
                                   (float) sld_theta_curve.getValue());
    else if (s == &sld_in_order)
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::InOrderParam,
                                   (float) sld_in_order.getValue() / (float) AMBI_ORDER);
    else if (s == &sld_out_order)
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::OutOrderParam,
                                   (float) sld_out_order.getValue() / (float) AMBI_ORDER);
}

void Ambix_warpAudioProcessorEditor::buttonClicked (Button* b)
{
    if (b == &btn_preemp)
    {
        auto* p = getProcessor();
        setParameterNotifyingHost (p, Ambix_warpAudioProcessor::PreEmpParam,
                                   btn_preemp.getToggleState() ? 1.0f : 0.0f);
    }
}

void Ambix_warpAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster*)
{
    _changed = true;
}

void Ambix_warpAudioProcessorEditor::timerCallback()
{
    auto* p = getProcessor();

    float phi = p->getParameter (Ambix_warpAudioProcessor::PhiParam);
    float phiC = p->getParameter (Ambix_warpAudioProcessor::PhiCurveParam);
    float theta = p->getParameter (Ambix_warpAudioProcessor::ThetaParam);
    float thetaC = p->getParameter (Ambix_warpAudioProcessor::ThetaCurveParam);
    float inOrd = p->getParameter (Ambix_warpAudioProcessor::InOrderParam);
    float outOrd = p->getParameter (Ambix_warpAudioProcessor::OutOrderParam);
    float preEmp = p->getParameter (Ambix_warpAudioProcessor::PreEmpParam);

    // Update sliders from processor state
    sld_phi.setValue (phi * 1.8 - 0.9, dontSendNotification);
    sld_theta.setValue (theta * 1.8 - 0.9, dontSendNotification);
    sld_phi_curve.setValue (phiC > 0.5f ? 1.0 : 0.0, dontSendNotification);
    sld_theta_curve.setValue (thetaC > 0.5f ? 1.0 : 0.0, dontSendNotification);
    sld_in_order.setValue (round (inOrd * AMBI_ORDER), dontSendNotification);
    sld_out_order.setValue (round (outOrd * AMBI_ORDER), dontSendNotification);
    btn_preemp.setToggleState (preEmp > 0.5f, dontSendNotification);

    // Update curve labels to match VST parameter text
    lbl_phi_curve.setText (phiC > 0.5f ? "180 / 0 deg" : "-90 / 90 deg", dontSendNotification);
    lbl_theta_curve.setText (thetaC > 0.5f ? "equator" : "northpole", dontSendNotification);

    // Update visualization (internally checks if params actually changed)
    visualizer.setWarpParams (phi, phiC, theta, thetaC);
}
