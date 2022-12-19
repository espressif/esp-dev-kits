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
    bsp_i2c_init();
    lv_disp_t *disp = bsp_display_start();

    usb_keyboard_init();
}
