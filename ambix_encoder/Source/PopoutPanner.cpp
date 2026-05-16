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

private:
    void setHammerView (bool h)
    {
        if (hammerView == h) return;
        hammerView = h;
        applyVisibility();
        resized();
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
};

//==============================================================================
PopoutPanner::PopoutPanner (Ambix_encoderAudioProcessor& proc,
                            bool initialHammerView,
                            std::function<void()> onCloseCallback)
    : juce::DocumentWindow ("ambix_encoder panner",
                            juce::Colour (0xff141414),
                            juce::DocumentWindow::allButtons),
      onClose (std::move (onCloseCallback))
{
    setUsingNativeTitleBar (true);
    setResizable (true, true);
    setResizeLimits (320, 240, 4096, 4096);

    // setContentOwned takes ownership of the heap-allocated Content; it's
    // freed when the DocumentWindow itself is destroyed.
    setContentOwned (new Content (proc, initialHammerView), true);

    // Sensible default size — wider for H-A, squarer for Sphere. The OS
    // will let the user drag-resize and toggle fullscreen via the title
    // bar (native fullscreen button on macOS, Win/Linux equivalent).
    if (initialHammerView)
        centreWithSize (900, 480);
    else
        centreWithSize (560, 600);

    setVisible (true);
    toFront (true);
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

bool PopoutPanner::isHammerView() const
{
    if (auto* c = dynamic_cast<Content*> (getContentComponent()))
        return c->isHammerView();
    return false;
}
