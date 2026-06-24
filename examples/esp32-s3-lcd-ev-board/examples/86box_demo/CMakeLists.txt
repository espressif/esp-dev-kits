# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(BSP_EXTRA_BOARD_COMPONENT "esp32_s3_lcd_ev_board_noglib")
set(EXTRA_COMPONENT_DIRS
    ../../components
    )

add_compile_options(-fdiagnostics-color=always
                    -Wno-attributes
                    -Wno-unused-variable
                    -Wno-unused-function
                    -Wno-ignored-qualifiers)
project(86box_demo)
