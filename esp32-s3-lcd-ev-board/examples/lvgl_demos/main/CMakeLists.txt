set(LV_DEMO_DIR "../../../components/lvgl/demos")
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

idf_component_register(
    SRCS "main.c" "lv_port.c" ${LV_DEMOS_SOURCES}
    INCLUDE_DIRS "." ${LV_DEMO_DIR})
