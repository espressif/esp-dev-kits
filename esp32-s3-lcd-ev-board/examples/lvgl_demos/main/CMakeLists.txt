set(LV_DEMO_DIR ../../../components/lvgl/demos)
file(GLOB_RECURSE LV_DEMOS_SOURCES ${LV_DEMO_DIR}/*.c)

set(LV_DEMO1_DIR ./demo1)
file(GLOB_RECURSE LV_DEMO1_SOURCES ${LV_DEMO1_DIR}/*.c)

set(LV_DEMO2_DIR ./demo2)
file(GLOB_RECURSE LV_DEMO2_SOURCES ${LV_DEMO2_DIR}/*.c)

idf_component_register(
    SRCS "main.c" "lv_port.c" ${LV_DEMOS_SOURCES} ${LV_DEMO1_SOURCES} ${LV_DEMO2_SOURCES}
    INCLUDE_DIRS "." ${LV_DEMO_DIR} ${LV_DEMO1_DIR}/include ${LV_DEMO2_DIR}/include)
