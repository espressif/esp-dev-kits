# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(COMPONENTS main)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(factory_demo VERSION 1.0.0)

# Suppress warnings only for esp-dl component
idf_component_get_property(esp_dl_lib espressif__esp-dl COMPONENT_LIB)
if(esp_dl_lib)
    target_compile_options(${esp_dl_lib} PRIVATE 
        -Wno-unused-variable 
        -Wno-unused-but-set-variable
    )
    message(STATUS "Applied warning suppression to esp-dl component")
endif()
