/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "driver/i2c.h"
#include "esp_check.h"
#include "esp_psram.h"

#include "esp_lcd_touch_ft5x06.h"
#include "esp_lcd_touch_gt1151.h"
#include "bsp_probe.h"
#include "bsp_err_check.h"
#include "bsp/esp32_s3_lcd_ev_board.h"

#define MODULE_PSRAM_SIZE_R8    (8 * 1024 * 1024)

static const char *TAG = "bsp_probe";
static bsp_module_type_t module_type = MODULE_TYPE_UNKNOW;
static bsp_sub_board_type_t sub_board_type = SUB_BOARD_TYPE_UNKNOW;

bsp_module_type_t bsp_probe_module_type(void)
{
    if (module_type != MODULE_TYPE_UNKNOW) {
        return module_type;
    }

    int psram_size = esp_psram_get_size();
    if (psram_size > MODULE_PSRAM_SIZE_R8) {
        ESP_LOGI(TAG, "Detect module with 16MB PSRAM");
        module_type = MODULE_TYPE_R16;
    } else {
        ESP_LOGI(TAG, "Detect module with 8MB PSRAM");
        module_type = MODULE_TYPE_R8;
    }

    return module_type;
}

bsp_sub_board_type_t bsp_probe_sub_board_type(void)
{
    if (sub_board_type != SUB_BOARD_TYPE_UNKNOW) {
        return sub_board_type;
    }

    BSP_ERROR_CHECK_RETURN_ERR(bsp_i2c_init());

    uint8_t tp_address[] = {
        ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS,
        ESP_LCD_TOUCH_IO_I2C_GT1151_ADDRESS,
    };
    uint8_t i = 0;
    i2c_cmd_handle_t cmd;
    bsp_sub_board_type_t detect_type = SUB_BOARD_TYPE_UNKNOW;
    while (i < sizeof(tp_address)) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (tp_address[i] << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        if (i2c_master_cmd_begin(BSP_I2C_NUM, cmd, pdMS_TO_TICKS(20)) == ESP_OK) {
            if (tp_address[i] == ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS) {
                ESP_LOGI(TAG, "Detect sub_board2 with 480x480 LCD (GC9503), Touch (FT5x06)");
                detect_type = SUB_BOARD_TYPE_2_480_480;
            } else if (tp_address[i] == ESP_LCD_TOUCH_IO_I2C_GT1151_ADDRESS) {
                ESP_LOGI(TAG, "Detect sub_board3 with 800x480 LCD (ST7262), Touch (GT1151)");
                detect_type = SUB_BOARD_TYPE_3_800_480;
            }
        }
        i2c_cmd_link_delete(cmd);
        if (detect_type != SUB_BOARD_TYPE_UNKNOW) {
            break;
        }
        i++;
    }

    ESP_RETURN_ON_FALSE(detect_type != SUB_BOARD_TYPE_UNKNOW, ESP_ERR_INVALID_STATE, TAG,
                        "Failed to detect sub_board type, please check the hardware connection");
    if (CONFIG_BSP_LCD_SUB_BOARD_TYPE) {
        ESP_RETURN_ON_FALSE(detect_type == CONFIG_BSP_LCD_SUB_BOARD_TYPE, ESP_ERR_INVALID_STATE, TAG,
                            "Sub_board type mismatch, please check the software configuration and hardware connection");
    }
    sub_board_type = detect_type;

    return sub_board_type;
}
