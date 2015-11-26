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

#ifndef __ambix_directional_loudness__PanningGraph__
#define __ambix_directional_loudness__PanningGraph__

#include "JuceHeader.h"

#include "PluginProcessor.h"

// this is a class which draws the overlays...
// this avoids redrawing stuff which is fixed!
class GraphComponent : public Component
{
public:
    GraphComponent(int id)
    {
        id_ = id;
        setWantsKeyboardFocus(false);
        setInterceptsMouseClicks(false, false);
    };
    
    ~GraphComponent(){};
    
    void paint (Graphics& g)
    {
        // gain_ : -99 ... 20
        
        Colour fillColour;
        
        if (is_one_solo_ && !solo_)
        {
            // transparent if not soloed
            fillColour = Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.f);
        } else {
            if (gain_ < 0.f)
            {
                float alpha=-gain_/99.f;
                fillColour = Colour::fromFloatRGBA(0.f, 0.f, 1.f, sqrt(alpha));
            }
            else
            {
                float alpha=gain_/20.f;
                fillColour = Colour::fromFloatRGBA(1.f, 0.f, 0.f, sqrt(alpha));
            }
        }
        
        
        Colour traceColour;
        
        if (solo_)
            traceColour = Colour::fromFloatRGBA(1.f, 1.f, 0.f, 1.f);
        else
            traceColour = Colour::fromFloatRGBA(0.f, 0.f, 0.f, 0.6f);
         
        g.setColour (fillColour);
        //fill
        g.fillPath(path_mag_);
        
        g.setColour (traceColour);
        // and border if soloed or non soloed
        if ((is_one_solo_ && solo_) || (!is_one_solo_))
            g.strokePath (path_mag_, PathStrokeType (2.0f));
    };
    void resized(){};
    
    void setPath(Path* newpath, float gain, bool solo, bool is_one_solo)
    {
        path_mag_ = *newpath;
        gain_ = gain;
        solo_ = solo;
        is_one_solo_ = is_one_solo;
    };
    
private:
    Path path_mag_;
    float gain_;
    int id_;
    bool solo_;
    bool is_one_solo_;
};

class PanningGraph    :  public Component,
public SettableTooltipClient,
public ButtonListener
{
    friend class GraphComponent;
public:
    PanningGraph (AudioProcessor* processor);
    ~PanningGraph();
    
    void paint (Graphics& g);
    void resized();
    
    void setFilter(int idx, float az, float el, bool shape, float width, float height, float gain, bool solo);
    
    void setOneFilterSolo(bool is_one_solo);
    
    void buttonClicked (Button* buttonThatWasClicked);
    
    void mouseDown (const MouseEvent &event);
    void mouseUp (const MouseEvent &event);
    
    void mouseDrag	(const MouseEvent &event);
    void mouseWheelMove (const MouseEvent &event, const MouseWheelDetails &wheel);
    
    // Binary resources:
    static const char* drag_off_png;
    static const int drag_off_pngSize;
    static const char* drag_on_png;
    static const int drag_on_pngSize;
    static const char* drag_over_png;
    static const int drag_over_pngSize;
    
private:
    
    int degtoypos (float deg);
    
    float ypostodeg (int ypos);
    
    int degtoxpos (float deg);
    
    float xpostodeg (int xpos);
    
    OwnedArray<ImageButton> btn_drag;
    
    OwnedArray<GraphComponent> graphs_;
    
    float lxmargin; // space for y labels
    float rxmargin;
    float tymargin;
    float bymargin;
    
    bool one_filter_solo_; // true if one filter is on solo
    
    Path path_grid, path_w_grid;
    
    AudioProcessor* myprocessor_; // this is used for changing parameters
    
    TooltipWindow tooltipWindow;
    
    int mouse_near_filter_id;
    float mouse_down_width;
    float mouse_down_height;
    
    int mouse_dir_w;
    int mouse_dir_h;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanningGraph)
};


#endif /* defined(__ambix_directional_loudness__PanningGraph__) */
