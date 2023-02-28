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

//[Headers] You can add your own extra header files here...
#include "PluginProcessor.h"
//[/Headers]

#include "PluginEditor.h"


// Sliders are slightly longer than needed,
// to account for overshooting (so we can easily wrap the values).
// On my Debian/xfce4 system, the mouse-wheel increment is
// about 11.3 (less than 12)
#define SLIDER_MAX (180+12)

namespace {
    double sliderWrap (Slider& sld)
    {
        double v = sld.getValue();
        double v0=v;

        if (sld.isMouseButtonDown())
        {
            v = jlimit(-180., 180., v);
        } else {
            // wrap out-of-bound values back into -180..+180 range
            while(v < -180.) v += 360.;
            while(v >  180.) v -= 360.;
        }

        // avoid unneeded calls to setValue()
        if (v0 != v) {
            sld.setValue(v);
        }
        return v;
    }
}

//==============================================================================
Ambix_rotatorAudioProcessorEditor::Ambix_rotatorAudioProcessorEditor (Ambix_rotatorAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
    _changed(true)
{
    setLookAndFeel (&globalLaF);

    addAndMakeVisible (sld_yaw);
    sld_yaw.setTooltip (TRANS("yaw (left-right)"));
    sld_yaw.setRange (-SLIDER_MAX, SLIDER_MAX, 0.1);
    sld_yaw.setSliderStyle (Slider::LinearHorizontal);
    sld_yaw.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sld_yaw.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_yaw.addListener (this);
    sld_yaw.setDoubleClickReturnValue(true, 0.f);

    addAndMakeVisible (label2);
    label2.setText("Yaw", dontSendNotification);
    label2.setFont (Font (15.00f, Font::plain));
    label2.setJustificationType (Justification::centredLeft);
    label2.setEditable (false, false, false);
    label2.setColour (Label::textColourId, Colours::white);
    label2.setColour (TextEditor::textColourId, Colours::black);
    label2.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label3);
    label3.setText("Pitch", dontSendNotification);
    label3.setFont (Font (15.00f, Font::plain));
    label3.setJustificationType (Justification::centredLeft);
    label3.setEditable (false, false, false);
    label3.setColour (Label::textColourId, Colours::white);
    label3.setColour (TextEditor::textColourId, Colours::black);
    label3.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sld_pitch);
    sld_pitch.setTooltip (TRANS("pitch (up-down)"));
    sld_pitch.setRange (-SLIDER_MAX, SLIDER_MAX, 0.1);
    sld_pitch.setSliderStyle (Slider::LinearHorizontal);
    sld_pitch.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sld_pitch.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_pitch.addListener (this);
    sld_pitch.setDoubleClickReturnValue(true, 0.f);

    addAndMakeVisible (label4);
    label4.setText("Roll", dontSendNotification);
    label4.setFont (Font (15.00f, Font::plain));
    label4.setJustificationType (Justification::centredLeft);
    label4.setEditable (false, false, false);
    label4.setColour (Label::textColourId, Colours::white);
    label4.setColour (TextEditor::textColourId, Colours::black);
    label4.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sld_roll);
    sld_roll.setRange (-SLIDER_MAX, SLIDER_MAX, 0.1);
    sld_roll.setSliderStyle (Slider::LinearHorizontal);
    sld_roll.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sld_roll.setColour (Slider::thumbColourId, Colour (0xff2b1d69));
    sld_roll.addListener (this);
    sld_roll.setDoubleClickReturnValue(true, 0.f);

    addAndMakeVisible(label5);
    label5.setText("Euler rotation \norder:", dontSendNotification);
    label5.setFont (Font (15.00f, Font::plain));
    label5.setJustificationType (Justification::centredLeft);
    label5.setEditable (false, false, false);
    label5.setColour (Label::textColourId, Colours::white);
    label5.setColour (TextEditor::textColourId, Colours::black);
    label5.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (toggleButton);
    toggleButton.setButtonText ("yaw-pitch-roll");
    toggleButton.setRadioGroupId (1);
    toggleButton.addListener (this);
    toggleButton.setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible (toggleButton2);
    toggleButton2.setButtonText ("roll-pitch-yaw");
    toggleButton2.setRadioGroupId (1);
    toggleButton2.addListener (this);
    toggleButton2.setColour (ToggleButton::textColourId, Colours::white);

    addAndMakeVisible(txt_q0);
    txt_q0.setTooltip("q0, or w");
    txt_q0.setMultiLine(false);
    txt_q0.setReturnKeyStartsNewLine(false);
    txt_q0.setReadOnly(false);
    txt_q0.setScrollbarsShown(false);
    txt_q0.setCaretVisible(true);
    txt_q0.setPopupMenuEnabled(true);
    txt_q0.setText(TRANS("0.0"));
    txt_q0.addListener(this);
    txt_q0.setInputRestrictions(0, "0123456789.-+");

    addAndMakeVisible(label7);
    label7.setText("Euler", dontSendNotification);
    label7.setFont(Font(15.00f, Font::plain));
    label7.setJustificationType(Justification::centredRight);
    label7.setEditable(false, false, false);
    label7.setColour(Label::textColourId, Colours::white);
    label7.setColour(TextEditor::textColourId, Colours::black);
    label7.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    addAndMakeVisible(label8);
    label8.setText("Quaternion", dontSendNotification);
    label8.setFont(Font(15.00f, Font::plain));
    label8.setJustificationType(Justification::centredRight);
    label8.setEditable(false, false, false);
    label8.setColour(Label::textColourId, Colours::white);
    label8.setColour(TextEditor::textColourId, Colours::black);
    label8.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    addAndMakeVisible(label6);
    label6.setText("q0", dontSendNotification);
    label6.setFont(Font(15.00f, Font::plain));
    label6.setJustificationType(Justification::centredRight);
    label6.setEditable(false, false, false);
    label6.setColour(Label::textColourId, Colours::white);
    label6.setColour(TextEditor::textColourId, Colours::black);
    label6.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    addAndMakeVisible(txt_q1);
    txt_q1.setTooltip("q1 or x");
    txt_q1.setMultiLine(false);
    txt_q1.setReturnKeyStartsNewLine(false);
    txt_q1.setReadOnly(false);
    txt_q1.setScrollbarsShown(false);
    txt_q1.setCaretVisible(true);
    txt_q1.setPopupMenuEnabled(true);
    txt_q1.setText("0.0");
    txt_q1.addListener(this);
    txt_q1.setInputRestrictions(0, "0123456789.+-");

    addAndMakeVisible(label9);
    label9.setText("q1", dontSendNotification);
    label9.setFont(Font(15.00f, Font::plain));
    label9.setJustificationType(Justification::centredRight);
    label9.setEditable(false, false, false);
    label9.setColour(Label::textColourId, Colours::white);
    label9.setColour(TextEditor::textColourId, Colours::black);
    label9.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    addAndMakeVisible(txt_q2);
    txt_q2.setTooltip("q2 or y");
    txt_q2.setMultiLine(false);
    txt_q2.setReturnKeyStartsNewLine(false);
    txt_q2.setReadOnly(false);
    txt_q2.setScrollbarsShown(false);
    txt_q2.setCaretVisible(true);
    txt_q2.setPopupMenuEnabled(true);
    txt_q2.setText("0.0");
    txt_q2.addListener(this);
    txt_q2.setInputRestrictions(0, "0123456789.+-");

    addAndMakeVisible(label10);
    label10.setText("q2", dontSendNotification);
    label10.setFont(Font(15.00f, Font::plain));
    label10.setJustificationType(Justification::centredRight);
    label10.setEditable(false, false, false);
    label10.setColour(Label::textColourId, Colours::white);
    label10.setColour(TextEditor::textColourId, Colours::black);
    label10.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    addAndMakeVisible(txt_q3);
    txt_q3.setTooltip("q3 or z");
    txt_q3.setMultiLine(false);
    txt_q3.setReturnKeyStartsNewLine(false);
    txt_q3.setReadOnly(false);
    txt_q3.setScrollbarsShown(false);
    txt_q3.setCaretVisible(true);
    txt_q3.setPopupMenuEnabled(true);
    txt_q3.setText("0.0");
    txt_q3.addListener(this);
    txt_q3.setInputRestrictions(0, "0123456789.+-");

    addAndMakeVisible(label11);
    label11.setText("q3", dontSendNotification);
    label11.setFont(Font(15.00f, Font::plain));
    label11.setJustificationType(Justification::centredRight);
    label11.setEditable(false, false, false);
    label11.setColour(Label::textColourId, Colours::white);
    label11.setColour(TextEditor::textColourId, Colours::black);
    label11.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

    addAndMakeVisible(tgl_qinvert);
    tgl_qinvert.setTooltip("Internally inverse the quaternion resulting in the inverse rotation");
    tgl_qinvert.setButtonText("inverse quaternion rotation");
    tgl_qinvert.addListener(this);
    tgl_qinvert.setColour(ToggleButton::textColourId, Colours::white);

    setSize(400, 330);

    timerCallback();

    ownerFilter->addChangeListener(this);

    startTimer(40); // update display rate
}

Ambix_rotatorAudioProcessorEditor::~Ambix_rotatorAudioProcessorEditor()
{
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();

    // remove me as listener for changes
    ourProcessor->removeChangeListener(this);
    stopTimer();

}

//==============================================================================
void Ambix_rotatorAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();
    //[/UserPrePaint]

    g.fillAll(Colours::white);

    g.setGradientFill(ColourGradient(Colour(0xff4e4e4e),
      static_cast<float> (proportionOfWidth(0.6400f)), static_cast<float> (proportionOfHeight(0.6933f)),
      Colours::black,
      static_cast<float> (proportionOfWidth(0.1143f)), static_cast<float> (proportionOfHeight(0.0800f)),
      true));
    g.fillRect(0, 0, 410, 350);

    g.setColour(Colours::black);
    g.drawRect(0, 0, 410, 350, 1);

    g.setColour(Colours::azure);
    g.setFont(Font(17.20f, Font::bold));
    g.drawText(TRANS("AMBIX-ROTATOR"),
      10, 8, 380, 30,
      Justification::centred, true);

    g.setColour(Colour(0x932b1d69));
    g.fillRoundedRectangle(9.0f, 48.0f, 383.0f, 179.0f, 10.000f);

    g.setColour(Colour(0xff0e0e47));
    g.fillRoundedRectangle(19.0f, 173.0f, 357.0f, 48.0f, 10.000f);

    g.setColour(Colour(0xff39000b));
    g.fillRoundedRectangle(9.0f, 238.0f, 383.0f, 83.0f, 10.000f);


    /* Version text */
    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string,
                getWidth()-51, getHeight()-11, 50, 10,
                Justification::bottomRight, true);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Ambix_rotatorAudioProcessorEditor::resized()
{
    sld_yaw.setBounds(78, 66, 301, 24);
    label2.setBounds(21, 66, 54, 24);
    label3.setBounds(21, 100, 54, 24);
    sld_pitch.setBounds(78, 100, 301, 24);
    label4.setBounds(21, 134, 54, 24);
    sld_roll.setBounds(78, 134, 301, 24);
    label5.setBounds(35, 179, 109, 28);
    toggleButton.setBounds(146, 173, 141, 24);
    toggleButton2.setBounds(146, 195, 135, 24);
    txt_q0.setBounds(48, 260, 49, 24);
    label7.setBounds(273, 46, 109, 24);
    label8.setBounds(277, 236, 109, 24);
    label6.setBounds(17, 259, 28, 24);
    txt_q1.setBounds(133, 261, 49, 24);
    label9.setBounds(102, 260, 28, 24);
    txt_q2.setBounds(222, 261, 49, 24);
    label10.setBounds(191, 260, 28, 24);
    txt_q3.setBounds(311, 262, 49, 24);
    label11.setBounds(280, 261, 28, 24);
    tgl_qinvert.setBounds(23, 292, 216, 24);

}

void Ambix_rotatorAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    _changed = true;
}

void Ambix_rotatorAudioProcessorEditor::timerCallback()
{
    if (_changed)
    {
        _changed = false;

        Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();

        sld_yaw.setValue(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::YawParam)*360.f-180.f, dontSendNotification);

        sld_pitch.setValue(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::PitchParam)*360.f-180.f, dontSendNotification);

        sld_roll.setValue(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::RollParam)*360.f-180.f, dontSendNotification);

        if (ourProcessor->getParameter(Ambix_rotatorAudioProcessor::RotOrderParam) <= 0.5f)
        {
            toggleButton.setToggleState(true, dontSendNotification);
            toggleButton2.setToggleState(false, dontSendNotification);
        }
        else
        {
            toggleButton.setToggleState(false, dontSendNotification);
            toggleButton2.setToggleState(true, dontSendNotification);
        }

        txt_q0.setText(String(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::Q0Param)*2. - 1.).substring(0, 6), dontSendNotification);
        txt_q1.setText(String(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::Q1Param)*2. - 1.).substring(0, 6), dontSendNotification);
        txt_q2.setText(String(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::Q2Param)*2. - 1.).substring(0, 6), dontSendNotification);
        txt_q3.setText(String(ourProcessor->getParameter(Ambix_rotatorAudioProcessor::Q3Param)*2. - 1.).substring(0, 6), dontSendNotification);

        if (ourProcessor->getParameter(Ambix_rotatorAudioProcessor::Qinvert) <= 0.5f)
          tgl_qinvert.setToggleState(false, dontSendNotification);
        else
          tgl_qinvert.setToggleState(true, dontSendNotification);

        if (ourProcessor->isQuaternionActive())
        {
          label7.setColour(Label::textColourId, Colours::dimgrey);
          label7.setFont(Font(15.00f, Font::plain));
          label8.setColour(Label::textColourId, Colours::yellow);
          label8.setFont(Font(20.00f, Font::plain));
        }
        else
        {
          label7.setColour(Label::textColourId, Colours::yellow);
          label7.setFont(Font(20.00f, Font::plain));
          label8.setColour(Label::textColourId, Colours::dimgrey);
          label8.setFont(Font(15.00f, Font::plain));
        }
    }
}

void Ambix_rotatorAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();

    if (sliderThatWasMoved == &sld_yaw)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::YawParam, (sliderWrap(sld_yaw)+180.f)/360.f);
    }
    else if (sliderThatWasMoved == &sld_pitch)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::PitchParam, (sliderWrap(sld_pitch)+180.f)/360.f);
    }
    else if (sliderThatWasMoved == &sld_roll)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RollParam, (sliderWrap(sld_roll)+180.f)/360.f);
    }

}

void Ambix_rotatorAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();

    if (buttonThatWasClicked == &toggleButton)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RotOrderParam, 0.f);
    }
    else if (buttonThatWasClicked == &toggleButton2)
    {
        ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::RotOrderParam, 1.f);
    }
    else if (buttonThatWasClicked == &tgl_qinvert)
    {
      ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Qinvert, (float)tgl_qinvert.getToggleState());
    }

}

void Ambix_rotatorAudioProcessorEditor::textEditorReturnKeyPressed(TextEditor &editorChanged)
{
  Ambix_rotatorAudioProcessor* ourProcessor = getProcessor();

  float val = editorChanged.getText().getFloatValue();

  val = jlimit(-1.f, 1.f, val);

  val = (val + 1.f) * 0.5f;

  // editorChanged.setText(String(val), dontSendNotification);

  if (&editorChanged == &txt_q0)
  {
    ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q0Param, val);
  } else if (&editorChanged == &txt_q1)
  {
    ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q1Param, val);
  }
  else if (&editorChanged == &txt_q2)
  {
    ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q2Param, val);
  }
  else if (&editorChanged == &txt_q3)
  {
    ourProcessor->setParameterNotifyingHost(Ambix_rotatorAudioProcessor::Q3Param, val);
  }

}

void Ambix_rotatorAudioProcessorEditor::textEditorFocusLost(TextEditor &editorChanged)
{
  textEditorReturnKeyPressed(editorChanged);
}