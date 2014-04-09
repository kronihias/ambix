/*
  ==============================================================================

   Main symbol/entry for juce plugins

  ==============================================================================
*/

#ifndef JUCE_PLUGIN_MAIN_H_INCLUDED
#define JUCE_PLUGIN_MAIN_H_INCLUDED

#include "AppConfig.h"
// #include "JucePluginCharacteristics.h"

#include "modules/juce_audio_basics/juce_audio_basics.h"
#include "modules/juce_audio_devices/juce_audio_devices.h"
#include "modules/juce_audio_formats/juce_audio_formats.h"
#include "modules/juce_audio_processors/juce_audio_processors.h"
#include "modules/juce_audio_utils/juce_audio_utils.h"
#include "modules/juce_core/juce_core.h"
#include "modules/juce_data_structures/juce_data_structures.h"
#include "modules/juce_events/juce_events.h"
#include "modules/juce_graphics/juce_graphics.h"
#include "modules/juce_gui_basics/juce_gui_basics.h"
#include "modules/juce_gui_extra/juce_gui_extra.h"

#include "modules/juce_audio_plugin_client/utility/juce_CheckSettingMacros.h"

#if ! DONT_SET_USING_JUCE_NAMESPACE
// If your code uses a lot of JUCE classes, then this will obviously save you
// a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE.
using namespace juce;
#endif


#endif // JUCE_PLUGIN_MAIN_H_INCLUDED

