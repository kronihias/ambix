#!/bin/bash 
# apply all patches to the juce modules


echo ""
echo "...patch file: modules/juce_audio_devices/native/juce_linux_JackAudio.cpp"

patch modules/juce_audio_devices/native/juce_linux_JackAudio.cpp patches/juce_linux_JackAudio.cpp.patch

echo ""
echo ""

