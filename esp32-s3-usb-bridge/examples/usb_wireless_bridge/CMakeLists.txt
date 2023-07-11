# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

if(${IDF_VERSION_MAJOR} LESS 5)
    message(FATAL_ERROR "ESP-IDF v${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR} is not supported! "
        "Please use v5.0 or newer.")
endif()

add_compile_options(-fdiagnostics-color=always 
                    -Wno-cpp)

project(bridge)
