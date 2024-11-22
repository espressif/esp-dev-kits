idf_component_register(SRCS "bsp_esp32_s3_usb_otg_ev.c" "board_common_wifi.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "esp_adc_cal" display_screen button nvs_flash sdmmc vfs fatfs)
