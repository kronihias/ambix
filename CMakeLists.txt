cmake_minimum_required (VERSION 3.13)
include(CheckIncludeFiles)

################################################
#set name
PROJECT(ambix_plugin_suite)

################################################
#version
file(READ VERSION VERSION)
ADD_DEFINITIONS(-DVERSION=${VERSION})

################################################
#choices at compile time
SET(AMBI_ORDER 5 CACHE STRING "Ambisonic Order")
SET(WITH_ADVANCED_CONTROL TRUE CACHE BOOL "movement speed automation control for encoders")
SET(WITH_OSC TRUE CACHE BOOL "build in OSC functionallity")
SET(NUM_OUTPUTS_DECODER 64 CACHE STRING "output channels for decoder")
SET(NUM_FILTERS 8 CACHE STRING "number of directional filters that can be controlled for directional_loudness")
SET(NUM_FILTERS_VMIC 8 CACHE STRING "number of directional filters (==outputs) for vmic")

SET(BUILD_VST TRUE CACHE BOOL "build VST plugins")
SET(BUILD_STANDALONE FALSE CACHE BOOL "build standalone application (for those with editor)")
SET(BUILD_LV2 FALSE CACHE BOOL "build LV2 plugins")


IF(UNIX AND NOT APPLE AND NOT ANDROID)
	SET(WITH_ZITA_CONVOLVER FALSE CACHE BOOL "build with zita-convolver (better performance under linux)")
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

################################################
# add JUCE

ADD_SUBDIRECTORY ( JUCE )

#########
# preprocessor flags

# need this to make the file picker work (legacy code)
ADD_DEFINITIONS(-DJUCE_MODAL_LOOPS_PERMITTED=1)

# we use some deprecated stuff, yeah...
SET(CMAKE_WARN_DEPRECATED OFF)
SET(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

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

ADD_DEFINITIONS(-DAMBI_ORDER=${AMBI_ORDER})

# formats
set (AMBIX_FORMATS "")

if (BUILD_VST)
    SET (VST2SDKPATH "~/SDKs/vstsdk2.4" CACHE PATH "Path to the VST2 SDK")
    # checking VST2 SDK
    if (VST2SDKPATH STREQUAL "/usr/include")
        CHECK_INCLUDE_FILES(pluginterfaces/vst2.x/aeffectx.h HAVE_VST2_HEADERS)
        IF (NOT HAVE_VST2_HEADERS)
            message (FATAL_ERROR "You have to specify the VST2SDKPATH variable with the path to the VST2 SDK if you want to build VST2 versions.")
        endif()
    endif()
    juce_set_vst2_sdk_path (${VST2SDKPATH})
    list (APPEND AMBIX_FORMATS VST)
    message ("-- Building VST2 versions")
endif(BUILD_VST)

if (BUILD_LV2)
    message ("-- Building LV2 versions")
    list (APPEND AMBIX_FORMATS LV2)
endif(BUILD_LV2)

if (BUILD_STANDALONE)
    message ("-- Building Standalone versions")
    list (APPEND AMBIX_FORMATS Standalone)
endif(BUILD_STANDALONE)


#save path for later
SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET (BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})


# set output path
set_directory_properties (PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD ON)
set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "${BIN_DIR}/vst")
set_directory_properties (PROPERTIES JUCE_LV2_COPY_DIR "${BIN_DIR}/lv2")

IF (APPLE)
    SET (CMAKE_BUILD_TYPE MinSizeRel)
    SET (CMAKE_OSX_ARCHITECTURES arm64;x86_64)
    SET (CMAKE_OSX_DEPLOYMENT_TARGET 10.9)

    SET (LINK_FLAGS MINSIZEREL "-dead_strip")
ELSE (APPLE)
    SET (CMAKE_BUILD_TYPE Release)
ENDIF (APPLE)


IF (UNIX)
    # this is to make the convolver build under linux happy
    SET (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -msse3")
    SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse3")
ENDIF (UNIX)

################################################
# INCLUDES

INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/JUCE )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules )

INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/Ressources )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/SphericalHarmonic )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/SphFilter )
INCLUDE_DIRECTORIES ( BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/common/LegendreU )


################################################
# additional cmake find packages
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Eigen3 REQUIRED)

IF (WITH_ZITA_CONVOLVER OR WIN32)
	find_package(FFTW3 COMPONENTS single)
ENDIF (WITH_ZITA_CONVOLVER OR WIN32)

IF(UNIX AND NOT APPLE AND NOT ANDROID)
	find_package(FFTW3 COMPONENTS single threads)
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

include_directories(${EIGEN3_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR})


################################################
# add all subprojects (plugins)

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
ADD_SUBDIRECTORY ( ambix_widening )

