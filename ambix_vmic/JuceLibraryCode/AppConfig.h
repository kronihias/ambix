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

#ifndef __JUCE_APPCONFIG_RUPSZT__
#define __JUCE_APPCONFIG_RUPSZT__

//==============================================================================
// [BEGIN_USER_CODE_SECTION]

#ifndef AMBI_ORDER
    #define AMBI_ORDER 5   // N
#endif

#ifndef NUM_FILTERS_VMIC
    #define NUM_FILTERS_VMIC 8 // number of directional filters that can be controlled
#endif


#define PARAMS_PER_FILTER 6

/////////////////////////////
// this is the formular for 3d... not meant to be changed!
#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

// [END_USER_CODE_SECTION]

//==============================================================================
#define JUCE_MODULE_AVAILABLE_juce_audio_basics             1
#define JUCE_MODULE_AVAILABLE_juce_audio_plugin_client      1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors         1
#define JUCE_MODULE_AVAILABLE_juce_core                     1
#define JUCE_MODULE_AVAILABLE_juce_data_structures          1
#define JUCE_MODULE_AVAILABLE_juce_events                   1
#define JUCE_MODULE_AVAILABLE_juce_graphics                 1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics               1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra                1
#define JUCE_MODULE_AVAILABLE_juce_opengl                   1

//==============================================================================
// juce_audio_processors flags:

#ifndef    JUCE_PLUGINHOST_VST
 //#define JUCE_PLUGINHOST_VST
#endif

#ifndef    JUCE_PLUGINHOST_AU
 //#define JUCE_PLUGINHOST_AU
#endif

//==============================================================================
// juce_core flags:

#ifndef    JUCE_FORCE_DEBUG
 //#define JUCE_FORCE_DEBUG
#endif

#ifndef    JUCE_LOG_ASSERTIONS
 //#define JUCE_LOG_ASSERTIONS
#endif

#ifndef    JUCE_CHECK_MEMORY_LEAKS
 //#define JUCE_CHECK_MEMORY_LEAKS
#endif

#ifndef    JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
 //#define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
#endif

#ifndef    JUCE_INCLUDE_ZLIB_CODE
 //#define JUCE_INCLUDE_ZLIB_CODE
#endif

//==============================================================================
// juce_graphics flags:

#ifndef    JUCE_USE_COREIMAGE_LOADER
 //#define JUCE_USE_COREIMAGE_LOADER
#endif

#ifndef    JUCE_USE_DIRECTWRITE
 //#define JUCE_USE_DIRECTWRITE
#endif

//==============================================================================
// juce_gui_basics flags:

#ifndef    JUCE_ENABLE_REPAINT_DEBUGGING
 //#define JUCE_ENABLE_REPAINT_DEBUGGING
#endif

#ifndef    JUCE_USE_XSHM
 //#define JUCE_USE_XSHM
#endif

#ifndef    JUCE_USE_XRENDER
 //#define JUCE_USE_XRENDER
#endif

#ifndef    JUCE_USE_XCURSOR
 //#define JUCE_USE_XCURSOR
#endif

//==============================================================================
// juce_gui_extra flags:

#ifndef    JUCE_WEB_BROWSER
 //#define JUCE_WEB_BROWSER
#endif


//==============================================================================
// Audio plugin settings..

#ifndef  JucePlugin_Build_VST
 #define JucePlugin_Build_VST              1
#endif
#ifndef  JucePlugin_Build_VST3
 #define JucePlugin_Build_VST3             0
#endif
#ifndef  JucePlugin_Build_LV2
 #define JucePlugin_Build_LV2              0
#endif
#ifndef  JucePlugin_Build_AU
 #define JucePlugin_Build_AU               0
#endif
#ifndef  JucePlugin_Build_RTAS
 #define JucePlugin_Build_RTAS             0
#endif
#ifndef  JucePlugin_Build_AAX
 #define JucePlugin_Build_AAX              0
#endif
#ifndef  JucePlugin_Name
 #define JucePlugin_Name                   "ambix_vmic"
#endif
#ifndef  JucePlugin_Desc
 #define JucePlugin_Desc                   "ambix_vmic"
#endif
#ifndef  JucePlugin_Manufacturer
 #define JucePlugin_Manufacturer           "kronlachner"
#endif
#ifndef  JucePlugin_ManufacturerCode
 #define JucePlugin_ManufacturerCode       'Kron'
#endif
#ifndef  JucePlugin_PluginCode
 #define JucePlugin_PluginCode             'AmVm'
#endif
#ifndef  JucePlugin_IsSynth
 #define JucePlugin_IsSynth                0
#endif
#ifndef  JucePlugin_WantsMidiInput
 #define JucePlugin_WantsMidiInput         0
#endif
#ifndef  JucePlugin_ProducesMidiOutput
 #define JucePlugin_ProducesMidiOutput     0
#endif
#ifndef  JucePlugin_IsMidiEffect
 #define JucePlugin_IsMidiEffect           0
#endif
#ifndef  JucePlugin_SilenceInProducesSilenceOut
 #define JucePlugin_SilenceInProducesSilenceOut  1
#endif
#ifndef  JucePlugin_EditorRequiresKeyboardFocus
 #define JucePlugin_EditorRequiresKeyboardFocus  0
#endif
#ifndef  JucePlugin_Version
 #define JucePlugin_Version                1.0.0
#endif
#ifndef  JucePlugin_VersionCode
 #define JucePlugin_VersionCode            0x10000
#endif
#ifndef  JucePlugin_VersionString
 #define JucePlugin_VersionString          "1.0.0"
#endif
#ifndef  JucePlugin_VSTUniqueID
 #define JucePlugin_VSTUniqueID            JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_VSTCategory
 #define JucePlugin_VSTCategory            kPlugCategEffect
#endif
#ifndef  JucePlugin_AUMainType
 #define JucePlugin_AUMainType             kAudioUnitType_Effect
#endif
#ifndef  JucePlugin_AUSubType
 #define JucePlugin_AUSubType              JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_AUExportPrefix
 #define JucePlugin_AUExportPrefix         ambix_vmicAU
#endif
#ifndef  JucePlugin_AUExportPrefixQuoted
 #define JucePlugin_AUExportPrefixQuoted   "ambix_vmicAU"
#endif
#ifndef  JucePlugin_AUManufacturerCode
 #define JucePlugin_AUManufacturerCode     JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_CFBundleIdentifier
 #define JucePlugin_CFBundleIdentifier     com.kronlachner.ambix_vmic
#endif
#ifndef  JucePlugin_RTASCategory
 #define JucePlugin_RTASCategory           ePlugInCategory_None
#endif
#ifndef  JucePlugin_RTASManufacturerCode
 #define JucePlugin_RTASManufacturerCode   JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_RTASProductId
 #define JucePlugin_RTASProductId          JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_RTASDisableBypass
 #define JucePlugin_RTASDisableBypass      0
#endif
#ifndef  JucePlugin_RTASDisableMultiMono
 #define JucePlugin_RTASDisableMultiMono   0
#endif
#ifndef  JucePlugin_AAXIdentifier
 #define JucePlugin_AAXIdentifier          com.kronlachner.ambix_vmic
#endif
#ifndef  JucePlugin_AAXManufacturerCode
 #define JucePlugin_AAXManufacturerCode    JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_AAXProductId
 #define JucePlugin_AAXProductId           JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_AAXPluginId
 #define JucePlugin_AAXPluginId            JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_AAXCategory
 #define JucePlugin_AAXCategory            AAX_ePlugInCategory_Dynamics
#endif
#ifndef  JucePlugin_AAXDisableBypass
 #define JucePlugin_AAXDisableBypass       0
#endif

#ifndef JucePlugin_LV2URI
  #define JucePlugin_LV2URI               JucePlugin_Name
#endif
#ifndef JucePlugin_WantsLV2State
  #define JucePlugin_WantsLV2State        1
#endif
#ifndef JucePlugin_WantsLV2TimePos
  #define JucePlugin_WantsLV2TimePos      1
#endif

#endif  // __JUCE_APPCONFIG_RUPSZT__
