/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ==============================================================================

 PopoutPanner — detached visualisation window for the encoder.

 A standalone DocumentWindow that contains its own switchable Sphere /
 Hammer-Aitoff panner. Created on demand from the main editor's Popout
 button so the user can drag the panner onto a different screen (or take
 it fullscreen via the OS's title-bar buttons) while keeping the main
 plugin UI focused on the parameter controls.

 The popout reads from and writes to the same Ambix_encoderAudioProcessor
 as the main editor, so dragging a puck in either view updates both
 simultaneously. View choice is independent — main editor on Sphere,
 popout on H-A is a perfectly valid setup.

 Lifetime: the main editor owns a std::unique_ptr<PopoutPanner>. When
 the user clicks the OS close button the popout signals the editor (via
 a callback) which then resets the unique_ptr — guaranteeing destruction
 happens on the message thread, where DocumentWindow assumes it lives.

 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "SphereOpenGL.h"
#include "HammerAitoffView.h"

class PopoutPanner : public juce::DocumentWindow
{
public:
    PopoutPanner (Ambix_encoderAudioProcessor& proc,
                  bool initialHammerView,
                  std::function<void()> onCloseCallback);
    ~PopoutPanner() override;

    void closeButtonPressed() override;

    // True if the popout is currently showing Hammer-Aitoff; the editor
    // reads this to persist the view choice on the processor side.
    bool isHammerView() const;

private:
    class Content;  // forward — implementation lives in the .cpp
    std::function<void()> onClose;
};
