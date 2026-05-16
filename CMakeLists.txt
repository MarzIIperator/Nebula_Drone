cmake_minimum_required(VERSION 3.20)
project(Nebula VERSION 2.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

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
        /Users/emielkastner/Desktop/Rack-SDK/include
        /Users/emielkastner/Desktop/Rack-SDK/dep/include
)
target_link_libraries(Nebula PRIVATE
        /Users/emielkastner/Desktop/Rack-SDK/libRack.dylib
)