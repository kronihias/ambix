cmake_minimum_required (VERSION 3.22)

# Can be built either as a subdirectory of the ambix suite (JUCE already added
# by the parent), or standalone (we add JUCE ourselves from ../JUCE).
if (NOT TARGET juce::juce_core)
    project (ambix_visualizer VERSION 0.1.0)
    add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../JUCE JUCE)
    set (AMBIX_VISUALIZER_STANDALONE_BUILD TRUE)
endif()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE AND NOT IOS)
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    endif()
    if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set (CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "" FORCE)
    endif()
endif()

juce_add_gui_app (ambix_visualizer
    PRODUCT_NAME                      "ambix Visualizer"
    COMPANY_NAME                      "kronlachner"
    BUNDLE_ID                         "com.kronlachner.ambix.visualizer"
    VERSION                           "0.1.0"
    MICROPHONE_PERMISSION_ENABLED     FALSE
    LOCAL_NETWORK_PERMISSION_ENABLED  TRUE
    LOCAL_NETWORK_PERMISSION_TEXT     "ambix Visualizer uses the local network to receive OSC messages from ambix_encoder plugins running in your DAW."
    NEEDS_CURL                        FALSE
    NEEDS_WEB_BROWSER                 FALSE
    IPHONE_SCREEN_ORIENTATIONS        UIInterfaceOrientationPortrait
                                      UIInterfaceOrientationLandscapeLeft
                                      UIInterfaceOrientationLandscapeRight
    IPAD_SCREEN_ORIENTATIONS          UIInterfaceOrientationPortrait
                                      UIInterfaceOrientationLandscapeLeft
                                      UIInterfaceOrientationLandscapeRight
                                      UIInterfaceOrientationPortraitUpsideDown
    ICON_BIG                          "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png"
    ICON_SMALL                        "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png"
    PLIST_TO_MERGE                    "<key>NSBonjourServices</key><array><string>_ambix.encoder.v1._udp</string><string>_ambix.visualizer.v1._udp</string></array><key>ITSAppUsesNonExemptEncryption</key><false/>")

# --- Mobile build notes -------------------------------------------------------
#
# iOS:
#   cmake -S . -B build-ios -G Xcode \
#         -DCMAKE_SYSTEM_NAME=iOS \
#         -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
#         -DCMAKE_OSX_ARCHITECTURES=arm64
#   cmake --build build-ios --config Release
#   (NSLocalNetworkUsageDescription is already merged into Info.plist above.)
#
# Android:
#   JUCE's native Android support is via Projucer + Gradle. To ship on Android,
#   either (a) open this project in Projucer and export Android Studio project,
#   or (b) wrap this CMake in an Android Studio NDK project with Gradle and
#   declare INTERNET permission in AndroidManifest.xml.
# -----------------------------------------------------------------------------

set_target_properties (ambix_visualizer PROPERTIES
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf-with-dsym"
    XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] YES)

juce_generate_juce_header (ambix_visualizer)

file (GLOB_RECURSE VISUALIZER_SOURCES
    CONFIGURE_DEPENDS
    Source/*.cpp
    Source/*.h)

target_sources (ambix_visualizer PRIVATE ${VISUALIZER_SOURCES})

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX Source FILES ${VISUALIZER_SOURCES})

target_include_directories (ambix_visualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party)

target_compile_definitions (ambix_visualizer PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:ambix_visualizer,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:ambix_visualizer,JUCE_VERSION>")

target_link_libraries (ambix_visualizer PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_events
    juce::juce_osc
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)

# When built as part of the ambix plugin suite (repo root), drop the final app
# alongside the plugin-standalones in ${BIN_DIR}/standalone/ so the root build
# produces one tidy output tree. Skipped for the standalone-visualizer build
# (BIN_DIR undefined) and for iOS/Android toolchain builds.
if (DEFINED BIN_DIR AND NOT IOS AND NOT ANDROID AND NOT AMBIX_VISUALIZER_STANDALONE_BUILD)
    if (APPLE)
        add_custom_command (TARGET ambix_visualizer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_DIR}/standalone"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_BUNDLE_DIR:ambix_visualizer>"
                "${BIN_DIR}/standalone/$<TARGET_BUNDLE_DIR_NAME:ambix_visualizer>"
            COMMENT "Copying ambix Visualizer.app to ${BIN_DIR}/standalone/")
    else()
        add_custom_command (TARGET ambix_visualizer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_DIR}/standalone"
            COMMAND ${CMAKE_COMMAND} -E copy
                "$<TARGET_FILE:ambix_visualizer>"
                "${BIN_DIR}/standalone/$<TARGET_FILE_NAME:ambix_visualizer>"
            COMMENT "Copying ambix_visualizer to ${BIN_DIR}/standalone/")
    endif()
endif()
