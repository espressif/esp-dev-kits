# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options(-fdiagnostics-color=always
                    -DLV_LVGL_H_INCLUDE_SIMPLE
                    -Wno-format
                    -Wno-implicit-fallthrough
                    -Wno-unused-local-typedefs
                    -Wno-ignored-qualifiers)
project(c3_knob_panel)
