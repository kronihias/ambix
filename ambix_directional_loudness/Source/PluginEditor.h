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

#ifndef __JUCE_HEADER_CDA58EC13A3FDFE9__
#define __JUCE_HEADER_CDA58EC13A3FDFE9__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "PanningGraph.h"
#include "FilterTab.h"
#include "Colormap.h"
//[/Headers]



class TrackedTabbedComponent : public TabbedComponent {
public:
    using TabbedComponent::TabbedComponent;
    std::function<void()> onTabChanged;
    void currentTabChanged(int, const String&) override {
        if (onTabChanged) onTabChanged();
    }
};

// ----------------------------------------------------------------------------
// Vertical color bar with dB labels — modeled on IEM EnergyVisualizer's bar.
// Tracks the live [floor_db .. peak_db] range and the active colormap.
// ----------------------------------------------------------------------------
class EnergyColorBar : public juce::Component
{
public:
    void setColormap (Colormap::Map m)
    {
        if (map_ != m) { map_ = m; repaint(); }
    }

    void setRange (float floorDb, float peakDb)
    {
        if (! juce::approximatelyEqual (floor_db_, floorDb)
            || ! juce::approximatelyEqual (peak_db_, peakDb))
        {
            floor_db_ = floorDb;
            peak_db_  = peakDb;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override;

private:
    Colormap::Map map_     = Colormap::Map::Jet;
    float         floor_db_ = -45.f;
    float         peak_db_  = -10.f;
};

// ----------------------------------------------------------------------------
// Settings popup launched from the gear button on the main UI. Contains the
// energy-on/off toggle, auto-normalization toggle, dynamic-range slider,
// peak-level slider (active only when auto is off), and colormap dropdown.
// Mutates the processor's view-state directly; calls onChanged() so the
// editor can refresh dependent UI (color bar, panning graph).
// ----------------------------------------------------------------------------
class VizSettingsPopup : public juce::Component,
                         public juce::Button::Listener,
                         public juce::Slider::Listener,
                         public juce::ComboBox::Listener
{
public:
    explicit VizSettingsPopup (Ambix_directional_loudnessAudioProcessor& p);

    void resized() override;

    std::function<void()> onChanged;

private:
    void buttonClicked (juce::Button* b) override;
    void sliderValueChanged (juce::Slider* s) override;
    void comboBoxChanged (juce::ComboBox* c) override;
    void updateEnablement();
    void notifyChanged() { if (onChanged) onChanged(); }

    Ambix_directional_loudnessAudioProcessor& proc_;

    juce::TextButton btn_autonorm_ { "Auto Level" };
    juce::TextButton btn_hpf_      { "HP" };
    juce::TextButton btn_lpf_      { "LP" };
    juce::Slider     sld_range_;
    juce::Slider     sld_peak_;
    juce::Slider     sld_smoothing_;
    juce::Slider     sld_hpf_fc_;
    juce::Slider     sld_lpf_fc_;
    juce::Label      lbl_range_, lbl_peak_, lbl_smoothing_, lbl_colormap_, lbl_title_;
    juce::ComboBox   cb_colormap_;
};

//==============================================================================
class Ambix_directional_loudnessAudioProcessorEditor  : public AudioProcessorEditor,
                                                        public ChangeListener,
                                                        public Button::Listener,
                                                        private Timer
{
public:
    //==============================================================================
    Ambix_directional_loudnessAudioProcessorEditor (Ambix_directional_loudnessAudioProcessor* ownerFilter);
    ~Ambix_directional_loudnessAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;

    void changeListenerCallback (ChangeBroadcaster *source) override;

    void buttonClicked (Button* buttonThatWasClicked) override;

    bool keyPressed (const KeyPress& key) override;

    int getLastTouchedFilterId() const;

    void selectFilterTab(int id);

    // Refresh dependent UI after the popup mutates view state.
    void onViewStateChanged();

    // Binary resources:
    static const char* solo_symbol_png;
    static const int solo_symbol_pngSize;
    static const char* solo_symbol_over_png;
    static const int solo_symbol_over_pngSize;
    static const char* solo_symbol_act_png;
    static const int solo_symbol_act_pngSize;

private:
    void timerCallback() override;
    void pushEnergyToGraph();
    void openSettingsPopup();
    void pushMeshForProjection();
    void updateToolbarLabels();

    LookAndFeel_V3 globalLaF;

    //==============================================================================
    Label lbl_gd;
    Component filtergraph;

    TrackedTabbedComponent tabbedComponent;
    TrackedTabbedComponent tabbedComponent2;

    bool _lastTouchedIsRight = false;

    OwnedArray<FilterTab> _filtertabs;

    PanningGraph panninggraph;

    ImageButton btn_solo_reset;

    // Main-UI toolbar: [Energy Visualizer] [PRE/POST MOD] [Hammer-Aitoff] [⚙]
    TextButton btn_energy;       // Energy on/off
    TextButton btn_premod;       // PRE MOD (default on) ⇄ POST MOD
    TextButton btn_projection;   // Rectangular ⇄ Hammer-Aitoff
    TextButton btn_settings;     // gear — opens VizSettingsPopup

    // Vertical color bar to the right of the panning graph (always visible
    // when energy is on; greyed-out when off).
    EnergyColorBar color_bar;

    // Auto-norm peak follower (fast attack, slow release).
    float smoothed_max_db_ = -20.f;

    // Last applied [floor_db, peak_db] for the color bar labels.
    float displayed_floor_db_ = -45.f;
    float displayed_peak_db_  = -10.f;

    // Cached from processor so we don't reallocate on every timer tick.
    std::vector<float> energy_snapshot_;

    // Equirect mesh: precomputed once at startup (regular Naz × Nel grid
    // triangulation). HA mesh = IEM static `kIndices` — pulled directly when
    // pushing to PanningGraph.
    std::vector<int> eq_indices_;
    int eq_naz_ = 0, eq_nel_ = 0;

    bool _meshPushed = false;
    bool _meshIsHA   = false;     // tracks which mesh is currently in PanningGraph

    TooltipWindow tooltipWindow;

    Ambix_directional_loudnessAudioProcessor* getProcessor() const
    {
        return static_cast <Ambix_directional_loudnessAudioProcessor*> (getAudioProcessor());
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ambix_directional_loudnessAudioProcessorEditor)
};


#endif   // __JUCE_HEADER_CDA58EC13A3FDFE9__
