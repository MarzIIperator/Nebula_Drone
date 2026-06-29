cmake_minimum_required(VERSION 3.20)
project(Nebula VERSION 2.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Rack-SDK-Pfad — beim Aufruf setzen: cmake .. -DRACK_SDK_DIR=/pfad/zum/Rack-SDK
if(NOT DEFINED RACK_SDK_DIR)
    message(FATAL_ERROR "RACK_SDK_DIR nicht gesetzt. Beispiel: cmake .. -DRACK_SDK_DIR=/pfad/zum/Rack-SDK")
endif()

add_library(Nebula SHARED
        src/Nebula.cpp
        src/NebulaWidget.cpp
        src/plugin.cpp
        src/dsp/wavetable.cpp
)
target_include_directories(Nebula PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
        ${CMAKE_CURRENT_SOURCE_DIR}/dep
        ${RACK_SDK_DIR}/include
        ${RACK_SDK_DIR}/dep/include
)
target_link_libraries(Nebula PRIVATE
        ${RACK_SDK_DIR}/libRack.dylib
)
