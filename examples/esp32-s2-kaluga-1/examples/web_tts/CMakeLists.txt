# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "../../components/board"
                         "../../components/es8311"
                         "../../components/i2c_bus"
                         "../../components/helix"
                         "../../components/esp_tts"
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options(-fdiagnostics-color=always
                    -Wno-deprecated-declarations)
project(web_tts)