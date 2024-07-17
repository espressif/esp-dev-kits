set(LV_DEMO_DIR ../managed_components/lvgl__lvgl/demos)
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

idf_component_register(
    SRCS main.c ${LV_DEMOS_SOURCES}
    INCLUDE_DIRS . ${LV_DEMO_DIR})

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
        -Wno-format
        -Wno-incompatible-pointer-types
        -DLV_LVGL_H_INCLUDE_SIMPLE)
