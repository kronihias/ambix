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
#include "PluginEditor.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

//==============================================================================
Ambix_wideningAudioProcessorEditor::Ambix_wideningAudioProcessorEditor (Ambix_wideningAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
      _changed (true),
      paramWatcher (*ownerFilter)
{
    setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears (700);

    // --- Visualization ---
    addAndMakeVisible (visualizer);
    addAndMakeVisible (irDisplay);

    // --- Mod Depth slider (0-360°) ---
    addAndMakeVisible (sld_mod_depth);
    sld_mod_depth.setRange (0.0, 360.0, 0.1);
    sld_mod_depth.setSliderStyle (Slider::LinearHorizontal);
    sld_mod_depth.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
    sld_mod_depth.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_mod_depth.addListener (this);
    sld_mod_depth.setDoubleClickReturnValue (true, 0.0);
    sld_mod_depth.setTextValueSuffix (juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")));
    sld_mod_depth.setTooltip ("modulation depth / rotation angle in degrees");

    addAndMakeVisible (lbl_mod_depth);
    lbl_mod_depth.setText ("Mod Depth", dontSendNotification);
    lbl_mod_depth.setFont (Font (FontOptions {14.0f, Font::plain}));
    lbl_mod_depth.setJustificationType (Justification::centredRight);
    lbl_mod_depth.setColour (Label::textColourId, Colours::white);

    // --- Rotation Offset slider (-180..+180°) ---
    addAndMakeVisible (sld_rot_offset);
    sld_rot_offset.setRange (-180.0, 180.0, 0.1);
    sld_rot_offset.setSliderStyle (Slider::LinearHorizontal);
    sld_rot_offset.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
    sld_rot_offset.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_rot_offset.addListener (this);
    sld_rot_offset.setDoubleClickReturnValue (true, 0.0);
    sld_rot_offset.setTextValueSuffix (juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")));
    sld_rot_offset.setTooltip ("azimuth rotation offset in degrees");

    addAndMakeVisible (lbl_rot_offset);
    lbl_rot_offset.setText ("Rot Offset", dontSendNotification);
    lbl_rot_offset.setFont (Font (FontOptions {14.0f, Font::plain}));
    lbl_rot_offset.setJustificationType (Justification::centredRight);
    lbl_rot_offset.setColour (Label::textColourId, Colours::white);

    // --- Mod T slider (0.2-70 ms) ---
    addAndMakeVisible (sld_mod_t);
    sld_mod_t.setRange (0.2, 70.0, 0.1);
    sld_mod_t.setSliderStyle (Slider::LinearHorizontal);
    sld_mod_t.setTextBoxStyle (Slider::TextBoxLeft, false, 55, 20);
    sld_mod_t.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_mod_t.addListener (this);
    sld_mod_t.setDoubleClickReturnValue (true, 5.0);
    sld_mod_t.setTextValueSuffix (" ms");
    sld_mod_t.setTooltip ("time constant between FIR taps in ms");

    addAndMakeVisible (lbl_mod_t);
    lbl_mod_t.setText ("Mod T", dontSendNotification);
    lbl_mod_t.setFont (Font (FontOptions {14.0f, Font::plain}));
    lbl_mod_t.setJustificationType (Justification::centredRight);
    lbl_mod_t.setColour (Label::textColourId, Colours::white);

    // --- Single Sided toggle ---
    addAndMakeVisible (btn_single_sided);
    btn_single_sided.setButtonText ("Single Sided");
    btn_single_sided.setColour (ToggleButton::textColourId, Colours::white);
    btn_single_sided.addListener (this);
    btn_single_sided.setTooltip ("single-sided mode (causal FIR, no latency)");

    // --- Drag callback from visualizer ---
    // one gesture per drag, however many touches start one
    visualizer.onModDepthDragStarted = [this]
    {
        if (! depthGesture)
        {
            depthGesture = true;
            ambix::Parameter::at (*getProcessor(), Ambix_wideningAudioProcessor::ModDepthParam)->beginChangeGesture();
        }
    };

    visualizer.onModDepthDragEnded = [this]
    {
        if (depthGesture)
        {
            depthGesture = false;
            ambix::Parameter::at (*getProcessor(), Ambix_wideningAudioProcessor::ModDepthParam)->endChangeGesture();
        }
    };

    visualizer.onModDepthDragged = [this] (float normModDepth)
    {
        ambix::Parameter::at (*getProcessor(), Ambix_wideningAudioProcessor::ModDepthParam)->setValueNotifyingHost (normModDepth);
    };

    // --- Scroll wheel callback from visualizer (adjusts Mod T) ---
    visualizer.onModTScrolled = [this] (float delta)
    {
        auto* param = ambix::Parameter::at (*getProcessor(), Ambix_wideningAudioProcessor::ModTParam);
        float currentNorm = param->get();
        float newNorm = juce::jlimit (0.0f, 1.0f, currentNorm + delta);
        setInGesture (param, newNorm);
    };

    // --- Register for processor change notifications ---
    getProcessor()->addChangeListener (this);

    // --- Window size (resizable) ---
    setResizable (true, true);
    setResizeLimits (450, 400, 1200, 900);
    setSize (600, 530);

    // --- Start timer for UI refresh ---
    startTimer (40); // ~25 fps

    // --- Load initial state ---
    timerCallback();
}

Ambix_wideningAudioProcessorEditor::~Ambix_wideningAudioProcessorEditor()
{
    // closed mid-drag: don't leave the host's gesture open
    if (gestureSlider != nullptr)
        if (auto* param = parameterFor (gestureSlider))
            param->endChangeGesture();

    if (visualizer.onModDepthDragEnded)
        visualizer.onModDepthDragEnded();

    getProcessor()->removeChangeListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void Ambix_wideningAudioProcessorEditor::paint (Graphics& g)
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
    g.setFont (Font (FontOptions {17.2f, Font::bold}));
    {
        int order = ambiOrderFromChannels (getProcessor()->getTotalNumInputChannels());
        String title = "AMBIX-WIDENING";
        if (order > 0)
            title << " O" << order;
        g.drawText (title, 0, 4, getWidth(), 30, Justification::centred, true);
    }

    // Version string (bottom-right)
    g.setFont (Font (FontOptions {10.0f, Font::plain}));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string, getWidth() - 51, getHeight() - 11, 50, 10, Justification::bottomRight, true);
}

void Ambix_wideningAudioProcessorEditor::resized()
{
    int w = getWidth();
    int h = getHeight();
    int margin = 10;

    // Fixed-height elements
    int titleH = 35;
    int rowH = 28;
    int controlsH = rowH * 2 + 5 + 15; // 2 rows + gap + bottom padding
    int irHeight = juce::jmax (80, (int) (h * 0.17f));

    // Globe gets the remaining space
    int vizTop = titleH;
    int vizHeight = h - titleH - irHeight - controlsH - 15; // 15 for gaps
    vizHeight = juce::jmax (100, vizHeight);

    visualizer.setBounds (margin, vizTop, w - 2 * margin, vizHeight);

    // IR display
    int irTop = vizTop + vizHeight + 5;
    irDisplay.setBounds (margin, irTop, w - 2 * margin, irHeight);

    // Controls
    int controlsTop = irTop + irHeight + 10;
    int lblW = 75;
    int x0 = margin;
    int halfW = (w - 2 * x0) / 2;

    // Row 1: Mod Depth | Rot Offset
    int y = controlsTop;
    lbl_mod_depth.setBounds (x0, y, lblW, rowH);
    sld_mod_depth.setBounds (x0 + lblW + 5, y, halfW - lblW - 10, rowH);

    lbl_rot_offset.setBounds (x0 + halfW, y, lblW, rowH);
    sld_rot_offset.setBounds (x0 + halfW + lblW + 5, y, halfW - lblW - 10, rowH);

    // Row 2: Mod T | Single Sided
    y += rowH + 5;
    lbl_mod_t.setBounds (x0, y, lblW, rowH);
    sld_mod_t.setBounds (x0 + lblW + 5, y, halfW - lblW - 10, rowH);

    btn_single_sided.setBounds (x0 + halfW + 10, y, halfW - 15, rowH);
}

//==============================================================================
ambix::Parameter* Ambix_wideningAudioProcessorEditor::parameterFor (Slider* s) const
{
    auto& p = *getProcessor();

    if (s == &sld_mod_depth)  return ambix::Parameter::at (p, Ambix_wideningAudioProcessor::ModDepthParam);
    if (s == &sld_mod_t)      return ambix::Parameter::at (p, Ambix_wideningAudioProcessor::ModTParam);
    if (s == &sld_rot_offset) return ambix::Parameter::at (p, Ambix_wideningAudioProcessor::RotOffsetParam);

    return nullptr;
}

// one-shot change outside a drag (button, wheel, key): a gesture of its own
void Ambix_wideningAudioProcessorEditor::setInGesture (ambix::Parameter* p, float value)
{
    p->beginChangeGesture();
    p->setValueNotifyingHost (value);
    p->endChangeGesture();
}

// being dragged or typed into: don't refresh it from the processor
bool Ambix_wideningAudioProcessorEditor::isBusy (Slider& s) const
{
    if (&s == gestureSlider)
        return true;

    for (auto* child : s.getChildren())
        if (auto* label = dynamic_cast<Label*> (child))
            if (label->isBeingEdited())
                return true;

    return false;
}

void Ambix_wideningAudioProcessorEditor::sliderDragStarted (Slider* s)
{
    if (auto* param = parameterFor (s))
    {
        gestureSlider = s;
        param->beginChangeGesture();
    }
}

void Ambix_wideningAudioProcessorEditor::sliderDragEnded (Slider* s)
{
    if (auto* param = parameterFor (s))
    {
        param->endChangeGesture();

        if (gestureSlider == s)
            gestureSlider = nullptr;
    }
}

void Ambix_wideningAudioProcessorEditor::sliderValueChanged (Slider* s)
{
    auto* param = parameterFor (s);

    if (param == nullptr)
        return;

    float value = 0.f;

    if (s == &sld_mod_depth)
        value = (float) sld_mod_depth.getValue() / 360.0f;
    else if (s == &sld_mod_t)
        value = (float) (sld_mod_t.getValue() - 0.2) / 69.8f;
    else if (s == &sld_rot_offset)
        value = (float) (sld_rot_offset.getValue() + 180.0) / 360.0f;

    // drags, wheel, text entry and double-click arrive inside the slider's
    // drag notifications; anything else (keys) gets a gesture of its own
    if (s == gestureSlider)
        param->setValueNotifyingHost (value);
    else
        setInGesture (param, value);
}

void Ambix_wideningAudioProcessorEditor::buttonClicked (Button* b)
{
    if (b == &btn_single_sided)
    {
        setInGesture (ambix::Parameter::at (*getProcessor(), Ambix_wideningAudioProcessor::SingleSideParam),
                      btn_single_sided.getToggleState() ? 1.0f : 0.0f);
    }
}

void Ambix_wideningAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster*)
{
    _changed = true;
    // repaint (the order in the title) only when the channel count actually
    // changes; parameter changes are polled in timerCallback()
    const int chanKey = (getAudioProcessor()->getTotalNumInputChannels() << 16)
                      |  getAudioProcessor()->getTotalNumOutputChannels();
    if (chanKey != lastChanKey_) { lastChanKey_ = chanKey; repaint(); }
}

void Ambix_wideningAudioProcessorEditor::timerCallback()
{
    // a control skipped below (dragged / edited) is refreshed once it's free
    if (! paramWatcher.changed() && ! refreshPending)
        return;

    refreshPending = false;

    auto& p = *getProcessor();

    float modDepth = ambix::Parameter::at (p, Ambix_wideningAudioProcessor::ModDepthParam)->get();
    float modT = ambix::Parameter::at (p, Ambix_wideningAudioProcessor::ModTParam)->get();
    float rotOffset = ambix::Parameter::at (p, Ambix_wideningAudioProcessor::RotOffsetParam)->get();
    float singleSided = ambix::Parameter::at (p, Ambix_wideningAudioProcessor::SingleSideParam)->get();

    // Update sliders from processor state
    if (isBusy (sld_mod_depth)) refreshPending = true;
    else sld_mod_depth.setValue (modDepth * 360.0, dontSendNotification);

    if (isBusy (sld_mod_t)) refreshPending = true;
    else sld_mod_t.setValue (modT * 69.8 + 0.2, dontSendNotification);

    if (isBusy (sld_rot_offset)) refreshPending = true;
    else sld_rot_offset.setValue (rotOffset * 360.0 - 180.0, dontSendNotification);
    btn_single_sided.setToggleState (singleSided > 0.5f, dontSendNotification);

    // Update visualizations
    bool ss = singleSided > 0.5f;
    visualizer.setParams (modDepth, modT, rotOffset, ss);
    irDisplay.setParams (modDepth, modT, ss);
}
