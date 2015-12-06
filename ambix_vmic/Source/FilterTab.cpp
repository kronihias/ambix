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

#include "FilterTab.h"


FilterTab::FilterTab(int id, Ambix_vmicAudioProcessor* myProcessor)
{
    _id = id;
    _myProcessor = myProcessor;
    
    addAndMakeVisible (sld_az = new Slider ("new slider"));
    sld_az->setTooltip (TRANS("Azimuth Filter 1"));
    sld_az->setRange (-180, 180, 0.1);
    sld_az->setSliderStyle (Slider::Rotary);
    sld_az->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_az->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_az->setColour (Slider::trackColourId, Colours::aqua);
    sld_az->setColour (Slider::rotarySliderFillColourId, Colours::aqua);
    sld_az->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_az->addListener (this);
    sld_az->setDoubleClickReturnValue(true, 0.f);
    
    addAndMakeVisible (sld_el = new Slider ("new slider"));
    sld_el->setTooltip (TRANS("Elevation Filter 1"));
    sld_el->setRange (-180, 180, 0.1);
    sld_el->setSliderStyle (Slider::Rotary);
    sld_el->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_el->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_el->setColour (Slider::trackColourId, Colours::aqua);
    sld_el->setColour (Slider::rotarySliderFillColourId, Colours::aqua);
    sld_el->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_el->addListener (this);
    sld_el->setDoubleClickReturnValue(true, 0.f);
    
    addAndMakeVisible (box_shape = new ComboBox ("new combo box"));
    box_shape->setTooltip (TRANS("Shape of Filter 1"));
    box_shape->setEditableText (false);
    box_shape->setJustificationType (Justification::centredLeft);
    box_shape->setTextWhenNothingSelected (TRANS("circular"));
    box_shape->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    box_shape->addItem (TRANS("circular"), 1);
    box_shape->addItem (TRANS("rectangular"), 2);
    box_shape->addListener (this);
    
    addAndMakeVisible (sld_w = new Slider ("new slider"));
    sld_w->setTooltip (TRANS("Width Filter 1"));
    sld_w->setRange (0, 180, 0.1);
    sld_w->setSliderStyle (Slider::Rotary);
    sld_w->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_w->setColour (Slider::thumbColourId, Colour (0xff000002));
    sld_w->setColour (Slider::trackColourId, Colour (0xff010202));
    sld_w->setColour (Slider::rotarySliderFillColourId, Colours::yellow);
    sld_w->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_w->addListener (this);
    sld_w->setSkewFactor (0.8);
    sld_w->setDoubleClickReturnValue(true, 18.f);
    
    addAndMakeVisible (sld_h = new Slider ("new slider"));
    sld_h->setTooltip (TRANS("Height Filter 1"));
    sld_h->setRange (0, 180, 0.1);
    sld_h->setSliderStyle (Slider::Rotary);
    sld_h->setTextBoxStyle (Slider::TextBoxRight, false, 53, 18);
    sld_h->setColour (Slider::thumbColourId, Colour (0xff5a5a90));
    sld_h->setColour (Slider::trackColourId, Colours::aqua);
    sld_h->setColour (Slider::rotarySliderFillColourId, Colours::yellow);
    sld_h->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_h->addListener (this);
    sld_h->setSkewFactor (0.8);
    sld_h->setDoubleClickReturnValue(true, 18.f);
    
    addAndMakeVisible (sld_gain = new Slider ("new slider"));
    sld_gain->setTooltip (TRANS("Gain Filter 1"));
    sld_gain->setRange (-99, 20, 0.1);
    sld_gain->setSliderStyle (Slider::Rotary);
    sld_gain->setTextBoxStyle (Slider::TextBoxRight, false, 45, 18);
    sld_gain->setColour (Slider::thumbColourId, Colour (0xff000002));
    sld_gain->setColour (Slider::trackColourId, Colour (0xff010202));
    sld_gain->setColour (Slider::rotarySliderFillColourId, Colours::cornsilk);
    sld_gain->setColour (Slider::rotarySliderOutlineColourId, Colours::aqua);
    sld_gain->addListener (this);
    sld_gain->setDoubleClickReturnValue(true, 0.f);
}

FilterTab::~FilterTab()
{
    sld_az = nullptr;
    sld_el = nullptr;
    box_shape = nullptr;
    sld_w = nullptr;
    sld_h = nullptr;
    sld_gain = nullptr;
}

void FilterTab::paint(juce::Graphics &g)
{
    g.setColour (Colour (0xff2f2866));
    g.fillRoundedRectangle (0.0f, 0.0f, 234.0f, 110.0f, 10.000f);
    
    g.setColour (Colours::aqua);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("elevation"),
                21, 67, 82, 23,
                Justification::centred, true);
    
    g.setColour (Colours::aqua);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("azimuth"),
                24, 25, 82, 23,
                Justification::centred, true);
    
    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("width"),
                106, 62, 36, 23,
                Justification::centredRight, true);
    
    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("height"),
                99, 87, 44, 23,
                Justification::centredRight, true);
    
    g.setColour (Colours::white);
    g.setFont (Font (15.00f, Font::plain));
    g.drawText (TRANS("gain"),
                60, 5, 30, 23,
                Justification::centredLeft, true);
    
    g.setColour (Colours::yellow);
    g.setFont (Font (15.00f, Font::plain));
    String label("filter ");
    label << _id+1;
    
    g.drawText (label,
                5, 5, 47, 23,
                Justification::centredLeft, true);
}

void FilterTab::resized()
{
    // -50, -270
    sld_az->setBounds (6, 44, 84, 22);
    sld_el->setBounds (6, 85, 84, 22);
    box_shape->setBounds (111, 37, 113, 22);
    sld_w->setBounds (140, 63, 84, 22);
    sld_h->setBounds (140, 84, 84, 22);
    sld_gain->setBounds (86, 8, 76, 22);
}


void FilterTab::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sld_az)
    {
        _myProcessor->setParameterNotifyingHost(PARAMS_PER_FILTER*_id+Ambix_vmicAudioProcessor::AzimuthParam, Deg360ToParam(sliderThatWasMoved->getValue()) );
    }
    else if (sliderThatWasMoved == sld_el)
    {
        _myProcessor->setParameterNotifyingHost(PARAMS_PER_FILTER*_id+Ambix_vmicAudioProcessor::ElevationParam, Deg360ToParam(sliderThatWasMoved->getValue()) );
    }
    else if (sliderThatWasMoved == sld_w)
    {
        _myProcessor->setParameterNotifyingHost(PARAMS_PER_FILTER*_id+Ambix_vmicAudioProcessor::WidthParam, Deg180ToParam(sliderThatWasMoved->getValue()) );
    }
    else if (sliderThatWasMoved == sld_h)
    {
        _myProcessor->setParameterNotifyingHost(PARAMS_PER_FILTER*_id+Ambix_vmicAudioProcessor::HeightParam, Deg180ToParam(sliderThatWasMoved->getValue()) );
    }
    else if (sliderThatWasMoved == sld_gain)
    {
        _myProcessor->setParameterNotifyingHost(PARAMS_PER_FILTER*_id+Ambix_vmicAudioProcessor::GainParam, DbToParam(sliderThatWasMoved->getValue()) );
    }
}

void FilterTab::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == box_shape)
    {
        _myProcessor->setParameterNotifyingHost(PARAMS_PER_FILTER*_id+Ambix_vmicAudioProcessor::ShapeParam, (float)comboBoxThatHasChanged->getSelectedId()-1.f);
    }
}

void FilterTab::setFilter(float az, float el, bool shape, float width, float height, float gain)
{
    
    sld_az->setValue(az, dontSendNotification);
    
    sld_el->setValue(el, dontSendNotification);
    
    box_shape->setSelectedId(shape ? 2 : 1);
    
    sld_h->setEnabled(shape);
    
    sld_w->setValue(width , dontSendNotification);
    
    sld_h->setValue(height, dontSendNotification);
    
    sld_gain->setValue(gain, dontSendNotification);
    
}

