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


#include "PluginEditor.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

//==============================================================================
Ambix_vmicAudioProcessorEditor::Ambix_vmicAudioProcessorEditor (Ambix_vmicAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter),
    panninggraph(ownerFilter),
    tabbedComponent(TabbedButtonBar::TabsAtTop),
    tabbedComponent2(TabbedButtonBar::TabsAtTop)
{
    setLookAndFeel (&globalLaF);
    tooltipWindow.setMillisecondsBeforeTipAppears (700); // tooltip delay

    addAndMakeVisible (lbl_gd);
    lbl_gd.setText("Ambix_vmic", dontSendNotification);
    lbl_gd.setFont (Font (15.00f, Font::plain));
    lbl_gd.setJustificationType (Justification::centredLeft);
    lbl_gd.setEditable (false, false, false);
    lbl_gd.setColour (Label::textColourId, Colours::aquamarine);
    lbl_gd.setColour (TextEditor::textColourId, Colours::black);
    lbl_gd.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (filtergraph);
    filtergraph.setName ("new component");


    addAndMakeVisible (panninggraph);
    panninggraph.setName ("new component");
    panninggraph.addChangeListener(this);

    addAndMakeVisible (tabbedComponent);
    tabbedComponent.setTabBarDepth (30);


    addAndMakeVisible (tabbedComponent2);
    tabbedComponent2.setTabBarDepth (30);

    for (int i=0; i < NUM_FILTERS_VMIC; i++)
    {
        _filtertabs.add(new FilterTab(i, ownerFilter));
        String label("Filter ");
        label << i+1;
        if (i%2 == 0)
        {
            tabbedComponent.addTab (label, Colour (0xff2f2866), _filtertabs.getLast(), false);
        } else {
            tabbedComponent2.addTab (label, Colour (0xff2f2866), _filtertabs.getLast(), false);
        }

    }

    tabbedComponent.setCurrentTabIndex (ownerFilter->filter_sel_id_1);
    tabbedComponent2.setCurrentTabIndex (ownerFilter->filter_sel_id_2);

    setSize (630, 400);

    ownerFilter->addChangeListener(this);

    // ownerFilter->sendChangeMessage(); // get status from dsp
    changeListenerCallback(nullptr);

}


Ambix_vmicAudioProcessorEditor::~Ambix_vmicAudioProcessorEditor()
{
    Ambix_vmicAudioProcessor* ourProcessor = getProcessor();

    // remove me as listener for changes
    ourProcessor->removeChangeListener(this);

}

//==============================================================================
void Ambix_vmicAudioProcessorEditor::paint (Graphics& g)
{

    g.fillAll (Colour (0xff1a1a1a));


    /* Version text */
    g.setColour (Colours::white);
    g.setFont (Font (10.00f, Font::plain));
    String version_string;
    version_string << "v" << QUOTE(VERSION);
    g.drawText (version_string,
                getWidth()-51, getHeight()-11, 50, 10,
                Justification::bottomRight, true);

}

void Ambix_vmicAudioProcessorEditor::resized()
{
    lbl_gd.setBounds (0, 0, 204, 16);
    filtergraph.setBounds (26, 22, 580, 237);
    tabbedComponent.setBounds (20, 253, 282, 140);
    tabbedComponent2.setBounds (322, 253, 282, 140);
    panninggraph.setBounds (10, 25, 610, 220);
}


void Ambix_vmicAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster *source)
{
    Ambix_vmicAudioProcessor* ourProcessor = getProcessor();

    if (source == &panninggraph)
    {
        // select the currently changing tab
        int id = panninggraph.getCurrentId();
        if (id >= 0)
        {
            if (id%2 == 0)
            {
                tabbedComponent.setCurrentTabIndex((int)(id/2));
                ourProcessor->filter_sel_id_1 = (int)(id/2);
            }
            else
            {
                tabbedComponent2.setCurrentTabIndex((int)(id/2));
                ourProcessor->filter_sel_id_2 = (int)(id/2);
            }


        }


    } else {

        for (int i=0; i < NUM_FILTERS_VMIC; i++)
        {

            float az = ParamToDeg360( ourProcessor->getParameter(PARAMS_PER_FILTER*i+Ambix_vmicAudioProcessor::AzimuthParam) );

            float el = ParamToDeg360( ourProcessor->getParameter(PARAMS_PER_FILTER*i+Ambix_vmicAudioProcessor::ElevationParam) );

            bool sh = ourProcessor->getParameter(PARAMS_PER_FILTER*i+Ambix_vmicAudioProcessor::ShapeParam) <= 0.5f ? false : true;

            float w = ParamToDeg180(ourProcessor->getParameter(PARAMS_PER_FILTER*i+Ambix_vmicAudioProcessor::WidthParam) );

            float h = ParamToDeg180( ourProcessor->getParameter(PARAMS_PER_FILTER*i+Ambix_vmicAudioProcessor::HeightParam) );

            float g = ParamToDB( ourProcessor->getParameter(PARAMS_PER_FILTER*i+Ambix_vmicAudioProcessor::GainParam) );

            panninggraph.setFilter(i, az, el, sh, w, h, g);
            _filtertabs.getUnchecked(i)->setFilter(az, el, sh, w, h, g);

        }


    }

}

void Ambix_vmicAudioProcessorEditor::selectFilterTab(int id)
{
    tabbedComponent.setCurrentTabIndex(id);
}


