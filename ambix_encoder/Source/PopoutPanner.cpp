/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 ==============================================================================
 */

#include "PopoutPanner.h"

//==============================================================================
// Internal content component — owns the two panner views and the small
// header strip (Sphere / Hammer-Aitoff toggle). Sized by its parent
// DocumentWindow; resizes itself to fill the window's content area.
class PopoutPanner::Content : public juce::Component
{
public:
    Content (Ambix_encoderAudioProcessor& proc, bool initialHammer)
        : processor (proc), hammerView (initialHammer)
    {
        // Notify owner when the view toggle changes so it can persist.
        // (Set by PopoutPanner via setOnViewChange after construction.)
        sphere_opengl.processor = &processor;
        addChildComponent (sphere_opengl);

        hammer_view.setProcessor (&processor);
        addChildComponent (hammer_view);

        auto setupViewBtn = [this] (juce::TextButton& b, const juce::String& label,
                                     bool initiallyOn, juce::Button::ConnectedEdgeFlags edge)
        {
            addAndMakeVisible (b);
            b.setButtonText (label);
            b.setRadioGroupId (901);
            b.setClickingTogglesState (true);
            b.setConnectedEdges (edge);
            b.setToggleState (initiallyOn, juce::dontSendNotification);
        };
        setupViewBtn (btn_sphere, "Sphere",         ! hammerView, juce::Button::ConnectedOnRight);
        setupViewBtn (btn_hammer, "Hammer-Aitoff",    hammerView, juce::Button::ConnectedOnLeft);
        btn_sphere.onClick = [this]() { setHammerView (false); };
        btn_hammer.onClick = [this]() { setHammerView (true);  };

        applyVisibility();
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto top = r.removeFromTop (32).reduced (6, 4);
        btn_sphere.setBounds (top.removeFromLeft (70));
        btn_hammer.setBounds (top.removeFromLeft (110));

        // The sphere is square so we anchor it top-centred in the remaining
        // area. The Hammer-Aitoff view is 2:1 and centres horizontally
        // (HammerAitoffView::getProjectionBounds enforces the aspect itself).
        if (hammerView)
        {
            hammer_view.setBounds (r);
        }
        else
        {
            const int sz = juce::jmin (r.getWidth(), r.getHeight());
            sphere_opengl.setBounds (r.withSizeKeepingCentre (sz, sz));
        }
    }

    bool isHammerView() const noexcept { return hammerView; }

    void setOnViewChange (std::function<void(bool)> cb) { onViewChange = std::move (cb); }

private:
    void setHammerView (bool h)
    {
        if (hammerView == h) return;
        hammerView = h;
        applyVisibility();
        resized();
        if (onViewChange) onViewChange (h);
    }

    void applyVisibility()
    {
        sphere_opengl.setVisible (! hammerView);
        hammer_view  .setVisible (  hammerView);
    }

    Ambix_encoderAudioProcessor& processor;
    bool hammerView;

    SphereOpenGL     sphere_opengl;
    HammerAitoffView hammer_view;
    juce::TextButton btn_sphere, btn_hammer;
    std::function<void(bool)> onViewChange;
};

//==============================================================================
PopoutPanner::PopoutPanner (Ambix_encoderAudioProcessor& proc,
                            bool initialHammerView,
                            int  initialWidth,
                            int  initialHeight,
                            std::function<void()> onCloseCallback)
    : juce::DocumentWindow ("ambix_encoder panner",
                            juce::Colour (0xff141414),
                            juce::DocumentWindow::allButtons),
      processor (proc),
      onClose (std::move (onCloseCallback))
{
    setUsingNativeTitleBar (true);
    setResizable (true, true);
    setResizeLimits (320, 240, 4096, 4096);
    // Float above the DAW's plugin window so the popout stays visible
    // when the user clicks back into the main editor.
    setAlwaysOnTop (true);

    auto* content = new Content (proc, initialHammerView);
    content->setOnViewChange ([this] (bool h)
    {
        processor.popout_hammer_view = h;
    });
    setContentOwned (content, true);

    // Use the persisted size if it's sane; otherwise pick a sensible
    // default per view (wider for H-A, squarer for Sphere). Clamp to the
    // resize limits so old/corrupt state can't open the window off-screen.
    const int defaultW = initialHammerView ? 900 : 560;
    const int defaultH = initialHammerView ? 480 : 600;
    const int w = juce::jlimit (320, 4096, initialWidth  > 0 ? initialWidth  : defaultW);
    const int h = juce::jlimit (240, 4096, initialHeight > 0 ? initialHeight : defaultH);
    centreWithSize (w, h);

    setVisible (true);
    toFront (true);

    // Show the track name (and ID) in the window title so multiple popouts
    // from different encoder instances are immediately distinguishable. The
    // host can rename a track at any time, so poll periodically and refresh
    // when it changes — getTrackName is just a string copy under a lock.
    refreshTitle();
    startTimer (1000);
}

PopoutPanner::~PopoutPanner()
{
    // Content is owned and freed by DocumentWindow's destructor.
}

void PopoutPanner::closeButtonPressed()
{
    // Delegate to the owner (the editor) so it can drop the unique_ptr.
    // Destroying the window inline here would re-enter our own callbacks.
    if (onClose) onClose();
}

void PopoutPanner::resized()
{
    juce::DocumentWindow::resized();
    // Persist the popout's current size so a close-and-reopen (or
    // session save) restores exactly where the user left it.
    processor.popout_width  = getWidth();
    processor.popout_height = getHeight();
}

bool PopoutPanner::isHammerView() const
{
    if (auto* c = dynamic_cast<Content*> (getContentComponent()))
        return c->isHammerView();
    return false;
}

void PopoutPanner::timerCallback()
{
    const auto current = processor.getTrackName();
    if (current != lastTrackName)
        refreshTitle();
}

void PopoutPanner::refreshTitle()
{
    const auto trackName = processor.getTrackName();
    lastTrackName = trackName;

    juce::String title;
    if (trackName.isNotEmpty())
        title << trackName << " — ambix_encoder";
    else
        title << "ambix_encoder panner";

    title << " [ID " << processor.m_id << "]";
    setName (title);
}
