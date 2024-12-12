# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# The configuration is to prevent compilation errors on the Windows system.
set(CMAKE_OBJECT_PATH_MAX 512)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    ../common_components
    )

add_compile_options(-Wno-format
                    -Wno-unused-variable
                    -Wno-missing-field-initializers)
project(esp_brookesia_demo VERSION 1.0.0)
