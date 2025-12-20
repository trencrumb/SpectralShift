cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(SpectralShift VERSION 1.0.0)
set(COMPANY_NAME "trencrumb")

# Universal binary support for macOS (Intel + Apple Silicon)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We're going to use CPM as our package manager to bring in JUCE
# Check to see if we have CPM installed already.  Bring it in if we don't.
set(CPM_DOWNLOAD_VERSION 0.42.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if (NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

# Bring in JUCE locally
CPMAddPackage(
    NAME juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG origin/master
)

CPMAddPackage(
        NAME stretch
        GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
        GIT_TAG origin/main
)

CPMAddPackage(
        NAME clap_juce_extensions
        GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
        GIT_TAG origin/main
)

# Perfetto profiling (optional, enable with -DPERFETTO=ON)
option(PERFETTO "Enable Perfetto profiling" OFF)

if(PERFETTO)
    message(STATUS "Perfetto profiling enabled")
    CPMAddPackage(
            NAME melatonin_perfetto
            GIT_REPOSITORY https://github.com/sudara/melatonin_perfetto.git
            GIT_TAG origin/main
    )
else()
    message(STATUS "Perfetto profiling disabled")
endif()

# Make sure you include any new source files here
set(SourceFiles
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PresetManager.h
        Source/PresetManager.cpp
        Source/DSP/TiltEQ.h
        Source/DSP/SpectralCentroid.h
        Source/Component/XYPad.h
        Source/Component/XYPad.cpp
        Source/Component/CustomLookAndFeel.h
)


if(DEFINED JUCE_AAX_SDK_PATH)
    juce_set_aax_sdk_path("${JUCE_AAX_SDK_PATH}")
    set(PLUGIN_FORMATS VST3 AU AUv3 AAX Standalone)
    message(STATUS "AAX SDK path provided: ${JUCE_AAX_SDK_PATH}.")
    message("Building ${PLUGIN_FORMATS}.")
else()
    set(PLUGIN_FORMATS VST3 AU AUv3 Standalone)
    message(STATUS "AAX SDK path not provided. To build AAX, pass -DJUCE_AAX_SDK_PATH=path/to/sdk")
    message("Building ${PLUGIN_FORMATS}.")
endif()

# Change these to your own preferences
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME ${COMPANY_NAME}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE TreC
        PLUGIN_CODE Sps0
        FORMATS ${PLUGIN_FORMATS}
        PRODUCT_NAME ${PROJECT_NAME}
)

# Create .env file for GitHub Actions (if running in CI)
if(DEFINED ENV{GITHUB_ACTIONS})
    include(cmake/GitHubENV.cmake)
endif()

# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})


# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        signalsmith-stretch
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

clap_juce_extensions_plugin(TARGET ${PROJECT_NAME}
        CLAP_ID "com.${COMPANY_NAME}.${PROJECT_NAME}"
        CLAP_FEATURES "audio-effect")
add_dependencies(${PROJECT_NAME}_All ${PROJECT_NAME}_CLAP)
# Link Perfetto only if enabled
if(PERFETTO)
    target_link_libraries(${PROJECT_NAME} PUBLIC Melatonin::Perfetto)
endif()

