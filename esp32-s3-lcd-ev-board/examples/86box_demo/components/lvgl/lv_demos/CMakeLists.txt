cmake_minimum_required(VERSION 3.12.4)

project(lv_demos HOMEPAGE_URL https://github.com/lvgl/lv_demos)

# Option to define LV_DEMO_CONF_INCLUDE_SIMPLE, default: ON
option(LV_DEMO_CONF_INCLUDE_SIMPLE "Simple include of \"lv_demo_conf.h\"" ON)

# Option to set LV_DEMO_CONF_PATH, if set parent path LV_DEMO_CONF_DIR is added
# to includes
option(LV_DEMO_CONF_PATH "Path defined for lv_demo_conf.h")
get_filename_component(LV_DEMO_CONF_DIR ${LV_DEMO_CONF_PATH} DIRECTORY)

file(GLOB_RECURSE SOURCES src/*.c)
add_library(lv_demos STATIC ${SOURCES})
add_library(lvgl_demos ALIAS lv_demos)
add_library(lvgl::demos ALIAS lv_demos)

target_include_directories(lv_demos SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                  ${LV_DEMO_CONF_DIR})

target_link_libraries(lv_demos PUBLIC lvgl)
