set(SOURCES "lvgl_demo_main.c")

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ""
                    REQUIRES board lcd led_strip lvgl_esp32_drivers lvgl_touch lvgl_tft lvgl lv_examples)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)