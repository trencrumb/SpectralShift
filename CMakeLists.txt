cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(SpectralShift VERSION 0.1.1)
set(COMPANY_NAME "trencrumb")
set(PRODUCT_NAME "Spectral Shift")

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

# Performance optimization options
option(USE_IPP "Use Intel Performance Primitives for optimized DSP" OFF)
option(USE_PFFFT "Use PFFFT (Pretty Fast FFT) - lightweight alternative to IPP" OFF)

# IPP support (Windows/Linux)
if(USE_IPP)
    if(WIN32)
        message(STATUS "IPP support enabled for Windows")
        # Try to find IPP from nuget package location
        if(NOT DEFINED IPP_ROOT)
            file(GLOB IPP_NUGET_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/intelipp.static.win-x64.*")
            if(IPP_NUGET_DIRS)
                list(GET IPP_NUGET_DIRS 0 IPP_ROOT)
                message(STATUS "Found IPP nuget package at: ${IPP_ROOT}")
            endif()
        endif()

        # Set IPP paths for find_package
        if(DEFINED IPP_ROOT)
            set(CMAKE_PREFIX_PATH "${IPP_ROOT}/lib/native/win-x64;${CMAKE_PREFIX_PATH}")
            set(IPP_INCLUDE_DIR "${IPP_ROOT}/lib/native/include")
        endif()
    elseif(UNIX AND NOT APPLE)
        message(STATUS "IPP support enabled for Linux")
        # On Linux, IPP is typically installed system-wide via oneAPI
        # e.g., /opt/intel/oneapi/ipp/latest
        if(NOT DEFINED IPP_ROOT AND DEFINED ENV{IPPROOT})
            set(IPP_ROOT $ENV{IPPROOT})
            message(STATUS "Using IPP from IPPROOT: ${IPP_ROOT}")
        endif()
    endif()
    set(SIGNALSMITH_USE_IPP ON CACHE BOOL "Use IPP in signalsmith-linear" FORCE)
else()
    set(SIGNALSMITH_USE_IPP OFF CACHE BOOL "Use IPP in signalsmith-linear" FORCE)
endif()

# PFFFT support (cross-platform, lightweight alternative)
if(USE_PFFFT)
    message(STATUS "PFFFT support enabled - fetching library")

    # Fetch PFFFT
    CPMAddPackage(
        NAME pffft
        GIT_REPOSITORY https://bitbucket.org/jpommier/pffft.git
        GIT_TAG master
        DOWNLOAD_ONLY YES
    )

    # Create PFFFT library target
    if(pffft_ADDED)
        add_library(pffft STATIC ${pffft_SOURCE_DIR}/pffft.c)
        target_include_directories(pffft PUBLIC ${pffft_SOURCE_DIR})
        # PFFFT works best with these flags
        if(NOT MSVC)
            target_compile_options(pffft PRIVATE -O3 -ffast-math)
        endif()
        message(STATUS "PFFFT library created from ${pffft_SOURCE_DIR}")
    endif()

    set(SIGNALSMITH_USE_PFFFT ON CACHE BOOL "Use PFFFT in signalsmith-linear" FORCE)
else()
    set(SIGNALSMITH_USE_PFFFT OFF CACHE BOOL "Use PFFFT in signalsmith-linear" FORCE)
endif()

# Enable fast math for performance (except Apple Clang 16.0.x which has broken SIMD)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(APPLE AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "16.0.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.1.0")
        message(WARNING "Apple Clang 16.0.x detected (${CMAKE_CXX_COMPILER_VERSION}) - NOT enabling -ffast-math due to broken SIMD code generation")
    else()
        add_compile_options(-ffast-math)
        message(STATUS "Enabling -ffast-math for performance")
    endif()
elseif(MSVC)
    add_compile_options(/fp:fast)
    message(STATUS "Enabling /fp:fast for performance")
endif()

if(UNIX AND NOT APPLE AND NOT WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-march=x86-64-v2)
        message(STATUS "Linux portable CPU target: x86-64-v2")
    endif()
endif()

CPMAddPackage(
        NAME stretch
        GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
        GIT_TAG origin/main
        OPTIONS
            "SIGNALSMITH_USE_IPP ${SIGNALSMITH_USE_IPP}"
            "SIGNALSMITH_USE_PFFFT ${SIGNALSMITH_USE_PFFFT}"
)

# Optimize signalsmith-stretch even in Debug builds (it's very slow otherwise)
if(TARGET signalsmith-stretch)
    if(MSVC)
        target_compile_options(signalsmith-stretch INTERFACE /O2)
        message(STATUS "Enabling optimizations for signalsmith-stretch (MSVC)")
    else()
        target_compile_options(signalsmith-stretch INTERFACE -O2)
        message(STATUS "Enabling optimizations for signalsmith-stretch")
    endif()
endif()

# Suppress float-conversion warnings in signalsmith libraries (they're harmless)
if(TARGET signalsmith-linear)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(signalsmith-linear INTERFACE -Wno-implicit-float-conversion -Wno-float-conversion)
    endif()
endif()

if(TARGET signalsmith-stretch)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(signalsmith-stretch INTERFACE -Wno-implicit-float-conversion -Wno-float-conversion)
    endif()
endif()

# Link PFFFT to signalsmith-linear if enabled
if(USE_PFFFT AND TARGET pffft AND TARGET signalsmith-linear)
    target_link_libraries(signalsmith-linear INTERFACE pffft)
    message(STATUS "Linked PFFFT to signalsmith-linear")
endif()

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
    set(PLUGIN_FORMATS VST3 AU AUv3 LV2 AAX Standalone)
    message(STATUS "AAX SDK path provided: ${JUCE_AAX_SDK_PATH}.")
    message("Building ${PLUGIN_FORMATS}.")
else()
    set(PLUGIN_FORMATS VST3 AU AUv3 LV2 Standalone)
    message(STATUS "AAX SDK path not provided. To build AAX, pass -DJUCE_AAX_SDK_PATH=path/to/sdk")
    message("Building ${PLUGIN_FORMATS}.")
endif()

juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME ${COMPANY_NAME}
        VERSION ${PROJECT_VERSION}
        BUNDLE_VERSION ${PROJECT_VERSION}
        ICON_BIG "packaging/resources/icon.png"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        LV2URI "http://github.com/trencrumb/SpectralShift"
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE TreC
        PLUGIN_CODE Sps0
        FORMATS ${PLUGIN_FORMATS}
        PRODUCT_NAME ${PRODUCT_NAME}
)

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
        $<$<BOOL:${USE_IPP}>:JUCE_USE_IPP=1>
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

# Link IPP libraries if enabled
if(WIN32 AND USE_IPP AND DEFINED IPP_ROOT)
    target_include_directories(${PROJECT_NAME} PRIVATE "${IPP_INCLUDE_DIR}")
    target_link_directories(${PROJECT_NAME} PRIVATE "${IPP_ROOT}/lib/native/win-x64")
    target_link_libraries(${PROJECT_NAME} PRIVATE ippcore.lib ipps.lib ippi.lib ippvm.lib)
    message(STATUS "Linking IPP libraries for JUCE")
endif()

clap_juce_extensions_plugin(TARGET ${PROJECT_NAME}
        CLAP_ID "com.${COMPANY_NAME}.${PROJECT_NAME}"
        CLAP_FEATURES "audio-effect")
add_dependencies(${PROJECT_NAME}_All ${PROJECT_NAME}_CLAP)
# Link Perfetto only if enabled
if(PERFETTO)
    target_link_libraries(${PROJECT_NAME} PUBLIC Melatonin::Perfetto)
endif()

