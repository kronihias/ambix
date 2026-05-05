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
#include "../../common/JuceCompat.h"
#include "HammerAitov.h"
#include "HammerAitovSample.h"

namespace
{
    // The IEM Hammer-Aitoff `sphericalToXY` we use is normalized so its image
    // is the unit disk (|h|≤1). That means the chart's natural shape is a
    // circle of radius 1 in HA space (NOT the textbook 2:1 ellipse with
    // half-axes √2 / (1/√2)).

    // Build a unit cartesian vector from azimuth/elevation in radians.
    inline void sphToCart (float azRad, float elRad, float& x, float& y, float& z)
    {
        const float cE = std::cos (elRad);
        x = std::cos (azRad) * cE;
        y = std::sin (azRad) * cE;
        z = std::sin (elRad);
    }
}

PanningGraph::PanningGraph (AudioProcessor* processor) :
                            lxmargin(30.f),
                            rxmargin(15.f),
                            tymargin(5.f),
                            bymargin(12.f),
                            one_filter_solo_(false),
                            mouse_near_filter_id(-1)
{
    tooltipWindow.setMillisecondsBeforeTipAppears (200);

    myprocessor_ = processor;

    for (int i=0; i<NUM_FILTERS; i++)
    {
        graphs_.add(new GraphComponent(i));
        addAndMakeVisible(graphs_.getLast());
    }

    for (int i=0; i < NUM_FILTERS; i++)
    {
        btn_drag.add(new FilterPuckButton (String(i)));

        btn_drag.getLast()->addListener(this);
        btn_drag.getLast()->setImages (false, true, true,
                                       ImageCache::getFromMemory (drag_off_png, drag_off_pngSize), 1.000f, Colour (0x00000000),
                                       ImageCache::getFromMemory (drag_over_png, drag_over_pngSize), 1.000f, Colour (0x00000000),
                                       ImageCache::getFromMemory (drag_on_png,  drag_on_pngSize),  1.000f, Colour (0x00000000));
        btn_drag.getLast()->setTriggeredOnMouseDown(true);
        btn_drag.getLast()->setRepeatSpeed(50, 50);

        // Double-click on the puck → reset that filter's gain to 0 dB.
        btn_drag.getLast()->onDoubleClick = [this, i]()
        {
            const int paridx = PARAMS_PER_FILTER * i
                                + Ambix_directional_loudnessAudioProcessor::GainParam;
            setParameterNotifyingHost (myprocessor_, paridx, DbToParam (0.0f));
        };

        // Hovering the puck → show floating gain readout (sticky); on exit,
        // start the normal fade-out.
        btn_drag.getLast()->onHoverChanged = [this, i] (bool hovering)
        {
            if (hovering)
            {
                showGainLabel (i, /*sticky=*/ true);
            }
            else if (i < lbl_gain_db_.size())
            {
                gain_label_until_ms_[(size_t) i]
                    = (juce::int64) juce::Time::getMillisecondCounter()
                        + kGainLabelHoldMs + kGainLabelFadeMs;
                if (! isTimerRunning())
                    startTimerHz (30);
            }
        };

        String label(i+1);
        lbl_drag.add(new Label(label));
        lbl_drag.getLast()->setText(label, dontSendNotification);
        lbl_drag.getLast()->setInterceptsMouseClicks(false, false);
        lbl_drag.getLast()->setFont (Font (FontOptions {15.00f, Font::plain}));
        lbl_drag.getLast()->setJustificationType (Justification::centred);
        lbl_drag.getLast()->setEditable (false, false, false);
        lbl_drag.getLast()->setColour (Label::textColourId, Colours::white);
        lbl_drag.getLast()->setColour (TextEditor::textColourId, Colours::black);
        lbl_drag.getLast()->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    }

    for (int i=NUM_FILTERS-1; i >= 0; i--)
    {
        addAndMakeVisible(btn_drag.getUnchecked(i));
        addAndMakeVisible(lbl_drag.getUnchecked(i));
    }

    // Transient gain-dB labels shown next to the filter puck while the user
    // is actively changing gain. Hidden by default; fades out via timer.
    for (int i = 0; i < NUM_FILTERS; ++i)
    {
        auto* l = new GainHoverLabel ("gainDb_" + String (i + 1));
        l->setFont (Font (FontOptions { 12.0f, Font::bold }));
        l->setJustificationType (Justification::centred);
        l->setColour (Label::textColourId,       Colours::white);
        l->setColour (Label::backgroundColourId, Colour (0xa0000000));
        l->setColour (Label::outlineColourId,    Colour (0x60ffffff));
        l->setEditable (false, false, false);
        // Allow the label itself to receive hover events (children remain
        // unaffected). Clicks reach the label but do nothing — there's no
        // edit mode and no click handler.
        l->setInterceptsMouseClicks (true, false);
        l->setVisible (false);
        l->setBorderSize (BorderSize<int> (1, 4, 1, 4));
        l->onHoverChanged = [this, i] (bool hovering)
        {
            if (i >= lbl_gain_db_.size()) return;
            auto* lab = lbl_gain_db_.getUnchecked (i);
            if (! lab->isVisible()) return;
            if (hovering)
            {
                lab->setAlpha (1.0f);
                gain_label_until_ms_[(size_t) i] = std::numeric_limits<juce::int64>::max();
            }
            else
            {
                gain_label_until_ms_[(size_t) i]
                    = (juce::int64) juce::Time::getMillisecondCounter()
                        + kGainLabelHoldMs + kGainLabelFadeMs;
                if (! isTimerRunning())
                    startTimerHz (30);
            }
        };
        // addChildComponent (not addAndMakeVisible) so the label stays
        // hidden until showGainLabel makes it visible — addAndMakeVisible
        // would have flipped setVisible back to true and we'd see an empty
        // black box at startup.
        addChildComponent (l);
        lbl_gain_db_.add (l);
    }
    last_gain_db_.fill (std::nanf (""));
    gain_label_until_ms_.fill (0);
}

PanningGraph::~PanningGraph()
{
    btn_drag.clear();
    graphs_.clear();
}

// ============================================================================
// Projection helpers
// ============================================================================

Rectangle<int> PanningGraph::getProjectionBounds() const
{
    Rectangle<int> drawable ((int) lxmargin,
                             (int) tymargin,
                             (int) (getWidth()  - lxmargin - rxmargin),
                             (int) (getHeight() - tymargin - bymargin));

    if (projection_ == Projection::Equirect)
        return drawable;

    // HA: canonical 2:1 ellipse letterboxed inside the drawable. Matches IEM
    // EnergyVisualizer — both poles and both seams are visible, with black
    // bars on the long axis when the panel is wider than 2:1.
    const float dw = (float) drawable.getWidth();
    const float dh = (float) drawable.getHeight();
    int w, h;
    if (dw / dh > 2.0f) { h = (int) dh; w = h * 2; }
    else                { w = (int) dw; h = w / 2; }
    const int x = drawable.getX() + (drawable.getWidth()  - w) / 2;
    const int y = drawable.getY() + (drawable.getHeight() - h) / 2;
    return { x, y, w, h };
}

Point<float> PanningGraph::sphericalToScreen (float azDeg, float elDeg) const
{
    const auto rect = getProjectionBounds();

    if (projection_ == Projection::Equirect)
    {
        const float x = (float) rect.getX() + rect.getWidth()  * (azDeg + 180.f) / 360.f;
        const float y = (float) rect.getY() + rect.getHeight() * (-elDeg + 90.f) / 180.f;
        return { x, y };
    }

    // HA: IEM's `sphericalToXY` is normalized to a unit disk; we scale x by 2
    // to recover the canonical 2:1 ellipse (matches IEM EnergyVisualizer's
    // chart aspect). Map ellipse extent [-2,2] × [-1,1] linearly onto the
    // letterboxed 2:1 box returned by getProjectionBounds().
    //
    // The IEM convention puts positive azimuth on the *left* of the chart;
    // we flip x so positive azimuth lands on the *right*, matching the
    // rectangular projection.
    float hx, hy;
    HammerAitov::sphericalToXY (azDeg * MathConstants<float>::pi / 180.f,
                                elDeg * MathConstants<float>::pi / 180.f,
                                hx, hy);
    hx = -hx;
    hx *= 2.f;
    const float u = (hx + 2.f) * 0.25f;
    const float v = (1.f - hy) * 0.5f;
    return { (float) rect.getX() + u * (float) rect.getWidth(),
             (float) rect.getY() + v * (float) rect.getHeight() };
}

bool PanningGraph::screenToSpherical (Point<float> px, float& azDeg, float& elDeg) const
{
    const auto rect = getProjectionBounds();
    if (rect.getWidth() <= 0 || rect.getHeight() <= 0) return false;

    if (projection_ == Projection::Equirect)
    {
        const float u = (px.x - rect.getX()) / (float) rect.getWidth();
        const float v = (px.y - rect.getY()) / (float) rect.getHeight();
        azDeg = jlimit (-180.f, 180.f, u * 360.f - 180.f);
        elDeg = jlimit  (-90.f,  90.f, 90.f - v * 180.f);
        return true;
    }

    const float u = (px.x - rect.getX()) / (float) rect.getWidth();
    const float v = (px.y - rect.getY()) / (float) rect.getHeight();
    const float hx = u * 4.f - 2.f;     // [-2, 2]
    const float hy = 1.f - v * 2.f;     // [-1, 1]

    // Inverse expects the unit-disk normalization, so undo the x-scale.
    // We also flip the sign of hx to match the forward map's flip (positive
    // az → right on screen).
    const float hxUnit = -hx * 0.5f;
    if (! HammerAitov::isInside (hxUnit, hy)) return false;

    float az, el;
    HammerAitov::XYToSpherical (hxUnit, hy, az, el);
    azDeg = az * 180.f / MathConstants<float>::pi;
    elDeg = el * 180.f / MathConstants<float>::pi;
    return true;
}

// Equirect-only screen↔deg helpers (kept for the existing drag-resize math
// where a linear pixel-to-degree mapping is the intended UX in both modes).
int   PanningGraph::degtoypos (float deg) const
{
    float h = (float) getHeight() - tymargin - bymargin;
    return (int) (tymargin + h * (-deg + 90.f) / 180.f);
}
float PanningGraph::ypostodeg (int ypos) const
{
    float h = (float) getHeight() - tymargin - bymargin;
    return 90.f - (ypos - tymargin) / h * 180.f;
}
int   PanningGraph::degtoxpos (float deg) const
{
    float w = (float) getWidth() - lxmargin - rxmargin;
    return (int) (lxmargin + w * (deg + 180.f) / 360.f);
}
float PanningGraph::xpostodeg (int xpos) const
{
    float w = (float) getWidth() - lxmargin - rxmargin;
    return (xpos - lxmargin) / w * 360.f - 180.f;
}

// ============================================================================
// Paint / resize
// ============================================================================

void PanningGraph::paint (Graphics& g)
{
    const int width  = getWidth();
    const int height = getHeight();

    // Panel background. The legacy "solo-active" cue painted a fully
    // transparent rect to dim the panel — but with the heatmap layer on top,
    // that left the visualizer barely visible against the editor's dark
    // backdrop. When the energy heatmap is enabled, always paint the
    // gradient so the heatmap has a consistent surface to alpha-blend over.
    // (Solo state stays visually obvious via the soloed filter's yellow
    // stroke and the highlighted "S" indicator.)
    const bool dimForSolo = one_filter_solo_ && ! energy_enabled_;
    if (dimForSolo)
        g.setColour (Colour (0x00ffffff));
    else
        g.setGradientFill (ColourGradient (Colour (0xff232338), width / 2, height / 2,
                                           Colour (0xff21222a), 2.5f, height / 2, true));

    g.fillRoundedRectangle (lxmargin, tymargin,
                            width - lxmargin - rxmargin,
                            height - tymargin - bymargin, 10.000f);

    // Heatmap (if enabled and image valid). Drawn underneath the grid lines.
    // The lookup image is at moderate resolution; JUCE scales it to fit.
    // In HA mode we clip to the chart ellipse so the rasterized stairsteps
    // along the chart boundary are masked into a smooth curve.
    if (energy_enabled_ && heatmap_image_.isValid())
    {
        Graphics::ScopedSaveState ss (g);
        Path clip;
        if (projection_ == Projection::HammerAitoff)
            clip.addEllipse (projection_bounds_.toFloat());
        else
            clip.addRoundedRectangle ((float) lxmargin, (float) tymargin,
                                      (float) (width - lxmargin - rxmargin),
                                      (float) (height - tymargin - bymargin),
                                      10.000f);
        g.reduceClipRegion (clip);
        g.drawImage (heatmap_image_,
                     projection_bounds_.getX(),     projection_bounds_.getY(),
                     projection_bounds_.getWidth(), projection_bounds_.getHeight(),
                     0, 0,
                     heatmap_image_.getWidth(),     heatmap_image_.getHeight(),
                     /*fillAlphaChannelWithCurrentBrush*/ false);
    }

    // Axis labels (elevation on the left margin, azimuth on the bottom margin).
    g.setColour (Colour (0x60ffffff));

    {
        const int elgridlines = 180/45 + 1;
        for (int i=0; i < elgridlines; i++)
        {
            float deg_val = 90.f - i * 45.f;
            auto p = sphericalToScreen (0.f, deg_val);
            String axislabel = String((int)deg_val); axislabel << "°";
            g.setFont (Font (FontOptions {"Arial Rounded MT", 12.0f, Font::plain}));
            // place label aligned with the actual y of the parallel
            g.drawText (axislabel, 0, ((int) p.y) - 6, 34, 12, Justification::centred, false);
        }

        const int azgridlines = 360/45 + 1;
        for (int i=0; i < azgridlines; i++)
        {
            float deg_val = 180.f - i * 45.f;
            auto p = sphericalToScreen (deg_val, 0.f);
            String axislabel = String((int)deg_val); axislabel << "°";
            g.setFont (Font (FontOptions {"Arial Rounded MT", 12.0f, Font::plain}));
            g.drawText (axislabel, ((int) p.x) - 22, getHeight() - bymargin, 44, 12, Justification::centred, false);
        }
    }

    g.setColour (Colour (0x60ffffff));
    g.strokePath (path_grid, PathStrokeType (0.25f));

    g.setColour (Colour (0xffffffff));
    g.strokePath (path_w_grid, PathStrokeType (0.25f));
}

void PanningGraph::resized()
{
    const int width  = getWidth();
    const int height = getHeight();

    for (int i=0; i<graphs_.size(); i++)
        graphs_.getUnchecked(i)->setBounds(0, 0, width, height);

    projection_bounds_ = getProjectionBounds();
    rebuildGridLines();
    rebuildPixelCartCache();
    rebuildAllFilterPaths();
    rebuildHeatmapMesh();

    // Re-position drag buttons/labels for valid filters.
    for (int i = 0; i < NUM_FILTERS; ++i)
    {
        if (! filter_states_[i].valid) continue;
        auto p = sphericalToScreen (filter_states_[i].az, filter_states_[i].el);
        btn_drag.getUnchecked(i)->setBounds ((int) p.x - 8, (int) p.y - 8, 16, 16);
        lbl_drag.getUnchecked(i)->setBounds ((int) p.x - 12, (int) p.y - 12, 26, 24);
    }
}

void PanningGraph::rebuildGridLines()
{
    path_grid.clear();
    path_w_grid.clear();

    auto addLine = [&] (Path& p, Point<float> from, Point<float> to)
    {
        p.startNewSubPath (from);
        p.lineTo (to);
    };

    if (projection_ == Projection::Equirect)
    {
        const int el_gridlines = 180/45 + 1;
        for (int i=0; i < el_gridlines; ++i)
        {
            float deg_val = -90.f + i*45.f;
            Path& target = (deg_val == 0.f) ? path_w_grid : path_grid;
            addLine (target,
                     sphericalToScreen (-180.f, deg_val),
                     sphericalToScreen ( 180.f, deg_val));
        }

        const int az_gridlines = 360/45 + 1;
        for (int i=0; i < az_gridlines; ++i)
        {
            float deg_val = -180.f + i*45.f;
            Path& target = (deg_val == 0.f) ? path_w_grid : path_grid;
            addLine (target,
                     sphericalToScreen (deg_val,  90.f),
                     sphericalToScreen (deg_val, -90.f));
        }
    }
    else
    {
        // HA: meridians and parallels are curves — sample densely.
        constexpr int kSamples = 64;

        // Parallels (constant elevation).
        const int el_gridlines = 180/45 + 1;
        for (int i=0; i < el_gridlines; ++i)
        {
            float el = -90.f + i*45.f;
            Path& target = (el == 0.f) ? path_w_grid : path_grid;
            target.startNewSubPath (sphericalToScreen (-180.f, el));
            for (int s=1; s <= kSamples; ++s)
            {
                float az = -180.f + (360.f * s) / kSamples;
                target.lineTo (sphericalToScreen (az, el));
            }
        }

        // Meridians (constant azimuth). In HA, az=+180° and az=-180° map to
        // the LEFT and RIGHT chart edges respectively (different curves on
        // screen even though they're the same antimeridian on the sphere).
        // Draw both — the previous "skip duplicate" guard was hiding the
        // right-edge meridian.
        const int az_gridlines = 360/45 + 1;
        for (int i=0; i < az_gridlines; ++i)
        {
            float az = -180.f + i*45.f;
            Path& target = (az == 0.f) ? path_w_grid : path_grid;

            target.startNewSubPath (sphericalToScreen (az, -90.f));
            for (int s=1; s <= kSamples; ++s)
            {
                float el = -90.f + (180.f * s) / kSamples;
                target.lineTo (sphericalToScreen (az, el));
            }
        }
    }
}

// ============================================================================
// Filter shape construction
// ============================================================================

void PanningGraph::rebuildAllFilterPaths()
{
    for (int i = 0; i < NUM_FILTERS; ++i)
        if (filter_states_[i].valid)
            rebuildFilterPath (i);
}

void PanningGraph::rebuildFilterPath (int idx)
{
    const auto& fs = filter_states_[idx];

    // HA mode: bypass the path renderer entirely — rasterize a per-pixel mask
    // via the spherical inside-test. This trivially handles seam crossings,
    // pole wraps, and multi-piece regions that break a path-based renderer.
    if (projection_ == Projection::HammerAitoff)
    {
        rasterizeFilterMaskHA (idx);
        return;
    }

    Path filterarea;

    if (projection_ == Projection::Equirect)
    {
        // Preserve the existing equirect look (addEllipse / addRectangle plus
        // pole and azimuth wrap-arounds).
        const float az = fs.az;
        const float el = fs.el;
        const float w_deg  = fs.width;
        const float h_deg  = fs.height;

        if (! fs.shape) // circle
        {
            int x = degtoxpos(az);
            int y = degtoypos(el);
            int w = degtoxpos(w_deg) - degtoxpos(0.f);
            int h = degtoypos(w_deg) - degtoypos(0.f);

            filterarea.addEllipse(x-w, y-h, 2*w, 2*h);

            if (az + w_deg > 180.f)
            {
                int x2 = degtoxpos(az - 360.f);
                filterarea.addEllipse(x2-w, y-h, 2*w, 2*h);
            }
            if (az - w_deg < -180.f)
            {
                int x2 = degtoxpos(az + 360.f);
                filterarea.addEllipse(x2-w, y-h, 2*w, 2*h);
            }
            if (el + h_deg > 90.f)
            {
                float newaz = az - 180.f; if (newaz < -180.f) newaz += 360.f;
                int x2 = degtoxpos(newaz);
                int y2 = degtoypos(180.f - el);
                filterarea.addEllipse(x2-w, y2-h, 2*w, 2*h);
            }
            if (el - h_deg < -90.f)
            {
                float newaz = az - 180.f; if (newaz < -180.f) newaz += 360.f;
                int x2 = degtoxpos(newaz);
                int y2 = degtoypos(-180.f - el);
                filterarea.addEllipse(x2-w, y2-h, 2*w, 2*h);
            }
        }
        else // rectangle
        {
            int x = degtoxpos(az);
            int y = degtoypos(el);
            int w = degtoxpos(w_deg) - degtoxpos(0.f);
            int h = degtoypos(h_deg) - degtoypos(0.f);

            filterarea.addRectangle(x-w, y-h, 2*w, 2*h);

            if (az + w_deg > 180.f)
            {
                int x2 = degtoxpos(az - 360.f);
                filterarea.addRectangle(x2-w, y-h, 2*w, 2*h);
            }
            if (az - w_deg < -180.f)
            {
                int x2 = degtoxpos(az + 360.f);
                filterarea.addRectangle(x2-w, y-h, 2*w, 2*h);
            }
            if (el + h_deg > 90.f)
            {
                float newaz = az - 180.f; if (newaz < -180.f) newaz += 360.f;
                int x2 = degtoxpos(newaz);
                int y2 = degtoypos(180.f - el);
                filterarea.addRectangle(x2-w, y2-h, 2*w, 2*h);
            }
            if (el - h_deg < -90.f)
            {
                float newaz = az - 180.f; if (newaz < -180.f) newaz += 360.f;
                int x2 = degtoxpos(newaz);
                int y2 = degtoypos(-180.f - el);
                filterarea.addRectangle(x2-w, y2-h, 2*w, 2*h);
            }
        }
    }
    // (HA branch handled above by rasterizeFilterMaskHA.)

    graphs_.getUnchecked (idx)->setPath (&filterarea, fs.gain, fs.solo, one_filter_solo_);
    graphs_.getUnchecked (idx)->repaint();
}

// ============================================================================
// Per-pixel cartesian cache + filter-mask rasterizer (HA mode)
// ============================================================================

void PanningGraph::rebuildPixelCartCache()
{
    pixel_cart_.clear();
    pixel_cart_W_ = pixel_cart_H_ = 0;
    if (projection_ != Projection::HammerAitoff) return;

    const auto rect = projection_bounds_;
    const int dispW = rect.getWidth();
    const int dispH = rect.getHeight();
    if (dispW <= 0 || dispH <= 0) return;

    constexpr int kMaxDim = 480;
    int W = dispW, H = dispH;
    if (W > kMaxDim || H > kMaxDim)
    {
        const float s = (float) kMaxDim / (float) jmax (W, H);
        W = jmax (4, (int) std::round (W * s));
        H = jmax (4, (int) std::round (H * s));
    }
    pixel_cart_W_ = W;
    pixel_cart_H_ = H;
    pixel_cart_.assign ((size_t) W * (size_t) H, juce::Vector3D<float> (2.f, 0.f, 0.f));

    const float invSX = (float) dispW / (float) W;
    const float invSY = (float) dispH / (float) H;
    const float deg2rad = MathConstants<float>::pi / 180.f;

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            const float sx = (float) rect.getX() + (x + 0.5f) * invSX;
            const float sy = (float) rect.getY() + (y + 0.5f) * invSY;
            float az_deg, el_deg;
            if (! screenToSpherical ({ sx, sy }, az_deg, el_deg)) continue;
            const float azR = az_deg * deg2rad;
            const float elR = el_deg * deg2rad;
            float cx, cy, cz; sphToCart (azR, elR, cx, cy, cz);
            pixel_cart_[(size_t) y * (size_t) W + (size_t) x]
                = juce::Vector3D<float> (cx, cy, cz);
        }
    }
}

void PanningGraph::rasterizeFilterMaskHA (int idx)
{
    const auto& fs = filter_states_[idx];

    if (pixel_cart_.empty() || pixel_cart_W_ <= 0 || pixel_cart_H_ <= 0
        || projection_bounds_.getWidth() <= 0 || projection_bounds_.getHeight() <= 0)
    {
        // Mask not buildable yet — clear whatever was there.
        graphs_.getUnchecked (idx)->setMask (Image(), Image(), projection_bounds_,
                                             fs.gain, fs.solo, one_filter_solo_);
        graphs_.getUnchecked (idx)->repaint();
        return;
    }

    const int W = pixel_cart_W_;
    const int H = pixel_cart_H_;

    const float deg2rad = MathConstants<float>::pi / 180.f;
    const float fcAz = fs.az * deg2rad;
    const float fcEl = fs.el * deg2rad;
    const float wRad = fs.width  * deg2rad;
    const float hRad = fs.height * deg2rad;

    float fcx, fcy, fcz; sphToCart (fcAz, fcEl, fcx, fcy, fcz);

    // Anti-aliased rasterization: a pixel's fill alpha smoothly transitions
    // from 1 (well inside) to 0 (well outside) over a small angular band
    // centered on the boundary. Converting that angular band to a dot-product
    // band uses the local rate |d(dot)/d(angle)| = sin(width). Stroke alpha
    // peaks ON the boundary and falls off symmetrically over the same band.
    // Result: smooth fill edges and a soft outline ring, anti-aliased at
    // sub-pixel level rather than the hard 1-bit mask we had before.
    constexpr float kEdgeAngularRad = 1.5f * MathConstants<float>::pi / 180.f; // ~1.5°
    const float epsW = std::max (1e-4f, std::sin (wRad) * kEdgeAngularRad);
    const float epsH = std::max (1e-4f, std::sin (hRad) * kEdgeAngularRad);

    auto smoothFill = [] (float dotDelta, float eps) {
        // 0 at dotDelta = -eps, 1 at dotDelta = +eps, smoothstep between.
        const float t = jlimit (0.0f, 1.0f, (dotDelta + eps) * 0.5f / eps);
        return t * t * (3.0f - 2.0f * t);
    };
    auto edgeAlpha = [] (float dotDelta, float eps) {
        // Triangular kernel: 1 at dotDelta=0, 0 at |dotDelta|>=eps.
        const float a = 1.0f - std::abs (dotDelta) / eps;
        return jlimit (0.0f, 1.0f, a);
    };

    Image fillImg   (Image::ARGB, W, H, true);
    Image strokeImg (Image::ARGB, W, H, true);
    Image::BitmapData fbd (fillImg,   Image::BitmapData::writeOnly);
    Image::BitmapData sbd (strokeImg, Image::BitmapData::writeOnly);

    if (! fs.shape)
    {
        // Circular: angle from center via dot product.
        const float cosW = std::cos (wRad);
        for (int y = 0; y < H; ++y)
        {
            auto* fRow = fbd.getLinePointer (y);
            auto* sRow = sbd.getLinePointer (y);
            for (int x = 0; x < W; ++x)
            {
                const auto& p = pixel_cart_[(size_t) y * (size_t) W + (size_t) x];
                if (p.x > 1.5f) continue;
                const float dot      = p.x * fcx + p.y * fcy + p.z * fcz;
                const float dotDelta = dot - cosW; // >0 inside, <0 outside

                const float fA = smoothFill (dotDelta, epsW);
                const float sA = edgeAlpha (dotDelta, epsW);
                if (fA > 0.0f)
                {
                    juce::PixelARGB pa ((juce::uint8) (fA * 255.0f), 255, 255, 255);
                    *reinterpret_cast<juce::PixelARGB*> (fRow + x * fbd.pixelStride) = pa;
                }
                if (sA > 0.0f)
                {
                    juce::PixelARGB pa ((juce::uint8) (sA * 255.0f), 255, 255, 255);
                    *reinterpret_cast<juce::PixelARGB*> (sRow + x * sbd.pixelStride) = pa;
                }
            }
        }
    }
    else
    {
        // Rectangular: independent az and el angular distances.
        const float cosW = std::cos (wRad);
        const float cosH = std::cos (hRad);
        const float fcAzCos = std::cos (fcAz);
        const float fcAzSin = std::sin (fcAz);
        const float fcElCos = std::cos (fcEl);
        const float fcElSin = std::sin (fcEl);
        for (int y = 0; y < H; ++y)
        {
            auto* fRow = fbd.getLinePointer (y);
            auto* sRow = sbd.getLinePointer (y);
            for (int x = 0; x < W; ++x)
            {
                const auto& p = pixel_cart_[(size_t) y * (size_t) W + (size_t) x];
                if (p.x > 1.5f) continue;
                const float pAz = std::atan2 (p.y, p.x);
                const float pEl = std::asin  (jlimit (-1.f, 1.f, p.z));
                const float dotAz = std::cos (pAz) * fcAzCos + std::sin (pAz) * fcAzSin;
                const float dotEl = std::cos (pEl) * fcElCos + std::sin (pEl) * fcElSin;
                const float ddAz  = dotAz - cosW;
                const float ddEl  = dotEl - cosH;

                // Fill = intersection of two half-spaces.
                const float fA = smoothFill (ddAz, epsW) * smoothFill (ddEl, epsH);

                // Stroke = on either edge while well inside the perpendicular axis.
                const float strokeAz = edgeAlpha (ddAz, epsW) * smoothFill (ddEl + epsH, epsH * 2.f);
                const float strokeEl = edgeAlpha (ddEl, epsH) * smoothFill (ddAz + epsW, epsW * 2.f);
                const float sA = std::max (strokeAz, strokeEl);

                if (fA > 0.0f)
                {
                    juce::PixelARGB pa ((juce::uint8) (fA * 255.0f), 255, 255, 255);
                    *reinterpret_cast<juce::PixelARGB*> (fRow + x * fbd.pixelStride) = pa;
                }
                if (sA > 0.0f)
                {
                    juce::PixelARGB pa ((juce::uint8) (sA * 255.0f), 255, 255, 255);
                    *reinterpret_cast<juce::PixelARGB*> (sRow + x * sbd.pixelStride) = pa;
                }
            }
        }
    }

    graphs_.getUnchecked (idx)->setMask (std::move (fillImg), std::move (strokeImg),
                                          projection_bounds_,
                                          fs.gain, fs.solo, one_filter_solo_);
    graphs_.getUnchecked (idx)->repaint();
}

void PanningGraph::setFilter(int idx, float az, float el, bool shape, float width, float height, float gain, bool solo)
{
    // Wrap angles to canonical ranges (matches old behavior).
    if (el >  90.f) { el = 180.f - el; az += 180.f; }
    if (el < -90.f) { el = 180.f + el; az += 180.f; }
    if (az >  180.f) az -= 360.f;
    if (az < -180.f) az += 360.f;

    filter_states_[idx] = { az, el, shape, width, height, gain, solo, true };

    // Update drag handle position via the active projection.
    auto p = sphericalToScreen (az, el);
    btn_drag.getUnchecked(idx)->setBounds ((int) p.x - 8, (int) p.y - 8, 16, 16);
    lbl_drag.getUnchecked(idx)->setBounds ((int) p.x - 12, (int) p.y - 12, 26, 24);

    // If the gain label is currently visible, follow the puck.
    if (idx < lbl_gain_db_.size() && lbl_gain_db_.getUnchecked (idx)->isVisible())
        positionGainLabel (idx);

    // Show the transient gain-dB readout next to the puck whenever the gain
    // changes (skip on the very first setFilter after construction — that
    // carries saved/default state, with prev = NaN).
    const float prev = last_gain_db_[(size_t) idx];
    const bool gainChanged = std::isfinite (prev)
                              && std::abs (prev - gain) > 0.05f;
    last_gain_db_[(size_t) idx] = gain;
    if (gainChanged)
        showGainLabel (idx, /*sticky=*/ false);

    rebuildFilterPath (idx);
}

void PanningGraph::showGainLabel (int idx, bool sticky)
{
    if (idx >= lbl_gain_db_.size() || ! filter_states_[(size_t) idx].valid) return;
    auto* l = lbl_gain_db_.getUnchecked (idx);
    const float gain = filter_states_[(size_t) idx].gain;
    String text;
    if (gain >  0.05f) text << "+";
    text << String (gain, 1) << " dB";
    l->setText (text, dontSendNotification);
    l->setAlpha (1.0f);
    l->setVisible (true);
    positionGainLabel (idx);
    gain_label_until_ms_[(size_t) idx] = sticky
        ? std::numeric_limits<juce::int64>::max()
        : (juce::int64) juce::Time::getMillisecondCounter()
             + kGainLabelHoldMs + kGainLabelFadeMs;
    if (! isTimerRunning())
        startTimerHz (30);
}

void PanningGraph::positionGainLabel (int idx)
{
    if (idx >= lbl_gain_db_.size() || ! filter_states_[(size_t) idx].valid) return;
    const auto p = sphericalToScreen (filter_states_[(size_t) idx].az,
                                      filter_states_[(size_t) idx].el);
    const int labelW = 60;
    const int labelH = 16;
    int x = (int) p.x - labelW / 2;
    int y = (int) p.y + 14;                                  // below the puck
    if (y + labelH > getHeight() - (int) bymargin)
        y = (int) p.y - 14 - labelH;                          // above if no room below
    x = jlimit ((int) lxmargin + 2, getWidth() - (int) rxmargin - 2 - labelW, x);
    lbl_gain_db_.getUnchecked (idx)->setBounds (x, y, labelW, labelH);
    lbl_gain_db_.getUnchecked (idx)->toFront (false);
}

void PanningGraph::timerCallback()
{
    const auto now = (juce::int64) juce::Time::getMillisecondCounter();

    bool anyVisible = false;
    for (int i = 0; i < lbl_gain_db_.size(); ++i)
    {
        const auto until = gain_label_until_ms_[(size_t) i];
        if (until <= 0) continue;
        const auto remaining = until - now;
        if (remaining <= 0)
        {
            lbl_gain_db_.getUnchecked (i)->setVisible (false);
            gain_label_until_ms_[(size_t) i] = 0;
            continue;
        }
        anyVisible = true;
        if (remaining < kGainLabelFadeMs)
        {
            const float a = (float) remaining / (float) kGainLabelFadeMs;
            lbl_gain_db_.getUnchecked (i)->setAlpha (jlimit (0.0f, 1.0f, a));
        }
        else
        {
            lbl_gain_db_.getUnchecked (i)->setAlpha (1.0f);
        }
    }

    if (! anyVisible)
        stopTimer();
}

void PanningGraph::setOneFilterSolo(bool is_one_solo)
{
    one_filter_solo_ = is_one_solo;
}

// ============================================================================
// View-mode setters
// ============================================================================

void PanningGraph::setProjection (Projection p)
{
    if (projection_ == p) return;
    projection_ = p;
    projection_bounds_ = getProjectionBounds();
    rebuildGridLines();
    rebuildPixelCartCache();
    rebuildAllFilterPaths();
    rebuildHeatmapMesh();

    // Reposition drag handles to match new projection.
    for (int i = 0; i < NUM_FILTERS; ++i)
    {
        if (! filter_states_[i].valid) continue;
        auto pt = sphericalToScreen (filter_states_[i].az, filter_states_[i].el);
        btn_drag.getUnchecked(i)->setBounds ((int) pt.x - 8, (int) pt.y - 8, 16, 16);
        lbl_drag.getUnchecked(i)->setBounds ((int) pt.x - 12, (int) pt.y - 12, 26, 24);
    }

    repaint();
}

void PanningGraph::setEnergyEnabled (bool on)
{
    if (energy_enabled_ == on) return;
    energy_enabled_ = on;
    if (! on)
    {
        heatmap_image_ = Image();
    }
    else
    {
        // Rebuild fully — the image was destroyed on disable, and after a
        // projection switch the cached vertex pixels may be stale.
        rebuildHeatmapMesh();
    }
    // Drop the gain-tinted fill from filter overlays while the heatmap is
    // showing so it stays unobstructed.
    for (int i = 0; i < graphs_.size(); ++i)
        graphs_.getUnchecked (i)->setSuppressFill (on);
    repaint();
}

void PanningGraph::setColormap (Colormap::Map m)
{
    if (colormap_ == m) return;
    colormap_ = m;
    if (energy_enabled_)
    {
        renderHeatmapImage();
        repaint();
    }
}

void PanningGraph::setMesh (std::vector<Eigen::Vector3d> dirs, std::vector<int> indices)
{
    grid_dirs_     = std::move (dirs);
    mesh_indices_  = std::move (indices);
    grid_t_.assign (grid_dirs_.size(), 0.f);
    rebuildHeatmapMesh();
}

void PanningGraph::setGridDirections (const std::vector<Eigen::Vector3d>& dirs)
{
    // Legacy entry point: assume the IEM HA-mesh indices.
    std::vector<int> idx (HammerAitovSample::kIndices,
                          HammerAitovSample::kIndices + HammerAitovSample::kNumTriangles * 3);
    setMesh (dirs, std::move (idx));
}

void PanningGraph::setEnergyT (const std::vector<float>& tValues)
{
    if (! energy_enabled_) return;
    if (tValues.size() != grid_t_.size()) return;
    grid_t_ = tValues;
    renderHeatmapImage();
    repaint (projection_bounds_);
}

// ============================================================================
// Heatmap pixel lookup + rendering
// ============================================================================

void PanningGraph::rebuildHeatmapMesh()
{
    const auto rect = projection_bounds_;
    const int dispW = rect.getWidth();
    const int dispH = rect.getHeight();
    if (dispW <= 0 || dispH <= 0 || grid_dirs_.empty())
    {
        vertex_pixels_.clear();
        heatmap_image_ = Image();
        return;
    }

    // Cap the lookup-image resolution to keep resize cheap. The image is
    // scaled up to projection_bounds_ at draw time. Keep aspect of rect so
    // triangle rasterization preserves on-screen shapes.
    constexpr int kMaxDim = 720;
    int W = dispW, H = dispH;
    if (W > kMaxDim || H > kMaxDim)
    {
        const float s = (float) kMaxDim / (float) jmax (W, H);
        W = jmax (4, (int) std::round (W * s));
        H = jmax (4, (int) std::round (H * s));
    }

    // Compute each grid point's pixel position in image-local coords by
    // running the IEM HA forward map on its (az, el), then applying the same
    // uniform-by-half-diagonal scaling we use for the projection.
    const int N = (int) grid_dirs_.size();
    vertex_pixels_.resize ((size_t) N);

    if (projection_ == Projection::HammerAitoff)
    {
        // Map canonical 2:1 ellipse [-2,2] × [-1,1] onto image. The image is
        // 2:1 because rebuildHeatmapMesh is sized from projection_bounds_,
        // which letterboxes to 2:1 in HA mode.
        //
        // We deliberately overshoot the vertex positions slightly (radially
        // outward from the image center) so the outermost triangles cover
        // beyond the actual chart-boundary pixel positions. The smooth
        // anti-aliased ellipse clip in paint() then cuts the heatmap with
        // its own curve, so the chart edge follows the ellipse instead of
        // exposing the pixel-aligned rasterized boundary.
        constexpr float kOvershoot = 1.04f;
        const float halfW = 0.5f * (float) W;
        const float halfH = 0.5f * (float) H;
        for (int i = 0; i < N; ++i)
        {
            const auto& d = grid_dirs_[(size_t) i];
            const float az = std::atan2 ((float) d.y(), (float) d.x());
            const float el = std::asin  (jlimit (-1.f, 1.f, (float) d.z()));
            float hx, hy;
            HammerAitov::sphericalToXY (az, el, hx, hy);
            hx = -hx;                                  // flip so +az → right
            hx *= 2.f;
            const float u = (hx + 2.f) * 0.25f;       // 0..1
            const float v = (1.f - hy) * 0.5f;        // 0..1
            const float xCentered = (u - 0.5f) * (float) W;
            const float yCentered = (v - 0.5f) * (float) H;
            vertex_pixels_[(size_t) i] = { halfW + xCentered * kOvershoot,
                                           halfH + yCentered * kOvershoot };
        }
    }
    else
    {
        // Equirect: linear az/el → pixel.
        for (int i = 0; i < N; ++i)
        {
            const auto& d = grid_dirs_[(size_t) i];
            const float az = std::atan2 ((float) d.y(), (float) d.x()) * 180.f / MathConstants<float>::pi;
            const float el = std::asin  (jlimit (-1.f, 1.f, (float) d.z()))
                             * 180.f / MathConstants<float>::pi;
            const float u = (az + 180.f) / 360.f;
            const float v = (-el + 90.f) / 180.f;
            vertex_pixels_[(size_t) i] = { u * (float) W, v * (float) H };
        }
    }

    heatmap_image_ = Image (Image::ARGB, W, H, true);

    if (energy_enabled_) renderHeatmapImage();
}

namespace
{
    // Rasterize one triangle into an ARGB image with linear color
    // interpolation (Gouraud) across its three vertex colors. Pixels outside
    // the triangle are left untouched. Edge function is computed
    // incrementally per scanline for ~3× speedup over recomputing per pixel.
    inline void rasterizeTriangle (juce::Image::BitmapData& bd,
                                   juce::Point<float> v0, juce::Point<float> v1, juce::Point<float> v2,
                                   juce::Colour c0, juce::Colour c1, juce::Colour c2)
    {
        const float dx10 = v1.x - v0.x, dy10 = v1.y - v0.y;
        const float dx20 = v2.x - v0.x, dy20 = v2.y - v0.y;
        const float area2 = dx10 * dy20 - dy10 * dx20;
        if (std::abs (area2) < 1e-6f) return;
        const float invA2 = 1.0f / area2;

        // Bounding box clipped to image.
        const int W = bd.width, H = bd.height;
        const int minX = std::max (0,     (int) std::floor (std::min ({ v0.x, v1.x, v2.x })));
        const int maxX = std::min (W - 1, (int) std::ceil  (std::max ({ v0.x, v1.x, v2.x })));
        const int minY = std::max (0,     (int) std::floor (std::min ({ v0.y, v1.y, v2.y })));
        const int maxY = std::min (H - 1, (int) std::ceil  (std::max ({ v0.y, v1.y, v2.y })));
        if (minX > maxX || minY > maxY) return;

        const float r0 = (float) c0.getRed(),   g0 = (float) c0.getGreen(), b0 = (float) c0.getBlue(),  a0 = (float) c0.getAlpha();
        const float r1 = (float) c1.getRed(),   g1 = (float) c1.getGreen(), b1 = (float) c1.getBlue(),  a1 = (float) c1.getAlpha();
        const float r2 = (float) c2.getRed(),   g2 = (float) c2.getGreen(), b2 = (float) c2.getBlue(),  a2 = (float) c2.getAlpha();

        // w0 corresponds to vertex 0; w1 → v1; w2 = 1 - w0 - w1.
        // Edge function E_ab(p) = (b-a) × (p-a)
        //   = (b.x-a.x)*(p.y-a.y) - (b.y-a.y)*(p.x-a.x)
        // ∂E/∂p.x = a.y - b.y     ∂E/∂p.y = b.x - a.x
        //   E_12: a=v1, b=v2  →  dx = v1.y - v2.y,  dy = v2.x - v1.x
        //   E_20: a=v2, b=v0  →  dx = v2.y - v0.y,  dy = v0.x - v2.x
        const float e12_dx = v1.y - v2.y;
        const float e12_dy = v2.x - v1.x;
        const float e20_dx = v2.y - v0.y;
        const float e20_dy = v0.x - v2.x;

        const float pX = (float) minX + 0.5f;
        const float pY = (float) minY + 0.5f;
        float e12_row = (v2.x - v1.x) * (pY - v1.y) - (v2.y - v1.y) * (pX - v1.x);
        float e20_row = (v0.x - v2.x) * (pY - v2.y) - (v0.y - v2.y) * (pX - v2.x);

        for (int y = minY; y <= maxY; ++y)
        {
            float e12 = e12_row;
            float e20 = e20_row;
            auto* row = bd.getLinePointer (y);
            for (int x = minX; x <= maxX; ++x)
            {
                const float w0 = e12 * invA2;
                const float w1 = e20 * invA2;
                const float w2 = 1.0f - w0 - w1;
                if (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f)
                {
                    const float r = w0 * r0 + w1 * r1 + w2 * r2;
                    const float g = w0 * g0 + w1 * g1 + w2 * g2;
                    const float b = w0 * b0 + w1 * b1 + w2 * b2;
                    const float a = w0 * a0 + w1 * a1 + w2 * a2;
                    auto* pix = row + x * bd.pixelStride;
                    juce::PixelARGB pa ((juce::uint8) jlimit (0, 255, (int) a),
                                        (juce::uint8) jlimit (0, 255, (int) r),
                                        (juce::uint8) jlimit (0, 255, (int) g),
                                        (juce::uint8) jlimit (0, 255, (int) b));
                    *reinterpret_cast<juce::PixelARGB*> (pix) = pa;
                }
                e12 += e12_dx;
                e20 += e20_dx;
            }
            e12_row += e12_dy;
            e20_row += e20_dy;
        }
    }
}

void PanningGraph::renderHeatmapImage()
{
    if (! heatmap_image_.isValid() || vertex_pixels_.empty() || grid_t_.empty()) return;
    if (vertex_pixels_.size() != grid_t_.size()) return;
    if (mesh_indices_.empty() || mesh_indices_.size() % 3 != 0) return;

    // Clear to transparent each frame (rasterizer only writes covered pixels).
    heatmap_image_.clear (heatmap_image_.getBounds(), Colours::transparentBlack);

    // Pre-sample colormap once per grid point. Per-vertex alpha = 0.65 so the
    // background gradient remains faintly visible underneath the heatmap.
    std::vector<juce::Colour> palette (grid_t_.size());
    for (size_t i = 0; i < grid_t_.size(); ++i)
        palette[i] = Colormap::sample (colormap_, grid_t_[i]).withAlpha (0.65f);

    Image::BitmapData bd (heatmap_image_, Image::BitmapData::writeOnly);

    // Wrap-detection only matters for equirect, where antimeridian-crossing
    // triangles end up spanning the full panel width. In HA, after our
    // uniform-by-half-diagonal scaling the chart extends well beyond the
    // panel (rMax > halfH for any non-square rect), so plenty of legitimate
    // mesh triangles cover wide pixel ranges — skipping by width would erase
    // the heatmap entirely.
    const float wrapThresh = (projection_ == Projection::Equirect)
                              ? 0.5f * (float) bd.width
                              : std::numeric_limits<float>::infinity();

    const int numTri = (int) (mesh_indices_.size() / 3);
    const int N      = (int) vertex_pixels_.size();
    const float W_f  = (float) bd.width;

    for (int t = 0; t < numTri; ++t)
    {
        const int i0 = mesh_indices_[(size_t) (t * 3 + 0)];
        const int i1 = mesh_indices_[(size_t) (t * 3 + 1)];
        const int i2 = mesh_indices_[(size_t) (t * 3 + 2)];
        if (i0 < 0 || i1 < 0 || i2 < 0 || i0 >= N || i1 >= N || i2 >= N) continue;

        Point<float> v0 = vertex_pixels_[(size_t) i0];
        Point<float> v1 = vertex_pixels_[(size_t) i1];
        Point<float> v2 = vertex_pixels_[(size_t) i2];

        const float xMin = std::min ({ v0.x, v1.x, v2.x });
        const float xMax = std::max ({ v0.x, v1.x, v2.x });

        if (xMax - xMin > wrapThresh)
        {
            // Equirect wrap-around: unwrap by shifting outlier vertices and
            // render at the original AND ±W positions so both seam sides
            // are filled. (HA mode disables this branch via wrapThresh=∞.)
            auto unwrap = [W_f] (Point<float>& v, float anchor) {
                while (v.x - anchor >  W_f * 0.5f) v.x -= W_f;
                while (anchor - v.x >  W_f * 0.5f) v.x += W_f;
            };
            const float anchor = v0.x;
            unwrap (v1, anchor);
            unwrap (v2, anchor);

            rasterizeTriangle (bd, v0, v1, v2,
                               palette[(size_t) i0], palette[(size_t) i1], palette[(size_t) i2]);
            rasterizeTriangle (bd, { v0.x + W_f, v0.y }, { v1.x + W_f, v1.y }, { v2.x + W_f, v2.y },
                               palette[(size_t) i0], palette[(size_t) i1], palette[(size_t) i2]);
            rasterizeTriangle (bd, { v0.x - W_f, v0.y }, { v1.x - W_f, v1.y }, { v2.x - W_f, v2.y },
                               palette[(size_t) i0], palette[(size_t) i1], palette[(size_t) i2]);
            continue;
        }

        rasterizeTriangle (bd, v0, v1, v2,
                           palette[(size_t) i0],
                           palette[(size_t) i1],
                           palette[(size_t) i2]);
    }
}

// ============================================================================
// Mouse / button input
// ============================================================================

void PanningGraph::buttonClicked (Button* buttonThatWasClicked)
{
    Point<float> mouse ((float) (buttonThatWasClicked->getPosition().getX() + buttonThatWasClicked->getMouseXYRelative().getX()),
                        (float) (buttonThatWasClicked->getPosition().getY() + buttonThatWasClicked->getMouseXYRelative().getY()));

    float az = 0.f, el = 0.f;
    if (! screenToSpherical (mouse, az, el))
        return; // outside HA ellipse — ignore

    az = jlimit (-180.f, 180.f, az);
    el = jlimit  (-90.f,  90.f, el);

    int i = buttonThatWasClicked->getName().getIntValue();

    if (i != mouse_near_filter_id)
    {
        mouse_near_filter_id = i;
        sendChangeMessage();
    }

    setParameterNotifyingHost(myprocessor_, PARAMS_PER_FILTER*i + Ambix_directional_loudnessAudioProcessor::AzimuthParam,   Deg360ToParam(az));
    setParameterNotifyingHost(myprocessor_, PARAMS_PER_FILTER*i + Ambix_directional_loudnessAudioProcessor::ElevationParam, Deg360ToParam(el));
}

void PanningGraph::mouseDown(const MouseEvent &event)
{
    int numfilters = btn_drag.size();

    for (int i = 0; i < numfilters; i++)
    {
        if (event.getMouseDownPosition().getDistanceFrom(btn_drag.getUnchecked(i)->getPosition()) < 80)
        {
            if (i != mouse_near_filter_id)
            {
                mouse_near_filter_id = i;
                sendChangeMessage();
            }

            int w_idx = PARAMS_PER_FILTER*mouse_near_filter_id + Ambix_directional_loudnessAudioProcessor::WidthParam;
            int h_idx = PARAMS_PER_FILTER*mouse_near_filter_id + Ambix_directional_loudnessAudioProcessor::HeightParam;

            mouse_down_width  = ParamToDeg360 (myprocessor_->getParameter(w_idx));
            mouse_down_height = ParamToDeg180 (myprocessor_->getParameter(h_idx));

            mouse_dir_w = btn_drag.getUnchecked(i)->getX() < event.getMouseDownX() ?  1 : -1;
            mouse_dir_h = btn_drag.getUnchecked(i)->getY() > event.getMouseDownY() ?  1 : -1;
            break;
        }
    }
}

void PanningGraph::mouseUp(const MouseEvent &event)
{
    mouse_near_filter_id = -1;
}

void PanningGraph::mouseDrag(const MouseEvent &event)
{
    if (mouse_near_filter_id > -1)
    {
        int w_idx = PARAMS_PER_FILTER*mouse_near_filter_id + Ambix_directional_loudnessAudioProcessor::WidthParam;
        setParameterNotifyingHost (myprocessor_, w_idx,
            (float) jlimit (0.f, 1.f, Deg360ToParam (mouse_down_width  + xpostodeg (mouse_dir_w*event.getDistanceFromDragStartX() + degtoxpos (0))) ));

        int h_idx = PARAMS_PER_FILTER*mouse_near_filter_id + Ambix_directional_loudnessAudioProcessor::HeightParam;
        setParameterNotifyingHost (myprocessor_, h_idx,
            (float) jlimit (0.f, 1.f, Deg180ToParam (mouse_down_height + ypostodeg (mouse_dir_h*event.getDistanceFromDragStartY() + degtoypos (0))) ));
    }
}

void PanningGraph::mouseWheelMove (const MouseEvent &event, const MouseWheelDetails &wheel)
{
    int idx = -1;
    for (int i=0; i<btn_drag.size(); i++)
        if (btn_drag.getUnchecked(i)->getState() == 1) idx = i;

    if (idx > -1)
    {
        int paridx = PARAMS_PER_FILTER*idx + Ambix_directional_loudnessAudioProcessor::GainParam;
        setParameterNotifyingHost (myprocessor_, paridx,
            (float) jlimit (0.f, 1.f, myprocessor_->getParameter(paridx) + wheel.deltaY * 0.4f));

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
