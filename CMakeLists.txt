##  Copyright (C) 2022 Arthur Benilov <arthur.benilov@gmail.com>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)

project(Singing_Trombone)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "Runtime Library")
endif()

option(WITH_ASIO "Enable ASIO audio interface" ON)

add_subdirectory(JUCE)

set(plugin_formats
    VST3
    Standalone
)

if(APPLE)
    list(APPEND plugin_formats AU)
endif()

juce_add_plugin(${PROJECT_NAME}
    VERSION "0.0.1"
    PLUGIN_MANUFACTURER_CODE "Arbe"
    PLUGIN_CODE "S8gt"
    PLUGIN_DESCRIPTION "Singing Trombone"
    PRODUCT_NAME "Singing Trombone"
    COMPANY_NAME "Arthur Benilov"
    IS_SYNTH TRUE
    IS_MIDI_EFFECT FALSE
    NEEDS_MIDI_INPUT TRUE
    FORMATS ${plugin_formats}
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icons/icon256.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icons/icon64.png"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_audio_basics
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
)

juce_generate_juce_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

set(src
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/core/List.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/core/Queue.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/Noise.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/Noise.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/Glottis.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/Glottis.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/Tract.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/Tract.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/VoiceProcessor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/model/VoiceProcessor.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Envelope.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Envelope.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Parameter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Parameter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Interpolator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Interpolator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Voice.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Voice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Lyrics.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Lyrics.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Engine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/engine/Engine.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginParameters.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginParameters.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/LookAndFeel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/LookAndFeel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.cpp"
)

file(GLOB_RECURSE res "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.*")

juce_add_binary_data(${PROJECT_NAME}_res SOURCES ${res})
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_res)

target_sources(${PROJECT_NAME} PRIVATE ${src})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

if(WITH_ASIO)
    target_include_directories(${PROJECT_NAME}
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/asiosdk_2.3.3_2019-06-14/common"
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_ASIO=1)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_ASIO=0)
endif()

if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_AU=1)
endif()
