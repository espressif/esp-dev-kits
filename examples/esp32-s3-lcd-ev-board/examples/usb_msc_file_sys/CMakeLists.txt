cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ../../components
    )

# The following line can be removed after LVGL update to v8.3.5.
add_compile_options(-Wno-format
                    -fdiagnostics-color=always
                    -Wno-incompatible-pointer-types
                    -Wno-unused-variable
                    -Wno-ignored-qualifiers)

project(usb_msc_file_sys)