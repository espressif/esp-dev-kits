set(MAIN_DIR .)
# set(LVGL_DEMOS_DIR ../components/lvgl/demos)

file(GLOB_RECURSE MAIN_SOURCES ${MAIN_DIR}/*.c)
# file(GLOB_RECURSE LV_DEMO_SOURCES ${LVGL_DEMOS_DIR}/*.c)

idf_component_register(
    SRCS
        ${MAIN_SOURCES}
        # ${LV_DEMO_SOURCES}
    INCLUDE_DIRS
        ${MAIN_DIR}
        # ${LVGL_DEMOS_DIR}
)

idf_build_set_property(COMPILE_OPTIONS -Wno-address-of-packed-member APPEND)
