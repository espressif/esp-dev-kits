set(APPS_DIR ./)
file(GLOB_RECURSE APPS_C_SRCS ${APPS_DIR}/*.c)
file(GLOB_RECURSE APPS_CPP_SRCS ${APPS_DIR}/*.cpp)

idf_component_register(
    SRCS ${APPS_C_SRCS} ${APPS_CPP_SRCS}
    INCLUDE_DIRS ${APPS_DIR}
    REQUIRES lvgl__lvgl esp_event esp_wifi nvs_flash esp_driver_jpeg esp_mm esp-brookesia bsp_extra esp32_p4_function_ev_board esp_video pedestrian_detect)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
        -Wno-format
        -Wno-incompatible-pointer-types
        -Wno-int-conversion
        -DLV_LVGL_H_INCLUDE_SIMPLE)
