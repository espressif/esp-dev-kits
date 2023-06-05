set(SUB_BOARD_SRC "")

if (CONFIG_BSP_LCD_SUB_BOARD_480_480)
    set(SUB_BOARD_SRC "sub_board/sub_board_480x480.c")
elseif (CONFIG_BSP_LCD_SUB_BOARD_800_480)
    set(SUB_BOARD_SRC "sub_board/sub_board_800x480.c")
endif()

idf_component_register(
    SRCS "esp32_s3_lcd_ev_board.c" "lv_port_fs.c" ${SUB_BOARD_SRC}
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver
    PRIV_REQUIRES esp_timer esp_lcd
)
