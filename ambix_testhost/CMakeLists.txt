cmake_minimum_required(VERSION 3.13)

# ambix_testhost — offline CLI test harness for ambix VST3 plugins.
#
# Usage (from repo root):
#   cmake -B _build -DBUILD_VST3=ON -DBUILD_TESTHOST=ON
#   cmake --build _build --target ambix_testhost
#
# Python:
#   subprocess.run(["_build/testhost/ambix_testhost",
#                   "--plugin", "_build/vst3/ambix_encoder.vst3",
#                   "--in-channels",  "1",
#                   "--out-channels", "4",
#                   "--input",   "mono.wav",
#                   "--output",  "ambi.wav",
#                   "--params",  "params.json"], check=True)

juce_add_console_app(ambix_testhost
    PRODUCT_NAME "ambix_testhost"
    VERSION      "1.0.0"
)

juce_generate_juce_header(ambix_testhost)

target_sources(ambix_testhost PRIVATE
    Source/Main.cpp
)

target_compile_definitions(ambix_testhost PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=0
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_LADSPA=0
)

target_link_libraries(ambix_testhost PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_basics
    juce::juce_core
    juce::juce_data_structures
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Output into _build/testhost/ for easy subprocess discovery from tests.
set_target_properties(ambix_testhost PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY            "${CMAKE_BINARY_DIR}/testhost"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_BINARY_DIR}/testhost"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_BINARY_DIR}/testhost"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/testhost"
)
