cmake_minimum_required(VERSION 2.8)

################################################
#set name
PROJECT(ambix_plugin_suite)

################################################
#version
INCLUDE(VERSION)
ADD_DEFINITIONS(-DVERSION=${VERSION})

################################################
#choices at compile time
SET(AMBI_ORDER 5 CACHE STRING "Ambisonic Order")
SET(WITH_ADVANCED_CONTROL TRUE CACHE BOOL "movement speed automation control for encoders")
SET(WITH_OSC TRUE CACHE BOOL "build in OSC functionallity")
SET(NUM_OUTPUTS_DECODER 48 CACHE STRING "output channels for decoder")
SET(NUM_FILTERS 2 CACHE STRING "number of directional filters that can be controlled for directional_loudness")
SET(NUM_FILTERS_VMIC 4 CACHE STRING "number of directional filters (==outputs) for vmic")
SET(BUILD_STANDALONE TRUE CACHE BOOL "build standalone application (for those with editor)")

SET(BUILD_VST TRUE CACHE BOOL "build VST plugins")
IF (APPLE)
	SET(BUILD_AU FALSE CACHE BOOL "build AudioUnit plugins (OSX only)")
	MARK_AS_ADVANCED(BUILD_AU) # AU is not working correctly... and AU SDK has to be modified as well.
ENDIF (APPLE)
IF(UNIX AND NOT APPLE AND NOT ANDROID)
	SET(BUILD_LV2 FALSE CACHE BOOL "build LV2 plugins")
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

IF(BUILD_STANDALONE)

	IF(UNIX AND NOT APPLE AND NOT ANDROID)
		SET(JUCE_JACK TRUE CACHE BOOL "build standalone app with Jack support")
		SET(JUCE_ALSA TRUE CACHE BOOL "build standalone app with ALSA support")
	ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

	IF(WIN32)
		SET(JUCE_ASIO TRUE CACHE BOOL "build standalone app with ASIO support (you need the Steinberg ASIO SDK)")
	ENDIF(WIN32)

ENDIF(BUILD_STANDALONE)



#########
# preprocessor flags

ADD_DEFINITIONS(-DAMBI_ORDER=${AMBI_ORDER})

# boolean need to be treated special
IF(WITH_ADVANCED_CONTROL)
	ADD_DEFINITIONS(-DWITH_ADVANCED_CONTROL=1)
ELSEIF(WITH_ADVANCED_CONTROL)
	ADD_DEFINITIONS(-DWITH_ADVANCED_CONTROL=0)
ENDIF(WITH_ADVANCED_CONTROL)

IF(WITH_OSC)
	ADD_DEFINITIONS(-DWITH_OSC=1)
ELSEIF(WITH_OSC)
	ADD_DEFINITIONS(-DWITH_OSC=0)
ENDIF(WITH_OSC)

ADD_DEFINITIONS(-DNUM_OUTPUTS=${NUM_OUTPUTS_DECODER})
ADD_DEFINITIONS(-DNUM_FILTERS=${NUM_FILTERS})
ADD_DEFINITIONS(-DNUM_FILTERS_VMIC=${NUM_FILTERS_VMIC})

IF(BUILD_VST)
	ADD_DEFINITIONS(-DJucePlugin_Build_VST=1)
ELSEIF(BUILD_VST)
	ADD_DEFINITIONS(-DJucePlugin_Build_VST=0)
ENDIF(BUILD_VST)

IF(BUILD_AU)
	ADD_DEFINITIONS(-DJucePlugin_Build_AU=1)
ELSEIF(BUILD_AU)
	ADD_DEFINITIONS(-DJucePlugin_Build_AU=0)
ENDIF(BUILD_AU)

IF(BUILD_LV2)
	ADD_DEFINITIONS(-DJucePlugin_Build_LV2=1)
ELSEIF(BUILD_LV2)
	ADD_DEFINITIONS(-DJucePlugin_Build_LV2=0)
ENDIF(BUILD_LV2)

IF (JUCE_JACK)
	ADD_DEFINITIONS(-DJUCE_JACK=1)
ENDIF (JUCE_JACK)

IF (JUCE_ALSA)
	ADD_DEFINITIONS(-DJUCE_ALSA=1)
ENDIF (JUCE_ALSA)

IF (JUCE_ASIO)
	ADD_DEFINITIONS(-DJUCE_ASIO=1)
  
  SET(ASIO_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK2.3 CACHE FILEPATH "specify the path of the ASIO 2.3 SDK")
  INCLUDE_DIRECTORIES(BEFORE ${ASIO_SDK_DIR}/common)
  
ENDIF (JUCE_ASIO)

IF (WITH_ZITA_CONVOLVER)
	ADD_DEFINITIONS(-DUSE_ZITA_CONVOLVER=1)
ENDIF(WITH_ZITA_CONVOLVER)

# set release and optimization flags
# SET(CMAKE_C_FLAGS_RELEASE "-Os") # use small size?
# SET(CMAKE_CXX_FLAGS_RELEASE "-Os") # use small size?



ADD_DEFINITIONS(-DNDEBUG=1)
ADD_DEFINITIONS(-D_NDEBUG=1)

#save path for later
SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET (BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

IF(APPLE)
	 SET(CMAKE_BUILD_TYPE MinSizeRel)
	 
   SET( CMAKE_OSX_ARCHITECTURES i386;x86_64 )
   SET( CMAKE_OSX_DEPLOYMENT_TARGET 10.5 )
   
   SET ( LINK_FLAGS MINSIZEREL "-dead_strip" )
   # SET( CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "-dead_strip" )
   # SET( CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "-dead_strip" )
   # SET( CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-dead_strip" )
ELSE (APPLE)
	 SET(CMAKE_BUILD_TYPE Release)
	 
ENDIF (APPLE)
################################################
# VST SDK
IF (BUILD_VST)
	SET(VSTSDK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vstsdk2.4 CACHE FILEPATH "specify the path of the VST 2.4 SDK")

	INCLUDE_DIRECTORIES(BEFORE ${VSTSDK_INCLUDE_DIR})

	SET(VSTSDK_PLUGIN_SOURCE
	    ${VSTSDK_INCLUDE_DIR}/public.sdk/source/vst2.x/audioeffectx.cpp
	    ${VSTSDK_INCLUDE_DIR}/public.sdk/source/vst2.x/audioeffect.cpp
	    ${VSTSDK_INCLUDE_DIR}/public.sdk/source/vst2.x/vstplugmain.cpp
	)
ENDIF(BUILD_VST)

################################################
# AUDIO UNIT
IF (BUILD_AU)
	## you have to get the AU SDK especially and even patch it! thank you apple!
	## http://www.juce.com/forum/topic/aus-xcode
	## or here: http://teragonaudio.com/article/Building-AudioUnits-with-modern-Mac-OSX.html
	
	SET(AU_INCLUDE_DIR /Applications/Xcode.app/Contents/Developer/Extras/CoreAudio CACHE FILEPATH "specify the path of the CoreAudio SDK")
	MARK_AS_ADVANCED(AU_INCLUDE_DIR)
	
	INCLUDE_DIRECTORIES(BEFORE ${AU_INCLUDE_DIR}/PublicUtility)
	INCLUDE_DIRECTORIES(BEFORE ${AU_INCLUDE_DIR}/AudioUnits/AUPublic/Utility)
	INCLUDE_DIRECTORIES(BEFORE ${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase)
	INCLUDE_DIRECTORIES(BEFORE ${AU_INCLUDE_DIR}/AudioUnits/AUPublic/OtherBases)
	INCLUDE_DIRECTORIES(BEFORE ${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUCarbonViewBase)
	INCLUDE_DIRECTORIES(BEFORE ${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUViewBase)

	SET(AU_PLUGIN_SOURCE
		${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase/AUBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/Utility/AUBuffer.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewControl.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewDispatch.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase/AUDispatch.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/OtherBases/AUEffectBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase/AUInputElement.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/OtherBases/AUMIDIBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/OtherBases/AUMIDIEffectBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/OtherBases/AUOutputBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase/AUOutputElement.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase/AUScopeElement.cpp
    	${AU_INCLUDE_DIR}/PublicUtility/CAAudioChannelLayout.cpp
    	${AU_INCLUDE_DIR}/PublicUtility/CAAUParameter.cpp
    	${AU_INCLUDE_DIR}/PublicUtility/CAMutex.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUCarbonViewBase/CarbonEventHandler.cpp
    	${AU_INCLUDE_DIR}/PublicUtility/CAStreamBasicDescription.cpp
    	${AU_INCLUDE_DIR}/PublicUtility/CAVectorUnit.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/AUBase/ComponentBase.cpp
    	${AU_INCLUDE_DIR}/AudioUnits/AUPublic/OtherBases/MusicDeviceBase.cpp
	)
ENDIF(BUILD_AU)


################################################
# INCLUDES

INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/JUCE )

INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/Ressources )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/SphericalHarmonic )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/SphFilter )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/LegendreU )


################################################
# additional cmake find packages
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Eigen3 REQUIRED)

find_package(liblo REQUIRED)
find_package(libsoxr REQUIRED)

IF (WITH_ZITA_CONVOLVER)
	find_package(FFTW3 COMPONENTS single)
ENDIF (WITH_ZITA_CONVOLVER)

include_directories(${EIGEN3_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR} ${LIBLO_INCLUDE_DIRS} ${LIBSOXR_INCLUDE_DIRS})


################################################
# this are the link libraries for juce


IF(APPLE)
   FIND_LIBRARY(ACCELERATE_LIBRARY Accelerate)
   FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY AudioToolbox)
   FIND_LIBRARY(AUDIOUNIT_LIBRARY AudioUnit)
   FIND_LIBRARY(CARBON_LIBRARY Carbon)
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
   FIND_LIBRARY(COREMIDI_LIBRARY CoreMIDI)
   FIND_LIBRARY(DISCRECORDING_LIBRARY DiscRecording)
   FIND_LIBRARY(IOKIT_LIBRARY IOKit)
   FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
   FIND_LIBRARY(QTKIT_LIBRARY QTKit)
   FIND_LIBRARY(QUARTZCORE_LIBRARY QuartzCore)
#   FIND_LIBRARY(QUICKTIME_LIBRARY QuickTime)
   FIND_LIBRARY(WEBKIT_LIBRARY WebKit)
   MARK_AS_ADVANCED (ACCELERATE_LIBRARY AUDIOTOOLBOX_LIBRARY AUDIOUNIT_LIBRARY CARBON_LIBRARY COCOA_LIBRARY COREAUDIO_LIBRARY COREMIDI_LIBRARY COREMIDI_LIBRARY DISCRECORDING_LIBRARY IOKIT_LIBRARY OPENGL_LIBRARY QTKIT_LIBRARY QUARTZCORE_LIBRARY QUICKTIME_LIBRARY WEBKIT_LIBRARY)
   SET(JUCE_LIBRARIES ${ACCELERATE_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} ${AUDIOUNIT_LIBRARY} ${CARBON_LIBRARY} ${COCOA_LIBRARY} ${COREAUDIO_LIBRARY} ${COREMIDI_LIBRARY} ${COREMIDI_LIBRARY} ${DISCRECORDING_LIBRARY} ${IOKIT_LIBRARY} ${OPENGL_LIBRARY} ${QTKIT_LIBRARY} ${QUARTZCORE_LIBRARY} ${QUICKTIME_LIBRARY} ${WEBKIT_LIBRARY})

	ADD_DEFINITIONS(-DJUCE_MAC=1)
ENDIF (APPLE)


IF(UNIX AND NOT APPLE AND NOT ANDROID)

   ADD_DEFINITIONS(-DLINUX=1)

   FIND_PACKAGE    ( OpenGL REQUIRED )
   FIND_PACKAGE	   (GLUT REQUIRED)
   FIND_LIBRARY    ( X11_LIBRARY X11 )
   FIND_LIBRARY    ( XEXT_LIBRARY Xext )
   FIND_LIBRARY    ( XINERAMA_LIBRARY Xinerama )
   FIND_LIBRARY    ( ASOUND_LIBRARY asound )
   FIND_LIBRARY    ( DL_LIBRARY dl )
   FIND_LIBRARY    ( FREETYPE_LIBRARY freetype )
   FIND_LIBRARY    ( PTHREAD_LIBRARY pthread )
   FIND_LIBRARY    ( RT_LIBRARY rt )
   MARK_AS_ADVANCED(GL_LIBRARY GLUT_LIBRARY X11_LIBRARY XEXT_LIBRARY XINERAMA_LIBRARY ASOUND_LIBRARY DL_LIBRARY FREETYPE_LIBRARY PTHREAD_LIBRARY RT_LIBRARY FFTW3F_LIBRARY)
   
   FIND_PACKAGE    ( FreeType REQUIRED )
   
   INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})

   SET ( JUCE_LIBRARIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_LIBRARY} ${XEXT_LIBRARY} ${XINERAMA_LIBRARY} ${ASOUND_LIBRARY} ${DL_LIBRARY} ${FREETYPE_LIBRARY} ${PTHREAD_LIBRARY} ${RT_LIBRARY} )
   

   FIND_LIBRARY    ( LIBLO_LIBRARY lo )
   FIND_LIBRARY    ( LIBSOXR_LIBRARY soxr )
   IF(WITH_ZITA_CONVOLVER)
	    FIND_LIBRARY    ( ZITA_CONVOLVER_LIBRARY zita-convolver )
   		FIND_LIBRARY    ( FFTW3F_LIBRARY fftw3f )
   ENDIF(WITH_ZITA_CONVOLVER)	
   
   ADD_DEFINITIONS(-DJUCE_LINUX=1)
   
   #add sse
   set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_ALL} -msse3")
	 set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_ALL} -msse3")
   
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)


IF(WIN32)
  set(THREADS_USE_PTHREADS_WIN32 true)
	find_package(Threads REQUIRED)
  FIND_LIBRARY    ( FFTW3F_LIBRARY fftw3f )
  
  include_directories(${THREADS_PTHREADS_INCLUDE_DIR})
  ADD_DEFINITIONS(-DJUCE_WINDOWS=1)
ENDIF(WIN32)

################################################
# juce source modules

IF(APPLE)

	SET ( JUCE_SOURCE
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_basics/juce_audio_basics.mm
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_devices/juce_audio_devices.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_formats/juce_audio_formats.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/utility/juce_PluginUtilities.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_processors/juce_audio_processors.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_utils/juce_audio_utils.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_box2d/juce_box2d.cpp
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_browser_plugin_client/juce_browser_plugin.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_core/juce_core.mm
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_cryptography/juce_cryptography.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_data_structures/juce_data_structures.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_events/juce_events.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_graphics/juce_graphics.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_gui_basics/juce_gui_basics.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_gui_extra/juce_gui_extra.mm
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_opengl/juce_opengl.mm
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_video/juce_video.mm
	)
	SET ( JUCE_PLUGIN_CLIENT_SOURCE
	)

	IF (BUILD_VST)
		LIST ( APPEND JUCE_PLUGIN_CLIENT_SOURCE 
			${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/VST/juce_VST_Wrapper.mm
			${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/VST/juce_VST_Wrapper.cpp
		)
	ENDIF (BUILD_VST)

	IF (BUILD_AU)
		LIST ( APPEND JUCE_PLUGIN_CLIENT_SOURCE ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/AU/juce_AU_Wrapper.mm )
	ENDIF (BUILD_AU)

	SET ( JUCE_STANDALONE_SOURCE
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_devices/juce_audio_devices.mm
	)

ELSE(APPLE)

	SET ( JUCE_SOURCE
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_basics/juce_audio_basics.cpp
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_devices/juce_audio_devices.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_formats/juce_audio_formats.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/utility/juce_PluginUtilities.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_processors/juce_audio_processors.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_utils/juce_audio_utils.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_box2d/juce_box2d.cpp
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_browser_plugin_client/juce_browser_plugin.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_core/juce_core.cpp
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_cryptography/juce_cryptography.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_data_structures/juce_data_structures.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_events/juce_events.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_graphics/juce_graphics.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_gui_basics/juce_gui_basics.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_gui_extra/juce_gui_extra.cpp
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_opengl/juce_opengl.cpp
		#        ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_video/juce_video.cpp
	)

	SET ( JUCE_PLUGIN_CLIENT_SOURCE
	)

	IF (BUILD_VST)
		LIST ( APPEND JUCE_PLUGIN_CLIENT_SOURCE ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/VST/juce_VST_Wrapper.cpp )
	ENDIF (BUILD_VST)

	IF (BUILD_LV2)
		LIST ( APPEND JUCE_PLUGIN_CLIENT_SOURCE ${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_plugin_client/LV2/juce_LV2_Wrapper.cpp )
	ENDIF (BUILD_LV2)
	
	SET ( JUCE_STANDALONE_SOURCE
		${CMAKE_HOME_DIRECTORY}/JUCE/modules/juce_audio_devices/juce_audio_devices.cpp
	)
	
ENDIF(APPLE)


	
################################################
# add all subprojects (plugins)

ADD_SUBDIRECTORY ( JUCE ) # build static juce first
ADD_SUBDIRECTORY ( ambix_binaural )
ADD_SUBDIRECTORY ( ambix_decoder )
ADD_SUBDIRECTORY ( ambix_converter )
ADD_SUBDIRECTORY ( ambix_directional_loudness )
ADD_SUBDIRECTORY ( ambix_encoder )
ADD_SUBDIRECTORY ( ambix_maxre )
ADD_SUBDIRECTORY ( ambix_mirror )
ADD_SUBDIRECTORY ( ambix_rotator )
ADD_SUBDIRECTORY ( ambix_rotator_z )
ADD_SUBDIRECTORY ( ambix_vmic )
ADD_SUBDIRECTORY ( ambix_warp )
# ADD_SUBDIRECTORY ( ambix_hoafilter )
ADD_SUBDIRECTORY ( ambix_widening )

