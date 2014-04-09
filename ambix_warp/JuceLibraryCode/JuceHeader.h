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

#ifndef __APPHEADERFILE_NM3SVY__
#define __APPHEADERFILE_NM3SVY__

#include "AppConfig.h"
#include "modules/juce_audio_basics/juce_audio_basics.h"
#include "modules/juce_audio_formats/juce_audio_formats.h"
#include "modules/juce_audio_plugin_client/juce_audio_plugin_client.h"
#include "modules/juce_audio_processors/juce_audio_processors.h"
#include "modules/juce_core/juce_core.h"
#include "modules/juce_data_structures/juce_data_structures.h"
#include "modules/juce_events/juce_events.h"
#include "modules/juce_graphics/juce_graphics.h"
#include "modules/juce_gui_basics/juce_gui_basics.h"
#include "modules/juce_gui_extra/juce_gui_extra.h"
#include "modules/juce_opengl/juce_opengl.h"

#if ! DONT_SET_USING_JUCE_NAMESPACE
 // If your code uses a lot of JUCE classes, then this will obviously save you
 // a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE.
 using namespace juce;
#endif

#ifndef VERSION
    #define VERSION 1.0.0
#endif

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace ProjectInfo
{
    const char* const  projectName    = "ambix_warp";
    const char* const  versionString  = QUOTE(VERSION);
    const int          versionNumber  = 0x10000;
}

#endif   // __APPHEADERFILE_NM3SVY__
