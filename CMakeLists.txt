cmake_minimum_required(VERSION 3.22)

project(RackStrip500 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Locate or Fetch JUCE Framework
if(NOT TARGET juce::juce_audio_processors)
    include(FetchContent)
    message(STATUS "Locating JUCE framework...")
    
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
    )
    
    FetchContent_MakeAvailable(JUCE)
endif()

# JUCE Plugin Target Definition
juce_add_plugin(RackStrip500
    COMPANY_NAME "HopeStudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE "Hope"
    PLUGIN_CODE "Rk50"
    FORMATS VST3 Standalone
    PRODUCT_NAME "RackStrip500"
)

target_sources(RackStrip500
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/GUI/FaceplateModuleComponent.cpp
        Source/GUI/VintageRackLookAndFeel.cpp
        Source/GUI/VUMeterComponent.cpp
)

target_include_directories(RackStrip500
    PRIVATE
        Source
        Source/GUI
)

target_compile_definitions(RackStrip500
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(RackStrip500
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
