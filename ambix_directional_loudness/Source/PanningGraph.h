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
#include "Colormap.h"

// Hover-aware Label subclass — used for the per-filter gain readout so it
// stays visible as long as the mouse is over the label itself, not just
// while the cursor is on the puck.
class GainHoverLabel : public juce::Label
{
public:
    using juce::Label::Label;
    std::function<void(bool)> onHoverChanged;
    void mouseEnter (const juce::MouseEvent& e) override
    {
        juce::Label::mouseEnter (e);
        if (onHoverChanged) onHoverChanged (true);
    }
    void mouseExit (const juce::MouseEvent& e) override
    {
        juce::Label::mouseExit (e);
        if (onHoverChanged) onHoverChanged (false);
    }
};

// ImageButton subclass that exposes double-click + hover hooks — used for
// the per-filter drag pucks so a double-click can reset that filter's gain
// and hovering can show the floating gain readout.
class FilterPuckButton : public juce::ImageButton
{
public:
    using juce::ImageButton::ImageButton;
    std::function<void()>      onDoubleClick;
    std::function<void(bool)>  onHoverChanged;

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        if (onDoubleClick) onDoubleClick();
    }
    void mouseEnter (const juce::MouseEvent& e) override
    {
        juce::ImageButton::mouseEnter (e);
        if (onHoverChanged) onHoverChanged (true);
    }
    void mouseExit (const juce::MouseEvent& e) override
    {
        juce::ImageButton::mouseExit (e);
        if (onHoverChanged) onHoverChanged (false);
    }
};

// Filter overlay component (single shape on top of the heatmap/grid). Two
// rendering modes:
//  - Path mode  (equirect): vector path with fill + stroke. Wrap-around at
//    az=±180° is handled by the path-builder duplicating shapes ±360°.
//  - Mask mode  (HA): an alpha-mask image rasterized per-pixel by the
//    PanningGraph's spherical inside-test. Avoids all path-topology issues
//    when filters cross the seam or wrap over a pole.
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
        Colour fillColour;

        // When the energy visualizer is on we want the heatmap to stay
        // unobstructed — drop the gain-tinted fill entirely (outline only).
        // Otherwise leave some transparency at max gain so the panel
        // background stays partially visible.
        constexpr float kMaxFillAlpha = 0.65f;

        if (suppress_fill_ || (is_one_solo_ && !solo_))
        {
            fillColour = Colour::fromFloatRGBA(1.f, 1.f, 1.f, 0.f);
        } else {
            if (gain_ < 0.f)
            {
                float alpha = -gain_ / 99.f;
                fillColour = Colour::fromFloatRGBA(0.f, 0.f, 1.f, std::sqrt (alpha) * kMaxFillAlpha);
            }
            else
            {
                float alpha = gain_ / 20.f;
                fillColour = Colour::fromFloatRGBA(1.f, 0.f, 0.f, std::sqrt (alpha) * kMaxFillAlpha);
            }
        }


        Colour traceColour;

        if (solo_)
            traceColour = Colour::fromFloatRGBA(1.f, 1.f, 0.f, 1.f);
        else
            traceColour = Colour::fromFloatRGBA(0.f, 0.f, 0.f, 0.6f);

        if (has_mask_ && filter_mask_.isValid())
        {
            // Mask mode (HA): clip to the chart ellipse so the rasterized
            // stairsteps at the chart boundary are masked into a smooth curve.
            Graphics::ScopedSaveState ss (g);
            Path ellipse;
            ellipse.addEllipse (mask_target_.toFloat());
            g.reduceClipRegion (ellipse);

            g.setColour (fillColour);
            g.drawImage (filter_mask_,
                         mask_target_.getX(),     mask_target_.getY(),
                         mask_target_.getWidth(), mask_target_.getHeight(),
                         0, 0, filter_mask_.getWidth(), filter_mask_.getHeight(),
                         true /* fillAlphaChannelWithCurrentBrush */);

            if (stroke_mask_.isValid()
                && ((is_one_solo_ && solo_) || (! is_one_solo_)))
            {
                g.setColour (traceColour);
                g.drawImage (stroke_mask_,
                             mask_target_.getX(),     mask_target_.getY(),
                             mask_target_.getWidth(), mask_target_.getHeight(),
                             0, 0, stroke_mask_.getWidth(), stroke_mask_.getHeight(),
                             true /* fillAlphaChannelWithCurrentBrush */);
            }
            return;
        }

        g.setColour (fillColour);
        g.fillPath(path_mag_);

        g.setColour (traceColour);
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
        has_mask_ = false;
        filter_mask_ = Image();
        stroke_mask_ = Image();
    };

    void setMask(Image mask, Image stroke, Rectangle<int> targetRect,
                 float gain, bool solo, bool is_one_solo)
    {
        filter_mask_ = std::move (mask);
        stroke_mask_ = std::move (stroke);
        mask_target_ = targetRect;
        has_mask_ = true;
        gain_ = gain;
        solo_ = solo;
        is_one_solo_ = is_one_solo;
    }

    void setSuppressFill (bool suppress)
    {
        if (suppress_fill_ == suppress) return;
        suppress_fill_ = suppress;
        repaint();
    }

private:
    Path path_mag_;
    Image filter_mask_;
    Image stroke_mask_;
    Rectangle<int> mask_target_;
    bool has_mask_ = false;
    bool suppress_fill_ = false;
    float gain_;
    int id_;
    bool solo_;
    bool is_one_solo_;
};

class PanningGraph    :  public Component,
public SettableTooltipClient,
public Button::Listener,
public ChangeBroadcaster,
private juce::Timer
{
    friend class GraphComponent;
public:
    enum class Projection { Equirect, HammerAitoff };

    PanningGraph (AudioProcessor* processor);
    ~PanningGraph();

    void paint (Graphics& g) override;
    void resized() override;

    void setFilter(int idx, float az, float el, bool shape, float width, float height, float gain, bool solo);

    void setOneFilterSolo(bool is_one_solo);

    void buttonClicked (Button* buttonThatWasClicked) override;

    void mouseDown (const MouseEvent &event) override;
    void mouseUp (const MouseEvent &event) override;

    void mouseDrag	(const MouseEvent &event) override;
    void mouseWheelMove (const MouseEvent &event, const MouseWheelDetails &wheel) override;

    int getCurrentId();

    // --- new view-mode controls ---

    void setProjection (Projection p);
    Projection getProjection() const { return projection_; }

    void setEnergyEnabled (bool on);
    bool getEnergyEnabled() const { return energy_enabled_; }

    void setColormap (Colormap::Map m);
    Colormap::Map getColormap() const { return colormap_; }

    // Push the active mesh (cartesian unit directions + triangle indices).
    // Called by the editor whenever the projection changes — HA uses the IEM
    // 426-point Hammer-Aitoff Delaunay; equirect uses a regular Naz×Nel grid.
    void setMesh (std::vector<Eigen::Vector3d> dirs, std::vector<int> indices);

    // Legacy alias preserved for the existing call sites; assumes the IEM HA
    // mesh indices.
    void setGridDirections (const std::vector<Eigen::Vector3d>& cartesianUnitDirs);

    // Called from the editor's timer with normalized [0,1] colormap inputs
    // (already auto-normalized or floor-mapped upstream). Size must equal
    // the grid set via setGridDirections().
    void setEnergyT (const std::vector<float>& tValues);

    // Binary resources:
    static const char* drag_off_png;
    static const int drag_off_pngSize;
    static const char* drag_on_png;
    static const int drag_on_pngSize;
    static const char* drag_over_png;
    static const int drag_over_pngSize;

private:
    // Cached filter parameters so we can rebuild paths on projection change.
    struct FilterState {
        float az = 0.f, el = 0.f;
        bool shape = false;
        float width = 0.f, height = 0.f, gain = 0.f;
        bool solo = false;
        bool valid = false;
    };
    std::array<FilterState, NUM_FILTERS> filter_states_;

    // Projection helpers — return screen-space coordinates within the
    // *panel* (PanningGraph component) coordinate system. For equirect they
    // map directly into the rounded-rect drawable area. For HA they map into
    // a 2:1 letterboxed area centered inside the drawable area.
    Point<float> sphericalToScreen (float azDeg, float elDeg) const;
    bool screenToSpherical (Point<float> px, float& azDeg, float& elDeg) const;

    // Equirect-only helpers retained for mouse-drag arithmetic that needs
    // simple linear deltas (the existing width/height drag math). For HA we
    // fall back to a coarse approximation using the projection's local scale.
    int   degtoypos (float deg) const;
    float ypostodeg (int ypos) const;
    int   degtoxpos (float deg) const;
    float xpostodeg (int xpos) const;

    void rebuildGridLines();
    void rebuildAllFilterPaths();
    void rebuildFilterPath (int idx);
    void rebuildHeatmapMesh();
    void renderHeatmapImage();
    void rebuildPixelCartCache();
    void rasterizeFilterMaskHA (int idx);

    // Returns the rectangle (in panel coords) where the projection is drawn.
    // Equirect: full rounded-rect drawable area. HA: largest 2:1 box centered
    // in that area.
    Rectangle<int> getProjectionBounds() const;

    OwnedArray<FilterPuckButton> btn_drag;
    OwnedArray<Label>            lbl_drag;
    OwnedArray<GainHoverLabel> lbl_gain_db_;     // transient gain readout per filter
    OwnedArray<GraphComponent> graphs_;

    // Per-filter cached gain (NaN until first setFilter) and the timestamp
    // until which the floating dB label should remain visible (with linear
    // alpha fade over the last `kGainLabelFadeMs` of that window).
    std::array<float,        NUM_FILTERS> last_gain_db_   {};
    std::array<juce::int64,  NUM_FILTERS> gain_label_until_ms_ {};

    static constexpr int kGainLabelHoldMs = 800;
    static constexpr int kGainLabelFadeMs = 400;

    void positionGainLabel (int idx);
    void showGainLabel (int idx, bool sticky); // sticky=true → hold until cleared
    void timerCallback() override;

    float lxmargin;
    float rxmargin;
    float tymargin;
    float bymargin;

    bool one_filter_solo_;

    Path path_grid, path_w_grid;

    AudioProcessor* myprocessor_;

    TooltipWindow tooltipWindow;

    int mouse_near_filter_id;
    float mouse_down_width;
    float mouse_down_height;

    int mouse_dir_w;
    int mouse_dir_h;

    // --- new view state ---
    Projection projection_ = Projection::Equirect;
    bool energy_enabled_ = false;
    Colormap::Map colormap_ = Colormap::Map::Jet;

    std::vector<Eigen::Vector3d>    grid_dirs_;       // unit cartesian, from processor
    std::vector<int>                mesh_indices_;    // 3 indices per triangle
    std::vector<float>              grid_t_;          // last colormap-t per grid point
    std::vector<juce::Point<float>> vertex_pixels_;   // HA-mode: grid-point positions in image pixels
    std::vector<float>              vertex_az_deg_;   // equirect-mode: per-vertex az (degrees)
    std::vector<float>              vertex_el_deg_;   // equirect-mode: per-vertex el (degrees)
    Rectangle<int>                  projection_bounds_;
    Image                           heatmap_image_;

    // Per-pixel unit cartesian cache used for rasterizing filter shape masks
    // in HA mode (sized to a low-res grid, scaled at draw time). Pixels
    // outside the chart get the sentinel (2, 0, 0).
    std::vector<juce::Vector3D<float>> pixel_cart_;
    int pixel_cart_W_ = 0, pixel_cart_H_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanningGraph)
};


#endif /* defined(__ambix_directional_loudness__PanningGraph__) */
