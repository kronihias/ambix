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

#include "PanningGraph.h"


PanningGraph::PanningGraph (AudioProcessor* processor) :
                            lxmargin(30.f),
                            rxmargin(15.f),
                            tymargin(5.f),
                            bymargin(12.f),
                            one_filter_solo_(false),
                            mouse_near_filter_id(-1)
{
    tooltipWindow.setMillisecondsBeforeTipAppears (200); // tooltip delay
    
    myprocessor_ = processor;
    
    // this draws the rectangular/elliptical shape
    for (int i=0; i<NUM_FILTERS; i++)
    {
        graphs_.add(new GraphComponent(i));
        addAndMakeVisible(graphs_.getLast());
    }
    
    // this is the drag button
    for (int i=0; i < NUM_FILTERS; i++)
    {
        btn_drag.add(new ImageButton (String(i)));
        
        addAndMakeVisible(btn_drag.getLast());
        btn_drag.getLast()->addListener(this);
        btn_drag.getLast()->setImages (false, true, true,
                                       ImageCache::getFromMemory (drag_off_png, drag_off_pngSize), 1.000f, Colour (0x00000000),
                                       ImageCache::getFromMemory (drag_over_png, drag_over_pngSize), 1.000f, Colour (0x00000000),
                                       ImageCache::getFromMemory (drag_on_png, drag_on_pngSize), 1.000f, Colour (0x00000000));
        btn_drag.getLast()->setTriggeredOnMouseDown(true);
        btn_drag.getLast()->setRepeatSpeed(50, 50);
        String tooltip("Filter ");
        tooltip << i+1;
        btn_drag.getLast()->setTooltip(tooltip);
        
        // the label
        String label(i+1);
        lbl_drag.add(new Label(label));
        lbl_drag.getLast()->setText(label, dontSendNotification);
        lbl_drag.getLast()->setInterceptsMouseClicks(false, false);
        lbl_drag.getLast()->setFont (Font (15.00f, Font::plain));
        lbl_drag.getLast()->setJustificationType (Justification::centred);
        lbl_drag.getLast()->setEditable (false, false, false);
        lbl_drag.getLast()->setColour (Label::textColourId, Colours::white);
        lbl_drag.getLast()->setColour (TextEditor::textColourId, Colours::black);
        lbl_drag.getLast()->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
        addAndMakeVisible(lbl_drag.getLast());
        
    }
    
}

PanningGraph::~PanningGraph()
{
    btn_drag.clear();
    graphs_.clear();
}

void PanningGraph::paint (Graphics& g)
{
    
    int width = getWidth();
    int height = getHeight();
    
    // background
    if (one_filter_solo_)
    {
        g.setColour (Colour (0x00ffffff));
    } else {
        g.setGradientFill (ColourGradient (Colour (0xff232338), width / 2, height / 2, Colour (0xff21222a), 2.5f, getHeight() / 2, true));
    }
    
    
    
    g.fillRoundedRectangle (lxmargin, tymargin, width - lxmargin - rxmargin, height-tymargin-bymargin, 10.000f);
    
    g.setColour (Colour (0x60ffffff));
    
    int elgridlines = 180/45+1;
    
    for (int i=0; i < elgridlines; i++)
    {
        float deg_val = 90-i*45;
        
        int ypos = degtoypos(deg_val);
        
        // text
        String axislabel = String((int)deg_val);
        axislabel << "°";
        g.setFont (Font ("Arial Rounded MT", 12.0f, Font::plain));
        g.drawText (axislabel, 0, ypos-6, 34, 12, Justification::centred, false);
        // g.drawText (String ("-") + axisLabel, 6, (int) (numHorizontalLines * (height - 5) / (numHorizontalLines + 1) + 3.5f), 45, 12, Justification::left, false);
    }
    
    int azgridlines = 360/45+1;
    
    for (int i=0; i < azgridlines; i++)
    {
        float deg_val = 180-i*45;
        
        int xpos = degtoxpos(deg_val);
        
        // text
        String axislabel = String((int)deg_val);
        axislabel << "°";
        g.setFont (Font ("Arial Rounded MT", 12.0f, Font::plain));
        g.drawText (axislabel, xpos-22, getHeight()-bymargin, 44, 12, Justification::centred, false);
    }
    
    g.setColour (Colour (0x60ffffff));
    g.strokePath (path_grid, PathStrokeType (0.25f));
    
    
    g.setColour (Colour (0xffffffff));
    g.strokePath (path_w_grid, PathStrokeType (0.25f));
    
}

void PanningGraph::resized()
{
    int width = getWidth();
    int height = getHeight();
    
    for (int i=0; i<graphs_.size(); i++) {
        graphs_.getUnchecked(i)->setBounds(0, 0, width, height);
    }
    
    // create the grid path
    path_w_grid.clear();
    path_grid.clear();
    
    
    int el_gridlines = 180/45+1;
    
    for (int i=0; i < el_gridlines; i++)
    {
        float deg_val = -90+i*45;
        
        int ypos = degtoypos(deg_val);
        
        if (deg_val == 0)
        {
            path_w_grid.startNewSubPath(degtoxpos(-180), ypos);
            path_w_grid.lineTo(degtoxpos(180), ypos);
        }
        else
        {
            path_grid.startNewSubPath(degtoxpos(-180), ypos);
            path_grid.lineTo(degtoxpos(180), ypos);
        }
        
    }
    
    
    int az_gridlines = 360/45+1;
    
    for (int i=0; i < az_gridlines; i++)
    {
        float deg_val = -180+i*45;
        
        int xpos = degtoxpos(deg_val);
        
        if (deg_val == 0)
        {
            path_w_grid.startNewSubPath(xpos, degtoypos(90));
            path_w_grid.lineTo(xpos, degtoypos(-90));
        }
        else
        {
            path_grid.startNewSubPath(xpos, degtoypos(90));
            path_grid.lineTo(xpos, degtoypos(-90));
        }
        
    }
    
    
}

int PanningGraph::degtoypos (float deg)
{
    float height = (float) getHeight()-tymargin-bymargin;
    
    return tymargin+height*(-deg+90)/180;
}

float PanningGraph::ypostodeg (int ypos)
{
    float height = (float) getHeight()-tymargin-bymargin;
    
    return 90-(ypos-tymargin)/height*180;
}

int PanningGraph::degtoxpos (float deg)
{
    float width = (float) getWidth()-lxmargin-rxmargin;
    
    return lxmargin + width*(deg+180)/360;
}

float PanningGraph::xpostodeg (int xpos)
{
    float width = (float) getWidth()-lxmargin-rxmargin;
    
    return (xpos-lxmargin)/width*360-180;
}

void PanningGraph::setFilter(int idx, float az, float el, bool shape, float width, float height, float gain, bool solo)
{
    // restrict the angles
    if (el > 90.f)
    {
        el = 180.f-el;
        az += 180.f;
    }
    
    if (el < -90.f)
    {
        el = 180.f+el;
        az += 180.f;
    }
    
    if (az > 180.f)
        az -= 360.f;
    
    if (az < -180.f)
        az += 360.f;
    
    // set the drag button
    btn_drag.getUnchecked(idx)->setBounds (degtoxpos(az)-8, degtoypos(el)-8, 16, 16);
    
    // set the label
    lbl_drag.getUnchecked(idx)->setBounds (degtoxpos(az)-12, degtoypos(el)-12, 26, 24);
    
    // create the path for the area display
    
    Path filterarea;
    filterarea.clear();
    
    
    if (!shape) {
        // circle
        int x = degtoxpos(az);
        int y = degtoypos(el);
        int w = degtoxpos(width)-degtoxpos(0.f);
        int h = degtoypos(width)-degtoypos(0.f);
        
        filterarea.addEllipse(x-w, y-h, 2*w, 2*h);
        
        // check if area goes beyond +-180
        // wrapping about the poles is not done yet!
        if (az+width > 180)
        {
            int x = degtoxpos(az-360);
            int y = degtoypos(el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(width)-degtoypos(0.f);
            
            filterarea.addEllipse(x-w, y-h, 2*w, 2*h);
        }
        if (az-width < -180)
        {
            int x = degtoxpos(az+360);
            int y = degtoypos(el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(width)-degtoypos(0.f);
            
            filterarea.addEllipse(x-w, y-h, 2*w, 2*h);
        }
        if (el+height > 90)
        {
            int newaz = az-180;
            if (newaz < -180)
                newaz+=360;
            int x = degtoxpos(newaz);
            int y = degtoypos(180-el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(width)-degtoypos(0.f);
            
            filterarea.addEllipse(x-w, y-h, 2*w, 2*h);
        }
        if (el-height < -90)
        {
            int newaz = az-180;
            if (newaz < -180)
                newaz+=360;
            int x = degtoxpos(newaz);
            int y = degtoypos(-180-el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(width)-degtoypos(0.f);
            
            filterarea.addEllipse(x-w, y-h, 2*w, 2*h);
        }
        
    } else {
        // rectangle
        int x = degtoxpos(az);
        int y = degtoypos(el);
        int w = degtoxpos(width)-degtoxpos(0.f);
        int h = degtoypos(height)-degtoypos(0.f);
        
        filterarea.addRectangle(x-w, y-h, 2*w, 2*h);
        
        
        // check if area goes beyond +-180
        // wrapping about the poles is not done yet!
        if (az+width > 180)
        {
            int x = degtoxpos(az-360);
            int y = degtoypos(el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(height)-degtoypos(0.f);
            
            filterarea.addRectangle(x-w, y-h, 2*w, 2*h);
        }
        if (az-width < -180)
        {
            int x = degtoxpos(az+360);
            int y = degtoypos(el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(height)-degtoypos(0.f);
            
            filterarea.addRectangle(x-w, y-h, 2*w, 2*h);
        }
        
        if (el+height > 90)
        {
            int newaz = az-180;
            if (newaz < -180)
                newaz+=360;
            int x = degtoxpos(newaz);
            int y = degtoypos(180-el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(height)-degtoypos(0.f);
            
            filterarea.addRectangle(x-w, y-h, 2*w, 2*h);
        }
        if (el-height < -90)
        {
            int newaz = az-180;
            if (newaz < -180)
                newaz+=360;
            int x = degtoxpos(newaz);
            int y = degtoypos(-180-el);
            int w = degtoxpos(width)-degtoxpos(0.f);
            int h = degtoypos(height)-degtoypos(0.f);
            
            filterarea.addRectangle(x-w, y-h, 2*w, 2*h);
        }
        
        
    }
    
    
    // filterarea.lineTo(100, 100);
    
    // filterarea
    //if (idx < graphs_.size())
    //{
        graphs_.getUnchecked(idx)->setPath(&filterarea, gain, solo, one_filter_solo_);
        graphs_.getUnchecked(idx)->repaint();
    //}
    
}

void PanningGraph::setOneFilterSolo(bool is_one_solo)
{
    one_filter_solo_ = is_one_solo;
}

void PanningGraph::buttonClicked (Button* buttonThatWasClicked)
{
    float az =  xpostodeg(buttonThatWasClicked->getPosition().getX() + buttonThatWasClicked->getMouseXYRelative().getX());
    
    az = jlimit(-180.f, 180.f, az);
    
    float el = ypostodeg(buttonThatWasClicked->getPosition().getY() + buttonThatWasClicked->getMouseXYRelative().getY());
    
    el = jlimit(-90.f, 90.f, el);
    
    int i = buttonThatWasClicked->getName().getIntValue();
    
    if (i != mouse_near_filter_id)
    {
        mouse_near_filter_id = i;
        sendChangeMessage();
    }
    
    
    myprocessor_->setParameterNotifyingHost(PARAMS_PER_FILTER*i+Ambix_directional_loudnessAudioProcessor::AzimuthParam, Deg360ToParam(az) );
    
    myprocessor_->setParameterNotifyingHost(PARAMS_PER_FILTER*i+Ambix_directional_loudnessAudioProcessor::ElevationParam, Deg360ToParam(el) );
    
}

void PanningGraph::mouseDown(const MouseEvent &event)
{
    // check if you are near a filter...
    // now the distance is static... could be more intuitive
    
    int numfilters = btn_drag.size();
    
    for (int i = 0; i < numfilters; i++) {
        if (event.getMouseDownPosition().getDistanceFrom(btn_drag.getUnchecked(i)->getPosition()) < 80)
        {
            if (i != mouse_near_filter_id)
            {
                mouse_near_filter_id = i;
                sendChangeMessage();
            }
            
            int w_idx = PARAMS_PER_FILTER*mouse_near_filter_id+Ambix_directional_loudnessAudioProcessor::WidthParam;
            int h_idx = PARAMS_PER_FILTER*mouse_near_filter_id+Ambix_directional_loudnessAudioProcessor::HeightParam;
            
            mouse_down_width = ParamToDeg360(myprocessor_->getParameter(w_idx));
            mouse_down_height = ParamToDeg180(myprocessor_->getParameter(h_idx));
            
            mouse_dir_w = btn_drag.getUnchecked(i)->getX() < event.getMouseDownX() ? 1 : -1;
            mouse_dir_h = btn_drag.getUnchecked(i)->getY() > event.getMouseDownY() ? 1 : -1;
            break;
        }
        
    }
}

void PanningGraph::mouseUp(const MouseEvent &event)
{
    mouse_near_filter_id = -1;
}

// use drag to change width/height
void PanningGraph::mouseDrag(const MouseEvent &event)
{
    // std::cout << "dragging x: " << event.getDistanceFromDragStartX() << " y: " << event.getDistanceFromDragStartY() << std::endl;
    
    if (mouse_near_filter_id > -1)
    {
        int w_idx = PARAMS_PER_FILTER*mouse_near_filter_id+Ambix_directional_loudnessAudioProcessor::WidthParam;
        
        
        myprocessor_->setParameterNotifyingHost(w_idx, (float)jlimit(0.f, 1.f, Deg360ToParam( mouse_down_width + xpostodeg(mouse_dir_w*event.getDistanceFromDragStartX()+degtoxpos(0))  ) ) );
        
        int h_idx = PARAMS_PER_FILTER*mouse_near_filter_id+Ambix_directional_loudnessAudioProcessor::HeightParam;
        
        myprocessor_->setParameterNotifyingHost(h_idx, (float)jlimit(0.f, 1.f, Deg180ToParam( mouse_down_height + ypostodeg(mouse_dir_h*event.getDistanceFromDragStartY()+degtoypos(0))  ) ) );
    }
    
}

void PanningGraph::mouseWheelMove (const MouseEvent &event, const MouseWheelDetails &wheel)
{
    
    int idx = -1;
    
    for (int i=0; i<btn_drag.size(); i++)
    {
        if (btn_drag.getUnchecked(i)->getState() == 1)
            idx = i;
        
    }
    
    if (idx > -1)
    {
        int paridx = PARAMS_PER_FILTER*idx+Ambix_directional_loudnessAudioProcessor::GainParam;
    
        myprocessor_->setParameterNotifyingHost(paridx, (float)jlimit(0.f, 1.f, myprocessor_->getParameter(paridx) +wheel.deltaY*0.4f) );
        
        if (idx != mouse_near_filter_id)
        {
            mouse_near_filter_id = idx;
            sendChangeMessage();
        }
    }
    
}

int PanningGraph::getCurrentId()
{
    return mouse_near_filter_id;
}

// JUCER_RESOURCE: drag_off_png, 903, "drag_off.png"
static const unsigned char resource_FilterGraph_drag_off_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,31,0,0,0,31,8,6,0,0,1,104,169,38,175,0,0,0,9,112,72,89,115,0,
    0,17,141,0,0,17,141,1,26,43,169,153,0,0,3,57,73,68,65,84,120,218,189,87,205,111,76,81,20,127,211,94,157,81,31,19,70,75,136,38,118,54,172,172,37,18,177,242,81,98,131,77,117,49,66,66,8,18,218,68,137,175,
    96,65,124,196,103,106,65,125,68,50,58,17,75,44,252,7,42,93,10,42,40,90,25,58,163,58,211,142,243,155,252,78,221,60,111,94,223,155,118,252,146,95,59,239,190,123,207,61,239,220,223,57,231,61,83,44,22,29,
    226,164,176,221,56,127,209,142,63,24,56,37,108,211,81,99,95,148,6,104,99,161,240,163,206,24,22,198,56,161,104,172,11,32,130,1,172,137,112,32,99,44,63,134,93,179,139,186,66,55,176,111,58,180,178,213,232,
    238,101,208,106,92,38,183,9,239,9,159,9,87,8,227,182,15,192,97,97,175,48,109,63,185,99,121,126,154,215,8,207,43,225,3,76,120,110,61,150,2,225,154,167,22,86,150,113,240,155,112,20,19,134,124,158,162,20,
    135,184,207,132,62,76,152,89,230,102,41,178,152,144,21,174,19,62,98,52,23,11,223,171,227,238,56,56,124,170,85,194,49,90,201,11,103,9,107,224,52,93,202,186,227,164,177,122,43,92,226,76,12,204,93,47,124,
    226,14,116,80,68,232,221,69,213,104,206,9,143,152,158,67,175,135,212,130,96,17,226,4,3,221,150,62,218,66,24,248,80,82,35,47,106,133,59,25,139,6,202,164,28,6,133,115,198,143,209,186,113,149,4,206,10,247,
    91,1,27,101,226,44,19,206,245,202,107,47,108,100,210,100,132,157,94,94,25,215,53,44,127,165,96,32,162,31,194,58,230,80,13,147,172,217,107,241,17,225,49,225,33,225,25,15,79,48,247,151,173,21,93,220,194,
    133,126,2,42,8,167,9,191,216,101,6,184,237,147,69,110,52,210,80,10,139,111,82,117,217,144,138,203,99,113,43,19,32,12,10,170,54,60,231,211,10,228,58,190,184,18,140,24,42,104,54,207,52,12,162,134,154,133,
    130,54,135,92,92,107,168,217,207,33,23,94,67,208,176,184,159,71,5,3,11,2,46,222,161,110,3,51,168,154,204,4,53,89,171,235,26,13,152,93,175,178,52,130,163,75,90,61,108,187,240,134,213,50,94,122,101,21,60,
    72,8,123,132,239,88,44,52,103,79,8,59,130,230,179,13,180,217,163,60,145,56,13,214,186,230,140,210,251,1,225,125,150,184,126,63,163,198,231,30,50,245,58,235,129,195,144,60,22,62,20,190,240,200,37,104,109,
    53,122,62,67,182,135,227,57,142,165,131,108,190,87,120,158,191,223,8,151,7,76,90,136,60,69,42,18,108,114,90,240,119,11,47,123,109,158,224,1,215,177,26,54,58,147,199,0,91,177,67,219,151,88,40,209,44,114,
    198,234,73,125,60,179,114,149,116,178,128,110,206,9,15,8,127,226,225,116,243,30,110,124,167,74,27,43,14,10,151,10,215,10,95,99,243,77,20,203,24,123,104,181,145,100,148,27,12,219,35,210,230,123,200,110,
    80,41,62,241,229,167,222,176,36,33,111,163,206,255,67,84,95,245,59,89,16,98,60,130,84,149,55,110,209,202,103,152,86,87,152,131,93,76,141,66,149,54,198,3,222,162,184,143,171,218,81,141,154,132,27,120,30,
    243,153,163,83,137,38,22,156,8,163,221,97,23,153,102,110,222,205,247,178,65,22,158,169,192,16,155,142,227,215,149,210,244,12,95,88,119,41,68,28,193,5,190,7,231,3,110,54,157,37,58,105,141,109,193,119,86,
    144,198,210,69,214,179,240,236,99,101,210,239,138,49,254,255,77,103,53,83,106,172,183,161,17,134,119,23,127,255,131,63,229,47,212,182,38,118,159,122,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* PanningGraph::drag_off_png = (const char*) resource_FilterGraph_drag_off_png;
const int PanningGraph::drag_off_pngSize = 903;

// JUCER_RESOURCE: drag_on_png, 1172, "drag_on.png"
static const unsigned char resource_FilterGraph_drag_on_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,31,0,0,0,31,8,6,0,0,1,104,169,38,175,0,0,0,9,112,72,89,115,0,0,
    17,141,0,0,17,141,1,26,43,169,153,0,0,4,70,73,68,65,84,120,218,181,87,75,104,156,85,20,62,231,159,191,157,48,153,52,154,113,180,166,165,150,10,69,132,236,20,5,193,93,21,55,73,42,105,67,226,66,68,177,180,
    144,108,18,10,137,144,250,168,138,134,118,211,16,37,98,81,90,169,148,210,38,233,162,224,86,80,119,34,106,45,210,46,74,147,210,188,52,111,19,51,153,227,119,238,189,127,250,103,94,157,63,38,7,206,63,247,
    117,238,121,159,115,199,23,17,114,240,33,240,29,223,142,147,193,106,183,46,124,68,33,208,133,110,96,215,218,130,187,163,22,120,215,158,144,101,33,78,217,109,153,195,66,48,81,224,42,61,49,103,6,22,230,
    103,253,53,57,194,164,142,156,152,217,183,12,146,163,196,180,30,204,45,77,226,7,220,11,67,203,63,78,179,121,38,229,197,111,96,124,9,120,101,145,232,64,6,131,106,63,100,11,114,10,94,7,14,133,53,39,75,93,
    21,58,119,28,216,211,130,207,183,80,107,225,39,226,199,114,120,127,10,252,250,2,209,45,28,160,248,115,133,5,28,211,207,42,44,53,67,37,0,118,144,26,202,179,193,125,184,131,27,56,169,122,230,109,201,116,
    22,132,79,168,22,11,216,175,135,169,135,173,169,247,97,243,23,53,115,44,236,207,16,229,194,143,68,137,231,33,31,38,203,56,8,148,106,140,61,35,52,8,171,237,165,97,59,57,82,162,63,64,240,108,1,85,214,70,
    49,35,174,117,116,61,198,87,139,24,250,1,160,103,101,10,42,197,123,93,140,46,82,100,48,246,152,239,240,173,239,227,209,47,160,189,250,201,234,5,131,80,45,139,248,240,108,8,148,9,242,171,218,38,101,109,
    96,92,210,115,20,216,79,244,100,16,38,69,8,71,151,136,171,43,64,204,185,94,248,204,226,173,32,26,59,66,62,88,117,137,83,135,217,236,186,68,207,139,131,251,112,16,248,12,80,131,245,44,112,50,247,128,159,
    51,175,33,89,157,178,1,3,147,240,95,16,183,18,227,237,159,24,247,51,107,146,53,230,19,203,124,47,81,101,39,177,38,99,127,129,224,193,81,153,104,32,222,38,193,170,11,158,204,91,196,201,78,84,140,18,166,
    70,218,243,195,248,253,19,231,31,151,160,204,104,68,126,65,180,171,76,127,237,7,78,43,223,203,200,205,149,175,76,130,208,76,132,136,123,4,220,255,62,8,206,222,235,182,44,69,129,140,81,214,183,150,189,
    182,129,112,205,228,185,42,2,172,40,231,44,76,159,130,233,167,34,18,199,213,218,30,34,231,100,154,232,104,68,98,143,181,208,213,145,180,222,35,142,66,124,26,73,194,190,111,83,201,67,133,184,158,32,122,
    186,76,226,183,43,84,238,32,53,17,192,123,132,100,4,227,221,165,11,130,205,233,151,48,249,215,95,87,2,249,161,5,20,190,4,241,69,76,223,7,222,14,250,21,136,62,71,72,198,52,155,95,196,194,247,133,178,170,
    18,82,160,96,53,227,250,230,180,171,164,226,174,62,137,239,137,114,243,57,12,181,8,99,16,103,155,64,226,202,175,151,115,68,93,110,214,38,33,192,5,247,66,24,43,117,105,241,32,147,204,155,248,14,64,120,
    199,69,27,140,222,249,29,240,135,2,185,164,177,246,2,126,95,73,147,28,106,135,63,218,221,198,34,132,105,13,55,236,226,204,101,182,135,120,199,123,182,248,252,142,133,151,203,76,90,13,242,97,139,107,97,
    135,126,47,63,35,138,118,12,186,133,54,96,95,33,230,41,104,59,78,84,229,153,146,203,251,233,255,3,172,206,181,110,140,48,144,189,103,96,5,84,85,210,120,88,12,152,239,66,249,29,177,218,30,119,253,96,179,
    161,14,119,163,231,74,23,162,153,209,203,233,81,87,137,151,111,160,206,99,115,96,139,24,7,128,24,100,180,72,57,236,65,128,223,148,249,171,176,126,210,52,28,147,91,91,13,202,163,73,179,48,237,219,246,8,
    55,243,100,180,110,176,97,208,194,177,4,77,43,61,223,114,196,88,18,165,158,138,155,12,113,118,157,72,31,4,222,199,118,161,222,165,203,86,66,139,43,124,54,213,38,224,252,62,146,88,27,9,228,224,157,65,143,
    220,2,72,184,26,205,170,249,7,65,170,181,99,97,15,73,188,1,15,19,8,240,212,131,42,227,198,158,152,166,163,24,198,250,252,58,17,46,50,141,216,0,115,127,144,228,38,14,161,222,152,151,227,38,128,140,227,
    173,159,136,185,152,42,218,149,134,156,100,175,161,52,158,55,239,122,125,100,178,62,60,79,217,250,94,22,232,203,237,221,37,146,35,120,210,50,204,108,58,142,249,159,85,78,99,249,198,32,51,156,228,159,195,
    191,1,88,164,43,22,234,96,174,139,173,184,150,184,157,109,170,120,236,154,187,109,123,76,95,226,247,152,54,238,66,76,254,3,189,72,60,250,162,136,174,242,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* PanningGraph::drag_on_png = (const char*) resource_FilterGraph_drag_on_png;
const int PanningGraph::drag_on_pngSize = 1172;

// JUCER_RESOURCE: drag_over_png, 693, "drag_over.png"
static const unsigned char resource_FilterGraph_drag_over_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,31,0,0,0,31,8,6,0,0,1,104,169,38,175,0,0,0,9,112,72,89,115,0,
    0,17,141,0,0,17,141,1,26,43,169,153,0,0,2,103,73,68,65,84,120,218,189,87,207,75,84,81,20,190,175,185,80,76,133,144,216,194,160,254,2,119,65,173,90,182,44,141,54,234,162,69,110,10,108,99,155,12,146,168,
    148,114,169,20,20,133,144,98,171,208,252,19,92,216,62,145,246,253,32,75,23,253,178,8,199,219,247,13,231,200,157,231,157,55,239,221,169,247,193,199,48,119,238,185,231,220,243,243,142,117,206,25,193,61,
    240,166,149,47,186,58,202,133,113,227,129,11,163,224,141,221,5,57,163,27,252,168,59,92,90,196,100,45,124,179,158,29,206,236,69,98,69,193,7,19,134,179,170,189,9,126,169,206,36,165,98,11,220,6,59,124,27,
    140,92,112,13,92,76,91,29,50,176,31,124,193,13,175,155,232,159,215,13,167,50,140,172,89,147,141,164,213,134,119,220,112,8,252,17,248,113,7,60,193,13,63,193,115,224,171,212,134,138,31,79,31,43,224,233,
    102,70,211,121,114,232,158,232,58,211,26,21,207,92,90,181,100,11,8,167,193,43,77,106,142,198,98,196,74,236,99,177,195,3,22,196,229,251,34,14,232,180,158,115,174,128,15,115,8,253,6,15,72,138,55,68,225,
    145,144,120,192,251,121,181,80,147,194,233,97,29,54,20,105,32,15,20,125,224,73,240,43,248,12,220,104,85,225,71,192,205,192,65,247,229,147,69,214,27,18,158,4,175,183,184,247,121,201,149,134,123,15,229,
    16,52,169,76,221,45,175,39,17,33,123,73,225,153,200,132,233,163,240,165,72,97,103,219,72,85,211,142,176,179,190,235,11,162,238,109,102,78,87,172,48,115,246,83,65,65,22,137,165,240,186,52,255,106,1,97,
    86,215,126,117,216,193,130,45,234,44,248,199,247,118,34,93,179,154,149,146,224,25,112,57,20,42,90,208,9,190,17,39,86,60,139,238,130,99,121,235,217,71,183,8,95,4,15,103,88,102,36,122,243,242,66,88,143,
    77,178,203,224,227,2,125,82,115,141,55,190,38,212,73,62,224,15,236,44,229,183,192,219,230,223,161,42,13,159,24,6,167,67,202,25,171,207,145,19,33,47,166,164,171,210,59,91,170,252,24,248,222,148,3,122,226,
    59,120,84,149,191,53,229,130,222,93,165,242,11,242,22,42,27,93,86,198,99,217,168,191,54,172,204,229,178,145,232,60,224,131,96,34,114,38,180,5,42,255,34,181,55,92,146,78,237,209,119,52,219,217,141,142,
    203,163,164,12,151,211,219,99,126,147,233,21,229,11,255,65,97,77,255,36,100,77,165,69,177,108,16,156,109,83,161,62,105,157,212,117,253,127,86,158,193,50,39,100,55,122,46,30,169,4,98,231,82,67,37,73,53,
    146,167,224,85,14,238,144,146,191,151,1,140,114,231,143,189,71,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

const char* PanningGraph::drag_over_png = (const char*) resource_FilterGraph_drag_over_png;
const int PanningGraph::drag_over_pngSize = 693;

