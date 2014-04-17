/*

    IMPORTANT! This file is auto-generated each time you save your
    project - if you alter its contents, your changes may be overwritten!

    There's a section below where you can add your own custom code safely, and the
    Introjucer will preserve the contents of that block, but the best way to change
    any of these definitions is by using the Introjucer's project settings.

    Any commented-out settings will assume their default values.

*/

#ifndef __JUCE_APPCONFIG_G4U1ZF__
#define __JUCE_APPCONFIG_G4U1ZF__

//==============================================================================
// [BEGIN_USER_CODE_SECTION]

// (You can add your own code in this section, and the Introjucer will not overwrite it)

#ifndef    AMBI_ORDER
    #define AMBI_ORDER 5   // N
#endif

#ifndef    BINAURAL_DECODER
    #define BINAURAL_DECODER 1  // if 0 use as normal decoder without convolution
    #warning BINAURAL_DECODER was not defined - falling back to TRUE
#endif

#ifndef    NUM_OUTPUTS
    #define NUM_OUTPUTS 48 // number of outputs for non-binaural decoder
#endif

#ifdef Plugin_Name
    #undef Plugin_Name
#endif

// quotes for our plugin name
#define QU(x) #x
#define QUH(x) QU(x)

// get single quotes around our identifier - ugly, but is there another way?
#define APOS           '
#define CHAR2(a,b,c)   a##b##c
#define CHAR1(a,b,c)   CHAR2(a,b,c)
#define CHAR(x)        CHAR1(APOS,x,APOS)

#define PASTER(x,y) x ## y
#define EVALUATOR(x,y)  PASTER(x,y)
#define NAME(fun) EVALUATOR(fun, AMBI_ORDER)

// generate plugin name with ambi order
#if BINAURAL_DECODER
    #define Plugin_Name                   ambix_binaural_o
    #define PluginCode         NAME(ABi)
#else
    #define Plugin_Name                   ambix_decoder_o
    #define PluginCode         NAME(ADe)
#endif


#define JucePlugin_Name                   QUH(Plugin_Name) QUH(AMBI_ORDER)

#define JucePlugin_PluginCode             CHAR(PluginCode)

/* definition to expand macro then apply to pragma message */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

// debugging
//#pragma message(VAR_NAME_VALUE(BINAURAL_DECODER))
//#pragma message(VAR_NAME_VALUE(JucePlugin_Name))
//#pragma message(VAR_NAME_VALUE(JucePlugin_PluginCode))

//////////////////////////////////////////////

// this is the formular for 3d... not meant to be changed!
#define AMBI_CHANNELS (AMBI_ORDER + 1) * (AMBI_ORDER + 1) // (N+1)^2 for 3D!

#ifndef _WIN32
// #define USE_ZITA_CONVOLVER // zita convolver is not compatible with Steinberg VST SDK! use only for LV2 version
#endif

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
 //#define JUCE_ENABLE_REPAINT_DEBUGGING 1
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
 #define JucePlugin_Name                   "ambix_binaural"
#endif
#ifndef  JucePlugin_Desc
 #define JucePlugin_Desc                   "ambix_binaural"
#endif
#ifndef  JucePlugin_Manufacturer
 #define JucePlugin_Manufacturer           "kronlachner"
#endif
#ifndef  JucePlugin_ManufacturerCode
 #define JucePlugin_ManufacturerCode       'Kron'
#endif
#ifndef  JucePlugin_PluginCode
 #define JucePlugin_PluginCode             'ABi5'
#endif
#ifndef  JucePlugin_MaxNumInputChannels
 #define JucePlugin_MaxNumInputChannels    AMBI_CHANNELS
#endif
#ifndef  JucePlugin_MaxNumOutputChannels

    #if BINAURAL_DECODER
        #define JucePlugin_MaxNumOutputChannels   2
    #else
        #define JucePlugin_MaxNumOutputChannels   NUM_OUTPUTS
    #endif
 
#endif
#ifndef  JucePlugin_PreferredChannelConfigurations
 
    #if BINAURAL_DECODER
        #define JucePlugin_PreferredChannelConfigurations  {AMBI_CHANNELS, 2}
    #else
            #define JucePlugin_PreferredChannelConfigurations  {AMBI_CHANNELS, NUM_OUTPUTS}
    #endif
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
 #define JucePlugin_AUExportPrefix         ambix_binauralAU
#endif
#ifndef  JucePlugin_AUExportPrefixQuoted
 #define JucePlugin_AUExportPrefixQuoted   "ambix_binauralAU"
#endif
#ifndef  JucePlugin_AUManufacturerCode
 #define JucePlugin_AUManufacturerCode     JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_CFBundleIdentifier
 #define JucePlugin_CFBundleIdentifier     com.kronlachner.ambix_binaural
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
 #define JucePlugin_AAXIdentifier          com.kronlachner.ambix_binaural
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

#endif  // __JUCE_APPCONFIG_G4U1ZF__
