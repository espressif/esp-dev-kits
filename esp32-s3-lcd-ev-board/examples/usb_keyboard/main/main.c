/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "bsp/esp-bsp.h"

extern void usb_keyboard_init(void);

void app_main(void)
{
#if CONFIG_BSP_LCD_SUB_BOARD_480_480
    // For the newest version sub board, we need to set `BSP_LCD_VSYNC` to high before initialize LCD
    // It's a requirement of the LCD module and will be added into BSP in the future
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = BIT64(BSP_LCD_VSYNC);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = true;
    gpio_config(&io_conf);
    gpio_set_level(BSP_LCD_VSYNC, 1);
#endif

    bsp_i2c_init();
    bsp_display_start();

    usb_keyboard_init();
}
