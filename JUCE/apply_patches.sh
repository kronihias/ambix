#!/bin/bash 
# apply all patches to the juce modules


echo ""
echo "...patch file: modules/juce_audio_devices/native/juce_linux_JackAudio.cpp"

patch modules/juce_audio_devices/native/juce_linux_JackAudio.cpp patches/juce_linux_JackAudio.cpp.patch

echo ""
echo ""


echo "...patch file: modules/juce_gui_basics/widgets/juce_Slider.cpp"

patch modules/juce_gui_basics/widgets/juce_Slider.cpp patches/juce_Slider_cpp.patch

echo ""
echo ""


echo "...patch file: modules/juce_gui_basics/widgets/juce_Slider.h"

patch modules/juce_gui_basics/widgets/juce_Slider.h patches/juce_Slider_h.patch