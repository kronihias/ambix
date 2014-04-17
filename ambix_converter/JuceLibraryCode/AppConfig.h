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

#ifndef __JUCE_APPCONFIG_XARBCB__
#define __JUCE_APPCONFIG_XARBCB__

//==============================================================================
// [BEGIN_USER_CODE_SECTION]

//////////////////////////////////////////////
// define the Ambisonics Order here!!!

#ifndef AMBI_ORDER
    #define AMBI_ORDER 5   // N
#endif


#define Plugin_Name                   ambix_converter_o
#define Plugin_Code                   ACo


#define QU(x) #x
#define QUH(x) QU(x)
#define JucePlugin_Name                   QUH(Plugin_Name) QUH(AMBI_ORDER)

// get single quotes around our identifier - ugly, but is there another way?
#define APOS           '
#define CHAR2(a,b,c)   a##b##c
#define CHAR1(a,b,c)   CHAR2(a,b,c)
#define CHAR(x)        CHAR1(APOS,x,APOS)

#define PASTER(x,y) x ## y
#define EVALUATOR(x,y)  PASTER(x,y)
#define ADDORDER(fun) EVALUATOR(fun, AMBI_ORDER)

#define PluginCode                        ADDORDER(Plugin_Code)
#define JucePlugin_PluginCode             CHAR(PluginCode)

//////////////////////////////////////////////

// this is the formular for 3d... not meant to be changed!
#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

// [END_USER_CODE_SECTION]

//==============================================================================
#define JUCE_MODULE_AVAILABLE_juce_audio_basics             1
#define JUCE_MODULE_AVAILABLE_juce_audio_devices            1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats            1
#define JUCE_MODULE_AVAILABLE_juce_audio_plugin_client      1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors         1
#define JUCE_MODULE_AVAILABLE_juce_core                     1
#define JUCE_MODULE_AVAILABLE_juce_data_structures          1
#define JUCE_MODULE_AVAILABLE_juce_events                   1
#define JUCE_MODULE_AVAILABLE_juce_graphics                 1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics               1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra                1

//==============================================================================
// juce_audio_devices flags:

#ifndef    JUCE_ASIO
 //#define JUCE_ASIO
#endif

#ifndef    JUCE_WASAPI
 //#define JUCE_WASAPI
#endif

#ifndef    JUCE_DIRECTSOUND
 //#define JUCE_DIRECTSOUND
#endif

#ifndef    JUCE_ALSA
 //#define JUCE_ALSA
#endif

#ifndef    JUCE_JACK
 //#define JUCE_JACK
#endif

#ifndef    JUCE_USE_ANDROID_OPENSLES
 //#define JUCE_USE_ANDROID_OPENSLES
#endif

#ifndef    JUCE_USE_CDREADER
 //#define JUCE_USE_CDREADER
#endif

#ifndef    JUCE_USE_CDBURNER
 //#define JUCE_USE_CDBURNER
#endif

//==============================================================================
// juce_audio_formats flags:

#ifndef    JUCE_USE_FLAC
 //#define JUCE_USE_FLAC
#endif

#ifndef    JUCE_USE_OGGVORBIS
 //#define JUCE_USE_OGGVORBIS
#endif

#ifndef    JUCE_USE_MP3AUDIOFORMAT
 //#define JUCE_USE_MP3AUDIOFORMAT
#endif

#ifndef    JUCE_USE_LAME_AUDIO_FORMAT
 //#define JUCE_USE_LAME_AUDIO_FORMAT
#endif

#ifndef    JUCE_USE_WINDOWS_MEDIA_FORMAT
 //#define JUCE_USE_WINDOWS_MEDIA_FORMAT
#endif

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
 #define JucePlugin_Name                   "ambix_converter"
#endif
#ifndef  JucePlugin_Desc
 #define JucePlugin_Desc                   "ambix_converter"
#endif
#ifndef  JucePlugin_Manufacturer
 #define JucePlugin_Manufacturer           "kronlachner"
#endif
#ifndef  JucePlugin_ManufacturerCode
 #define JucePlugin_ManufacturerCode       'Kron'
#endif
#ifndef  JucePlugin_PluginCode
 #define JucePlugin_PluginCode             'AMCO'
#endif
#ifndef  JucePlugin_MaxNumInputChannels
 #define JucePlugin_MaxNumInputChannels    AMBI_CHANNELS
#endif
#ifndef  JucePlugin_MaxNumOutputChannels
 #define JucePlugin_MaxNumOutputChannels   AMBI_CHANNELS
#endif
#ifndef  JucePlugin_PreferredChannelConfigurations
 #define JucePlugin_PreferredChannelConfigurations  {AMBI_CHANNELS, AMBI_CHANNELS}
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
#ifndef  JucePlugin_SilenceInProducesSilenceOut
 #define JucePlugin_SilenceInProducesSilenceOut  1
#endif
#ifndef  JucePlugin_EditorRequiresKeyboardFocus
 #define JucePlugin_EditorRequiresKeyboardFocus  1
#endif
#ifndef  JucePlugin_Version
 #define JucePlugin_Version                0.1.0
#endif
#ifndef  JucePlugin_VersionCode
 #define JucePlugin_VersionCode            0x100
#endif
#ifndef  JucePlugin_VersionString
 #define JucePlugin_VersionString          "0.1.0"
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
 #define JucePlugin_AUExportPrefix         ambix_converterAU
#endif
#ifndef  JucePlugin_AUExportPrefixQuoted
 #define JucePlugin_AUExportPrefixQuoted   "ambix_converterAU"
#endif
#ifndef  JucePlugin_AUManufacturerCode
 #define JucePlugin_AUManufacturerCode     JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_CFBundleIdentifier
 #define JucePlugin_CFBundleIdentifier     com.kronlachner.ambix_converter
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
 #define JucePlugin_AAXIdentifier          com.kronlachner.ambix_converter
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

#endif  // __JUCE_APPCONFIG_XARBCB__
