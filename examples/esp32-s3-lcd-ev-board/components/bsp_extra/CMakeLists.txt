file(GLOB_RECURSE SRCS src/*.c)

idf_build_get_property(bsp_extra_board_component BSP_EXTRA_BOARD_COMPONENT)
if(NOT bsp_extra_board_component)
    set(bsp_extra_board_component "esp32_s3_lcd_ev_board")
endif()

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include"
    PRIV_REQUIRES ${bsp_extra_board_component})
