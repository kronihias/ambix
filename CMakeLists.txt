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

SET(BUILD_VST TRUE CACHE BOOL "build VST2 plugins (fixed order, _oX naming)")
SET(BUILD_VST3 TRUE CACHE BOOL "build VST3 plugins (universal order, no _oX suffix)")
SET(BUILD_AU FALSE CACHE BOOL "build AudioUnit plugins (macOS only, universal order)")
SET(BUILD_STANDALONE FALSE CACHE BOOL "build standalone application (for those with editor)")
SET(BUILD_LV2 FALSE CACHE BOOL "build LV2 plugins")
SET(BUILD_VISUALIZER TRUE CACHE BOOL "build the cross-platform ambix_visualizer GUI app (companion to ambix_encoder)")
SET(BUILD_TESTHOST FALSE CACHE BOOL "build the ambix_testhost CLI used by tests/ to host VST3s with asymmetric main buses")
SET(MAX_AMBI_ORDER 7 CACHE STRING "Maximum ambisonic order for universal (VST3/AU) builds")


IF(UNIX AND NOT APPLE AND NOT ANDROID)
	SET(WITH_ZITA_CONVOLVER FALSE CACHE BOOL "build with zita-convolver (better performance under linux)")
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

################################################
# additional cmake find packages
# Set early — find_package(Jack) below uses our cmake/FindJack.cmake.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

################################################
# Apply local patches to the JUCE submodule.
#
# Patches must be applied before ADD_SUBDIRECTORY(JUCE): configuring JUCE
# compiles juceaide (which pulls in juce_graphics and other modules), so any
# patch that affects module sources has to be in place by then.
#
# Locate GNU patch. On Windows it ships with Git for Windows under usr/bin but
# isn't on PowerShell's PATH by default, so probe the usual install locations.
find_program(PATCH_EXECUTABLE
    NAMES patch
    HINTS
        "$ENV{ProgramFiles}/Git/usr/bin"
        "$ENV{ProgramFiles\(x86\)}/Git/usr/bin"
        "$ENV{LOCALAPPDATA}/Programs/Git/usr/bin"
    DOC "GNU patch executable used to apply JUCE patches")

if (NOT PATCH_EXECUTABLE)
    message(FATAL_ERROR
        "Could not find the `patch` executable, needed to apply the JUCE patches "
        "in JUCE_patches/. On Windows, install Git for Windows (which ships "
        "patch.exe in <install>/usr/bin) or add patch.exe to PATH.")
endif()

# Apply a patch idempotently: dry-run first so an already-applied patch on a
# re-configure is reported as a no-op instead of a hard failure. Applies with
# -p1 rooted at the JUCE submodule so a single patch can touch multiple files.
function(apply_juce_patch patch_file)
    set(juce_root "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    execute_process(
        COMMAND "${PATCH_EXECUTABLE}" --binary --forward --dry-run --silent
                -p1 -d "${juce_root}" -i "${patch_file}"
        RESULT_VARIABLE dry_result
        OUTPUT_VARIABLE dry_output
        ERROR_VARIABLE  dry_error)

    if (dry_result EQUAL 0)
        execute_process(
            COMMAND "${PATCH_EXECUTABLE}" --binary --forward
                    -p1 -d "${juce_root}" -i "${patch_file}"
            RESULT_VARIABLE apply_result
            OUTPUT_VARIABLE apply_output
            ERROR_VARIABLE  apply_error)
        if (NOT apply_result EQUAL 0)
            message(FATAL_ERROR
                "Failed to apply ${patch_file}:\n${apply_output}\n${apply_error}")
        endif()
        message(STATUS "Applied patch: ${patch_file}")
    else()
        # GNU patch reports the "no-op" condition with several different
        # phrasings depending on version: "already applied", "previously
        # applied", "Reversed (or previously applied)". Detect any.
        set(combined "${dry_output}${dry_error}")
        string(FIND "${combined}" "already applied"   already_idx)
        string(FIND "${combined}" "previously applied" previously_idx)
        string(FIND "${combined}" "Reversed"          reversed_idx)
        string(FIND "${combined}" "reversed"          reversed_lc_idx)
        if (already_idx GREATER -1
            OR previously_idx GREATER -1
            OR reversed_idx GREATER -1
            OR reversed_lc_idx GREATER -1)
            message(STATUS "Patch already applied: ${patch_file}")
        else()
            message(FATAL_ERROR
                "Patch ${patch_file} cannot be applied under ${juce_root}:\n"
                "${dry_output}\n${dry_error}")
        endif()
    endif()
endfunction()

# Implement IPAddress::getInterfaceBroadcastAddress on Windows (upstream JUCE
# leaves it as a TODO that returns 0.0.0.0) and add an explicit loopback
# broadcast in NetworkServiceDiscovery::Advertiser. Without this, Windows peers
# never appear on remote machines and same-host listeners miss advertisements
# (ambix_encoder ↔ ambix_visualizer discovery).
apply_juce_patch(
    "${CMAKE_CURRENT_SOURCE_DIR}/JUCE_patches/juce_network_discovery.patch")

# GCC 9 compatibility (e.g. Ubuntu 20.04): two source-level fixes for JUCE 8.
#  1. juce_graphics_Harfbuzz.cpp fakes __has_builtin(x)=1 on GCC < 10, which
#     makes libstdc++ 9's <utility> take a Clang-only builtin path. Include
#     <utility> before the fake.
#  2. A C-style cast in a braced initializer inside a constexpr template in
#     juce_Midi_linux.cpp ICEs GCC 9; use static_cast instead.
# Both changes are semantically neutral, but only apply them where they are
# needed so other configurations keep a pristine JUCE tree.
IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
    apply_juce_patch(
        "${CMAKE_CURRENT_SOURCE_DIR}/JUCE_patches/juce_gcc9_compat.patch")
ENDIF ()

################################################
# add JUCE

ADD_SUBDIRECTORY ( JUCE )

# Linux standalone host is vendored under common/standalone instead of patching
# JUCE. ambix owns the standalone window, the audio-settings UI (a "how many
# channels" selector instead of JUCE's stereo-pair list) and its own JACK device
# (ports sized to the plugin's channel count, no auto-connect, all ports active).
# Wired per Standalone target in Subproject.cmake via
# JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 (JUCE keeps JUCE_JACK=0, so there is no
# duplicate JACK type). Other platforms keep JUCE's stock standalone.
IF (UNIX AND NOT APPLE AND NOT ANDROID)
    find_package(Jack REQUIRED)
    set (AMBIX_VENDOR_STANDALONE TRUE)
    # The shared host (StandaloneFilterWindow.h) and channel-count selector
    # (AudioDeviceSelectorComponent.h) are header-only; only the app entry and
    # the JACK device are compiled units.
    set (AMBIX_STANDALONE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/common/standalone/StandaloneApp.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/standalone/JackAudioDevice.cpp")
    # Channel ceiling for the standalone device re-open + settings dialog. The
    # universal standalone exposes a full ambisonic(MAX_AMBI_ORDER) bus.
    math (EXPR AMBIX_STANDALONE_MAX_CHANNELS "(${MAX_AMBI_ORDER} + 1) * (${MAX_AMBI_ORDER} + 1)")
ENDIF ()

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

# AMBI_ORDER is set per-target in Subproject.cmake (fixed for VST2, MAX for universal)
ADD_DEFINITIONS(-DMAX_AMBI_ORDER=${MAX_AMBI_ORDER})

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

# Universal formats (VST3, AU, Standalone) - built separately via UNIVERSAL_BUILD in Subproject.cmake
set (AMBIX_UNIVERSAL_FORMATS "")

if (BUILD_VST3)
    message ("-- Building VST3 versions (universal order up to ${MAX_AMBI_ORDER})")
    list (APPEND AMBIX_UNIVERSAL_FORMATS VST3)
endif(BUILD_VST3)

if (BUILD_STANDALONE)
    message ("-- Building Standalone versions (universal order up to ${MAX_AMBI_ORDER})")
    list (APPEND AMBIX_UNIVERSAL_FORMATS Standalone)
endif(BUILD_STANDALONE)

if (BUILD_AU AND APPLE)
    message ("-- Building AudioUnit versions (universal order up to ${MAX_AMBI_ORDER})")
    list (APPEND AMBIX_UNIVERSAL_FORMATS AU)
endif()


#save path for later
SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET (BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})


# set output path
set_directory_properties (PROPERTIES JUCE_COPY_PLUGIN_AFTER_BUILD ON)
set_directory_properties (PROPERTIES JUCE_VST_COPY_DIR "${BIN_DIR}/vst_o${AMBI_ORDER}")
set_directory_properties (PROPERTIES JUCE_LV2_COPY_DIR "${BIN_DIR}/lv2_o${AMBI_ORDER}")
set_directory_properties (PROPERTIES JUCE_VST3_COPY_DIR "${BIN_DIR}/vst3")
IF (APPLE)
    set_directory_properties (PROPERTIES JUCE_AU_COPY_DIR "${BIN_DIR}/au")
ENDIF(APPLE)

IF (APPLE)
    SET (CMAKE_BUILD_TYPE MinSizeRel)
    SET (CMAKE_OSX_ARCHITECTURES arm64;x86_64)
    SET (CMAKE_OSX_DEPLOYMENT_TARGET 10.11)

    SET (LINK_FLAGS MINSIZEREL "-dead_strip")
ELSE (APPLE)
    SET (CMAKE_BUILD_TYPE Release)
ENDIF (APPLE)


IF (UNIX)
    # this is to make the convolver build under linux happy
    IF (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|i386|amd64)")
        SET (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -msse3")
        SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse3")
    ENDIF()
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
# (CMAKE_MODULE_PATH was set earlier so find_package(Jack) could see our
# cmake/FindJack.cmake during the patch-apply block.)

# Eigen3 ships as a git submodule under eigen/ pinned to 3.4.0. Forcing
# EIGEN3_INCLUDE_DIR up front bypasses any ambient/system installs (including
# the macos-14 brew bottle whose Macros.h tripped FindEigen3's version probe).
set(EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/eigen" CACHE PATH "Eigen3 include directory" FORCE)
find_package(Eigen3 REQUIRED)

# Force MODULE mode so our cmake/FindFFTW3.cmake is used (sets FFTW3F_LIBRARY
# and FFTW3F_THREADS_LIBRARY). vcpkg installs FFTW3Config.cmake files which
# would otherwise win CONFIG-mode resolution and our targets would see empty
# library variables.
IF (WITH_ZITA_CONVOLVER OR WIN32)
	find_package(FFTW3 MODULE COMPONENTS single)
ENDIF (WITH_ZITA_CONVOLVER OR WIN32)

IF(UNIX AND NOT APPLE AND NOT ANDROID)
	find_package(FFTW3 MODULE COMPONENTS single threads)
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

include_directories(${EIGEN3_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR} ${JACK_INCLUDE_DIRS})


################################################
# add all subprojects (plugins)

ADD_SUBDIRECTORY ( ambix_binaural )
ADD_SUBDIRECTORY ( ambix_decoder )
ADD_SUBDIRECTORY ( ambix_converter )
ADD_SUBDIRECTORY ( ambix_directional_loudness )
ADD_SUBDIRECTORY ( ambix_encoder )
ADD_SUBDIRECTORY ( ambix_maxre )
ADD_SUBDIRECTORY ( ambix_mirror )
ADD_SUBDIRECTORY ( ambix_move )
ADD_SUBDIRECTORY ( ambix_rotator )
ADD_SUBDIRECTORY ( ambix_rotator_z )
ADD_SUBDIRECTORY ( ambix_vmic )
ADD_SUBDIRECTORY ( ambix_warp )
ADD_SUBDIRECTORY ( ambix_widening )

IF (BUILD_VISUALIZER)
	message ("-- Building ambix_visualizer (standalone GUI app)")
	ADD_SUBDIRECTORY ( ambix_visualizer )
ENDIF()

IF (BUILD_TESTHOST)
	message ("-- Building ambix_testhost (CLI test harness)")
	ADD_SUBDIRECTORY ( ambix_testhost )
ENDIF()

