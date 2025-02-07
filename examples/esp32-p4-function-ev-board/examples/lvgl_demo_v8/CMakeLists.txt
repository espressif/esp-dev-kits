# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ../common_components
    )

add_compile_options(-Wno-format)
add_compile_options(-Wno-ignored-qualifiers)
project(lvgl_demo_v8)
