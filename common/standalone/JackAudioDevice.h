#pragma once

#include <juce_audio_devices/juce_audio_devices.h>

// Factory for the vendored JACK device type (see JackAudioDevice.cpp).
// The standalone host registers this with its AudioDeviceManager. JUCE is built
// with JUCE_JACK=0, so this is the only JACK AudioIODeviceType in the app.
juce::AudioIODeviceType* createJackAudioDeviceType();
