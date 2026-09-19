cmake_minimum_required(VERSION 3.13)

# ambix_bustest — raw VST3 bus-layout probe used by tests/test_bus_layouts.py.
#
# Deliberately not a JUCE host: it links the VST3 interfaces JUCE already
# vendors and calls IComponent / IAudioProcessor itself. JUCE's own host
# substitutes discreteChannels(count) whenever a plugin cannot answer
# getBusArrangement(), which is precisely the failure these tests look for —
# hosting through JUCE would hide it (as pluginval does).
#
# Usage (from repo root):
#   cmake -B _build -DBUILD_VST3=ON -DBUILD_TESTHOST=ON
#   cmake --build _build --target ambix_bustest

set (VST3_SDK_DIR
     "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK")

if (NOT EXISTS "${VST3_SDK_DIR}/public.sdk/source/vst/vstinitiids.cpp")
    # Older JUCE kept the SDK in juce_audio_processors; newer may move again.
    set (VST3_SDK_DIR
         "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE/modules/juce_audio_processors/format_types/VST3_SDK")
endif()

if (NOT EXISTS "${VST3_SDK_DIR}/public.sdk/source/vst/vstinitiids.cpp")
    message (WARNING "-- VST3 SDK not found in JUCE; skipping ambix_bustest")
    return()
endif()

add_executable (ambix_bustest
    Source/Main.cpp
    "${VST3_SDK_DIR}/pluginterfaces/base/funknown.cpp"
    "${VST3_SDK_DIR}/pluginterfaces/base/coreiids.cpp"
    "${VST3_SDK_DIR}/pluginterfaces/base/conststringtable.cpp"
    "${VST3_SDK_DIR}/pluginterfaces/base/ustring.cpp"
    "${VST3_SDK_DIR}/public.sdk/source/vst/vstinitiids.cpp"
)

target_include_directories (ambix_bustest PRIVATE "${VST3_SDK_DIR}")

target_compile_features (ambix_bustest PRIVATE cxx_std_17)

if (APPLE)
    target_link_libraries (ambix_bustest PRIVATE "-framework CoreFoundation")
elseif (UNIX)
    target_link_libraries (ambix_bustest PRIVATE ${CMAKE_DL_LIBS})
endif()

# Alongside ambix_testhost, so tests find both in the same place.
set_target_properties (ambix_bustest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY            "${CMAKE_BINARY_DIR}/testhost"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_BINARY_DIR}/testhost"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_BINARY_DIR}/testhost"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/testhost"
)
