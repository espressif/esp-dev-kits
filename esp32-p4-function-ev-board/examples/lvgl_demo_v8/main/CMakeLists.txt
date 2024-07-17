idf_component_register(
    SRCS main.c
    INCLUDE_DIRS .)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
        -Wno-format
        -Wno-incompatible-pointer-types
        -DLV_LVGL_H_INCLUDE_SIMPLE)
